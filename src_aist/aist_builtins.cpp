/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include <stdio.h>
#include <time.h>
#include "aist_runtime.h"
#include "aist.h"


static aist_ast_T* builtin_isDef(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count == 1)
    {
        runtime->expect_args(line_pos12, args, 1, AIST::AST_type::AST_STRING);
        aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
        ast_ret->m_boolean_value = runtime->rt_find_var(args->m_ast_items[0]->m_string_value) != NULL;
        return ast_ret;
    }else{
        runtime->expect_args(line_pos12, args, 2, AIST::AST_type::AST_OBJECT, AIST::AST_type::AST_STRING);
        aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
#ifdef AIST_USE_HASH_NAME
        ast_ret->m_boolean_value = args->m_ast_items[0]->m_object_children->find(aist_str_hash(args->m_ast_items[1]->m_string_value, args->m_ast_items[1]->m_string_value_len)) != NULL;
#else
        ast_ret->m_boolean_value = args->m_ast_items[0]->m_object_children->find(args->m_ast_items[1]->m_string_value) != NULL;
#endif
        return ast_ret;
    }
}


static aist_ast_T* builtin_isXXX(aist_runtime_T* runtime, const aist_ast_list_T* args, AIST::AST_type test_type)
{
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
    ast_ret->m_boolean_value = (args->m_ast_count == 1) && (args->m_ast_items[0]->m_ast_type == test_type);
    return ast_ret;
}


static aist_ast_T* builtin_isChar  (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_UCHAR); }
static aist_ast_T* builtin_isBool  (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_BOOLEAN); }
static aist_ast_T* builtin_isInt   (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_INTEGER); }
static aist_ast_T* builtin_isFloat (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_FLOAT); }
static aist_ast_T* builtin_isString(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_STRING); }
static aist_ast_T* builtin_isObject(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_OBJECT); }
static aist_ast_T* builtin_isList  (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_LIST); }
static aist_ast_T* builtin_isFn    (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_FUNCTION_DEFINITION); }
static aist_ast_T* builtin_isNULL  (aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args) { return builtin_isXXX(runtime, args, AIST::AST_type::AST_NULL); }


static aist_ast_T* builtin_isConst(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
    ast_ret->m_boolean_value = (args->m_ast_count == 1) && (args->m_ast_items[0]->m_flags & AIST::FLAG_CONST_VAR) != 0;
    return ast_ret;
}


static aist_ast_T* builtin_toChar(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1)
        runtime->rt_error(NULL, line_pos12, "Expected %u parameters, but transferred 1.", (int)args->m_ast_count);
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_UCHAR);
    aist_ast_T* ast_in = args->m_ast_items[0];
    switch(ast_in->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:   ast_ret->m_char_value = ast_in->m_char_value; break;
        case AIST::AST_type::AST_BOOLEAN: ast_ret->m_char_value = (char)ast_in->m_boolean_value; break;
        case AIST::AST_type::AST_INTEGER: ast_ret->m_char_value = ast_in->m_int_value&0xFF; break;
        case AIST::AST_type::AST_FLOAT:   ast_ret->m_char_value = ((aist_int)ast_in->m_float_value)&0xFF; break;
        case AIST::AST_type::AST_STRING:  ast_ret->m_char_value = atoi(ast_in->m_string_value)&0xFF; break;
        default:
            runtime->rt_error(ast_ret, line_pos12, "Cannot convert %s to %s", aist_type_name(ast_in->m_ast_type), aist_type_name(ast_ret->m_ast_type));
    }
    return ast_ret;
}


