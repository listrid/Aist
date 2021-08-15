/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include "aist_runtime.h"
#include "aist.h"
#include <string.h>


aist_T* aist_create()
{
    aist_mem_T* mem_rt = new aist_mem_T();
    mem_rt->Init(AIST::MAX_RT_STRING_SIZE);
    aist_T* new_aist = (aist_T*)aist_os_malloc(sizeof(aist_T));
    new_aist->m_runtime = aist_runtime_T::create(mem_rt);
    return new_aist;
}


void aist_destroy(aist_T*& runtime)
{
    if(!runtime)
        return;
    if(runtime->m_runtime)
    {
        aist_mem_T* mem_rt = aist_runtime_T::destroy(runtime->m_runtime);
        mem_rt->FreeStorage();
        delete mem_rt;
    }
    free(runtime);
    runtime = NULL;
}


bool aist_clone(const aist_T* src_rt, aist_T* dst_rt)
{
    if(src_rt == NULL || dst_rt == NULL)
        return false;
    if(src_rt->m_runtime == NULL || dst_rt->m_runtime == NULL)
        return false;
    dst_rt->rt_reset(src_rt->m_runtime);
    return true;
}


bool aist_T::run(const char* aist_code, size_t len, const char* label_src, size_t shift_line_num)
{
    if(!label_src)
        label_src = "main";
    m_runtime->rt_clear_parser();
    const  aist_ast_T* ast = m_runtime->parse(label_src, aist_code, len, shift_line_num);
//aist_ast_dump("dump_ast.txt", ast);

    return m_runtime->rt_run(label_src, ast);
}


void aist_T::rt_reset(aist_runtime_T* src)
{
    if(!m_runtime)
        return;
    aist_mem_T* mem_parser = m_runtime->m_mem_parser;
    m_runtime->m_mem_parser = NULL;

    aist_def_load_include include_fn = m_runtime->m_include_fn;
    aist_def_free_include include_free_fn = m_runtime->m_include_free_fn;
    if(src)
    {
        include_fn = src->m_include_fn;
        include_free_fn = src->m_include_free_fn;
    }
    aist_mem_T* mem_rt = aist_runtime_T::destroy(m_runtime);
    mem_rt->Clear();
    m_runtime = aist_runtime_T::create(mem_rt);
    if(mem_parser)
        mem_parser->Clear();
    m_runtime->m_mem_parser = mem_parser;
    if(src)
    {
        m_runtime->m_rt_stack->CopyGlobal(src->m_rt_stack);
    }
    set_include_callback(include_fn, include_free_fn);
}


void aist_T::set_include_callback(aist_def_load_include loadFn, aist_def_free_include freeFn)
{
    if(loadFn)
        m_runtime->m_include_fn = loadFn;
    if(freeFn)
        m_runtime->m_include_free_fn = freeFn;
}


void aist_T::reset()
{
    rt_reset(NULL);
}


const aist_T::error_T* aist_T::get_error()
{
    if(m_runtime->m_error.m_label_src)
        return &m_runtime->m_error;
    return NULL;
}


void aist_T::set_user_data(void* data, size_t index)
{
    if(index < sizeof(m_runtime->m_user_data)/sizeof(m_runtime->m_user_data[0]))
        m_runtime->m_user_data[index] = data;
}


