/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once

#ifndef _AIST_H_
#define _AIST_H_
#include <stdlib.h>

//#define AIST_DEBUG_
//#define AIST_DEBUG_AST_

//#define AIST_USE_HASH_NAME


typedef long long aist_int;
typedef double    aist_float;

class  aist_runtime_T;
struct aist_ast_T;
class  aist_ast_list_T;
class  aist_T;

typedef aist_ast_T* (*aist_function_bind)(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args);

typedef bool (*aist_def_load_include)(const char* name, char*& outData, size_t& outDataLen);
typedef void (*aist_def_free_include)(const char* name, char*  outData, size_t  outDataLen);


// врапер для работы с типом List
class aist_list_T
{
    friend class aist_object_T;
    friend class aist_T;
    aist_runtime_T* m_runtime;
    aist_ast_T*     m_list;
    size_t          m_cRef;

public:
    size_t AddRef();
    size_t Release();

    bool inset_int(aist_int   val);
    bool inset_float(aist_float val);
    bool inset_string(char* val, size_t len = 0);
    bool inset_bool  (bool  val);
    bool inset_char  (char  val);

    bool inset_object(aist_object_T* obj);
    bool inset_list(aist_list_T* list);
};


// врапер для работы с типом Object
class aist_object_T
{
    friend class aist_list_T;
    friend class aist_T;
    aist_runtime_T* m_runtime;
    aist_ast_T*     m_object;
    size_t          m_cRef;

    aist_ast_T* find_create_variable(const char* name, size_t data_type);
public:
    size_t AddRef();
    size_t Release();

    bool set_flag_const(const char* name, bool isConst = true);
    bool set_flag_no_copy(const char* name, bool noCopy = true);

    bool set_int   (const char* name, aist_int   val);
    bool set_float (const char* name, aist_float val);
    bool set_bool  (const char* name, bool val);
    bool set_char  (const char* name, char val);
    bool set_string(const char* name, char* val, size_t len = 0);
    bool set_any   (const char* name, aist_ast_T* val);

    bool register_object(const char* name, aist_object_T* obj);
    bool register_list  (const char* name, aist_list_T* list);
    bool register_func  (const char* name, aist_function_bind fptr);

    bool get_string(const char* name, const char*& val, size_t* outLen = NULL);// return string do not free
    bool get_int   (const char* name, aist_int& val);
    bool get_float (const char* name, aist_float& val);
    bool get_bool  (const char* name, bool& val);
    bool get_char  (const char* name, char& val);
    bool get_any   (const char* name, aist_ast_T*& val);
    bool get_func  (const char* name, aist_ast_T*& val);

    bool get_object(const char* name, aist_object_T*& obj, bool create_if_none);
    bool get_list  (const char* name, aist_list_T*&  list, bool create_if_none);

};


// врапер настройка и запуск скриптов
class aist_T
{
    friend class aist_object_T;
    friend class aist_list_T;
    friend aist_T* aist_create();
    friend void    aist_destroy(aist_T*& runtime);
    friend bool    aist_clone(const aist_T* src_rt, aist_T* dst_rt);

    aist_runtime_T* m_runtime;
    void rt_reset(aist_runtime_T* src);

    aist_ast_T* create_var(const char* name, size_t data_type);
    aist_ast_T* find_global_path_var(const char* name, size_t len, bool toValue);
public:
    struct error_T
    {
        const char* m_label_src;
        char   m_errorStr[128];
        size_t m_line;
        size_t m_pos;
        bool   m_isParser; //ошибка в парсере (иначе при выполнении)
    };

    aist_T(){ m_runtime = NULL; };
    aist_T(aist_runtime_T* runtime){ m_runtime = runtime; };

    void set_include_callback(aist_def_load_include loadFn, aist_def_free_include freeFn);
    void set_user_data(void* data, size_t index);// index = 0...8

    bool run(const char* aist_src_code, size_t len = 0, const char* label_src = NULL, size_t shift_line_num = 0);
    void reset();

    const char* get_return(size_t* outLen = NULL);
    const error_T* get_error();
    
    bool register_function(const char* global_fname, aist_function_bind fptr);//ошибка если уже есть

    //создать или заменить глобальное значение (ошибка если типы не совпадают)
    bool set_string(const char* global_name, const char* value, size_t len = 0);
    bool set_int   (const char* global_name, aist_int value);
    bool set_float (const char* global_name, aist_float value);
    bool set_bool  (const char* global_name, bool value);
    bool set_char  (const char* global_name, char value);

    bool get_string(const char* path_name, const char*& value, size_t* outLen = NULL);// return string do not free
    bool get_int   (const char* path_name, aist_int& value);
    bool get_float (const char* path_name, aist_float& value);
    bool get_bool  (const char* path_name, bool& value);
    bool get_char  (const char* path_name, char& value);
    bool get_object(const char* path_name, aist_object_T*& obj, bool create_if_none);
    bool get_list  (const char* path_name, aist_list_T*&  list, bool create_if_none);

    const char* get_def_string(const char* path_name, const char* def_val){ this->get_string(path_name, def_val); return def_val; }
    aist_int    get_def_int   (const char* path_name, aist_int    def_val){ this->get_int(path_name, def_val); return def_val; }
    aist_float  get_def_float (const char* path_name, aist_float  def_val){ this->get_float(path_name, def_val); return def_val; }
    bool        get_def_bool  (const char* path_name, bool def_val){ this->get_bool(path_name, def_val); return def_val; }
    char        get_def_char  (const char* path_name, char def_val){ this->get_char(path_name, def_val); return def_val; }

    bool set_flag_const(const char* path_name, bool isConst = true);
};


aist_T* aist_create();
bool aist_clone(const aist_T* src_rt, aist_T* dst_rt);
void aist_destroy(aist_T*& rt);

bool aist_process_template(aist_T aist, const char* label_src, const char *buf, size_t buf_len, char*& outData, size_t& outDataLen);// delete outData use free(outData);

#endif // _AIST_H_
