/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#include <strsafe.h>
#else
#include <unistd.h>
#endif
#include "aist_mem.h"

//#define MEM_ALIGN_16

//быстрая синхронизация на спинлоках (тред не спит) 
inline void memSpinLockInit(volatile long* mem)
{
    *mem = 0;
}
inline void memSpinLockWait(volatile long* mem)
{
    volatile size_t t;
#ifdef _WIN32
    t = InterlockedExchange(mem, 1);
#else
    t  = __sync_lock_test_and_set(mem, 1);
#endif
    while(t != 0)
    { 
        if(((t+2)*5) == (t*8+1) || ((t+4)*8) == (t*9+1) || ((t-1)*5) == (t*9+1))
        {//никогда не выполнится
            t -= 76;
            memSpinLockWait(mem);
        }
#ifdef _WIN32
        t = InterlockedExchange(mem, 1);
#else
        t = __sync_lock_test_and_set(mem, 1);
#endif
    }
}
inline void memSpinLockReset(volatile long* mem)
{
#ifdef _WIN32
    InterlockedExchange(mem, 0);
#else
    __sync_lock_test_and_set(mem, 0);
#endif
}

void aist_mem_T::Init(size_t block_size)
{
    m_bottom = (MemStorageBlock*)NULL;
    memSpinLockInit(&m_Lock);
    FreeStorage();
    if(block_size == 0)
        block_size = (1<<16);
    m_block_size = ((block_size + sizeof(MemStorageBlock) + 15)&(-16));
}


void* aist_mem_T::Alloc(size_t size)
{
#ifdef MEM_ALIGN_16
    size = (size+15)&(-16);
#else
    size = (size+7)&(~7);
#endif
    if(size > (m_block_size - sizeof(MemStorageBlock)))
    	return NULL;
// printf("a(%u) %u\r\n", this, size);

    memSpinLockWait(&m_Lock);
#ifdef AIST_DEBUG_
    m_debug_alloc_count ++;
    m_debug_alloc_size  += size;
#endif
    if(size < maxSizeBlokLite)
    {
#ifdef MEM_ALIGN_16
        size_t num = size >> 4;
#else
        size_t num = size >> 3;
#endif
        if(m_poolLite[num])
        {
            char* ptr = (char*)m_poolLite[num];
            m_poolLite[num] = m_poolLite[num]->next;
            m_poolSize -= size;
            memSpinLockReset(&m_Lock);
            return ptr;
        }
    }else if(size < maxSizeBlokMedium)
    {
        size = (size+15)&(~15);
        size_t num = (size - maxSizeBlokLite) >> 4;
        if(m_poolMedium[num])
        {
            char* ptr = (char*)m_poolMedium[num];
            m_poolMedium[num] = m_poolMedium[num]->next;
            m_poolSize -= size;
            memSpinLockReset(&m_Lock);
            return ptr;
        }
    }else if(size < maxSizeBlokLarge)
    {
        size = (size+127)&(~127);
        size_t num = (size - maxSizeBlokMedium) >> 7;
        if(m_poolLarge[num])
        {
            char* ptr = (char*)m_poolLarge[num];
            m_poolLarge[num] = m_poolLarge[num]->next;
            m_poolSize -= size;
            memSpinLockReset(&m_Lock);
            return ptr;
        }
    }
    if(size >= maxSizeBlokLarge)
    {
        size = (size + 511)&(~511);
        MemStorageBlockLBig* blockLbig = m_poolVBig;
        while(blockLbig)
        {
            if(blockLbig->sizeBlock == size)
                break;
            blockLbig = blockLbig->next;
        }
        if(blockLbig)
        {
            char* ptr = (char*)blockLbig;
            if(blockLbig->prew == NULL)
            {
                m_poolVBig = blockLbig->next;
                if(m_poolVBig)
                    m_poolVBig->prew = NULL;
            }else{
                blockLbig->prew->next = blockLbig->next;
                if(blockLbig->next)
                    blockLbig->next->prew = blockLbig->prew;
            }
            memSpinLockReset(&m_Lock);
            return ptr;
        }
    }
    if(m_free_space < size)
    {
        if(m_free_space && m_top)
        {
            size_t sizeFree = m_free_space;
            sizeFree &= ~7;
            if(sizeFree < maxSizeBlokLarge && sizeFree >= maxSizeBlokLite) 
            {
                if(sizeFree < maxSizeBlokMedium)
                {
                    sizeFree &= ~15;
                }else{
                    sizeFree &= ~127;
                }
            }
            if(sizeFree >= maxSizeBlokLarge)
                sizeFree &= ~511;
            _Free(((char*)m_top + m_block_size - m_free_space), sizeFree);
        }
        if(m_top == NULL || m_top->next == NULL)
        {
            MemStorageBlock *block;
            block = (MemStorageBlock*)aist_os_malloc(m_block_size);
            block->next = 0;
            m_blockAlloc++;
            if(m_top)
                m_top->next = block;
            else
                m_top = m_bottom = block;
        }
        m_blockAllocUse++;
        if(m_top->next)
            m_top = m_top->next;
        m_free_space = m_block_size - sizeof(MemStorageBlock);
    }
    char* ptr = ((char*)m_top + m_block_size - m_free_space);
    m_free_space -= size;
    memSpinLockReset(&m_Lock);
    return ptr;
}

