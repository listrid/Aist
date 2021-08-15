/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include "aist_runtime.h"

#ifdef AIST_USE_HASH_NAME
aist_ast_T* aist_builtins_sys_variable_find(aist_hash_T name);
aist_ast_T* aist_builtins_list_fn_find(aist_hash_T name);
aist_ast_T* aist_builtins_string_fn_find(aist_hash_T name);
#else
aist_ast_T* aist_builtins_sys_variable_find(const char* name);
aist_ast_T* aist_builtins_list_fn_find(const char* name);
aist_ast_T* aist_builtins_string_fn_find(const char* name);
#endif

void aist_builtins_init();


static bool aist_def_load_include_impl(const char* name, char*& outData, size_t& outDataLen)
{
    return false;
}


static void aist_def_free_include_impl(const char* name, char* outData, size_t outDataLen)
{
    if(outData)
        aist_os_free(outData);
}


void* aist_runtime_T::rt_malloc(size_t size)
{
    return m_mem_rt->Alloc(size);
}


bool aist_runtime_T::ast_to_str(aist_ast_T* ast, const char*& outStr, size_t& outLen)
{
    if(!ast)
        return false;
    if(ast->m_ast_type == AIST::AST_type::AST_STRING)
    {
        outStr = ast->m_string_value;
        outLen = ast->m_string_value_len;
        if(!outStr)
            outStr = "";
    }else if(ast->m_ast_type == AIST::AST_type::AST_INTEGER)
    {
        outStr = m_tmp_str;
        if(sizeof(ast->m_int_value) == 8)
        {
            outLen = aist_os_snprintf(m_tmp_str, 64, "%lli", (long long int)ast->m_int_value);
        }else{
            outLen = aist_os_snprintf(m_tmp_str, 64, "%i", (int)ast->m_int_value);
        }
    }else if(ast->m_ast_type == AIST::AST_type::AST_FLOAT)
    {
        outStr = m_tmp_str;
        outLen = aist_os_snprintf(m_tmp_str, 64, "%f", ast->m_float_value);
    }else if(ast->m_ast_type == AIST::AST_type::AST_BOOLEAN)
    {
        if(ast->m_boolean_value == 0)
        {
            outStr = "false";
            outLen = 5;
        }else{
            outStr = "true";
            outLen = 4;
        }
    }else if(ast->m_ast_type == AIST::AST_type::AST_UCHAR)
    {
        outStr = m_tmp_str;
        m_tmp_str[0] = ast->m_char_value;
        m_tmp_str[1] = 0;
        outLen = 1;
    }else{
        return false;
    }
    return true;
}


void aist_runtime_T::expect_args(size_t line_pos12, const aist_ast_list_T* in_args, int argc, ... )
{
    if((int)in_args->m_ast_count < argc)
    {
        rt_error(NULL, line_pos12, "Expected %u parameters, but transferred %u.", (int)in_args->m_ast_count, argc);
    }
    va_list arg_ptr;
    va_start(arg_ptr, argc);
    for(int i = 0; i < argc; i++)
    {
        AIST::AST_type t = va_arg(arg_ptr, AIST::AST_type);
        if(t == AIST::AST_type::AST_ANY)
            continue;
        aist_ast_T* ast = in_args->m_ast_items[i];
        if(ast->m_ast_type != t)
        {
            va_end(arg_ptr);
            rt_error(NULL, line_pos12, "Invalid function parameter type (%s => %s) position %u", aist_type_name(t), aist_type_name(ast->m_ast_type), i);
        }
    }
    va_end(arg_ptr);
}