static aist_ast_T* builtin_toBool(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1)
        runtime->rt_error(NULL, line_pos12, "Expected %u parameters, but transferred 1.", (int)args->m_ast_count);
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
    aist_ast_T* ast_in = args->m_ast_items[0];
    switch(ast_in->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:   ast_ret->m_boolean_value = ast_in->m_char_value != 0; break;
        case AIST::AST_type::AST_BOOLEAN: ast_ret->m_boolean_value = ast_in->m_boolean_value != 0; break;
        case AIST::AST_type::AST_INTEGER: ast_ret->m_boolean_value = ast_in->m_int_value != 0; break;
        case AIST::AST_type::AST_FLOAT:   ast_ret->m_boolean_value = ast_in->m_float_value != 0.; break;
        case AIST::AST_type::AST_STRING:  ast_ret->m_boolean_value = aist_os_strcmp(ast_in->m_string_value, "true") == 0; break;
        default:
            runtime->rt_error(ast_ret, line_pos12, "Cannot convert %s to %s", aist_type_name(ast_in->m_ast_type), aist_type_name(ast_ret->m_ast_type));
    }
    return ast_ret;
}


static aist_ast_T* builtin_toInt(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1)
        runtime->rt_error(NULL, line_pos12, "Expected %u parameters, but transferred 1.", (int)args->m_ast_count);
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_INTEGER);
    aist_ast_T* ast_in = args->m_ast_items[0];
    switch(ast_in->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:   ast_ret->m_int_value = ast_in->m_char_value; break;
        case AIST::AST_type::AST_BOOLEAN: ast_ret->m_int_value = ast_in->m_boolean_value; break;
        case AIST::AST_type::AST_INTEGER: ast_ret->m_int_value = ast_in->m_int_value; break;
        case AIST::AST_type::AST_FLOAT:   ast_ret->m_int_value = (aist_int)ast_in->m_float_value; break;
        case AIST::AST_type::AST_STRING:
        {
            bool minus = false;
            aist_int v = 0;
            const char* h = ast_in->m_string_value;
            if(h[0] == '-')
            {
                minus = true;
                h++;
            }
            while(h[0] >= '0' && h[0] <= '9')
            {
                v = v*10 + (h[0] - '0');
                h++;
            }
            if(minus)
                v = -v;
            ast_ret->m_int_value = v;
        }break;
        default:
            runtime->rt_error(ast_ret, line_pos12, "Cannot convert %s to %s", aist_type_name(ast_in->m_ast_type), aist_type_name(ast_ret->m_ast_type));
    }
    return ast_ret;
}


static aist_ast_T* builtin_toFloat(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1)
        runtime->rt_error(NULL, line_pos12, "Expected %u parameters, but transferred 1.", (int)args->m_ast_count);
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_FLOAT);
    aist_ast_T* ast_in = args->m_ast_items[0];
    switch(ast_in->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:   ast_ret->m_float_value = (aist_float)ast_in->m_char_value; break;
        case AIST::AST_type::AST_BOOLEAN: ast_ret->m_float_value = (aist_float)ast_in->m_boolean_value; break;
        case AIST::AST_type::AST_INTEGER: ast_ret->m_float_value = (aist_float)ast_in->m_int_value; break;
        case AIST::AST_type::AST_FLOAT:   ast_ret->m_float_value = ast_in->m_float_value; break;
        case AIST::AST_type::AST_STRING:  ast_ret->m_float_value = (aist_float)atof(ast_in->m_string_value); break;
        default:
            runtime->rt_error(ast_ret, line_pos12, "Cannot convert %s to %s", aist_type_name(ast_in->m_ast_type), aist_type_name(ast_ret->m_ast_type));
    }
    return ast_ret;
}


static aist_ast_T* builtin_toString(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1)
        runtime->rt_error(NULL, line_pos12, "Expected %u parameters, but transferred 1.", (int)args->m_ast_count);
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    aist_ast_T* ast_in = args->m_ast_items[0];
    const char* add_str;
    size_t add_str_len;
    if(runtime->ast_to_str(ast_in, add_str, add_str_len))
    {
        ast_ret->m_string_value_len = (unsigned int)add_str_len;
        if(add_str_len)
            ast_ret->m_string_value = runtime->rt_str_clone(add_str, add_str_len);
    }else{
        runtime->rt_error(ast_ret, line_pos12, "Cannot convert %s to %s", aist_type_name(ast_in->m_ast_type), aist_type_name(ast_ret->m_ast_type));
    }
    return ast_ret;
}