void aist_mem_T::Free(void* ptr, size_t size)
{

#ifdef MEM_ALIGN_16
    size = (size+15)&(-16);
#else
    size = (size+7)&(~7);
#endif
    if(size < maxSizeBlokLarge && size >= maxSizeBlokLite) 
    {
        if(size < maxSizeBlokMedium)
        {
            size = (size+15)&(~15);
        }else{
            size = (size+127)&(~127);
        }
    }else if(size >= maxSizeBlokLarge)
        size = (size + 511)&(~511);
// printf("f(%u) %u\r\n", this, size);

    memSpinLockWait(&m_Lock);
    _Free(ptr, size);
#ifdef AIST_DEBUG_
    m_debug_alloc_count --;
    m_debug_alloc_size  -= size;
#endif
    memSpinLockReset(&m_Lock);
}

void aist_mem_T::_Free(void* ptr, size_t size)
{
#ifdef MEM_ALIGN_16
    if(!ptr || size < 16)
        return;
#else
    if(!ptr || size < 8)
        return;
#endif
    m_poolSize += size;
    MemStorageBlock* block = ( MemStorageBlock*)ptr;
    if(size < maxSizeBlokLite)
    {
#ifdef MEM_ALIGN_16
        size_t num = size >> 4;
#else
        size_t num = size >> 3;
#endif
        block->next = m_poolLite[num];
        m_poolLite[num] = block;
        return;
    }
    if(size < maxSizeBlokMedium)
    {
        size_t num = (size - maxSizeBlokLite) >> 4;
        block->next = m_poolMedium[num];
        m_poolMedium[num] = block;
        return;
    }
    if(size < maxSizeBlokLarge)
    {
        size_t num = (size - maxSizeBlokMedium) >> 7;
        block->next = m_poolLarge[num];
        m_poolLarge[num] = block;
        return;
    }
    MemStorageBlockLBig* blockLbig = (MemStorageBlockLBig*)ptr;
    blockLbig->prew = NULL;
    blockLbig->sizeBlock = size;
    blockLbig->next = m_poolVBig;
    if(m_poolVBig)
        m_poolVBig->prew = blockLbig;
    m_poolVBig = blockLbig;
}

void aist_mem_T::FreeStorage()
{
    memSpinLockWait(&m_Lock);
    MemStorageBlock *block = m_bottom;
    while(block != NULL)
    {
        MemStorageBlock *temp = block;
        block = block->next;
        aist_os_free((void*)temp);
    }
    m_bottom     = NULL;
    m_blockAlloc = 0;

    m_blockAllocUse = (m_bottom != NULL) ? 1 : 0;
    m_top        = m_bottom;
    m_free_space = (m_bottom != NULL) ? (m_block_size - sizeof(MemStorageBlock)) : 0;
    m_poolSize   = 0;
    aist_os_memset(m_poolLite,   0, sizeof(m_poolLite));
    aist_os_memset(m_poolMedium, 0, sizeof(m_poolMedium));
    aist_os_memset(m_poolLarge,  0, sizeof(m_poolLarge));
    m_poolVBig = NULL;

#ifdef AIST_DEBUG_
    m_debug_alloc_count = 0;
    m_debug_alloc_size  = 0;
#endif
    memSpinLockReset(&m_Lock);
}

void aist_mem_T::Clear()
{
    memSpinLockWait(&m_Lock);
    m_blockAllocUse = (m_bottom != NULL) ? 1 : 0;
    m_top        = m_bottom;
    m_free_space = (m_bottom != NULL) ? (m_block_size - sizeof(MemStorageBlock)) : 0;
    m_poolSize   = 0;
    aist_os_memset(m_poolLite,   0, sizeof(m_poolLite));
    aist_os_memset(m_poolMedium, 0, sizeof(m_poolMedium));
    aist_os_memset(m_poolLarge,  0, sizeof(m_poolLarge));
    m_poolVBig = NULL;
#ifdef AIST_DEBUG_
    m_debug_alloc_count = 0;
    m_debug_alloc_size  = 0;
#endif
    memSpinLockReset(&m_Lock);
}

size_t aist_mem_T::AllocSize()
{
    return m_blockAllocUse*m_block_size - m_free_space - m_poolSize; 
}

#ifdef _WIN32

size_t aist_os_vsnprintf(char* dst, size_t dst_size, const char* format, va_list arg)
{
    StringCbVPrintfA(dst, dst_size, format, arg);
    return lstrlenA(dst);
}


size_t aist_os_snprintf(char* dst, size_t dst_size, const char* format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    size_t len = aist_os_vsnprintf(dst, dst_size, format, arg_ptr);
    va_end(arg_ptr);
    return len;
}

#else


#endif
