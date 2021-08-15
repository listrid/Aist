#include <stdio.h>
#include "../test.h"
#include "../../src_aist/aist_dynamic_array.hpp"

void test_On_file(const char* fileName);
bool compilre_to_file(const char* in_fileName, const char* out_fileName);


const char* test_hash_str = "\n\
print(\"#define AIST_LEX_THIS   \" + \"this\".hash());\n\
print(\"#define AIST_LEX_DELETE \" + \"delete\".hash());\n\
print(\"#define AIST_LEX_NEW    \" + \"new\".hash());\n\
print(\"#define AIST_LEX_LEN    \" + \"len\".hash());\n\
print(\"#define AIST_LEX_SIZE   \" + \"size\".hash());\n\
print(\"#define AIST_LEX_COUNT  \" + \"count\".hash());\n\
print(\"======================================\");\n\
print();\n\
print(\"#define sys_print    \"+\"print\".hash());\n\
print(\"#define sys_strftime \"+\"strftime\".hash());\n\
print(\"#define sys_dbgMem   \"+\"dbgMem\".hash());\n\
print(\"#define sys_toChar   \"+\"toChar\".hash());\n\
print(\"#define sys_toBool   \"+\"toBool\".hash());\n\
print(\"#define sys_toInt    \"+\"toInt\".hash());\n\
print(\"#define sys_toFloat  \"+\"toFloat\".hash());\n\
print(\"#define sys_toString \"+\"toString\".hash());\n\
print(\"#define sys_isDef    \"+\"isDef\".hash());\n\
print(\"#define sys_isChar   \"+\"isChar\".hash());\n\
print(\"#define sys_isBool   \"+\"isBool\".hash());\n\
print(\"#define sys_isInt    \"+\"isInt\".hash());\n\
print(\"#define sys_isFloat  \"+\"isFloat\".hash());\n\
print(\"#define sys_isString \"+\"isString\".hash());\n\
print(\"#define sys_isObject \"+\"isObject\".hash());\n\
print(\"#define sys_isList   \"+\"isList\".hash());\n\
print(\"#define sys_isNULL   \"+\"isNULL\".hash());\n\
print(\"#define sys_isFn     \"+\"isFn\".hash());\n\
print(\"#define sys_isConst  \"+\"isConst\".hash());\n\
print(\"#define sys_pop      \"+\"pop\".hash());\n\
print(\"#define sys_push     \"+\"push\".hash());\n\
print(\"#define sys_remove   \"+\"remove\".hash());\n\
print(\"#define sys_clear    \"+\"clear\".hash());\n\
print(\"#define sys_rstrip   \"+\"rstrip\".hash());\n\
print(\"#define sys_lstrip   \"+\"lstrip\".hash());\n\
print(\"#define sys_strip    \"+\"strip\".hash());\n\
print(\"#define sys_test     \"+\"test\".hash());\n\
print(\"#define sys_hash     \"+\"hash\".hash());\n\
";



void main_test_hash()
{
    aist_T* aist = aist_create();

    if(!aist->run(test_hash_str, 0, "test"))
    {
        const aist_T::error_T* err = aist->get_error();
        printf("%s '%s'[line %u:%u] %s\n", err->m_label_src, err->m_isParser?"ErrorParser":"ErrorRt", err->m_line, err->m_pos, err->m_errorStr);
        getchar();
    }else{


    }
    getchar();
}



void main_test()
{
    //main_test_hash(); return;

    //compilre_to_file("./test/include2.aist", "./test/include2.bin");


    //test_On_file("./test/list1.txt");
    //test_On_file("./test/object1.txt");

    //test_On_file("./test/var_level.txt");


    test_On_file("./test/test.txt");
    //test_On_file("./test/obj.txt");
    //test_On_file("./test/arithmetic.txt");
    // test_On_file("./test/include.txt");


    //test_On_file("./test/test_fn_call.txt");

   // test_On_file("./test/test_dump.txt");

    //test_On_file("sample/bubbleSort.aist");
   // test_On_file("sample/factorial.aist");


}



void test_On_file(const char* fileName)
{
    aist_T* aist = aist_create();
    size_t src_dataLen;
    char* src_data = read_file_text(fileName, src_dataLen);
    if(src_dataLen)
    {
        const char* label = fileName;
        for(size_t i = 0; fileName[i]; i++)
        {
            if(fileName[i] == '/' || fileName[i] == '\\')
                label = &fileName[i+1];
        }

        if(!aist->run(src_data, 0, label))
        {
            const aist_T::error_T* err = aist->get_error();
            printf("%s '%s'[line %u:%u] %s\n", err->m_label_src, err->m_isParser?"ErrorParser":"ErrorRt",err->m_line, err->m_pos, err->m_errorStr);
            getchar();
        }else{
            const char* ret = aist->get_return();
            printf("return '%s'\n", ret);
        }
    }else{
        printf("--- no read file  '%s' -----\r\n", fileName);
    }
    free(src_data);
    aist_int r, ii;
    aist->get_int("o.l[1].l[3][1].", r);
    aist->set_int("o.l[1].l[1].qq", 57);


    aist->get_int("o.qq", ii);


    main_Shell(aist);
}




static bool __stdcall flush_data(size_t offset, const void* data, size_t size, void* lParam)
{
    fwrite(data, size, 1, (FILE*)lParam);
    return true;
}


bool compilre_to_file(const char* in_fileName, const char* out_fileName)
{
    size_t src_dataLen;
    char* src_data = read_file_text(in_fileName, src_dataLen);
    if(!src_dataLen)
        return false;
    const char* label = in_fileName;
    for(size_t i = 0; in_fileName[i]; i++)
    {
        if(in_fileName[i] == '/' || in_fileName[i] == '\\')
            label = &in_fileName[i+1];
    }
    aist_mem_T* mem_rt = new aist_mem_T();
    mem_rt->Init(AIST::MAX_RT_STRING_SIZE);
    aist_runtime_T* rt = aist_runtime_T::create(mem_rt);
    const aist_ast_T* ast = rt->parse(label, src_data, src_dataLen,  0);
    free(src_data);
    if(!ast)
    {
        aist_T aist(rt);
        const aist_T::error_T* err = aist.get_error();
        printf("%s '%s'[line %u:%u] %s\n", err->m_label_src, err->m_isParser?"ErrorParser":"ErrorRt", err->m_line, err->m_pos, err->m_errorStr);
        aist_runtime_T::destroy(rt);
        delete mem_rt;
        printf("=======================================\r\n");
        return false;
    }
    aist_dynamic_array_T outData;
    aist_ast_save_zpak(ast, &outData);
    aist_runtime_T::destroy(rt);
    delete mem_rt;

    FILE* f = fopen(out_fileName, "wb+");
    if(!f)
        return false;
    outData.flush(flush_data, f);
    fclose(f);
    return true;
}



char* read_file_text(const char* filename, size_t& outLen)
{
    char* buffer = 0;
    long length;
    FILE* f = fopen(filename, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char*)malloc(length+1);
        if(buffer)
            fread(buffer, 1, length, f);
        fclose(f);
        buffer[length] = 0;
        outLen = length;
        return buffer;
    }
    outLen = 0;
    return NULL;
}