static aist_ast_T* builtin_strftime(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    runtime->expect_args(line_pos12, args, 1, AIST::AST_type::AST_STRING);
    char str[128];
    time_t rawtime;
    time(&rawtime);
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    size_t len = strftime(str, sizeof(str)-1, args->m_ast_items[0]->m_string_value, &timeinfo);
    aist_ast_T* ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ret->m_string_value = runtime->rt_str_clone(str, len);
    ret->m_string_value_len = (unsigned int)len;
    return ret;
}


static aist_ast_T* builtin_print(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    for(size_t i = 0; i < args->m_ast_count; i++)
    {
        aist_ast_T* ast_arg = args->m_ast_items[i];
        const char* str;
        size_t  outLen;
        if(!runtime->ast_to_str(ast_arg, str, outLen))
        {
            printf("NULL");
        }else{
            printf("%s", str);
        }
    }
    printf("\n");
    return NULL;
}


static aist_ast_T* builtin_dbg_mem(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args)
{
    char str[128];
    size_t len = aist_os_snprintf(str, 128, "rt_memAllocSize: %u rt_fulMem: %u\r\n", runtime->m_mem_rt->AllocSize(), runtime->m_mem_rt->StorageSize());
    aist_ast_T* ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ret->m_string_value = runtime->rt_str_clone(str, len);
    ret->m_string_value_len = (unsigned int)len;
    return ret;
}


//==============================================================================
//==================================== list ====================================
//==============================================================================


static aist_ast_T* aist_list_method_pop(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* list_this, const aist_ast_list_T* args)
{
    if((list_this->m_flags&AIST::FLAG_CONST_VAR) != 0)
        runtime->rt_error(NULL, line_pos12, "Cant modify const list");
    if(!list_this->m_list_children->m_ast_count)
        return runtime->rt_get_null();
    list_this->m_list_children->m_ast_count--;
    aist_ast_T* ret_ast =  list_this->m_list_children->m_ast_items[list_this->m_list_children->m_ast_count];
    return ret_ast;
}


static aist_ast_T* aist_list_method_push(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* list_this, const aist_ast_list_T* args)
{
    if((list_this->m_flags&AIST::FLAG_CONST_VAR) != 0)
        runtime->rt_error(NULL, line_pos12, "Cant modify const list");
    for(size_t i = 0; i < args->m_ast_count; i++)
        list_this->m_list_children->push(args->m_ast_items[i]);
    return NULL;
}


static aist_ast_T* aist_list_method_clear(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* list_this, const aist_ast_list_T* args)
{
    if((list_this->m_flags&AIST::FLAG_CONST_VAR) != 0)
        runtime->rt_error(NULL, line_pos12, "Cant modify const list");
    list_this->m_list_children->removeAll(runtime);
    return NULL;
}


static aist_ast_T* aist_list_method_remove(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* list_this, const aist_ast_list_T* args)
{
    if((list_this->m_flags&AIST::FLAG_CONST_VAR) != 0)
        runtime->rt_error(NULL, line_pos12, "Cant modify const list");
    runtime->expect_args(line_pos12, args, 1, AIST::AST_type::AST_INTEGER);
    aist_ast_T* ast_int = args->m_ast_items[0];
    if(ast_int->m_int_value >(int)list_this->m_list_children->m_ast_count-1)
        runtime->rt_error(NULL, line_pos12, "Index out of range (%u (index) > %u)", (int)ast_int->m_int_value, (int)list_this->m_list_children->m_ast_count-1);
    list_this->m_list_children->remove(list_this->m_list_children->m_ast_items[ast_int->m_int_value], runtime);
    return NULL;
}