aist_ast_T* aist_T::create_var(const char* name, size_t data_type)
{
    if(data_type <= AIST::AST_type::AST_VOID || data_type > AIST::AST_type::AST_STRING)//not  object, list, fn_def, any
        return NULL;
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* vdef = m_runtime->m_rt_stack->find_global_variable(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* vdef = m_runtime->m_rt_stack->find_global_variable(name);
#endif
    if(vdef)
    {
        if(vdef->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
            return NULL;
        if(vdef->m_var_def->m_type != (AIST::AST_type)data_type)
            return NULL;
        m_runtime->ast_release(vdef->m_var_def->m_value);
        vdef->m_var_def->m_value = m_runtime->rt_ast_create((AIST::AST_type)data_type);
    } else{
        for(size_t i = 0; name[i]; i++)
        {
            if(name[i] == '.' || name[i] == '[' || name[i] == ']')
                return NULL;
        }
        vdef = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        vdef->m_var_def->m_name_hash = aist_str_hash(name, aist_os_strlen(name));
#else
        vdef->m_var_def->m_name  = m_runtime->rt_str_clone(name);
#endif
        vdef->m_var_def->m_type  = (AIST::AST_type)data_type;
        vdef->m_var_def->m_value = m_runtime->rt_ast_create((AIST::AST_type)data_type);
        m_runtime->m_rt_stack->push_global_variable(vdef);
    }
    return vdef;
}


bool aist_T::set_flag_const(const char* path_name, bool isConst)
{
    aist_ast_T* vdef = this->find_global_path_var(path_name, 0, false);
    if(!vdef)
        return false;
    if(vdef->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION &&
       vdef->m_ast_type != AIST::AST_type::AST_LIST &&
       vdef->m_ast_type != AIST::AST_type::AST_OBJECT)
        return false;
    if(isConst)
    {
        vdef->m_flags |= AIST::FLAG_CONST_VAR;
        if(vdef->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
            vdef->m_var_def->m_value->m_flags |= AIST::FLAG_CONST_VAR;
    }else{
        vdef->m_flags &= ~AIST::FLAG_CONST_VAR;
        if(vdef->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION)
            vdef->m_var_def->m_value->m_flags &= ~AIST::FLAG_CONST_VAR;
    }
    return true;
}


bool aist_T::register_function(const char* global_fname, aist_function_bind fptr)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(global_fname, aist_os_strlen(global_fname));
    if(m_runtime->m_rt_stack->find_variable(hash))
        return false;
    aist_ast_T* fdef = m_runtime->ast_create(AIST::AST_type::AST_FUNCTION_DEFINITION, AIST::FLAG_AST_MEM_RT);
    fdef->m_fn_def->m_function_name_hash = hash;
#else
    if(m_runtime->m_rt_stack->find_variable(global_fname))
        return false;
    aist_ast_T* fdef = m_runtime->ast_create(AIST::AST_type::AST_FUNCTION_DEFINITION, AIST::FLAG_AST_MEM_RT);
    fdef->m_fn_def->m_function_name = aist_str_clone(m_runtime->m_mem_rt, global_fname);
#endif
    fdef->m_fn_def->m_fn_ptr = fptr;
    m_runtime->m_rt_stack->push_global_variable(fdef);
    return true;
}


bool aist_T::set_string(const char* global_name, const char* val, size_t len)
{
    aist_ast_T* ast = create_var(global_name, AIST::AST_type::AST_STRING);
    if(!ast)
        return false;
    if(!len)
        len = aist_os_strlen(val);
    ast->m_var_def->m_value->m_string_value_len = (unsigned int)len;
    ast->m_var_def->m_value->m_string_value = m_runtime->rt_str_clone(val, len);
    return true;
}


bool aist_T::set_int(const char* global_name, aist_int val)
{
    aist_ast_T* ast = create_var(global_name, AIST::AST_type::AST_INTEGER);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_int_value = val;
    return true;
}


bool aist_T::set_float(const char* global_name, aist_float val)
{
    aist_ast_T* ast = create_var(global_name, AIST::AST_type::AST_FLOAT);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_float_value = val;
    return true;
}


bool aist_T::set_bool(const char* global_name, bool val)
{
    aist_ast_T* ast = create_var(global_name, AIST::AST_type::AST_BOOLEAN);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_boolean_value = val;
    return true;
}


bool aist_T::set_char(const  char* global_name, char val)
{
    aist_ast_T* ast = create_var(global_name, AIST::AST_type::AST_UCHAR);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_char_value = val;
    return true;
}


aist_ast_T* aist_T::find_global_path_var(const char* name, size_t len, bool toValue)
{
    aist_ast_T* ret = NULL;
    size_t prev_pos = 0;
    char lex[AIST::MAX_VARIABLE_NAME_SIZE + 2];
    size_t lex_len = 0;
    for(size_t pos = 0;;pos++)
    {
        lex[lex_len++] = name[pos];
        if(lex_len > AIST::MAX_VARIABLE_NAME_SIZE)
            return NULL;
        if(name[pos] != 0 && name[pos] != '.'&& name[pos] != '['&& name[pos] != ']')
            continue;
        if(lex_len < 2)
        {
            if(lex[0] != 0)
                return NULL;
            break;
        }
        size_t len = lex_len -1;
        lex_len = 0;
        lex[len] = 0;
        if(!ret)
        {
#ifdef AIST_USE_HASH_NAME
            ret = m_runtime->m_rt_stack->find_global_variable(aist_str_hash(lex, len));
#else
            ret = m_runtime->m_rt_stack->find_global_variable(lex);
#endif
            if(!ret)
                return NULL;
            continue;
        }
        if(ret->m_ast_type == AIST::AST_type::AST_OBJECT)
        {
#ifdef AIST_USE_HASH_NAME
            ret = ret->m_object_children->find(aist_str_hash(lex, len));
#else
            ret = ret->m_object_children->find(lex);
#endif
            if(!ret)
                return NULL;
            if(name[pos] == 0)
                break;
            continue;
        }
        if(ret->m_ast_type == AIST::AST_type::AST_LIST)
        {
            size_t index = 0;
            for(size_t i = 0; lex[i]; i++)
            {
                if(lex[i] == ' ')
                    continue;
                if(lex[i] < '0' || lex[i] > '9')
                    return NULL;
                index = index*10 + lex[i]-'0';
            }
            aist_ast_list_T* l = ret->m_list_children;
            if(l->m_ast_count <= index)
                return NULL;
            ret = l->m_ast_items[index];
            if(!ret)
                return NULL;
            if(name[pos+2] != 0 && (name[pos+1] == '.' || name[pos+1] == '['))
                pos++;
            continue;
        }
        if(ret->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
            return NULL;
        if(!ret->m_var_def->m_value ||
           ret->m_var_def->m_value->m_ast_type != AIST::AST_type::AST_OBJECT && 
           ret->m_var_def->m_value->m_ast_type != AIST::AST_type::AST_LIST)
            return NULL;
        if(ret->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_OBJECT)
        {
#ifdef AIST_USE_HASH_NAME
            ret = ret->m_var_def->m_value->m_object_children->find(aist_str_hash(lex, len));
#else
            ret = ret->m_var_def->m_value->m_object_children->find(lex);
#endif
        }else{//list
            size_t index = 0;
            for(size_t i = 0; lex[i]; i++)
            {
                if(lex[i] == ' ')
                    continue;
                if(lex[i] < '0' || lex[i] > '9')
                    return NULL;
                index = index*10 + lex[i]-'0';
            }
            aist_ast_list_T* l = ret->m_var_def->m_value->m_list_children;
            if(l->m_ast_count <= index)
                return NULL;
            ret = l->m_ast_items[index];
            if(name[pos+2] != 0 && (name[pos+1] == '.' || name[pos+1] == '['))
                pos++;
        }
        if(!ret)
            return NULL;
    }
    if(!ret)
        return NULL;
    if(ret->m_ast_type == AIST::AST_type::AST_VARIABLE_DEFINITION && toValue)
    {
        if(ret->m_var_def->m_value)
            return ret->m_var_def->m_value;
        return ret->m_var_def->m_init_value;
    }
    return ret;
}


bool aist_T::get_string(const char* path_name, const char*& val, size_t* outLen)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value || value->m_ast_type != AIST::AST_type::AST_STRING)
        return false;
    val = value->m_string_value;
    if(outLen)
        *outLen = value->m_string_value_len;
    return true;
}


bool aist_T::get_int(const char* path_name, aist_int & val)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value || value->m_ast_type != AIST::AST_type::AST_INTEGER)
        return false;
    val = value->m_int_value;
    return true;
}