aist_ast_T* aist_runtime_T::eval_variable(aist_ast_T* node)
{
    aist_ast_T* var_find = m_rt_stack->find_variable(AIST_NAME_HASH(node->m_variable_name));
    if(var_find == NULL)
    {
        var_find = aist_builtins_sys_variable_find(AIST_NAME_HASH(node->m_variable_name));
        if(!var_find)
            rt_error(NULL, node->m_line_pos12, "Undefined variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(node->m_variable_name));
    }
    if(var_find->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
        return var_find->AddRef();
    if(var_find->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
    {
        if(!var_find->m_var_def->m_value && var_find->m_var_def->m_init_value)//еще не инициирована
        {
            if(var_find->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
            {
                if((var_find->m_var_def->m_init_value->m_flags & AIST::FLAG_AST_MEM_RT) == 0)
                {
                    var_find->m_var_def->m_value = rt_ast_copy(var_find->m_var_def->m_init_value);
                }else{
                    var_find->m_var_def->m_value = var_find->m_var_def->m_init_value->AddRef();
                }
            }else{
                if(var_find->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_OBJECT)
                {
                    m_object_copy_call_new = true;
                    var_find->m_var_def->m_value = rt_ast_copy(var_find->m_var_def->m_init_value);
                    m_object_copy_call_new = false;
                }else{
                    var_find->m_var_def->m_value = eval(var_find->m_var_def->m_init_value);
                }
            }
        }
        if(var_find->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
            return var_find->m_var_def->m_value->AddRef();
        return eval(var_find->m_var_def->m_value);
    }
    return var_find->AddRef();// this
}


aist_ast_T* aist_runtime_T::eval_variable_definition(aist_ast_T* node)
{
    aist_ast_T* new_var = ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION, node->m_flags | AIST::FLAG_AST_MEM_RT);//копия ast из данных тк может меняться
#ifdef AIST_USE_HASH_NAME
    new_var->m_var_def->m_name_hash = node->m_var_def->m_name_hash;
#else
    new_var->m_var_def->m_name = aist_str_clone(m_mem_rt, node->m_var_def->m_name);
#endif
    new_var->m_var_def->m_type = node->m_var_def->m_type;
    if(node->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
    {
        if((node->m_var_def->m_init_value->m_flags & AIST::FLAG_AST_MEM_RT) == 0)
        {
            new_var->m_var_def->m_value = rt_ast_copy(node->m_var_def->m_init_value);
        }else{
            new_var->m_var_def->m_value = node->m_var_def->m_init_value->AddRef();
        }
    }else if(node->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_OBJECT)
    {
        m_object_copy_call_new = true;
        new_var->m_var_def->m_value = rt_ast_copy(node->m_var_def->m_init_value);
        m_object_copy_call_new = false;
    }else{
        new_var->m_var_def->m_value = eval(node->m_var_def->m_init_value);
    }
    if(new_var->m_var_def->m_type != AIST::AST_type::AST_ANY && new_var->m_var_def->m_type != new_var->m_var_def->m_value->m_ast_type)
       rt_error(new_var, node->m_line_pos12, "Invalid type for assigned value (%s = %s)", aist_type_name(new_var->m_var_def->m_type), aist_type_name(new_var->m_var_def->m_value->m_ast_type));
    m_rt_stack->push_variable(new_var);
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_variable_modifier(aist_ast_T* node)
{
    aist_ast_T* left = node->m_var_modif->m_left;
    aist_ast_T* obj = NULL;
    if((node->m_flags & AIST::FLAG_RESTORE_THIS) != 0)
        obj = m_rt_stack->get_this_one();
    aist_ast_T* var_def = m_rt_stack->find_variable(AIST_NAME_HASH(left->m_variable_name));
    m_rt_stack->set_this_one(obj);

    if(!var_def)
        rt_error(NULL, node->m_line_pos12, "Cant set undefined variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(left->m_variable_name));

    if((var_def->m_flags&AIST::FLAG_CONST_VAR) != 0)
        rt_error(NULL, node->m_line_pos12, "Cant modify const variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(left->m_variable_name));
    aist_ast_T* newVal;
    if(node->m_var_modif->m_right->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
    {
        if((node->m_var_modif->m_right->m_flags & AIST::FLAG_AST_MEM_RT) == 0)
        {
            newVal = rt_ast_copy(node->m_var_modif->m_right);
        }else{
            newVal = node->m_var_modif->m_right->AddRef();
        }
    }else{
        newVal = eval(node->m_var_modif->m_right);
    }

    if(var_def->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        rt_error(newVal, node->m_line_pos12, "Cannot be assigned values (%s (not variable) = %s)", aist_type_name(var_def->m_ast_type), aist_type_name(newVal->m_ast_type));
    if(var_def->m_var_def->m_type != AIST::AST_type::AST_ANY && var_def->m_var_def->m_type != newVal->m_ast_type)
        rt_error(newVal, node->m_line_pos12, "Invalid type for assigned value (%s = %s)", aist_type_name(var_def->m_var_def->m_type), aist_type_name(newVal->m_ast_type));
    ast_release(var_def->m_var_def->m_value);
    var_def->m_var_def->m_value = newVal;
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_function_definition(aist_ast_T* node)
{
    if((node->m_flags&AIST::FLAG_AST_MEM_RT) == 0)
    {
        node = rt_ast_copy(node);
    }else{
        node->AddRef();
    }
    m_rt_stack->push_variable(node);
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::function_call(aist_ast_T* fn_def, aist_ast_list_T* arguments, aist_ast_T* obj_this, unsigned int line_pos12)
{
    aist_ast_T* ret;
    if(fn_def->m_fn_def->m_fn_ptr)
    {// native function
        aist_ast_list_T* list_fn_args = list_create(arguments->m_ast_count, false);
        for(size_t x = 0; x < arguments->m_ast_count; x++)
        {
            aist_ast_T* ast_arg = arguments->m_ast_items[x];
            aist_ast_T* arg_val = NULL;

            if(ast_arg->m_ast_type == AIST::AST_type::AST_VARIABLE)
            {
                aist_ast_T* vdef = m_rt_stack->find_variable(AIST_NAME_HASH(ast_arg->m_variable_name));
                if(vdef)
                {
                    if(vdef->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
                    {
                        arg_val = vdef->m_var_def->m_value->AddRef();
                    }else{// AST_FUNCTION_DEFINITION, this
                        arg_val = vdef->AddRef();
                    }
                }else{
                    list_free(list_fn_args);
                    rt_error(fn_def, line_pos12, "Undefined variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(ast_arg->m_variable_name));
                }
            }else{
                arg_val = eval(ast_arg);
            }
            list_fn_args->push(arg_val);
        }
        if(!obj_this)
            obj_this = m_rt_stack->get_this();
        m_rt_stack->set_this_one(NULL);
        ret = fn_def->m_fn_def->m_fn_ptr(this, line_pos12, obj_this, list_fn_args);
        list_free(list_fn_args);
    }else{//========================= body ====================================
        if(fn_def->m_fn_def->m_body == NULL)
            rt_error(fn_def, line_pos12, "Undefined method " AIST_PR_NAME_HASH, AIST_NAME_HASH(fn_def->m_fn_def->m_function_name));
        if(arguments->m_ast_count != fn_def->m_fn_def->m_arguments->m_ast_count)
            rt_error(fn_def, line_pos12, AIST_PR_NAME_HASH" Expected %u arguments but found %u arguments", AIST_NAME_HASH(fn_def->m_fn_def->m_function_name), (int)fn_def->m_fn_def->m_arguments->m_ast_count, (int)arguments->m_ast_count);
        if(arguments->m_ast_count > fn_def->m_fn_def->m_arguments->m_ast_count)
            rt_error(fn_def, line_pos12, "Too many arguments");
        m_rt_stack->fn_init();
        for(size_t i = 0; i < arguments->m_ast_count; i++)
        {
            aist_ast_T* arg_value = arguments->m_ast_items[i];
            aist_ast_T* proto_arg = fn_def->m_fn_def->m_arguments->m_ast_items[i];
            aist_ast_T* new_variable_def = ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION, AIST::FLAG_AST_MEM_RT);
            new_variable_def->m_var_def->m_type = proto_arg->m_var_def->m_type;
#ifdef AIST_USE_HASH_NAME
            new_variable_def->m_var_def->m_name_hash = proto_arg->m_var_def->m_name_hash;
#else
            new_variable_def->m_var_def->m_name = aist_str_clone(m_mem_rt, proto_arg->m_var_def->m_name);
#endif
            new_variable_def->m_flags |= proto_arg->m_flags;

            if(arg_value->m_ast_type == AIST::AST_type::AST_VARIABLE)
            {
                aist_ast_T* find_var = m_rt_stack->find_variable(AIST_NAME_HASH(arg_value->m_variable_name));
                if(find_var)
                {
                    if(find_var->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
                    {
                        if(find_var->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION ||
                           find_var->m_var_def->m_value->m_ast_type < AIST::AST_type::AST_ANY)
                        {
                            new_variable_def->m_var_def->m_value = find_var->m_var_def->m_value->AddRef();
                        }else{
                            new_variable_def->m_var_def->m_value = eval(find_var);
                        }
                    }else{// AST_FUNCTION_DEFINITION, this
                        new_variable_def->m_var_def->m_value = find_var->AddRef();
                    }
                }else{
                    aist_ast_T* vdef = m_rt_stack->find_variable(AIST_NAME_HASH(arg_value->m_variable_name));
                    ast_release(new_variable_def);
                    m_rt_stack->fn_start(fn_def->m_fn_def->m_label_src);
                    rt_error(fn_def, line_pos12, "Undefined variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(arg_value->m_variable_name));
                }
            }else{
                new_variable_def->m_var_def->m_value = eval(arg_value);
            }
            if(new_variable_def->m_var_def->m_type != AIST::AST_type::AST_ANY && new_variable_def->m_var_def->m_type != new_variable_def->m_var_def->m_value->m_ast_type)
            {
                m_rt_stack->fn_start(fn_def->m_fn_def->m_label_src);
                ast_release(fn_def);
                rt_error(new_variable_def, line_pos12, "Invalid function parameter type (%s => %s) position %u",
                         aist_type_name(new_variable_def->m_var_def->m_value->m_ast_type), aist_type_name(new_variable_def->m_var_def->m_type), (int)i);
            }
            m_rt_stack->fn_push_variable(new_variable_def);
        }
        m_rt_stack->fn_start(fn_def->m_fn_def->m_label_src);
        aist_ast_T* tmp_this = NULL;
        if(obj_this)
            tmp_this = m_rt_stack->set_this(obj_this);
        ret = eval(fn_def->m_fn_def->m_body);
        if(obj_this)
            m_rt_stack->set_this(tmp_this);

        m_rt_stack->fn_end();
        AIST::AST_type test_ret_type = fn_def->m_fn_def->m_return_type;
        if(test_ret_type == AIST::AST_type::AST_VOID && ret->m_ast_type == AIST::AST_type::AST_NULL)
            test_ret_type = AIST::AST_type::AST_NULL;
        if(ret && (fn_def->m_fn_def->m_return_type != AIST::AST_type::AST_ANY) && (test_ret_type != ret->m_ast_type)) // test return
        {
            AIST::AST_type ret_ast_type = ret->m_ast_type;
            ast_release(ret);
            rt_error(fn_def, line_pos12, "Invalid function return type (%s => %s)", aist_type_name(ret_ast_type), aist_type_name(fn_def->m_fn_def->m_return_type));
        }
    }
    if(!ret)
        ret = m_ast_null;
    ast_release(fn_def);
    if(ret->m_ast_type == AIST::AST_type::AST_VOID)
        ret = m_ast_null;
    return ret;
}


aist_ast_T* aist_runtime_T::eval_function_call(aist_ast_T* node)
{
    aist_ast_T* obj_this = m_rt_stack->get_this_one();
    aist_ast_T* fn_def = eval(node->m_fn_call->m_name);
    if(fn_def->m_ast_type != AIST::AST_type::AST_FUNCTION_DEFINITION)
    {
        if(node->m_fn_call->m_name->m_ast_type == AIST::AST_type::AST_VARIABLE)
            rt_error(fn_def, node->m_line_pos12, "Undefined function '%s'. type is '%s'", node->m_fn_call->m_name->m_string_value, aist_type_name(fn_def->m_ast_type));
        rt_error(fn_def, node->m_line_pos12, "Undefined function. type is '%s'", aist_type_name(fn_def->m_ast_type));
    }
    return function_call(fn_def, node->m_fn_call->m_arguments, obj_this, node->m_line_pos12);
}


aist_ast_T* aist_runtime_T::eval_attribute_access(aist_ast_T* node)
{
    aist_ast_T* left = eval(node->m_attribute->m_left);
    aist_ast_T* right = node->m_attribute->m_right;
    if(left->m_ast_type == AIST::AST_type::AST_LIST || left->m_ast_type == AIST::AST_type::AST_STRING)
    {
        if(right->m_ast_type == AIST::AST_type::AST_VARIABLE)
        {
#ifdef AIST_USE_HASH_NAME
            if(right->m_variable_name_hash == AIST_LEX_SIZE ||
               right->m_variable_name_hash == AIST_LEX_LEN  ||
               right->m_variable_name_hash == AIST_LEX_COUNT )
#else
            if(aist_os_strcmp(right->m_variable_name, "size") == 0 ||
               aist_os_strcmp(right->m_variable_name, "len") == 0 ||
               aist_os_strcmp(right->m_variable_name, "count") == 0 )
#endif
            {
                aist_ast_T* int_ast = ast_create(AIST::AST_type::AST_INTEGER, AIST::FLAG_AST_MEM_RT);
                if(left->m_ast_type == AIST::AST_type::AST_LIST)
                {
                    int_ast->m_int_value = left->m_list_children->m_ast_count;
                }else if(left->m_ast_type == AIST::AST_type::AST_STRING)
                {
                    int_ast->m_int_value = left->m_string_value_len;
                }
                ast_release(left);
                return int_ast;
            }
        }
        if(right->m_ast_type == AIST::AST_type::AST_FUNCTION_CALL)
        {
            if(left->m_ast_type == AIST::AST_type::AST_LIST)
            {
                aist_ast_T* fn_def = aist_builtins_list_fn_find(AIST_NAME_HASH(right->m_fn_call->m_name->m_variable_name));
                if(fn_def)
                {
                    aist_ast_T* ret = function_call(fn_def->AddRef(), right->m_fn_call->m_arguments, left, node->m_line_pos12);
                    ast_release(left);
                    return ret;
                }
            }else{
                aist_ast_T* fn_def = aist_builtins_string_fn_find(AIST_NAME_HASH(right->m_fn_call->m_name->m_variable_name));
                if(fn_def)
                {
                    aist_ast_T* ret = function_call(fn_def->AddRef(), right->m_fn_call->m_arguments, left, node->m_line_pos12);
                    ast_release(left);
                    return ret;
                }
            }
        }
        rt_error(left, node->m_line_pos12, "Undefined attribute");
    }
    if(left->m_ast_type != AIST::AST_type::AST_OBJECT)
        rt_error(left, node->m_line_pos12, "Undefined attribute");
    if(right->m_ast_type == AIST::AST_type::AST_VARIABLE_MODIFIER ||
       right->m_ast_type == AIST::AST_type::AST_ATTRIBUTE_ACCESS  ||
       right->m_ast_type == AIST::AST_type::AST_FUNCTION_CALL     ||
       right->m_ast_type == AIST::AST_type::AST_VARIABLE 
       )
    {//чтение, запись
        m_rt_stack->set_this_one(left);
        right = eval(right);
        ast_release(left);
        return right;
    }
    if(right->m_ast_type == AIST::AST_type::AST_BINOP)
    {
        m_rt_stack->set_this_one(left);
        right = eval(right);
        ast_release(left);
        return right;
    }
    rt_error(left, node->m_line_pos12, "Incorrect attribute");
    return NULL;
}


aist_ast_T* aist_runtime_T::eval_array_access(aist_ast_T* node)
{
    aist_ast_T* left = eval(node->m_arr_access->m_left);
    if(node->m_arr_access->m_right) //присвоение по индексу
    {
        if(left->m_ast_type == AIST::AST_type::AST_LIST)
        {
            if((left->m_flags&AIST::FLAG_CONST_VAR) != 0)
                rt_error(left, node->m_line_pos12, "Cant modify const list");
            if(node->m_arr_access->m_right->m_ast_type == AIST::AST_type::AST_FUNCTION_CALL)
                rt_error(left, node->m_line_pos12, "Cannot call a function from list.");
            aist_ast_T* index = eval(node->m_arr_access->m_pointer);
            if(index->m_ast_type != AIST::AST_type::AST_INTEGER)
            {
                ast_release(index);
                rt_error(left, node->m_line_pos12, "Incorrect index. not integer.");
            }
            aist_int i = index->m_int_value;
            ast_release(index);
            if(i < 0 || size_t (i) >= left->m_list_children->m_ast_count)
                rt_error(left, node->m_line_pos12, "Access index %u. list length %u .", (int)i, (int)left->m_list_children->m_ast_count);
            aist_ast_T* right = eval(node->m_arr_access->m_right);
            ast_release(left->m_list_children->m_ast_items[i]);
            left->m_list_children->m_ast_items[i] = right;
            ast_release(left);
            return m_ast_void;
        }
        if(left->m_ast_type == AIST::AST_type::AST_OBJECT)
        {
            aist_ast_T* index = eval(node->m_arr_access->m_pointer);
            if(index->m_ast_type != AIST::AST_type::AST_STRING)
            {
                ast_release(index);
                rt_error(left, node->m_line_pos12, "Incorrect index. not string.");
            }
            char name[255];
            size_t len = index->m_string_value_len;
            if(len > 254)
                len = 254;
            aist_os_memcpy(name, index->m_string_value, len);
            name[len] = 0;
#ifdef AIST_USE_HASH_NAME
            aist_hash_T name_hash = aist_str_hash(name, len);
#endif
            ast_release(index);
            aist_ast_T* find_obj = NULL;
            aist_ast_list_T* object_children = left->m_object_children;
            for(size_t i = 0; i < object_children->m_ast_count; i++)
            {
                aist_ast_T* obj_child = object_children->m_ast_items[i];
#ifdef AIST_USE_HASH_NAME
                if(obj_child->m_var_def->m_name_hash == name_hash)
#else
                if(aist_os_strcmp(obj_child->m_var_def->m_name, name) == 0)
#endif
                {
                    find_obj = obj_child;
                    break;
                }
            }
            if(node->m_arr_access->m_right->m_ast_type == AIST::AST_type::AST_FUNCTION_CALL)
            {
                if(!find_obj || find_obj->m_ast_type != AIST::AST_type::AST_FUNCTION_DEFINITION)
                    rt_error(left, node->m_arr_access->m_right->m_line_pos12, "Can't call method '%s'", name);
                aist_ast_T* fn_call = node->m_arr_access->m_right;
#ifdef AIST_USE_HASH_NAME
                if(fn_call->m_fn_call->m_name->m_variable_name_hash == 0)
#else
                if(fn_call->m_fn_call->m_name->m_variable_name[0] == 0)
#endif
                {
                    size_t flags = fn_call->m_fn_call->m_name->m_flags;
                    ast_release(fn_call->m_fn_call->m_name);
                    fn_call->m_fn_call->m_name = ast_create(AIST::AST_type::AST_VARIABLE, flags);
#ifdef AIST_USE_HASH_NAME
                    if((flags&AIST::FLAG_AST_MEM_RT) == 0)
                    {
                        fn_call->m_fn_call->m_name->m_variable_name_hash = name_hash;
                    }else{
                        fn_call->m_fn_call->m_name->m_variable_name_hash = name_hash;
                    }
#else
                    if((flags&AIST::FLAG_AST_MEM_RT) == 0)
                    {
                        fn_call->m_fn_call->m_name->m_variable_name = aist_str_clone(m_mem_parser, name, len);
                    }else{
                        fn_call->m_fn_call->m_name->m_variable_name = aist_str_clone(m_mem_rt, name, len);
                    }
#endif
                }
                m_rt_stack->set_this_one(left);
                fn_call = eval(fn_call);
                ast_release(left);
                return fn_call;
            }
            if(!find_obj)// создать
            {
                if(node->m_arr_access->m_right->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
                {
                    aist_ast_T* right = rt_ast_copy(node->m_arr_access->m_right);
#ifdef AIST_USE_HASH_NAME
                    right->m_fn_def->m_function_name_hash = name_hash;
#else
                    char* t = (char*)right->m_fn_def->m_function_name;
                    rt_free(t, aist_os_strlen(t)+1);
                    right->m_fn_def->m_function_name = rt_str_clone(name, len);
#endif
                    object_children->push(right);
                }else{
                    aist_ast_T* right = eval(node->m_arr_access->m_right);
                    aist_ast_T* new_variable_def = ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION, AIST::FLAG_AST_MEM_RT);
                    new_variable_def->m_var_def->m_type = right->m_ast_type;
#ifdef AIST_USE_HASH_NAME
                    new_variable_def->m_var_def->m_name_hash = name_hash;
#else
                    new_variable_def->m_var_def->m_name = rt_str_clone(name, len);
#endif
                    new_variable_def->m_var_def->m_value = right;
                    object_children->push(new_variable_def);
                }
                ast_release(left);
                return m_ast_void;
            }
            if((find_obj->m_flags&AIST::FLAG_CONST_VAR) != 0)
                rt_error(left, find_obj->m_line_pos12, "Cant modify const variable '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(find_obj->m_var_def->m_name));
            if(find_obj->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION && node->m_arr_access->m_right->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
            {
                object_children->remove(find_obj, this);
                aist_ast_T* right = rt_ast_copy(node->m_arr_access->m_right);
#ifdef AIST_USE_HASH_NAME
                right->m_fn_def->m_function_name_hash = name_hash;
#else
                char* t = (char*)right->m_fn_def->m_function_name;
                rt_free(t, aist_os_strlen(t)+1);
                right->m_fn_def->m_function_name = rt_str_clone(name, len);
#endif
                object_children->push(right);
                ast_release(left);
                return m_ast_void;
            }
            if(find_obj->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
                rt_error(left, node->m_line_pos12, "Cannot be assigned values '%s' (not variable)", aist_type_name(find_obj->m_ast_type));

            aist_ast_T* right = eval(node->m_arr_access->m_right);
            if(find_obj->m_var_def->m_type != AIST::AST_type::AST_ANY && find_obj->m_var_def->m_type != right->m_ast_type)
            {
                ast_release(left);
                rt_error(right, node->m_line_pos12, "Invalid type for assigned value (%s = %s)", aist_type_name(find_obj->m_var_def->m_type), aist_type_name(right->m_ast_type));
            }
            ast_release(find_obj->m_var_def->m_value);
            find_obj->m_var_def->m_value = right;
            ast_release(left);
            return m_ast_void;
        }
        rt_error(left, node->m_line_pos12, "access.");
    }
// доступ к значению
    if(left->m_ast_type == AIST::AST_type::AST_LIST)
    {
        aist_ast_T* index = eval(node->m_arr_access->m_pointer);
        if(index->m_ast_type != AIST::AST_type::AST_INTEGER)
        {
            ast_release(index);
            rt_error(left, node->m_line_pos12, "Incorrect index. not integer.");
        }
        aist_int i = index->m_int_value;
        ast_release(index);
        if(i < 0 || size_t(i) >= left->m_list_children->m_ast_count)
            rt_error(left, node->m_line_pos12, "Access index %u. list length %u.", (int)i, (int)left->m_list_children->m_ast_count);
        aist_ast_T* ret = left->m_list_children->m_ast_items[i]->AddRef();
        ast_release(left);
        return ret;
    }
    if(left->m_ast_type == AIST::AST_type::AST_STRING)
    {
        aist_ast_T* index = eval(node->m_arr_access->m_pointer);
        if(index->m_ast_type != AIST::AST_type::AST_INTEGER)
        {
            ast_release(index);
            rt_error(left, node->m_line_pos12, "Incorrect index. not integer.");
        }
        size_t idx = (size_t)index->m_int_value;
        ast_release(index);
        if(idx >= left->m_string_value_len)
            rt_error(left, node->m_line_pos12, "Access index %u. string length %u.", (int)idx, (int)left->m_string_value_len);
        aist_ast_T* ast = ast_create(AIST::AST_type::AST_UCHAR, AIST::FLAG_AST_MEM_RT);
        ast->m_char_value = left->m_string_value[idx];
        ast_release(left);
        return ast;
    }
    if(left->m_ast_type == AIST::AST_type::AST_OBJECT)
    {
        aist_ast_T* index = eval(node->m_arr_access->m_pointer);
        if(index->m_ast_type != AIST::AST_type::AST_STRING)
        {
            ast_release(index);
            rt_error(left, node->m_line_pos12, "Incorrect index. not string.");
        }
        char name[255];
        size_t len = index->m_string_value_len;
        if(len > 254)
            len = 254;
        aist_os_memcpy(name, index->m_string_value, len);
        name[len] = 0;
#ifdef AIST_USE_HASH_NAME
        aist_hash_T name_hash = aist_str_hash(name, len);
#endif
        ast_release(index);
        aist_ast_list_T* object_children = left->m_object_children;
        for(size_t i = 0; i < object_children->m_ast_count; i++)
        {
            aist_ast_T* obj_child = object_children->m_ast_items[i];
#ifdef AIST_USE_HASH_NAME
            if(obj_child->m_var_def->m_name_hash == name_hash)
#else
            if(aist_os_strcmp(obj_child->m_var_def->m_name, name) == 0)
#endif
            {
                if(obj_child->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
                {
                    if(obj_child->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
                    {
                        obj_child = obj_child->m_var_def->m_value->AddRef();
                    }else{
                        obj_child = eval(obj_child->m_var_def->m_value);
                    }
                }else{
                    obj_child->AddRef();
                }
                ast_release(left);
                return obj_child;
            }
        }
        ast_release(left);
        return m_ast_null;
    }
    rt_error(left, node->m_line_pos12, "List_access left value is not iterable.");
    return NULL;
}


aist_ast_T* aist_runtime_T::eval_var_inc(aist_ast_T* node)
{
    if((node->m_flags&AIST::FLAG_AST_MEM_RT) == 0)
        return rt_ast_copy(node);
    return node->AddRef();
}


aist_ast_T* aist_runtime_T::eval_include(aist_ast_T* node)
{
    aist_ast_T* ast_incl = eval(node->m_new_del_incl_right);
    if(ast_incl->m_ast_type != AIST::AST_type::AST_STRING)
        rt_error(ast_incl, node->m_line_pos12, "Incorrect include parameter (not string)");
    //xx тест на рантайм включения


    char* src_data;
    size_t src_data_len = 0;
    if(!m_include_fn(ast_incl->m_string_value, src_data, src_data_len))
        rt_error(ast_incl, node->m_line_pos12, "Could not load data '%s'", ast_incl->m_string_value);
    m_rt_stack->m_level += 2;
    bool ok = rt_run(ast_incl->m_string_value, parse(ast_incl->m_string_value, src_data, src_data_len, 0));
    m_rt_stack->m_level -= 2;
    m_include_free_fn(ast_incl->m_string_value, src_data, src_data_len);
    ast_release(ast_incl);
    if(!ok)
        longjmp(*m_error_jmp_buf, 1);
    aist_ast_T* ret = m_return_ast;
    m_return_ast = NULL;
    return ret;

}


aist_ast_T* aist_runtime_T::eval(aist_ast_T* node)
{
    if(!node)
        return m_ast_null;
    switch(node->m_ast_type)
    {
        case AIST::AST_type::AST_NULL: return m_ast_null;
        case AIST::AST_type::AST_VOID: return m_ast_void;
    
        case AIST::AST_type::AST_UCHAR:   return eval_var_inc(node);
        case AIST::AST_type::AST_BOOLEAN: return eval_var_inc(node);
        case AIST::AST_type::AST_INTEGER: return eval_var_inc(node);
        case AIST::AST_type::AST_FLOAT:   return eval_var_inc(node);
        case AIST::AST_type::AST_STRING:  return eval_var_inc(node);
        case AIST::AST_type::AST_LIST:    return eval_var_inc(node);
        case AIST::AST_type::AST_OBJECT:  return eval_var_inc(node);

        case AIST::AST_type::AST_VARIABLE:            return eval_variable(node);
        case AIST::AST_type::AST_VARIABLE_DEFINITION: return eval_variable_definition(node);
        case AIST::AST_type::AST_VARIABLE_MODIFIER:   return eval_variable_modifier(node);
        case AIST::AST_type::AST_FUNCTION_DEFINITION: return eval_function_definition(node);
        case AIST::AST_type::AST_FUNCTION_CALL:       return eval_function_call(node);

        case AIST::AST_type::AST_ATTRIBUTE_ACCESS: return eval_attribute_access(node);
        case AIST::AST_type::AST_ARRAY_ACCESS:     return eval_array_access(node);

        case AIST::AST_type::AST_BLOCK:   return eval_block(node);
        case AIST::AST_type::AST_BINOP:   return eval_binop(node);
        case AIST::AST_type::AST_UNOP:    return eval_unary(node);
        case AIST::AST_type::AST_BREAK:   return eval_break(node);
        case AIST::AST_type::AST_CONTINUE:return eval_continue(node);
        case AIST::AST_type::AST_RETURN:  return eval_return(node);
        case AIST::AST_type::AST_IF:      return eval_if(node);
        case AIST::AST_type::AST_TERNARY: return eval_ternary(node);
        case AIST::AST_type::AST_WHILE:   return eval_while(node);
        case AIST::AST_type::AST_FOR:     return eval_for(node);
        case AIST::AST_type::AST_NEW:     return eval_new(node);
        case AIST::AST_type::AST_DELETE:  return eval_delete(node);
        case AIST::AST_type::AST_INCLUDE: rt_error(NULL, node->m_line_pos12, "Can only be included in the global context");
    }
    rt_error(NULL, node->m_line_pos12, "Uncaught statement %d", node->m_ast_type);
    return NULL;
}


aist_ast_T* aist_runtime_T::eval_main(aist_ast_T* node)
{
    if(node->m_ast_type != AIST::AST_type::AST_BLOCK)
        return eval(node);
    aist_ast_T* eval_ret = NULL;
    for(size_t i = 0; i < node->m_block_list->m_ast_count; i++)
    {
        aist_ast_T* ast = node->m_block_list->m_ast_items[i];
        if(ast->m_ast_type == AIST::AST_type::AST_INCLUDE)
        {
            ast_release(eval_ret);
            eval_ret = eval_include(ast);
            continue;
        }
        eval_ret = eval(ast);
        if(eval_ret->m_ast_type == AIST::AST_type::AST_RETURN)
        {
            if(eval_ret->m_return->m_value)
            {
                aist_ast_T* ret_val;
                if(eval_ret->m_return->m_init_value)
                {
                    ret_val = eval(eval_ret->m_return->m_init_value);
                }else{
                    ret_val = eval_ret->m_return->m_value->AddRef();
                }
                ast_release(eval_ret);
                return ret_val;
            }else{
                ast_release(eval_ret);
                return m_ast_void;
            }
        }
        if(i+1 == node->m_block_list->m_ast_count)
            break;
        ast_release(eval_ret);
    }
    return eval_ret;
}


aist_ast_T* aist_runtime_T::rt_find_var(const char* name)
{
#ifdef AIST_USE_HASH_NAME
    return m_rt_stack->find_variable(aist_str_hash(name, aist_os_strlen(name)));
#else
    return m_rt_stack->find_variable(name);
#endif
}


void aist_runtime_T::rt_error(aist_ast_T* release, size_t line_pos12, const char* str, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, str);

    m_error.m_isParser = false;
    m_error.m_pos  = line_pos12&0xFFF;
    m_error.m_line = line_pos12 >> 12;
    m_error.m_label_src = rt_str_clone(m_rt_stack->get_active_label_src());
    aist_os_vsnprintf(m_error.m_errorStr, 127, str, arg_ptr);
    va_end(arg_ptr);
    if(release)
        ast_release(release);
    longjmp(*m_error_jmp_buf, 1);
}


aist_runtime_T* aist_runtime_T::create(aist_mem_T* mem_rt)
{
    aist_builtins_init();

    aist_runtime_T* runtime = (aist_runtime_T*)mem_rt->Alloc(sizeof(aist_runtime_T));
    aist_os_memset(runtime, 0, sizeof(aist_runtime_T));
    runtime->m_mem_rt = mem_rt;

    runtime->m_rt_stack = (aist_rt_stack_T*)runtime->rt_malloc(sizeof(aist_rt_stack_T));
    runtime->m_rt_stack->Init(runtime);

    runtime->m_ast_null = runtime->ast_create(AIST::AST_type::AST_NULL, AIST::FLAG_AST_MEM_RT | AIST::FLAG_NOT_RELEASE);
    runtime->m_ast_void = runtime->ast_create(AIST::AST_type::AST_VOID, AIST::FLAG_AST_MEM_RT | AIST::FLAG_NOT_RELEASE);

    runtime->m_include_fn      = aist_def_load_include_impl;
    runtime->m_include_free_fn = aist_def_free_include_impl;
    return runtime;
}



aist_mem_T* aist_runtime_T::destroy(aist_runtime_T* rt)
{
    rt->m_rt_stack->Reset(true);
    if(rt->m_mem_parser)
    {
        rt->m_mem_parser->FreeStorage();
        delete rt->m_mem_parser;
    }
    return rt->m_mem_rt;
}


void aist_runtime_T::rt_clear_parser()
{
    if(!m_parser)
    {
        if(!m_mem_parser)
        {
            m_mem_parser = new aist_mem_T();
            m_mem_parser->Init(0x4000);//16 kb
        }
    }else{
        if(m_parser->m_result_ast)
            ast_release(m_parser->m_result_ast);
        m_mem_parser->Clear();
    }
    m_error_jmp_buf = NULL;
    m_parser = (aist_parser_T*)m_mem_parser->Alloc(sizeof(aist_parser_T));
}


const aist_ast_T* aist_runtime_T::parse(const char* label_src, const char* src_code, size_t src_len, size_t shift_line_num)
{
    m_error.m_errorStr[0] = 0;
    if(m_error.m_label_src)
    {
        char* t = (char*)m_error.m_label_src;
        rt_free(t, aist_os_strlen(t)+1);
        m_error.m_label_src = NULL;
    }
    if(!m_parser)
        rt_clear_parser();
    if(!m_parser->parse(this, label_src, src_code, src_len, shift_line_num))
    {
        size_t line_pos12 = m_parser->get_errorPos();
        m_error.m_isParser = true;
        m_error.m_pos  = line_pos12&0xFFF;
        m_error.m_line = line_pos12 >> 12;
        m_error.m_label_src = rt_str_clone(label_src);
        size_t len = aist_os_strlen(m_parser->get_error());
        if(len > sizeof(m_error.m_errorStr)-1)
            len = sizeof(m_error.m_errorStr)-1;
        aist_os_memcpy(m_error.m_errorStr, m_parser->get_error(), len);
        m_error.m_errorStr[len] = 0;
        return NULL;
    }
    return m_parser->m_result_ast;
}


bool aist_runtime_T::rt_run(const char* label_src, const aist_ast_T* ast)
{
    if(!ast)
        return false;
    jmp_buf error_jmp_buf;
    jmp_buf* old_error_jmp_buf = m_error_jmp_buf;
    const char* old_label_src = m_rt_stack->set_global_label_src(label_src);
    m_error_jmp_buf = &error_jmp_buf;
    if(setjmp(*m_error_jmp_buf))
    {
        if(m_return_ast)
            ast_release(m_return_ast);
        m_return_ast = NULL;
        m_error_jmp_buf = old_error_jmp_buf;
        return false;
    }
    m_rt_stack->Reset(false);
    ast_release(m_return_ast);
    m_return_ast = NULL; // при ошибке не занулилось и вызовется еще раз ast_release
    m_return_ast = eval_main((aist_ast_T*)ast);
    m_error_jmp_buf = old_error_jmp_buf;
    m_rt_stack->set_global_label_src(old_label_src);
    return true;
}