//==============================================================================
//=================================== string ===================================
//==============================================================================


static aist_ast_T* aist_string_method_rstrip(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* str_this, const aist_ast_list_T* args)
{
    char t = ' ';
    if(args->m_ast_count > 0 && args->m_ast_items[0]->m_ast_type == AIST::AST_type::AST_UCHAR)
        t = args->m_ast_items[0]->m_char_value;
    size_t len = str_this->m_string_value_len;
    char* str  = str_this->m_string_value;
    while(len && str[len-1] == t)
        len--;
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ast_ret->m_string_value_len = (unsigned int)len;
    ast_ret->m_string_value = runtime->rt_str_clone(str, len);
    return ast_ret;
}


static aist_ast_T* aist_string_method_lstrip(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* str_this, const aist_ast_list_T* args)
{
    char t = ' ';
    if(args->m_ast_count > 0 && args->m_ast_items[0]->m_ast_type == AIST::AST_type::AST_UCHAR)
        t = args->m_ast_items[0]->m_char_value;
    size_t len = str_this->m_string_value_len;
    char* str  = str_this->m_string_value;
    size_t p = 0;
    while(p < len && str[p] == t)
        p++;
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ast_ret->m_string_value_len = (unsigned int)(len - p);
    ast_ret->m_string_value = runtime->rt_str_clone(str+p, len-p);
    return ast_ret;
}


static aist_ast_T* aist_string_method_strip(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* str_this, const aist_ast_list_T* args)
{
    char t = ' ';
    if(args->m_ast_count > 0 && args->m_ast_items[0]->m_ast_type == AIST::AST_type::AST_UCHAR)
        t = args->m_ast_items[0]->m_char_value;
    size_t len = str_this->m_string_value_len;
    char* str  = str_this->m_string_value;
    while(len && str[len-1] == t)
        len--;
    size_t p = 0;
    while(p < len && str[p] == t)
        p++;
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ast_ret->m_string_value_len = (unsigned int)(len - p);
    ast_ret->m_string_value = runtime->rt_str_clone(str+p, len-p);
    return ast_ret;
}


static bool FullCheckString(const char* name, size_t nameLen, const char* mask, size_t maskLen)
{
    size_t i, iM, endI = nameLen;
    if(endI > maskLen)
        endI = maskLen;
    for(i = 0; i<endI; i++)
    {
        if(mask[i] != '?' && mask[i] != name[i])
            break;
    }
    if(i == maskLen)
        return (maskLen == nameLen);
    if(mask[i] != '*')
        return false;
    iM = i;
    size_t lastM;
    for(;; iM++, i++)
    {
        while(mask[iM] == '*')
        {
            lastM = iM++;
            if(iM >= maskLen)
                return true;
        }
        if(i == nameLen)
            return (iM == maskLen);
        if(mask[iM] != '?' && mask[iM] != name[i])
        {
            i  -= iM - lastM -1;
            iM  = lastM;
        }
    }
}


static aist_ast_T* aist_string_method_test(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* str_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 1 || args->m_ast_items[0]->m_ast_type != AIST::AST_type::AST_STRING)
        runtime->rt_error(NULL, line_pos12, "Incorrect mask. not string.");
    aist_ast_T* mask = args->m_ast_items[0];
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
    ast_ret->m_boolean_value = FullCheckString(str_this->m_string_value, str_this->m_string_value_len, mask->m_string_value, mask->m_string_value_len);
    return ast_ret;
}


static aist_ast_T* aist_string_method_hash(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* str_this, const aist_ast_list_T* args)
{
    if(args->m_ast_count != 0)
        runtime->rt_error(NULL, line_pos12, "Incorrect param.");
    char str[64];
    size_t len = aist_os_snprintf(str, 64, "0x%.16llX", aist_str_hash(str_this->m_string_value, str_this->m_string_value_len));
    aist_ast_T* ast_ret = runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    ast_ret->m_string_value_len = (unsigned int)len;
    ast_ret->m_string_value = runtime->rt_str_clone(str, len);
    return ast_ret;

}


