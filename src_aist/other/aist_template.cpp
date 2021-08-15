/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include "../aist_dynamic_array.hpp"
#include "../aist_runtime.h"


static aist_ast_T* aist_function_write(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    aist_dynamic_array_T* dynamic_buf = (aist_dynamic_array_T*)runtime->m_user_data[0];
    if(!dynamic_buf)
        runtime->rt_error(NULL, line_pos12, "'write' aist_dynamic_string_T == NULL");
    for(size_t i = 0; i < args->m_ast_count; i++)
    {
        size_t len;
        const char* value;
        if(!runtime->ast_to_str(args->m_ast_items[i], value, len))
            continue;
        dynamic_buf->write(value, len);
    }
    return NULL;
}


static bool __stdcall flush_template_data(size_t offset, const void* data, size_t size, void* lParam)
{
    aist_os_memcpy(((char*)lParam) + offset, data, size);
    return true;
}


bool aist_process_template(aist_T aist, const char* label_src, const char *buf, size_t buf_len, char*& outData, size_t& outDataLen)
{
    aist_dynamic_array_T dynamic_buf;
    size_t pos = 0;
    size_t shift_line = 0;
    aist.set_user_data(&dynamic_buf, 0);
    aist.register_function("write", aist_function_write);

    while(pos < buf_len)
    {
        size_t start_pos = pos;
        for(;pos < buf_len; pos++)
        {
            if(buf[pos] == '\n')
            {
                shift_line ++;
                continue;
            }
            if(pos && buf[pos-1] == '!' && buf[pos] == '{')
            {
                pos++;
                break;
            }
        }
        if(pos == buf_len)
        {
            dynamic_buf.write(&buf[start_pos], pos - start_pos);
            break;
        }
        dynamic_buf.write(&buf[start_pos], pos - start_pos - 2);
        size_t tmp_shift_line = 0;
        start_pos = pos;
        for(;pos < buf_len; pos++)
        {
            if(buf[pos] == '\n')
            {
                tmp_shift_line ++;
                continue;
            }
            if(buf[pos-1] == '}' && buf[pos] == '!')
            {
                pos++;
                break;
            }
        }
        if(pos == buf_len && buf[pos-1] != '!')
        {
            if(!aist.run(&buf[start_pos], pos - start_pos, label_src, shift_line))
            {
                aist.set_user_data(NULL, 0);
                return false;
            }
            size_t outLen = 0;
            const char* ret = aist.get_return(&outLen);
            dynamic_buf.write(ret, outLen);
            break;
        }else{
            if(!aist.run(&buf[start_pos], pos - start_pos - 2, label_src, shift_line))
            {
                aist.set_user_data(NULL, 0);
                return false;
            }
            size_t outLen = 0;
            const char* ret = aist.get_return(&outLen);
            dynamic_buf.write(ret, outLen);
        }
        shift_line += tmp_shift_line;
    }
    outDataLen = dynamic_buf.size();
    outData = (char*)aist_os_malloc(outDataLen+1);
    dynamic_buf.flush(flush_template_data, outData);
    outData[outDataLen] = 0;
    aist.set_user_data(NULL, 0);
    return true;
}

