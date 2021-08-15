/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif
#include "aist_runtime.h"
#include <stdlib.h>
#include <string.h>


aist_hash_T aist_str_hash(const char* str, size_t len)
{
    if(!len)
        return 0;
    if(len < 7)
    {
        aist_hash_T hash = 0;
        aist_os_memcpy(&hash, str, len);
        return (hash<<1)|1;
    }
    aist_hash_T hash = 5381;//DJBHash
    for(size_t i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + (aist_hash_T)str[i];
    }
    return hash<<1;
}


char* aist_str_clone(aist_mem_T* mem, const char* value, size_t len)
{
    if(len == 0 && value)
        len = aist_os_strlen(value);
    char* ret = (char*)mem->Alloc(len+1);
    aist_os_memcpy(ret, value, len);
    ret[len] = 0;
    return ret;
}


void aist_str_free(aist_mem_T* mem, char*& value, size_t len)
{
    if(!value)
        return;
    if(len == 0)
        len = aist_os_strlen(value);
    mem->Free(value, len+1);
    value = NULL;
}


const char* aist_type_name(AIST::AST_type type)
{
    switch(type)
    {
        case AIST::AST_type::AST_NULL:   return "NULL";
        case AIST::AST_type::AST_VOID:   return "VOID";
        case AIST::AST_type::AST_UCHAR:  return "UCHAR";
        case AIST::AST_type::AST_BOOLEAN:return "BOOLEAN";
        case AIST::AST_type::AST_INTEGER:return "INT";
        case AIST::AST_type::AST_FLOAT:  return "FLOAT";
        case AIST::AST_type::AST_STRING: return "STRING";
        case AIST::AST_type::AST_LIST:   return "LIST";
        case AIST::AST_type::AST_OBJECT: return "OBJECT";
        case AIST::AST_type::AST_ANY:    return "ANY";
        case AIST::AST_type::AST_FUNCTION_DEFINITION: return "FN_DEF";
    }
    return "???";
}


const char* aist_LEXEME_name(AIST::LEXEME_type type)
{
    switch(type)
    {
        case AIST::LEXEME_type::LEXEME_LBRACE:       return "{";
        case AIST::LEXEME_type::LEXEME_RBRACE:       return "}";
        case AIST::LEXEME_type::LEXEME_LBRACKET:     return "[";
        case AIST::LEXEME_type::LEXEME_RBRACKET:     return "]";
        case AIST::LEXEME_type::LEXEME_LPAREN:       return "(";
        case AIST::LEXEME_type::LEXEME_RPAREN:       return ")";
        case AIST::LEXEME_type::LEXEME_DOT:          return ".";
        case AIST::LEXEME_type::LEXEME_EQUALS:       return "=";
        case AIST::LEXEME_type::LEXEME_SEMI:         return ";";
        case AIST::LEXEME_type::LEXEME_COMMA:        return ",";
        case AIST::LEXEME_type::LEXEME_PLUS:         return "+";
        case AIST::LEXEME_type::LEXEME_STAR:         return "*";
        case AIST::LEXEME_type::LEXEME_DIV:          return "/";
        case AIST::LEXEME_type::LEXEME_LESS:         return "<";
        case AIST::LEXEME_type::LEXEME_SHIFT_LEFT:   return "<<";
        case AIST::LEXEME_type::LEXEME_LEQUAL:       return "<=";
        case AIST::LEXEME_type::LEXEME_LARGER:       return ">";
        case AIST::LEXEME_type::LEXEME_SHIFT_RIGHT:  return ">>";
        case AIST::LEXEME_type::LEXEME_LAQUAL:       return ">=";
        case AIST::LEXEME_type::LEXEME_BOOLEAN_AND:  return "&&";
        case AIST::LEXEME_type::LEXEME_BOOLEAN_OR:   return "||";
        case AIST::LEXEME_type::LEXEME_EQUALS_EQUALS:return "==";
        case AIST::LEXEME_type::LEXEME_NOT_EQUALS:   return "!=";
        case AIST::LEXEME_type::LEXEME_BINARY_AND:   return "&";
        case AIST::LEXEME_type::LEXEME_BINARY_OR:    return "|";
        case AIST::LEXEME_type::LEXEME_PERCENTAGE:   return "%";
        case AIST::LEXEME_type::LEXEME_MINUS:        return "-";
        case AIST::LEXEME_type::LEXEME_BINARY_NOT:   return "~";
        case AIST::LEXEME_type::LEXEME_BINARY_XOR:   return "^";
        case AIST::LEXEME_type::LEXEME_PLUS_EQUALS:  return "+=";
        case AIST::LEXEME_type::LEXEME_MINUS_EQUALS: return "-=";
        case AIST::LEXEME_type::LEXEME_PLUS_PLUS:    return "++";
        case AIST::LEXEME_type::LEXEME_MINUS_MINUS:  return "--";
        case AIST::LEXEME_type::LEXEME_NOT:          return "!";
        case AIST::LEXEME_type::LEXEME_QUESTION:     return "?";
        case AIST::LEXEME_type::LEXEME_COLON:        return ":";
    };
    return "????";
}