static aist_ast_list_T gArgNull;
static const char* gS = "sys";

#ifdef AIST_USE_HASH_NAME

#define sys_print    0x000000E8DCD2E4E1
#define sys_strftime 0x0035CF1920196D66
#define sys_dbgMem   0x0000DACA9ACEC4C9
#define sys_toChar   0x0000E4C2D086DEE9
#define sys_toBool   0x0000D8DEDE84DEE9
#define sys_toInt    0x000000E8DC92DEE9
#define sys_toFloat  0x0001A16DDA7F27FC
#define sys_toString 0x0035CF29682F5D7E
#define sys_isDef    0x000000CCCA88E6D3
#define sys_isChar   0x0000E4C2D086E6D3
#define sys_isBool   0x0000D8DEDE84E6D3
#define sys_isInt    0x000000E8DC92E6D3
#define sys_isFloat  0x0001A1674FA7346E
#define sys_isString 0x0035CE518258F830
#define sys_isObject 0x0035CE516D1B5630
#define sys_isList   0x0000E8E6D298E6D3
#define sys_isNULL   0x00009898AA9CE6D3
#define sys_isFn     0x00000000DC8CE6D3
#define sys_isConst  0x0001A1674F3DE7D0
#define sys_pop      0x0000000000E0DEE1
#define sys_push     0x00000000D0E6EAE1
#define sys_remove   0x0000CAECDEDACAE5
#define sys_clear    0x000000E4C2CAD8C7
#define sys_rstrip   0x0000E0D2E4E8E6E5
#define sys_lstrip   0x0000E0D2E4E8E6D9
#define sys_strip    0x000000E0D2E4E8E7
#define sys_test     0x00000000E8E6CAE9
#define sys_hash     0x00000000D0E6C2D1

#else

#define sys_print    "print"
#define sys_strftime "strftime"
#define sys_dbgMem   "dbgMem"
#define sys_toChar   "toChar"
#define sys_toBool   "toBool"
#define sys_toInt    "toInt"
#define sys_toFloat  "toFloat"
#define sys_toString "toString"
#define sys_isDef    "isDef"
#define sys_isChar   "isChar"
#define sys_isBool   "isBool"
#define sys_isInt    "isInt"
#define sys_isFloat  "isFloat"
#define sys_isString "isString"
#define sys_isObject "isObject"
#define sys_isList   "isList"
#define sys_isNULL   "isNULL"
#define sys_isFn     "isFn"
#define sys_isConst  "isConst"

#define sys_pop      "pop"
#define sys_push     "push"
#define sys_remove   "remove"
#define sys_clear    "clear"
#define sys_rstrip   "rstrip"
#define sys_lstrip   "lstrip"
#define sys_strip    "strip"
#define sys_test     "test"
#define sys_hash     "hash"

#endif


