/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>


inline size_t aist_os_strlen(const char* str){ return strlen(str); }
inline void aist_os_strcpy(char* dst, const char* str){ strcpy(dst, str); }
inline int  aist_os_strcmp(char const* str1, char const* str2){ return strcmp(str1, str2); }
inline void aist_os_memcpy(void* dst, void const* src, size_t size){ memcpy(dst, src, size); }
inline void aist_os_memset(void* dst, char val, size_t size){ memset(dst, val, size); }
inline void aist_os_free(void* mem){ free(mem); }
inline void* aist_os_malloc(size_t size){ return malloc(size); }


size_t aist_os_vsnprintf(char* dst, size_t dst_size, const char* format, va_list arg);
size_t aist_os_snprintf(char* dst, size_t dst_size, const char* format, ... );


// мем сторедж
class aist_mem_T
{
    static const size_t countBlokLite   = 128;
    static const size_t countBlokMedium = 192;
    static const size_t countBlokLarge  = 160;

    static const size_t maxSizeBlokLite   = countBlokLite * 8;                        //0 ... 1  kb
    static const size_t maxSizeBlokMedium = maxSizeBlokLite   + countBlokMedium * 16; //1 ... 4  kb
    static const size_t maxSizeBlokLarge  = maxSizeBlokMedium + countBlokLarge * 128; //4 ... 24 kb
public:
    ~aist_mem_T(){ FreeStorage(); };
    void Init(size_t block_size = 0); // настроить сторедж (указать какими блоками аллоцировать из системы)
    void FreeStorage();               // освободить всю память системе
    void Clear();                     // обнулить все выделения и вернуть себе память

    void* Alloc(size_t size);           // выделить память (размер не должен быть больше блока)
    void  Free(void* ptr, size_t size); // вернуть в пул

    size_t AllocSize();   //сколько памяти выделили
    size_t MaxAlloc()   { return m_block_size - 512; }; //максимальный выделяемый блок
    size_t StorageSize(){ return m_blockAlloc*m_block_size; }; //размер всего стореджа
    size_t PoolSize()   { return m_poolSize; };

    template <typename T>
    T* New()
    {
        void* buf = Alloc(sizeof(T));
        return new(buf)T();
    };
private:
    void  _Free(void* ptr, size_t size);
    struct MemStorageBlock
    {
        struct MemStorageBlock* next;
    };
    struct MemStorageBlockLBig
    {
        struct MemStorageBlockLBig* next;
        struct MemStorageBlockLBig* prew;
        size_t sizeBlock;
    };
    volatile long    m_Lock;   //блокировка по тредам
    MemStorageBlock* m_bottom; //корень блоков
    MemStorageBlock* m_top;    //текущий блок
    size_t m_block_size;    //размер блока
    size_t m_free_space;    //сколько места свободно в текущем блоке
    size_t m_blockAlloc;    //сколько блоков выделили
    size_t m_blockAllocUse; //сколько блоков использовали
    size_t m_poolSize;      //сколько памяти в пулах
    MemStorageBlock* m_poolLite[countBlokLite];    //0 ... 1  kb
    MemStorageBlock* m_poolMedium[countBlokMedium];//1 ... 4  kb
    MemStorageBlock* m_poolLarge[countBlokLarge];  //4 ... 24 kb
    MemStorageBlockLBig* m_poolVBig;               //24... ?? kb
#ifdef AIST_DEBUG_
    int m_debug_alloc_count;
    int m_debug_alloc_size;
#endif
};