aist_ast_list_T* aist_runtime_T::list_create(size_t initSize, bool useParseMem)
{
    aist_mem_T* mem = m_mem_rt;
    if(useParseMem)
        mem = m_mem_parser;
    if(!initSize)
        initSize = 2;
    aist_ast_list_T* ret = (aist_ast_list_T*)mem->Alloc(sizeof(aist_ast_list_T));
    ret->init(mem, initSize);
    return ret;

}


void aist_runtime_T::list_free(aist_ast_list_T*& ls)
{
    if(!ls)
        return;
    for(size_t i = 0; i < ls->m_ast_count; i++)
        ast_release(ls->m_ast_items[i]);
    ls->m_mem->Free(ls->m_ast_items, ls->m_real_size*sizeof(aist_ast_T*));
    ls->m_mem->Free(ls, sizeof(aist_ast_list_T));
    ls = NULL;
}


void aist_ast_list_T::init(aist_mem_T* mem, size_t initSize)
{
    m_ast_count = 0;
    m_real_size = initSize;
    m_ast_items = (aist_ast_T**)mem->Alloc(m_real_size*sizeof(aist_ast_T*));
    m_mem = mem;
}


void aist_ast_list_T::push(aist_ast_T* item)
{
    if(m_ast_count == m_real_size)
    {
        m_real_size = m_real_size + m_real_size/2 + 1;
        aist_ast_T** temp_ast_items = (aist_ast_T**)m_mem->Alloc(m_real_size*sizeof(aist_ast_T*));
        aist_os_memcpy(temp_ast_items, m_ast_items, m_ast_count*sizeof(aist_ast_T*));
        m_mem->Free(m_ast_items, m_ast_count*sizeof(aist_ast_T*));
        m_ast_items = temp_ast_items;
    }
    m_ast_items[m_ast_count] = item;
    m_ast_count++;
}


void aist_ast_list_T::reserve(size_t countItems)
{
    if(m_ast_count)
       return;
    m_mem->Free(m_ast_items, m_real_size*sizeof(aist_ast_T*));
    m_real_size = countItems;
    m_ast_items = (aist_ast_T**)m_mem->Alloc(m_real_size*sizeof(aist_ast_T*));
}


void aist_ast_list_T::removeAll(aist_runtime_T* free_rt)
{
    if(free_rt)
    {
        for(size_t i = 0; i < m_ast_count; i++)
            free_rt->ast_release(m_ast_items[i]);
    }
    m_ast_count = 0;
}