bool aist_T::get_float(const char* path_name, aist_float& val)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value || value->m_ast_type != AIST::AST_type::AST_FLOAT)
        return false;
    val = value->m_float_value;
    return true;
}


bool aist_T::get_bool(const char* path_name, bool& val)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value || value->m_ast_type != AIST::AST_type::AST_BOOLEAN)
        return false;
    val = value->m_boolean_value != 0;
    return true;
}


bool aist_T::get_char(const char* path_name, char& val)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value || value->m_ast_type != AIST::AST_type::AST_UCHAR)
        return false;
    val = value->m_char_value;
    return true;
}


bool aist_T::get_object(const char* path_name, aist_object_T*& obj, bool create_if_none)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value && create_if_none)
    {
        for(size_t i = 0; path_name[i]; i++)
        {
            if(path_name[i] == '.' || path_name[i] == '[' || path_name[i] == ']')
                return false;
        }
        aist_ast_T* ast = m_runtime->ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION, AIST::FLAG_AST_MEM_RT);
#ifdef AIST_USE_HASH_NAME
        ast->m_var_def->m_name_hash = aist_str_hash(path_name, aist_os_strlen(path_name));
#else
        ast->m_var_def->m_name = m_runtime->rt_str_clone(path_name);
#endif
        value = m_runtime->ast_create(AIST::AST_type::AST_OBJECT, AIST::FLAG_AST_MEM_RT);
        ast->m_var_def->m_type  = AIST::AST_type::AST_OBJECT;
        ast->m_var_def->m_value = value;
        m_runtime->m_rt_stack->push_global_variable(ast);
    }
    if(!value || value->m_ast_type != AIST::AST_type::AST_OBJECT)
        return false;
    aist_object_T* new_obj = (aist_object_T*)m_runtime->rt_malloc(sizeof(aist_object_T));
    new_obj->m_runtime = m_runtime;
    new_obj->m_object  = value->AddRef();
    new_obj->m_cRef    = 1;
    obj = new_obj;
    return true;
}


