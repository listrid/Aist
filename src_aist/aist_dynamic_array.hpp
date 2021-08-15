/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once

#include "aist_mem.h"
#include <string.h>

class aist_dynamic_array_T
{
    static const size_t useMemSize    = 0x1000;   //использовать блоки по 4 килобайт
    static const size_t maxStringSize = 0x2000000; //32 мегабайт максимальный размер
    static const size_t m_sizeL1 = useMemSize;
    static const size_t m_sizeL2 = useMemSize/sizeof(char*);// x32 -> 1024  x64 -> 512
    static const size_t m_sizeL3 = (maxStringSize+(m_sizeL2*useMemSize-1))/(m_sizeL2*useMemSize);

    aist_mem_T* m_mem;
    bool   m_localMem;
    size_t m_useL1; // заполненность текущей строки
    size_t m_useL2; // заполненность текущей ветки
    size_t m_useL3; // заполненность корня
    char*  m_l1;    // текущая заполняемая строка
    char** m_l2;    // текущая ветка 
    char** m_l3[m_sizeL3]; //корень  (8192 блоков  x32 -> 8  x64 -> 16)

    size_t m_size;

    inline void new_line()
    {
        if(m_useL2 == m_sizeL2)//l2 полный
        {
            m_l3[m_useL3++] = m_l2 = (char**)m_mem->Alloc(useMemSize);
            m_useL2 = 0;
        }
        m_l2[m_useL2++] = m_l1 = (char*)m_mem->Alloc(useMemSize);
        if(m_useL2 != m_sizeL2)
            m_l2[m_useL2] = NULL;
        m_useL1 = 0;
    }
    inline void init(aist_mem_T* mem, bool isLocal)
    {
        m_mem      = mem;
        m_localMem = isLocal;
        m_useL3    = 0;
        clear();
    }
public:
    typedef bool( __stdcall * flush_Callback)(size_t offset, const void* data, size_t size, void* lParam);
    aist_dynamic_array_T()
    {
        aist_mem_T* mem = new aist_mem_T();
        mem->Init(0x4000);//16 кил
        init(mem, true);
    }
    aist_dynamic_array_T(aist_mem_T* mem)
    {
        init(mem, false);
    }
    ~aist_dynamic_array_T()
    {
        clear();
        if(m_localMem)
            delete m_mem;
    }

    inline void clear()
    {
        for(size_t i3 = 0; i3 < m_useL3; i3++)
        {
            char** l2 = m_l3[i3];
            for(size_t i2 = 0; i2 < m_sizeL2; i2++)
            {
                if(!l2[i2])
                    break;
                m_mem->Free(l2[i2], useMemSize);
            }
            m_mem->Free(l2, useMemSize);
        }
        memset(m_l3, 0, sizeof(char**)*m_sizeL3);
        m_useL1 = 0;
        m_useL2 = m_useL3 = 1;
        m_l3[0] = m_l2 = (char**)m_mem->Alloc(useMemSize);
        m_l2[0] = m_l1 = (char*) m_mem->Alloc(useMemSize);
        m_l2[1] = NULL;
        m_size = 0;
    }
    inline void write(const void* data, size_t len)
    {
        m_size += len;
        size_t freeL1 = m_sizeL1 - m_useL1;
        if(len < freeL1)
        {
            memcpy(m_l1 + m_useL1, data, len);
            m_useL1 += len;
            return;
        }
        {//добьем до целого
            memcpy(m_l1 + m_useL1, data, freeL1);
            len -= freeL1;
            data = ((char*)data) + freeL1;
            new_line();
        }
        while(len >= m_sizeL1)
        {//пишем блоками
            memcpy(m_l1, data, m_sizeL1);
            len -= m_sizeL1;
            data = ((char*)data) + m_sizeL1;
            new_line();
        }
        if(len)
        {//запишем остатки
            memcpy(m_l1 + m_useL1, data, len);
            m_useL1 += len;
            if(m_useL1 == m_sizeL1)
                new_line();
        }
    }
    inline void zWriteU64(unsigned long long value)
    {
        unsigned char data[16];
        size_t pos = 0;
        while(value > 0x7F)
        {
            data[pos++] = (value&0x7F)|0x80;
            value >>= 7;
        }
        data[pos++] = (unsigned char)value;
        write(data, pos);
    }
    inline void zWriteI64(long long value)
    {
        if(value < 0)
        {
            value = ~(value<<1);
        }else{
            value <<= 1;
        }
        zWriteU64((unsigned long long)value);
    }
    inline void zWriteU32(unsigned int value)
    {
        unsigned char data[8];
        size_t pos = 0;
        while(value > 0x7F)
        {
            data[pos++] = (value&0x7F)|0x80;
            value >>= 7;
        }
        data[pos++] = (unsigned char)value;
        write(data, pos);
    }
    inline void zWriteI32(int value)
    {
        if(value < 0)
        {
            value = ~(value<<1);
        }else{
            value <<= 1;
        }
        zWriteU32((unsigned int)value);
    }
    inline void flush(flush_Callback callBack, void* lParam)
    {
        size_t offset = 0;
        for(size_t i3 = 0; i3 < m_useL3; i3++)
        {
            char** l2 = m_l3[i3];
            for(size_t i2 = 0; i2 < m_sizeL2; i2++)
            {
                if(l2[i2] == m_l1)
                    break;
                if(!callBack(offset, l2[i2], m_sizeL1, lParam))
                    return;
                offset += m_sizeL1;
            }
        }
        if(m_useL1)
            callBack(offset, m_l1, m_useL1, lParam);
    }
    inline size_t size()
    {
        return m_size;//(m_useL3-1)*(m_sizeL2*m_sizeL1) + (m_useL2-1)*m_sizeL1 + m_useL1;
    }
    inline size_t Read(void* buf, size_t offset, size_t count)
    {
        size_t allSize = size();
        if(offset + count > allSize)
        {
            if(offset >= allSize)
                return 0;
            count = allSize - offset;
        }
        char* out_buf = (char*)buf;
        size_t out_count = count;
        size_t pos_l1 = offset % m_sizeL1;
        size_t pos_l2 = offset / m_sizeL1;
        size_t pos_l3 = pos_l2 / m_sizeL2;
        pos_l2 %= m_sizeL2;
        {//читаем до полного блока
            size_t cRead = m_sizeL1 - pos_l1;
            if(cRead > out_count)
                cRead = out_count;
            memcpy(out_buf, &m_l3[pos_l3][pos_l2][pos_l1], cRead);
            out_buf   += cRead;
            out_count -= cRead;
        }
        while(out_count >= m_sizeL1)
        {//читаем блоками
            pos_l2++;
            if(pos_l2 == m_sizeL2)
            {
                pos_l2 = 0;
                pos_l3++;
            }
            memcpy(out_buf, m_l3[pos_l3][pos_l2], m_sizeL1);
            out_buf   += m_sizeL1;
            out_count -= m_sizeL1;
        }
        if(out_count)//дочитываем кусок
            memcpy(out_buf, m_l3[pos_l3][pos_l2], out_count);
        return count;
    }
};