#ifdef AIST_USE_HASH_NAME
aist_ast_T* aist_ast_list_T::find(aist_hash_T name_hash)
#else
aist_ast_T* aist_ast_list_T::find(const char* name)
#endif
{
    for(size_t i = 0; i < m_ast_count; i++)
    {
        aist_ast_T* test_var = m_ast_items[i];
        if(test_var->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
        {
#ifdef AIST_USE_HASH_NAME
            if(test_var->m_var_def->m_name_hash == name_hash)
                return test_var;
#else
            if(aist_os_strcmp(test_var->m_var_def->m_name, name) == 0)
                return test_var;
#endif
        }
        if(test_var->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
        {
#ifdef AIST_USE_HASH_NAME
            if(test_var->m_fn_def->m_function_name_hash == name_hash)
                return test_var;
#else
            if(aist_os_strcmp(test_var->m_fn_def->m_function_name, name) == 0)
                return test_var;
#endif
        }
    }
    return NULL;
}


void aist_ast_list_T::remove(aist_ast_T* element, aist_runtime_T* free_rt)
{
    if(element == NULL)
        return;
    int index = -1;
    for(size_t i = 0; i < m_ast_count; i++)
    {
        if(m_ast_items[i] == element)
        {
            index = (int)i;
            break;
        }
    }
    if(index == -1)
        return;
    if(free_rt != NULL)
        free_rt->ast_release(element);
    for(size_t i = index; i < m_ast_count - 1; i++)
        m_ast_items[i] = m_ast_items[i + 1];
    m_ast_count --;
}


#ifdef AIST_DEBUG_
extern size_t gDbg_Init_num;
extern size_t gDbg_Free_num;
#endif


aist_ast_T* aist_runtime_T::ast_create(AIST::AST_type type, size_t flags)
{
    bool useParseMem = true;
    aist_mem_T* mem = m_mem_parser;
    if(flags&AIST::FLAG_AST_MEM_RT)
    {
        useParseMem = false;
        mem = m_mem_rt;
    }
    aist_ast_T* ast = (aist_ast_T*)mem->Alloc(sizeof(aist_ast_T));
    aist_os_memset(ast, 0, sizeof(aist_ast_T));
    ast->m_flags    = (unsigned char)flags;
    ast->m_ast_type = type;
    ast->m_line_pos12     = -1;
    ast->m_countUse = 1;
#ifdef AIST_DEBUG_AST_
    ast->dbg_num = gDbg_Init_num;
#endif
#ifdef AIST_DEBUG_
    gDbg_Init_num++;
#endif
    switch(type)
    {
        case AIST::AST_type::AST_STRING:ast->m_string_value_len = 0; break;
        case AIST::AST_type::AST_OBJECT:ast->m_object_children = list_create(2, useParseMem);break;
        case AIST::AST_type::AST_LIST:  ast->m_list_children   = list_create(2, useParseMem);break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
            ast->m_fn_def = (aist_ast_T::fn_def_T*)mem->Alloc(sizeof(aist_ast_T::fn_def_T));
            aist_os_memset(ast->m_fn_def, 0, sizeof(aist_ast_T::fn_def_T));
            ast->m_fn_def->m_arguments = list_create(2, useParseMem);
        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            ast->m_fn_call = (aist_ast_T::fn_call_T*)mem->Alloc(sizeof(aist_ast_T::fn_call_T));
            aist_os_memset(ast->m_fn_call, 0, sizeof(aist_ast_T::fn_call_T));
            ast->m_fn_call->m_arguments = list_create(2, useParseMem);
        }break;
        case AIST::AST_type::AST_BLOCK:ast->m_block_list = list_create(2, useParseMem);break;
        case AIST::AST_type::AST_FOR:
        {
            ast->m_for = (aist_ast_T::for_T*)mem->Alloc(sizeof(aist_ast_T::for_T));
            aist_os_memset(ast->m_for, 0, sizeof(aist_ast_T::for_T));
        }break;
        case AIST::AST_type::AST_IF:
        {
            ast->m_if = (aist_ast_T::if_T*)mem->Alloc(sizeof(aist_ast_T::if_T));
            aist_os_memset(ast->m_if, 0, sizeof(aist_ast_T::if_T));
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            ast->m_while = (aist_ast_T::while_T*)mem->Alloc(sizeof(aist_ast_T::while_T));
            aist_os_memset(ast->m_while, 0, sizeof(aist_ast_T::while_T));
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            ast->m_var_def = (aist_ast_T::var_def_T*)mem->Alloc(sizeof(aist_ast_T::var_def_T));
            aist_os_memset(ast->m_var_def, 0, sizeof(aist_ast_T::var_def_T));
        }break;
        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            ast->m_var_modif = (aist_ast_T::var_modif_T*)mem->Alloc(sizeof(aist_ast_T::var_modif_T));
            aist_os_memset(ast->m_var_modif, 0, sizeof(aist_ast_T::var_modif_T));
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            ast->m_ternary = (aist_ast_T::ternary_T*)mem->Alloc(sizeof(aist_ast_T::ternary_T));
            aist_os_memset(ast->m_ternary, 0, sizeof(aist_ast_T::ternary_T));
        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            ast->m_arr_access = (aist_ast_T::array_access_T*)mem->Alloc(sizeof(aist_ast_T::array_access_T));
            aist_os_memset(ast->m_arr_access, 0, sizeof(aist_ast_T::array_access_T));
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            ast->m_unop = (aist_ast_T::unop_T*)mem->Alloc(sizeof(aist_ast_T::unop_T));
            aist_os_memset(ast->m_unop, 0, sizeof(aist_ast_T::unop_T));
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            ast->m_attribute = (aist_ast_T::attr_access_T*)mem->Alloc(sizeof(aist_ast_T::attr_access_T));
            aist_os_memset(ast->m_attribute, 0, sizeof(aist_ast_T::attr_access_T));
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            ast->m_binop = (aist_ast_T::binop_T*)mem->Alloc(sizeof(aist_ast_T::binop_T));
            aist_os_memset(ast->m_binop, 0, sizeof(aist_ast_T::binop_T));
        }break;
        case AIST::AST_type::AST_RETURN:
        {
            ast->m_return = (aist_ast_T::return_T*)mem->Alloc(sizeof(aist_ast_T::return_T));
            aist_os_memset(ast->m_return, 0, sizeof(aist_ast_T::return_T));
        }break;
    };
    return ast;
}


char* aist_runtime_T::rt_str_clone(const char* value, size_t len)
{
    if(len == 0 && value)
        len = aist_os_strlen(value);
    char* ret = (char*)m_mem_rt->Alloc(len+1);
    aist_os_memcpy(ret, value, len);
    ret[len] = 0;
    return ret;
}


void aist_runtime_T::rt_free(void* ptr, size_t len)
{
    if(!ptr)
        return;
    m_mem_rt->Free(ptr, len);
}