bool aist_T::get_list(const char* path_name, aist_list_T*& list, bool create_if_none)
{
    aist_ast_T* value = this->find_global_path_var(path_name, 0, true);
    if(!value && create_if_none)
    {
        for(size_t i = 0; path_name[i]; i++)
        {
            if(path_name[i] == '.' || path_name[i] == '[' || path_name[i] == ']')
                return false;
        }
        aist_ast_T* ast = m_runtime->ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION, AIST::FLAG_AST_MEM_RT);
#ifdef AIST_USE_HASH_NAME
        ast->m_var_def->m_name_hash = aist_str_hash(path_name, aist_os_strlen(path_name));
#else
        ast->m_var_def->m_name = m_runtime->rt_str_clone(path_name);
#endif
        value = m_runtime->ast_create(AIST::AST_type::AST_LIST, AIST::FLAG_AST_MEM_RT);
        ast->m_var_def->m_type  = AIST::AST_type::AST_LIST;
        ast->m_var_def->m_value = value;
        m_runtime->m_rt_stack->push_global_variable(ast);
    }
    if(!value || value->m_ast_type != AIST::AST_type::AST_LIST)
        return false;
    aist_list_T* new_list = (aist_list_T*)m_runtime->rt_malloc(sizeof(aist_list_T));
    new_list->m_runtime = m_runtime;
    new_list->m_list    = value->AddRef();
    new_list->m_cRef    = 1;
    list = new_list;
    return true;
}


const char* aist_T::get_return(size_t* outLen)
{
    size_t t;
    if(!outLen)
        outLen = &t;
    *outLen = 0;
    if(m_runtime->m_error.m_label_src != NULL || m_runtime->m_return_ast == NULL)
        return "";
    const char* ret;
    if(!m_runtime->ast_to_str(m_runtime->m_return_ast, ret, *outLen))
        return "";
    return ret;
}