static aist_ast_T::fn_def_T gSysFnDef[] =
{
    {sys_print,    gS, &gArgNull, NULL, builtin_print,    AIST::AST_type::AST_NULL},
    {sys_strftime, gS, &gArgNull, NULL, builtin_strftime, AIST::AST_type::AST_STRING},
    {sys_dbgMem,   gS, &gArgNull, NULL, builtin_dbg_mem,  AIST::AST_type::AST_STRING},
// convert
    {sys_toChar,  gS, &gArgNull, NULL, builtin_toChar,  AIST::AST_type::AST_UCHAR},
    {sys_toBool,  gS, &gArgNull, NULL, builtin_toBool,  AIST::AST_type::AST_BOOLEAN},
    {sys_toInt,   gS, &gArgNull, NULL, builtin_toInt,   AIST::AST_type::AST_INTEGER},
    {sys_toFloat, gS, &gArgNull, NULL, builtin_toFloat, AIST::AST_type::AST_FLOAT},
    {sys_toString,gS, &gArgNull, NULL, builtin_toString,AIST::AST_type::AST_STRING},
// test type
    {sys_isDef,   gS, &gArgNull, NULL, builtin_isDef,   AIST::AST_type::AST_BOOLEAN},
    {sys_isChar,  gS, &gArgNull, NULL, builtin_isChar,  AIST::AST_type::AST_BOOLEAN},
    {sys_isBool,  gS, &gArgNull, NULL, builtin_isBool,  AIST::AST_type::AST_BOOLEAN},
    {sys_isInt,   gS, &gArgNull, NULL, builtin_isInt,   AIST::AST_type::AST_BOOLEAN},
    {sys_isFloat, gS, &gArgNull, NULL, builtin_isFloat, AIST::AST_type::AST_BOOLEAN},
    {sys_isString,gS, &gArgNull, NULL, builtin_isString,AIST::AST_type::AST_BOOLEAN},
    {sys_isObject,gS, &gArgNull, NULL, builtin_isObject,AIST::AST_type::AST_BOOLEAN},
    {sys_isList,  gS, &gArgNull, NULL, builtin_isList,  AIST::AST_type::AST_BOOLEAN},
    {sys_isNULL,  gS, &gArgNull, NULL, builtin_isNULL,  AIST::AST_type::AST_BOOLEAN},
    {sys_isFn,    gS, &gArgNull, NULL, builtin_isFn,    AIST::AST_type::AST_BOOLEAN},
    {sys_isConst, gS, &gArgNull, NULL, builtin_isConst, AIST::AST_type::AST_BOOLEAN},

};


static aist_ast_T::fn_def_T gListFnDef[] =
{
    {sys_pop,    gS, &gArgNull, NULL, aist_list_method_pop,    AIST::AST_type::AST_ANY},
    {sys_push,   gS, &gArgNull, NULL, aist_list_method_push,   AIST::AST_type::AST_NULL},
    {sys_remove, gS, &gArgNull, NULL, aist_list_method_remove, AIST::AST_type::AST_NULL},
    {sys_clear,  gS, &gArgNull, NULL, aist_list_method_clear,  AIST::AST_type::AST_NULL},

};


static aist_ast_T::fn_def_T gStringFnDef[] =
{
    {sys_rstrip, gS, &gArgNull, NULL, aist_string_method_rstrip, AIST::AST_type::AST_STRING},
    {sys_lstrip, gS, &gArgNull, NULL, aist_string_method_lstrip, AIST::AST_type::AST_STRING},
    {sys_strip,  gS, &gArgNull, NULL, aist_string_method_strip,  AIST::AST_type::AST_STRING},
    {sys_test,   gS, &gArgNull, NULL, aist_string_method_test,   AIST::AST_type::AST_BOOLEAN},
    {sys_hash,   gS, &gArgNull, NULL, aist_string_method_hash,   AIST::AST_type::AST_STRING},

    
};


static aist_ast_T gAst_SysDef[sizeof(gSysFnDef)/sizeof(gSysFnDef[0])];
static aist_ast_T gAst_ListDef[sizeof(gListFnDef)/sizeof(gListFnDef[0])];
static aist_ast_T gAst_StringDef[sizeof(gStringFnDef)/sizeof(gStringFnDef[0])];


#ifdef AIST_USE_HASH_NAME

aist_ast_T* aist_builtins_sys_variable_find(aist_hash_T name)
{
    for(size_t i = 0; i < (sizeof(gSysFnDef)/sizeof(gSysFnDef[0])); i++)
    {
        if(gSysFnDef[i].m_function_name_hash == name)
            return &gAst_SysDef[i];
    }
    return NULL;
}