void aist_runtime_T::ast_release(aist_ast_T* ast)
{
    if(ast == NULL)
        return;
#ifdef AIST_DEBUG_
    if(ast->m_countUse == 0)
        rt_error(NULL, ast->m_line_pos12, "Error ast_release  m_countUse == 0");
#endif
    if((ast->m_flags&AIST::FLAG_NOT_RELEASE) == 0)
        ast->m_countUse--;
    if(ast->m_countUse)
        return;
#ifdef AIST_DEBUG_
    gDbg_Free_num++;
#endif
    aist_mem_T* mem = m_mem_parser;
    if(ast->m_flags&AIST::FLAG_AST_MEM_RT)
        mem = m_mem_rt;
    switch(ast->m_ast_type)
    {
        case AIST::AST_type::AST_NULL:   break;
        case AIST::AST_type::AST_UCHAR:  break;
        case AIST::AST_type::AST_BOOLEAN:break;
        case AIST::AST_type::AST_INTEGER:break;
        case AIST::AST_type::AST_FLOAT:  break;
        case AIST::AST_type::AST_STRING:
        {
            if(ast->m_string_value_len)
            {
                size_t m = ast->m_flags&3;
                if(m == AIST::FLAG_STR_MEM_STOR)
                {
                    aist_str_free(mem, ast->m_string_value, ast->m_string_value_len);
                }else if(m == AIST::FLAG_STR_MEM_SYS)// системный аллокатор
                {
                    aist_os_free(ast->m_string_value);
                    ast->m_string_value = NULL;
                }else if(m == AIST::FLAG_STR_MEM_CONST) //константа
                {
                    ast->m_string_value = NULL;
                }else{
                    //FLAG_STR_MEM_USER
                }
            }
        }break;
        case AIST::AST_type::AST_OBJECT:
        {
            if((ast->m_flags&AIST::FLAG_AST_MEM_RT) != 0 && (ast->m_flags&AIST::FLAG_OBJECT_INIT) != 0)
            {
                aist_ast_T* fn_def = ast->m_object_children->find(AIST_LEX_DELETE);
                if(fn_def && fn_def->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
                {
                    aist_ast_list_T arguments;
                    aist_os_memset(&arguments, 0, sizeof(aist_ast_list_T));
                    function_call(fn_def->AddRef(), &arguments, ast, fn_def->m_line_pos12);
                }
            }
            list_free(ast->m_object_children);
        }break;
        case AIST::AST_type::AST_LIST:
        {
            list_free(ast->m_list_children);
        }break;
//AST_ANY
        case AIST::AST_type::AST_VARIABLE:
        {
#ifndef AIST_USE_HASH_NAME
            aist_str_free(mem, ast->m_variable_name);
#endif
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            ast_release(ast->m_var_def->m_value);
            ast_release(ast->m_var_def->m_init_value);
#ifndef AIST_USE_HASH_NAME
            char* t = (char*)ast->m_var_def->m_name;
            aist_str_free(mem, t);
#endif
            mem->Free(ast->m_var_def, sizeof(aist_ast_T::var_def_T));
        }break;

        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            ast_release(ast->m_var_modif->m_left);
            ast_release(ast->m_var_modif->m_right);
            mem->Free(ast->m_var_modif, sizeof(aist_ast_T::var_modif_T));

        }break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
            char* t;
#ifndef AIST_USE_HASH_NAME
            t = (char*)ast->m_fn_def->m_function_name;
            aist_str_free(mem, t);
#endif
            t = (char*)ast->m_fn_def->m_label_src;
            aist_str_free(mem, t);
            ast->m_fn_def->m_label_src = NULL;
            ast_release(ast->m_fn_def->m_body);
            list_free(ast->m_fn_def->m_arguments);
            mem->Free(ast->m_fn_def, sizeof(aist_ast_T::fn_def_T));

        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            ast_release(ast->m_fn_call->m_name);
            list_free(ast->m_fn_call->m_arguments);
            mem->Free(ast->m_fn_call, sizeof(aist_ast_T::fn_call_T));
        }break;
        case AIST::AST_type::AST_BLOCK:
        {
            list_free(ast->m_block_list);
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            ast_release(ast->m_binop->m_left);
            ast_release(ast->m_binop->m_right);
            mem->Free(ast->m_binop, sizeof(aist_ast_T::binop_T));
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            ast_release(ast->m_unop->m_right);
            mem->Free(ast->m_unop, sizeof(aist_ast_T::unop_T));
        }break;
        case AIST::AST_type::AST_CONTINUE:break;
        case AIST::AST_type::AST_BREAK:break;
        case AIST::AST_type::AST_RETURN:
        {
            ast_release(ast->m_return->m_value);
            ast_release(ast->m_return->m_init_value);
            mem->Free(ast->m_return, sizeof(aist_ast_T::return_T));
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            ast_release(ast->m_ternary->m_expr);
            ast_release(ast->m_ternary->m_body);
            ast_release(ast->m_ternary->m_else_body);
            mem->Free(ast->m_ternary, sizeof(aist_ast_T::ternary_T));
        }break;
        case AIST::AST_type::AST_IF:
        {
            ast_release(ast->m_if->m_if_expr);
            ast_release(ast->m_if->m_if_body);
            ast_release(ast->m_if->m_if_otherwise);
            mem->Free(ast->m_if, sizeof(aist_ast_T::if_T));
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            ast_release(ast->m_while->m_test_expr);
            ast_release(ast->m_while->m_body);
            mem->Free(ast->m_while, sizeof(aist_ast_T::while_T));
        }break;
        case AIST::AST_type::AST_FOR:
        {
            ast_release(ast->m_for->m_init_statement);
            ast_release(ast->m_for->m_test_expr);
            ast_release(ast->m_for->m_update_statement);
            ast_release(ast->m_for->m_body);
            mem->Free(ast->m_for, sizeof(aist_ast_T::for_T));
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            ast_release(ast->m_attribute->m_left);
            ast_release(ast->m_attribute->m_right);
            mem->Free(ast->m_attribute, sizeof(aist_ast_T::attr_access_T));
        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            ast_release(ast->m_arr_access->m_left);
            ast_release(ast->m_arr_access->m_right);
            ast_release(ast->m_arr_access->m_pointer);
            mem->Free(ast->m_arr_access, sizeof(aist_ast_T::array_access_T));
        }break;
        case AIST::AST_type::AST_NEW:
        case AIST::AST_type::AST_DELETE:
        case AIST::AST_type::AST_INCLUDE:
        {
            ast_release(ast->m_new_del_incl_right);
        }break;
#ifdef AIST_DEBUG_
        default: printf("WARNING free\n"); break;
#endif
    }