//===========================================================================================================
//============================================== aist_object_T ==============================================
//===========================================================================================================


size_t aist_object_T::AddRef()
{
    return ++m_cRef;
}


size_t aist_object_T::Release()
{
    if(--m_cRef == 0)
    {
        if(m_runtime)
            m_runtime->ast_release(m_object);
        aist_runtime_T* rt = m_runtime;
        m_runtime = NULL;
        rt->rt_free(this, sizeof(this));
        return 0;
    }
    return m_cRef;
}


bool aist_object_T::set_flag_const(const char* name, bool isConst)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* vdef = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* vdef = m_object->m_object_children->find(name);
#endif
    if(!vdef || vdef->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(isConst)
    {
        vdef->m_flags |= AIST::FLAG_CONST_VAR;
        vdef->m_var_def->m_value->m_flags |= AIST::FLAG_CONST_VAR;
    }else{
        vdef->m_flags &= ~AIST::FLAG_CONST_VAR;
        vdef->m_var_def->m_value->m_flags &= ~AIST::FLAG_CONST_VAR;
    }
    return true;
}


bool aist_object_T::set_flag_no_copy(const char* name, bool noCopy)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* vdef = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* vdef = m_object->m_object_children->find(name);
#endif
    if(!vdef || vdef->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(noCopy)
    {
        vdef->m_flags |= AIST::FLAG_VAR_NOT_COPY;
        vdef->m_var_def->m_value->m_flags |= AIST::FLAG_VAR_NOT_COPY;
    } else{
        vdef->m_flags &= ~AIST::FLAG_VAR_NOT_COPY;
        vdef->m_var_def->m_value->m_flags &= ~AIST::FLAG_VAR_NOT_COPY;
    }
    return true;
}


aist_ast_T* aist_object_T::find_create_variable(const char* name, size_t data_type)
{
    if(data_type <= AIST::AST_type::AST_VOID || data_type > AIST::AST_type::AST_STRING)//not  object, list, fn_def, any
        return NULL;
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    aist_ast_T* find_var = m_object->m_object_children->find(hash);
#else
    aist_ast_T* find_var = m_object->m_object_children->find(name);
#endif
    if(find_var == NULL)
    {
        for(size_t i = 0; name[i]; i++)
        {
            if(name[i] == '.' || name[i] == '[' || name[i] == ']')
                return NULL;
        }
        find_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        find_var->m_var_def->m_name_hash = hash;
#else
        find_var->m_var_def->m_name  = m_runtime->rt_str_clone(name);
#endif
        find_var->m_var_def->m_type  = (AIST::AST_type)data_type;
        find_var->m_var_def->m_value = m_runtime->rt_ast_create((AIST::AST_type)data_type);
        m_object->m_object_children->push(find_var);
    }else{
        if(find_var->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
            return NULL;
        if(find_var->m_var_def->m_type != data_type)
            return NULL;
        m_runtime->ast_release(find_var->m_var_def->m_value);
        find_var->m_var_def->m_value = m_runtime->rt_ast_create((AIST::AST_type)data_type);
    }
    return find_var;
}


bool aist_object_T::set_string(const char* name, char* val, size_t len)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = find_create_variable(name, AIST::AST_type::AST_STRING);
    if(!ast)
        return false;
    if(!len)
        len = aist_os_strlen(val);
    ast->m_var_def->m_value->m_string_value_len = (unsigned int)len;
    ast->m_var_def->m_value->m_string_value = m_runtime->rt_str_clone(val, len);
    return true;
}


bool aist_object_T::set_int(const char* name, aist_int val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = find_create_variable(name, AIST::AST_type::AST_INTEGER);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_int_value = val;
    return true;
}


bool aist_object_T::set_float(const char* name, aist_float val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = find_create_variable(name, AIST::AST_type::AST_FLOAT);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_float_value = val;
    return true;
}


bool aist_object_T::set_bool(const char* name, bool val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = find_create_variable(name, AIST::AST_type::AST_BOOLEAN);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_boolean_value = val;
    return true;
}