aist_ast_T* aist_builtins_list_fn_find(aist_hash_T name)
{
    for(size_t i = 0; i < (sizeof(gListFnDef)/sizeof(gListFnDef[0])); i++)
    {
        if(gListFnDef[i].m_function_name_hash == name)
            return &gAst_ListDef[i];
    }
    return NULL;
}


aist_ast_T* aist_builtins_string_fn_find(aist_hash_T name)
{
    for(size_t i = 0; i < (sizeof(gStringFnDef)/sizeof(gStringFnDef[0])); i++)
    {
        if(gStringFnDef[i].m_function_name_hash == name)
            return &gAst_StringDef[i];
    }
    return NULL;
}

#else

aist_ast_T* aist_builtins_sys_variable_find(const char* name)
{
    for(size_t i = 0; i < (sizeof(gSysFnDef)/sizeof(gSysFnDef[0])); i++)
    {
        if(aist_os_strcmp(gSysFnDef[i].m_function_name, name) == 0)
            return &gAst_SysDef[i];
    }
    return NULL;
}


aist_ast_T* aist_builtins_list_fn_find(const char* name)
{
    for(size_t i = 0; i < (sizeof(gListFnDef)/sizeof(gListFnDef[0])); i++)
    {
        if(aist_os_strcmp(gListFnDef[i].m_function_name, name) == 0)
            return &gAst_ListDef[i];
    }
    return NULL;
}


aist_ast_T* aist_builtins_string_fn_find(const char* name)
{
    for(size_t i = 0; i < (sizeof(gStringFnDef)/sizeof(gStringFnDef[0])); i++)
    {
        if(aist_os_strcmp(gStringFnDef[i].m_function_name, name) == 0)
            return &gAst_StringDef[i];
    }
    return NULL;
}
#endif



void aist_builtins_init()
{
    aist_os_memset(&gArgNull, 0, sizeof(gArgNull));
    if(gAst_SysDef[0].m_fn_def == NULL)
    {
        for(size_t i = 0; i < (sizeof(gSysFnDef)/sizeof(gSysFnDef[0])); i++)
        {
            gAst_SysDef[i].m_line_pos12 = 0;
            gAst_SysDef[i].m_ast_type = AIST::AST_type::AST_FUNCTION_DEFINITION;
            gAst_SysDef[i].m_flags    = AIST::FLAG_AST_MEM_RT | AIST::FLAG_NOT_RELEASE;
            gAst_SysDef[i].m_countUse = 1;
            gAst_SysDef[i].m_fn_def   = &gSysFnDef[i];
        }
    }
    if(gAst_ListDef[0].m_fn_def == NULL)
    {
        for(size_t i = 0; i < (sizeof(gListFnDef)/sizeof(gListFnDef[0])); i++)
        {
            gAst_ListDef[i].m_line_pos12 = 0;
            gAst_ListDef[i].m_ast_type = AIST::AST_type::AST_FUNCTION_DEFINITION;
            gAst_ListDef[i].m_flags    = AIST::FLAG_AST_MEM_RT | AIST::FLAG_NOT_RELEASE;
            gAst_ListDef[i].m_countUse = 1;
            gAst_ListDef[i].m_fn_def   = &gListFnDef[i];
        }
    }
    if(gAst_StringDef[0].m_fn_def == NULL)
    {
        for(size_t i = 0; i < (sizeof(gStringFnDef)/sizeof(gStringFnDef[0])); i++)
        {
            gAst_StringDef[i].m_line_pos12 = 0;
            gAst_StringDef[i].m_ast_type = AIST::AST_type::AST_FUNCTION_DEFINITION;
            gAst_StringDef[i].m_flags    = AIST::FLAG_AST_MEM_RT | AIST::FLAG_NOT_RELEASE;
            gAst_StringDef[i].m_countUse = 1;
            gAst_StringDef[i].m_fn_def   = &gStringFnDef[i];
        }
    }

}