#ifdef AIST_DEBUG_
    aist_os_memset(ast, 0, sizeof(ast));
#endif
    mem->Free(ast, sizeof(aist_ast_T));
}


aist_ast_T* aist_runtime_T::rt_ast_copy(aist_ast_T* ast)
{
    if(!ast)
        return NULL;
    if(ast->m_ast_type == AIST::AST_type::AST_NULL)
        return m_ast_null;
    if(ast->m_ast_type == AIST::AST_type::AST_VOID)
        return m_ast_void;

    aist_mem_T* mem = m_mem_rt;
    aist_ast_T* a = ast_create(ast->m_ast_type, ast->m_flags | AIST::FLAG_AST_MEM_RT);
    a->m_line_pos12 = ast->m_line_pos12;

    switch(ast->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:  a->m_char_value = ast->m_char_value; break;
        case AIST::AST_type::AST_BOOLEAN:a->m_boolean_value = ast->m_boolean_value; break;
        case AIST::AST_type::AST_INTEGER:a->m_int_value   = ast->m_int_value; break;
        case AIST::AST_type::AST_FLOAT:  a->m_float_value = ast->m_float_value; break;
        case AIST::AST_type::AST_STRING:
        {
            if(ast->m_string_value_len)
                a->m_string_value = rt_str_clone(ast->m_string_value, ast->m_string_value_len);
            a->m_string_value_len = ast->m_string_value_len;
        }break;
        case AIST::AST_type::AST_OBJECT:
        {
            a->m_object_children->reserve(ast->m_object_children->m_ast_count);
            for(size_t i = 0; i < ast->m_object_children->m_ast_count; i++)
            {
                aist_ast_T* child_copy = rt_ast_copy(ast->m_object_children->m_ast_items[i]);
                a->m_object_children->push(child_copy);
            }
            if(m_object_copy_call_new)
            {
                aist_ast_T* fn_new = a->m_object_children->find(AIST_LEX_NEW);
                if(fn_new && fn_new->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
                {
                    aist_ast_list_T arguments;
                    aist_os_memset(&arguments, 0, sizeof(aist_ast_list_T));
                    function_call(fn_new->AddRef(), &arguments, a, fn_new->m_line_pos12);
                }
                a->m_flags |= AIST::FLAG_OBJECT_INIT;
            }
        }break;
        case AIST::AST_type::AST_LIST:
        {
            a->m_list_children->reserve(ast->m_list_children->m_ast_count);
            for(size_t i = 0; i < ast->m_list_children->m_ast_count; i++)
            {
                aist_ast_T* child_copy = rt_ast_copy(ast->m_list_children->m_ast_items[i]);
                a->m_list_children->push(child_copy);
            }
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            a->m_var_def->m_type  = ast->m_var_def->m_type;
            if((a->m_flags&AIST::FLAG_VAR_NOT_COPY) != 0)
            {
                a->m_var_def->m_value = rt_ast_create(ast->m_var_def->m_value->m_ast_type);
                a->m_var_def->m_value->m_flags |= AIST::FLAG_VAR_NOT_COPY;
            }else{
                a->m_var_def->m_value = rt_ast_copy(ast->m_var_def->m_value);
            }
            a->m_var_def->m_init_value = rt_ast_copy(ast->m_var_def->m_init_value);
#ifdef AIST_USE_HASH_NAME
            a->m_var_def->m_name_hash = ast->m_var_def->m_name_hash;
#else
            a->m_var_def->m_name = rt_str_clone(ast->m_var_def->m_name);
#endif
        }break;
        case AIST::AST_type::AST_VARIABLE:
        {
#ifdef AIST_USE_HASH_NAME
            a->m_variable_name_hash = ast->m_variable_name_hash;
#else
            a->m_variable_name = rt_str_clone(ast->m_variable_name);
#endif
        }break;
        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            a->m_var_modif->m_left  = rt_ast_copy(ast->m_var_modif->m_left);
            a->m_var_modif->m_right = rt_ast_copy(ast->m_var_modif->m_right);
        }break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
#ifdef AIST_USE_HASH_NAME
            a->m_fn_def->m_function_name_hash = ast->m_fn_def->m_function_name_hash;
#else
            a->m_fn_def->m_function_name = rt_str_clone(ast->m_fn_def->m_function_name);
#endif
            a->m_fn_def->m_label_src   = rt_str_clone(ast->m_fn_def->m_label_src);
            a->m_fn_def->m_return_type = ast->m_fn_def->m_return_type;
            a->m_fn_def->m_body        = rt_ast_copy(ast->m_fn_def->m_body);
            a->m_fn_def->m_arguments->reserve(ast->m_fn_def->m_arguments->m_ast_count);
            for(size_t i = 0; i < ast->m_fn_def->m_arguments->m_ast_count; i++)
            {
                aist_ast_T* child_copy = rt_ast_copy(ast->m_fn_def->m_arguments->m_ast_items[i]);
                a->m_fn_def->m_arguments->push(child_copy);
            }
        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            a->m_fn_call->m_name = rt_ast_copy(ast->m_fn_call->m_name);
            a->m_fn_call->m_arguments->reserve(ast->m_fn_call->m_arguments->m_ast_count);
            for(size_t i = 0; i < ast->m_fn_call->m_arguments->m_ast_count; i++)
            {
                aist_ast_T* child_copy = rt_ast_copy(ast->m_fn_call->m_arguments->m_ast_items[i]);
                a->m_fn_call->m_arguments->push(child_copy);
            }
        }break;
        case AIST::AST_type::AST_BLOCK:
        {
            a->m_block_list->reserve(ast->m_block_list->m_ast_count);
            for(size_t i = 0; i < ast->m_block_list->m_ast_count; i++)
            {
                aist_ast_T* child_copy = rt_ast_copy(ast->m_block_list->m_ast_items[i]);
                a->m_block_list->push(child_copy);
            }
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            a->m_binop->m_left  = rt_ast_copy(ast->m_binop->m_left);
            a->m_binop->m_right = rt_ast_copy(ast->m_binop->m_right);
            a->m_binop->m_operator = ast->m_binop->m_operator;
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            a->m_unop->m_operator = ast->m_unop->m_operator;
            a->m_unop->m_right    = rt_ast_copy(ast->m_unop->m_right);
        }break;
        case AIST::AST_type::AST_CONTINUE:break;
        case AIST::AST_type::AST_BREAK:break;
        case AIST::AST_type::AST_RETURN:
        {
            a->m_return->m_init_value = rt_ast_copy(ast->m_return->m_init_value);
            a->m_return->m_value      = rt_ast_copy(ast->m_return->m_value);
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            a->m_ternary->m_expr = rt_ast_copy(ast->m_ternary->m_expr);
            a->m_ternary->m_body = rt_ast_copy(ast->m_ternary->m_body);
            a->m_ternary->m_else_body = rt_ast_copy(ast->m_ternary->m_else_body);
        }break;
        case AIST::AST_type::AST_IF:
        {
            a->m_if->m_if_expr = rt_ast_copy(ast->m_if->m_if_expr);
            a->m_if->m_if_body = rt_ast_copy(ast->m_if->m_if_body);
            a->m_if->m_if_otherwise = rt_ast_copy(ast->m_if->m_if_otherwise);
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            a->m_while->m_test_expr = rt_ast_copy(ast->m_while->m_test_expr);
            a->m_while->m_body      = rt_ast_copy(ast->m_while->m_body);
        }break;
        case AIST::AST_type::AST_FOR:
        {
            a->m_for->m_init_statement   = rt_ast_copy(ast->m_for->m_init_statement);
            a->m_for->m_test_expr        = rt_ast_copy(ast->m_for->m_test_expr);
            a->m_for->m_update_statement = rt_ast_copy(ast->m_for->m_update_statement);
            a->m_for->m_body             = rt_ast_copy(ast->m_for->m_body);
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            a->m_attribute->m_left  = rt_ast_copy(ast->m_attribute->m_left);
            a->m_attribute->m_right = rt_ast_copy(ast->m_attribute->m_right);
        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            a->m_arr_access->m_left    = rt_ast_copy(ast->m_arr_access->m_left);
            a->m_arr_access->m_right   = rt_ast_copy(ast->m_arr_access->m_right);
            a->m_arr_access->m_pointer = rt_ast_copy(ast->m_arr_access->m_pointer);
        }break;
        case AIST::AST_type::AST_NEW:
        {
            a->m_new_del_incl_right = rt_ast_copy(ast->m_new_del_incl_right);
        }break;
        case AIST::AST_type::AST_DELETE:
        {
            a->m_new_del_incl_right = rt_ast_copy(ast->m_new_del_incl_right);
        }break;
        case AIST::AST_type::AST_INCLUDE:
        {
            a->m_new_del_incl_right = rt_ast_copy(ast->m_new_del_incl_right);
        }break;
#ifdef AIST_DEBUG_
        default: printf("WARNING copy\n"); return NULL; break;
#endif
    }
    return a;
}