bool aist_object_T::set_char(const char* name, char val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = find_create_variable(name, AIST::AST_type::AST_UCHAR);
    if(!ast)
        return false;
    ast->m_var_def->m_value->m_char_value = val;
    return true;
}


bool aist_object_T::set_any(const char* name, aist_ast_T* val)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    aist_ast_T* find_var = m_object->m_object_children->find(hash);
#else
    aist_ast_T* find_var = m_object->m_object_children->find(name);
#endif
    if(val == NULL)
        val = m_runtime->rt_get_null();
    if(find_var)
    {
        if(find_var->m_var_def->m_type  != AIST::AST_type::AST_ANY)
            return false;
        m_runtime->ast_release(find_var->m_var_def->m_value);
        find_var->m_var_def->m_value = val->AddRef();
    }else{
        find_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        find_var->m_var_def->m_name_hash = hash;
#else
        find_var->m_var_def->m_name  = m_runtime->rt_str_clone(name);
#endif
        find_var->m_var_def->m_type  = AIST::AST_type::AST_ANY;
        find_var->m_var_def->m_value = val->AddRef();
        m_object->m_object_children->push(find_var);
    }
    return true;
}


bool aist_object_T::register_object(const char* name, aist_object_T* obj)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    if(m_runtime == NULL || m_runtime != obj->m_runtime || m_object->m_object_children->find(hash))
        return false;
    aist_ast_T* new_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
    new_var->m_var_def->m_name_hash = hash;
#else
    if(m_runtime == NULL || m_runtime != obj->m_runtime || m_object->m_object_children->find(name))
        return false;
    aist_ast_T* new_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
    new_var->m_var_def->m_name  = m_runtime->rt_str_clone(name);
#endif
    new_var->m_var_def->m_type  = AIST::AST_type::AST_OBJECT;
    new_var->m_var_def->m_value = obj->m_object->AddRef();
    if((obj->m_object->m_flags&AIST::FLAG_CONST_VAR) != 0)
        new_var->m_flags |= AIST::FLAG_CONST_VAR;
    m_object->m_object_children->push(new_var);
    return true;
}


bool aist_object_T::register_list(const char* name, aist_list_T* list)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    if(m_runtime == NULL || m_runtime != list->m_runtime || m_object->m_object_children->find(hash))
        return false;
    aist_ast_T* new_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
    new_var->m_var_def->m_name_hash = hash;
#else
    if(m_runtime == NULL || m_runtime != list->m_runtime || m_object->m_object_children->find(name))
        return false;
    aist_ast_T* new_var = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
    new_var->m_var_def->m_name  = m_runtime->rt_str_clone(name);
#endif
    new_var->m_var_def->m_type  = AIST::AST_type::AST_LIST;
    new_var->m_var_def->m_value = list->m_list->AddRef();
    if((list->m_list->m_flags&AIST::FLAG_CONST_VAR) != 0)
        new_var->m_flags |= AIST::FLAG_CONST_VAR;
    m_object->m_object_children->push(new_var);
    return true;
}


bool aist_object_T::register_func(const char* name, aist_function_bind fptr)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    if(m_runtime == NULL || m_object->m_object_children->find(hash))
        return false;
    aist_ast_T* fdef = m_runtime->rt_ast_create(AIST::AST_type::AST_FUNCTION_DEFINITION);
    fdef->m_fn_def->m_function_name_hash = hash;
#else
    if(m_runtime == NULL || m_object->m_object_children->find(name))
        return false;
    aist_ast_T* fdef = m_runtime->rt_ast_create(AIST::AST_type::AST_FUNCTION_DEFINITION);
    fdef->m_fn_def->m_function_name = m_runtime->rt_str_clone(name);
#endif
    fdef->m_fn_def->m_fn_ptr = fptr;
    m_object->m_object_children->push(fdef);
    return true;
}