void aist_rt_stack_T::Init(aist_runtime_T* rt)
{
    m_rt = rt;
    m_stack_pos = 0;
    m_stack_size_alloc = 4;
    m_stack = (stack_T*)m_rt->m_mem_rt->Alloc(sizeof(stack_T)*m_stack_size_alloc);
    for(size_t i = 0; i < m_stack_size_alloc; i++)
    {
        block_T* new_block = (block_T*)m_rt->m_mem_rt->Alloc(sizeof(block_T));
        aist_os_memset(new_block, 0, sizeof(block_T));
        new_block->m_var_def_list = rt->list_create(8, false);
        m_stack[i].m_base = m_stack[i].m_head = new_block;
    }
    m_global = m_stack[0].m_base;
    m_stack[0].m_label_src = NULL;
    m_this   = NULL;
    m_this_one = NULL;
    m_this_one_prev = NULL;
    m_fn_init_stack = NULL;
    m_level = 0;
}


void aist_rt_stack_T::CopyGlobal(const aist_rt_stack_T* src_stack)
{
    Reset(true);
    aist_ast_list_T* src_var_def = src_stack->m_global->m_var_def_list;
    aist_ast_list_T* dst_var_def = m_global->m_var_def_list;
    dst_var_def->reserve(src_var_def->m_ast_count+1);
    for(size_t i = 0; i < src_var_def->m_ast_count; i++)
    {
        dst_var_def->push(m_rt->rt_ast_copy(src_var_def->m_ast_items[i]));
    }
}


void aist_rt_stack_T::Reset(bool global)
{
    while(m_stack_pos)
        fn_end();
    while(m_stack[0].m_head != m_stack[0].m_base)
        level_down();
    if(global)
        clear_block(m_global);
}


void aist_rt_stack_T::clear_block(block_T* block)
{
    aist_ast_list_T* var = block->m_var_def_list;
    var->removeAll(m_rt);
}


void aist_rt_stack_T::level_up()
{
    if(m_stack[m_stack_pos].m_head->m_next == NULL)
    {
        block_T* new_block = (block_T*)m_rt->m_mem_rt->Alloc(sizeof(block_T));
        aist_os_memset(new_block, 0, sizeof(block_T));
        new_block->m_var_def_list = m_rt->list_create(8, false);
        m_stack[m_stack_pos].m_head->m_next = new_block;
        new_block->m_prev  = m_stack[m_stack_pos].m_head;
    }
    m_stack[m_stack_pos].m_head = m_stack[m_stack_pos].m_head->m_next;
    m_level++;
    if(AIST::MAX_RT_CALL_LEVEL < m_level)
        m_rt->rt_error(NULL, 0, "Error: too much nesting of calls %u > max %u", (int)m_level, AIST::MAX_RT_CALL_LEVEL);
}


void aist_rt_stack_T::level_down()
{
//    if(m_global == m_stack[m_stack_pos].m_head)
//        return;
    clear_block(m_stack[m_stack_pos].m_head);
    m_stack[m_stack_pos].m_head = m_stack[m_stack_pos].m_head->m_prev;
    if(!m_stack[m_stack_pos].m_head)
            m_rt->rt_error(NULL, 0, "Incorrect rt_stack level_down()");
    m_level--;
}


void aist_rt_stack_T::fn_end()
{
    if(!m_stack_pos)
        m_rt->rt_error(NULL, 0, "Incorrect rt_stack fn_end()");
    while(m_stack[m_stack_pos].m_head != m_stack[m_stack_pos].m_base)
        level_down();
    clear_block(m_stack[m_stack_pos].m_head);
    m_stack_pos--;
    m_level--;
}


void aist_rt_stack_T::fn_init()
{
    if(m_stack_pos+1 == m_stack_size_alloc)
    {
        m_stack_size_alloc += 4;
        stack_T* new_stack = (stack_T*)m_rt->m_mem_rt->Alloc(sizeof(stack_T)*m_stack_size_alloc);
        aist_os_memcpy(new_stack, m_stack, sizeof(stack_T)*(m_stack_pos+1));
        m_rt->m_mem_rt->Free(m_stack, sizeof(stack_T)*(m_stack_pos+1));
        m_stack = new_stack;
        for(size_t i = m_stack_pos+1; i < m_stack_size_alloc; i++)
        {
            block_T* new_block = (block_T*)m_rt->m_mem_rt->Alloc(sizeof(block_T));
            aist_os_memset(new_block, 0, sizeof(block_T));
            new_block->m_var_def_list = m_rt->list_create(8, false);
            m_stack[i].m_base = m_stack[i].m_head = new_block;
        }
    }
    m_stack[m_stack_pos+1].m_base = m_stack[m_stack_pos+1].m_head;
    m_fn_init_stack = m_stack[m_stack_pos+1].m_base;
}