bool aist_object_T::get_string(const char* name, const char*& val, size_t* outLen)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(ast->m_var_def->m_value && ast->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_STRING)
    {
        val = ast->m_var_def->m_value->m_string_value;
        if(outLen)
            *outLen = ast->m_var_def->m_value->m_string_value_len;
        return true;
    }
    if(ast->m_var_def->m_init_value && ast->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_STRING)
    {
        val = ast->m_var_def->m_init_value->m_string_value;
        if(outLen)
            *outLen = ast->m_var_def->m_init_value->m_string_value_len;
        return true;
    }
    return false;
}


bool aist_object_T::get_int(const char* name, aist_int& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(ast->m_var_def->m_value && ast->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_INTEGER)
    {
        val = ast->m_var_def->m_value->m_int_value;
        return true;
    }
    if(ast->m_var_def->m_init_value && ast->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_INTEGER)
    {
        val = ast->m_var_def->m_init_value->m_int_value;
        return true;
    }
    return false;
}


bool aist_object_T::get_float(const char* name, aist_float& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(ast->m_var_def->m_value && ast->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_FLOAT)
    {
        val = ast->m_var_def->m_value->m_float_value;
        return true;
    }
    if(ast->m_var_def->m_init_value && ast->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_FLOAT)
    {
        val = ast->m_var_def->m_init_value->m_float_value;
        return true;
    }
    return false;
}


bool aist_object_T::get_bool(const char* name, bool& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(ast->m_var_def->m_value && ast->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_BOOLEAN)
    {
        val = ast->m_var_def->m_value->m_boolean_value;
        return true;
    }
    if(ast->m_var_def->m_init_value && ast->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_BOOLEAN)
    {
        val = ast->m_var_def->m_init_value->m_boolean_value;
        return true;
    }
    return false;
}


bool aist_object_T::get_char(const char* name, char& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(ast->m_var_def->m_value && ast->m_var_def->m_value->m_ast_type == AIST::AST_type::AST_UCHAR)
    {
        val = ast->m_var_def->m_value->m_char_value;
        return true;
    }
    if(ast->m_var_def->m_init_value && ast->m_var_def->m_init_value->m_ast_type == AIST::AST_type::AST_UCHAR)
    {
        val = ast->m_var_def->m_init_value->m_char_value;
        return true;
    }
    return false;
}


bool aist_object_T::get_object(const char* name, aist_object_T*& obj, bool create_if_none)
{
#ifdef AIST_USE_HASH_NAME
    aist_hash_T hash = aist_str_hash(name, aist_os_strlen(name));
    aist_ast_T* ast = m_object->m_object_children->find(hash);
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast && create_if_none)
    {
        for(size_t i = 0; name[i]; i++)
        {
            if(name[i] == '.' || name[i] == '[' || name[i] == ']')
                return false;
        }
        ast = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        ast->m_var_def->m_name_hash = hash;
#else
        ast->m_var_def->m_name = m_runtime->rt_str_clone(name);
#endif
        aist_ast_T* value = m_runtime->rt_ast_create(AIST::AST_type::AST_OBJECT);
        ast->m_var_def->m_type  = AIST::AST_type::AST_OBJECT;
        ast->m_var_def->m_value = value;
        m_object->m_object_children->push(ast);
    }
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(!ast->m_var_def->m_value || ast->m_var_def->m_value->m_ast_type != AIST::AST_type::AST_OBJECT)
        return false;
    aist_object_T* new_obj = (aist_object_T*)m_runtime->rt_malloc(sizeof(aist_object_T));
    new_obj->m_runtime = m_runtime;
    new_obj->m_object  = ast->m_var_def->m_value->AddRef();
    new_obj->m_cRef    = 1;
    obj = new_obj;
    return true;
}