void aist_rt_stack_T::fn_start(const char* label_src)
{
    m_stack_pos++;
    m_level++;
    m_fn_init_stack = NULL;
    m_stack[m_stack_pos].m_label_src = label_src;
    if(AIST::MAX_RT_CALL_LEVEL < m_level)
        m_rt->rt_error(NULL, 0, "Error: too much nesting of calls %u > max %u", (int)m_level, AIST::MAX_RT_CALL_LEVEL);
}


void aist_rt_stack_T::fn_push_variable(aist_ast_T* var)
{
    m_fn_init_stack->m_var_def_list->push(var);
}


void aist_rt_stack_T::push_variable(aist_ast_T* var)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T name_hash = 0;
    if(var->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
        name_hash = var->m_var_def->m_name_hash;
    if(var->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
        name_hash = var->m_fn_def->m_function_name_hash;
    if(!name_hash)
        return;
    aist_ast_T* var_def = m_stack[m_stack_pos].m_head->m_var_def_list->find(name_hash);
    if(var_def)
    {
        if(var_def->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
            m_rt->rt_error(var, var->m_line_pos12, "The function '" AIST_PR_NAME_HASH "' is already defined.", name_hash);
        m_rt->rt_error(var, var->m_line_pos12, "The variable '" AIST_PR_NAME_HASH "' is already defined.", name_hash);
    }
    m_stack[m_stack_pos].m_head->m_var_def_list->push(var);
#else
    const char* name = NULL;
    if(var->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
        name = var->m_var_def->m_name;
    if(var->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
        name = var->m_fn_def->m_function_name;
    if(!name || name[0] == '\0')
        return;
    aist_ast_T* var_def = m_stack[m_stack_pos].m_head->m_var_def_list->find(name);
    if(var_def)
    {
        if(var_def->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
            m_rt->rt_error(var, var->m_line_pos12, "The function '%s' is already defined.", name);
        m_rt->rt_error(var, var->m_line_pos12, "The variable '%s' is already defined.", name);
    }
    m_stack[m_stack_pos].m_head->m_var_def_list->push(var);
#endif
}


void aist_rt_stack_T::push_global_variable(aist_ast_T* var)
{
    m_global->m_var_def_list->push(var);
}


#ifdef AIST_USE_HASH_NAME
aist_ast_T* aist_rt_stack_T::find_global_variable(aist_hash_T name)
#else
aist_ast_T* aist_rt_stack_T::find_global_variable(const char* name)
#endif
{
    return m_global->m_var_def_list->find(name);
}


#ifdef AIST_USE_HASH_NAME
aist_ast_T* aist_rt_stack_T::find_variable(aist_hash_T name)
#else
aist_ast_T* aist_rt_stack_T::find_variable(const char* name)
#endif
{
    if(m_this_one)
    {
        aist_ast_T* var = m_this_one->m_object_children->find(name);
        m_this_one_prev = m_this_one;
        m_this_one = NULL;
        return var;
    }
    block_T* block;
    block = m_stack[m_stack_pos].m_head;
    while(block)
    {
        aist_ast_T* var = block->m_var_def_list->find(name);
        if(var != NULL)
            return var;
        block = block->m_prev;
    }
    if(m_this)
    {
#ifdef AIST_USE_HASH_NAME
        if(name == AIST_LEX_THIS)
            return m_this;
#else
        if(aist_os_strcmp(name, AIST_LEX_THIS) == 0)
            return m_this;
#endif
        aist_ast_T* var = m_this->m_object_children->find(name);
        if(var)
            return var;
    }
    if(m_stack_pos)//тк поиск уже был а 0м
        return m_global->m_var_def_list->find(name);
    return NULL;
}


aist_ast_T* aist_rt_stack_T::set_this(aist_ast_T* obj)
{
    if(obj)
    {
        if(obj->m_ast_type != AIST::AST_type::AST_OBJECT)
            return m_this;
    }
    aist_ast_T* prev = m_this;
    m_this = obj;
    return prev;
}


void aist_rt_stack_T::set_this_one(aist_ast_T* obj)
{
    m_this_one = obj;
}


const char* aist_rt_stack_T::get_active_label_src()
{
    return m_stack[m_stack_pos].m_label_src;
}


const char* aist_rt_stack_T::set_global_label_src(const char* new_label)
{
    const char* old_ret = m_stack[0].m_label_src;
    m_stack[0].m_label_src = new_label;
    return old_ret;
}


#ifdef AIST_USE_HASH_NAME
void aist_rt_stack_T::delete_def(aist_hash_T name)
#else
void aist_rt_stack_T::delete_def(const char* name)
#endif
{
    block_T* block = m_stack[m_stack_pos].m_head;
    while(block)
    {
        aist_ast_T* var = block->m_var_def_list->find(name);
        if(var != NULL)
        {
            block->m_var_def_list->remove(var, m_rt);
            return;
        }
        block = block->m_prev;
    }
    aist_ast_T* var = m_global->m_var_def_list->find(name);
    if(var != NULL)
        m_global->m_var_def_list->remove(var, m_rt);
}