bool aist_object_T::get_list(const char* name, aist_list_T*& list, bool create_if_none)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast && create_if_none)
    {
        for(size_t i = 0; name[i]; i++)
        {
            if(name[i] == '.' || name[i] == '[' || name[i] == ']')
                return false;
        }
        ast = m_runtime->rt_ast_create(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        ast->m_var_def->m_name_hash = aist_str_hash(name, aist_os_strlen(name));
#else
        ast->m_var_def->m_name = m_runtime->rt_str_clone(name);
#endif
        aist_ast_T* value = m_runtime->rt_ast_create(AIST::AST_type::AST_LIST);
        ast->m_var_def->m_type  = AIST::AST_type::AST_LIST;
        ast->m_var_def->m_value = value;
        m_object->m_object_children->push(ast);
    }
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(!ast->m_var_def->m_value || ast->m_var_def->m_value->m_ast_type != AIST::AST_type::AST_LIST)
        return false;
    aist_list_T* new_list = (aist_list_T*)m_runtime->rt_malloc(sizeof(aist_list_T));
    new_list->m_runtime = m_runtime;
    new_list->m_list    = ast->m_var_def->m_value->AddRef();
    new_list->m_cRef    = 1;
    list = new_list;
    return true;
}


bool aist_object_T::get_any(const char* name, aist_ast_T*& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION)
        return false;
    if(!ast->m_var_def->m_value || ast->m_var_def->m_type != AIST::AST_type::AST_ANY)
        return false;
    val = ast;
    return true;
}


bool aist_object_T::get_func(const char* name, aist_ast_T*& val)
{
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* ast = m_object->m_object_children->find(aist_str_hash(name, aist_os_strlen(name)));
#else
    aist_ast_T* ast = m_object->m_object_children->find(name);
#endif
    if(!ast || ast->m_ast_type != AIST::AST_type::AST_FUNCTION_DEFINITION)
        return false;
    val = ast;
    return true;
}


//===========================================================================================================
//=============================================== aist_list_T ===============================================
//===========================================================================================================


size_t aist_list_T::AddRef()
{
    return ++m_cRef;
}


size_t aist_list_T::Release()
{
    if(--m_cRef == 0)
    {
        if(m_runtime)
            m_runtime->ast_release(m_list);
        aist_runtime_T* rt = m_runtime;
        m_runtime = NULL;
        rt->rt_free(this, sizeof(this));
        return 0;
    }
    return m_cRef;
}


bool aist_list_T::inset_string(char* val, size_t len)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = m_runtime->rt_ast_create(AIST::AST_type::AST_STRING);
    if(!len)
        len = aist_os_strlen(val);
    ast->m_string_value_len = (unsigned int)len;
    ast->m_string_value = m_runtime->rt_str_clone(val, len);
    m_list->m_list_children->push(ast);
    return true;
}


bool aist_list_T::inset_int(aist_int val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = m_runtime->rt_ast_create(AIST::AST_type::AST_INTEGER);
    ast->m_int_value = val;
    m_list->m_list_children->push(ast);
    return true;
}


bool aist_list_T::inset_float(aist_float val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = m_runtime->rt_ast_create(AIST::AST_type::AST_FLOAT);
    ast->m_float_value = val;
    m_list->m_list_children->push(ast);
    return true;
}


bool aist_list_T::inset_bool(bool val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = m_runtime->rt_ast_create(AIST::AST_type::AST_BOOLEAN);
    ast->m_boolean_value = val;
    m_list->m_list_children->push(ast);
    return true;
}


bool aist_list_T::inset_char(char val)
{
    if(!m_runtime)
        return false;
    aist_ast_T* ast = m_runtime->rt_ast_create(AIST::AST_type::AST_UCHAR);
    ast->m_char_value = val;
    m_list->m_list_children->push(ast);
    return true;
}


bool aist_list_T::inset_object(aist_object_T* obj)
{
    if(!m_runtime || m_runtime != obj->m_runtime)
        return false;
    m_list->m_list_children->push(obj->m_object->AddRef());
    return true;
}


bool aist_list_T::inset_list(aist_list_T* list)
{
    if(!m_runtime || m_runtime != list->m_runtime)
        return false;
    m_list->m_list_children->push(list->m_list->AddRef());
    return true;
}


