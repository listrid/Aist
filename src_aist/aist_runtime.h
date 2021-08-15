/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once

#ifndef _AIST_RUNTIME_H_
#define _AIST_RUNTIME_H_
//#include <stdio.h>
#include <setjmp.h>
#include "aist.h"
#include "aist_mem.h"


namespace AIST
{
    const static size_t MAX_RT_CALL_LEVEL      = 64;  // максимальная вложеность блоков
    const static size_t MAX_VARIABLE_NAME_SIZE = 64;  //максимальный размер имени переменной или функции
    const static size_t MAX_RT_STRING_SIZE     = 0x4000; //16 kb

    const static unsigned char FLAG_STR_MEM_STOR  = 0x00; // удалять через менеджер памяти
    const static unsigned char FLAG_STR_MEM_SYS   = 0x01; // удалять через aist_os_free
    const static unsigned char FLAG_STR_MEM_CONST = 0x02; // не удалять данные, они внешние (string)
    const static unsigned char FLAG_STR_MEM_USER  = 0x03; // заданные внешними функциями //xxx не реализовано
    const static unsigned char FLAG_AST_MEM_RT    = 0x04; // выделено в рантайме (иначе в парсере)

    const static unsigned char FLAG_NOT_RELEASE   = 0x08; // не удалять ast 'ALL' (зафиксировать счетчик использования)
    const static unsigned char FLAG_BLOCK_IS_FN   = 0x10; // является телом функции 'AST_BLOCK'
    const static unsigned char FLAG_CONST_VAR     = 0x10; // константная переменная 'AST_VARIABLE_DEFINITION, AST_LIST, AST_OBJECT'
    const static unsigned char FLAG_VAR_NOT_COPY  = 0x20; // при копировании значение не переносится (ставится дефолтное) ' AST_VARIABLE_DEFINITION, AST_UCHAR ... AST_LIST '
    const static unsigned char FLAG_RESTORE_THIS  = 0x20; // восставить this_one после левой части в операциях 'AST_BINOP', AST_VARIABLE_MODIFIER'
    const static unsigned char FLAG_OBJECT_INIT   = 0x40; // обьект инициализирован (вызван метод new если есть) 'AST_OBJECT'
    //0x80;
    enum LEXEME_type : unsigned char
    {
        LEXEME_EOF = 0,      //
        LEXEME_INTEGER_VALUE,//
        LEXEME_FLOAT_VALUE,  //
        LEXEME_CHAR_VALUE,   //
        LEXEME_STRING_VALUE, //
        LEXEME_NAME,         //
        LEXEME_LBRACE,       // {
        LEXEME_RBRACE,       // }
        LEXEME_LBRACKET,     // [
        LEXEME_RBRACKET,     // ]
        LEXEME_LPAREN,       // (
        LEXEME_RPAREN,       // )
        LEXEME_DOT,          // .
        LEXEME_EQUALS,       // =
        LEXEME_SEMI,         // ;
        LEXEME_COMMA,        // ,
// начало простых операций (конец LEXEME_MINUS)
        LEXEME_PLUS,         // +
        LEXEME_STAR,         // *
        LEXEME_DIV,          // /
        LEXEME_LESS,         // <
        LEXEME_LEQUAL,       // <=
        LEXEME_LARGER,       // >
        LEXEME_LAQUAL,       // >=
        LEXEME_BOOLEAN_AND,  // &&
        LEXEME_BOOLEAN_OR,   // ||
        LEXEME_EQUALS_EQUALS,// ==
        LEXEME_NOT_EQUALS,   // !=
        LEXEME_PERCENTAGE,   // %
        LEXEME_BINARY_AND,   // &
        LEXEME_BINARY_OR,    // |
        LEXEME_BINARY_NOT,   // ~ 
        LEXEME_BINARY_XOR,   // ^
        LEXEME_SHIFT_LEFT,   // <<
        LEXEME_SHIFT_RIGHT,  // >>
        LEXEME_MINUS,        // -

        LEXEME_PLUS_EQUALS,  // +=
        LEXEME_MINUS_EQUALS, // -=

        LEXEME_PLUS_PLUS,    // ++
        LEXEME_MINUS_MINUS,  // --

        LEXEME_NOT,          // !
        LEXEME_QUESTION,     // ?
        LEXEME_COLON,        // :
    };

    enum AST_type : unsigned char
    {
        AST_NULL = 0,
        AST_VOID,
//простой тип (!! порядок не менять)
        AST_UCHAR,
        AST_BOOLEAN,
        AST_INTEGER,
        AST_FLOAT,
        AST_STRING,

//составной
        AST_OBJECT,
        AST_LIST,
// любой тип
        AST_ANY,    

        //
        AST_VARIABLE,
        AST_VARIABLE_DEFINITION, 
        AST_VARIABLE_MODIFIER,
        AST_FUNCTION_DEFINITION,
        AST_FUNCTION_CALL,
        AST_BLOCK,
        AST_BINOP,
        AST_UNOP,

        AST_CONTINUE,
        AST_BREAK,
        AST_RETURN,
        AST_TERNARY,
        AST_IF,
        AST_WHILE,
        AST_FOR,
        AST_ATTRIBUTE_ACCESS,
        AST_ARRAY_ACCESS,
        AST_NEW,
        AST_DELETE,
        AST_INCLUDE
    };
};

#ifdef AIST_USE_HASH_NAME
#define AIST_LEX_THIS   0x00000000E6D2D0E9
#define AIST_LEX_DELETE 0x0000CAE8CAD8CAC9
#define AIST_LEX_NEW    0x0000000000EECADD
#define AIST_LEX_LEN    0x0000000000DCCAD9
#define AIST_LEX_SIZE   0x00000000CAF4D2E7
#define AIST_LEX_COUNT  0x000000E8DCEADEC7
#else
#define AIST_LEX_THIS   "this"
#define AIST_LEX_DELETE "delete"
#define AIST_LEX_NEW    "new"
#define AIST_LEX_LEN    "len"
#define AIST_LEX_SIZE   "size"
#define AIST_LEX_COUNT  "count"
#endif


#ifdef AIST_USE_HASH_NAME
#define AIST_PR_NAME_HASH "0x%.16lX"
#define AIST_NAME_HASH(x) x##_hash
#else
#define AIST_PR_NAME_HASH "%s"
#define AIST_NAME_HASH(x) x
#endif


typedef long long aist_hash_T;
aist_hash_T aist_str_hash(const char* str, size_t len);

class aist_ast_list_T
{
    friend class aist_runtime_T;
    aist_mem_T*  m_mem;
    size_t m_real_size;
    void init(aist_mem_T* mem, size_t initSize);
public:

    size_t m_ast_count;
    aist_ast_T** m_ast_items;

    void push(aist_ast_T* item);
    void reserve(size_t countItems);
    void remove(aist_ast_T* element, aist_runtime_T* free_rt = NULL);
    void removeAll(aist_runtime_T* free_rt);
#ifdef AIST_USE_HASH_NAME
    aist_ast_T* find(aist_hash_T name_hash);
#else
    aist_ast_T* find(const char* name);
#endif
};


class aist_rt_stack_T
{
private:
    struct block_T
    {
        aist_ast_list_T* m_var_def_list;
        block_T* m_next;
        block_T* m_prev;
    };

    size_t m_stack_pos;
    size_t m_stack_size_alloc;
    struct  stack_T
    {
        block_T* m_head;
        block_T* m_base;
        const char* m_label_src;
    };
    stack_T* m_stack;
    block_T* m_global;
    block_T* m_fn_init_stack;
    aist_ast_T* m_this;
    aist_ast_T* m_this_one; //до первого обращения (для поиска переменных в this обьекта) костыль упрощения кода
    aist_ast_T* m_this_one_prev;
    void clear_block(block_T* block);

    aist_runtime_T*  m_rt;
public:
    size_t m_level; // общий уровень вложенности (для контроля)

    void Init(aist_runtime_T* rt);
    void CopyGlobal(const aist_rt_stack_T* src_stack);
    void Reset(bool global); // вернуть к нулевому уровню 

    void level_up();   // вверх по стеку
    void level_down(); // вниз по стеку

    void fn_init(); // настройка начала входа
    void fn_push_variable(aist_ast_T* var); // заполнение кадра вызываемыми параметрами
    void fn_start(const char* label_src);   // вход в функцию
    void fn_end();  // выход из функции

#ifdef AIST_USE_HASH_NAME
    aist_ast_T* find_variable(aist_hash_T name);
    aist_ast_T* find_global_variable(aist_hash_T name);
#else
    aist_ast_T* find_variable(const char* name);
    aist_ast_T* find_global_variable(const char* name);
#endif
    void push_variable(aist_ast_T* var);
    void push_global_variable(aist_ast_T* var);
#ifdef AIST_USE_HASH_NAME
    void delete_def(aist_hash_T name);
#else
    void delete_def(const char* name);
#endif
    aist_ast_T* set_this(aist_ast_T* obj);
    aist_ast_T* get_this(){ return m_this; };
    void        set_this_one(aist_ast_T* obj); // установить обьект для единичного поиска
    aist_ast_T* get_this_one(){ return m_this_one; }; // узнать текущий единичный обьект без удаления

    const char* set_global_label_src(const char* new_label);
    const char* get_active_label_src();
};


struct aist_lexeme_T
{
    const char*    m_value;
    size_t         m_pos;
    unsigned short m_len;
    AIST::LEXEME_type m_lex_type;
};


struct aist_ast_T
{
    union
    {
        unsigned int m_line_pos12;
        unsigned int m_string_value_len;
    };

    AIST::AST_type m_ast_type;
    unsigned char  m_flags;
    unsigned short m_countUse;

    struct var_def_T
    {
#ifdef AIST_USE_HASH_NAME
        aist_hash_T  m_name_hash;
#else
        const char*  m_name;
#endif
        aist_ast_T*  m_value;
        aist_ast_T*  m_init_value;
        AIST::AST_type m_type;
    };
    struct fn_def_T
    {
#ifdef AIST_USE_HASH_NAME
        aist_hash_T        m_function_name_hash;
#else
        const char*        m_function_name;
#endif
        const char*        m_label_src;
        aist_ast_list_T*   m_arguments;
        aist_ast_T*        m_body;
        aist_function_bind m_fn_ptr;
        AIST::AST_type     m_return_type;
    };
    struct var_modif_T  // '='
    {
        aist_ast_T* m_right;
        aist_ast_T* m_left;
    };
    struct array_access_T
    {
        aist_ast_T* m_right; // if '='
        aist_ast_T* m_left;
        aist_ast_T* m_pointer; //index
    };
    struct for_T
    {
        aist_ast_T* m_init_statement;
        aist_ast_T* m_test_expr;
        aist_ast_T* m_update_statement;
        aist_ast_T* m_body;
    };
    struct if_T
    {
        aist_ast_T* m_if_expr;
        aist_ast_T* m_if_body;
        aist_ast_T* m_if_otherwise; //оптимизация цепочек if(){}else if(){}
        aist_ast_T* m_else_body;
    };
    struct while_T
    {
        aist_ast_T* m_test_expr;
        aist_ast_T* m_body;
    };
    struct ternary_T
    {
        aist_ast_T* m_expr;
        aist_ast_T* m_body;
        aist_ast_T* m_else_body;
    };
    struct unop_T
    {
        aist_ast_T*       m_right;
        AIST::LEXEME_type m_operator;
    };
    struct binop_T
    {
        aist_ast_T*       m_right;
        aist_ast_T*       m_left;
        AIST::LEXEME_type m_operator;
    };

    struct fn_call_T
    {
        aist_ast_T*      m_name; // AST_VARIABLE
        aist_ast_list_T* m_arguments;
    };
    struct attr_access_T
    {
        aist_ast_T* m_right;
        aist_ast_T* m_left;
    };

    struct return_T
    {
        aist_ast_T* m_value;
        aist_ast_T* m_init_value;
    };
    union
    {
        for_T*           m_for;        // AST_FOR
        if_T*            m_if;         // AST_IF
        while_T*         m_while;      // AST_WHILE
        var_def_T*       m_var_def;    // AST_VARIABLE_DEFINITION
        var_modif_T*     m_var_modif;  // AST_VARIABLE_MODIFIER
        ternary_T*       m_ternary;    // AST_TERNARY
        unop_T*          m_unop;       // AST_UNOP
        binop_T*         m_binop;      // AST_BINOP
        fn_def_T*        m_fn_def;     // AST_FUNCTION_DEFINITION
        fn_call_T*       m_fn_call;    // AST_FUNCTION_CALL
        array_access_T*  m_arr_access; // AST_ARRAY_ACCESS
        attr_access_T*   m_attribute;  // AST_ATTRIBUTE_ACCESS
        return_T*        m_return;     //AST_RETURN

        //одинарные
#ifdef AIST_USE_HASH_NAME
        aist_hash_T      m_variable_name_hash; // AST_VARIABLE
#else
        char*            m_variable_name;   // AST_VARIABLE
#endif
        aist_ast_T*      m_new_del_incl_right;  // AST_NEW, AST_DELETE, AST_INCLUDE
        aist_ast_list_T* m_object_children; // AST_OBJECT
        aist_ast_list_T* m_block_list;      // AST_BLOCK
        aist_ast_list_T* m_list_children;   // AST_LIST
        char*            m_string_value;    // AST_STRING
        //простые
        aist_int      m_int_value;
        char          m_boolean_value;
        aist_float    m_float_value;
        unsigned char m_char_value;
    };
    inline aist_ast_T* AddRef()
    {
        if(this && (m_flags&AIST::FLAG_NOT_RELEASE) == 0)
            m_countUse++;
        return this;
    }


#ifdef AIST_DEBUG_AST_
    size_t dbg_num;
#endif
};


struct zRead;
struct aist_parser_T
{

private:
    struct l_state
    {
        char   m_cur_char;
        size_t m_cur_index;
        size_t m_line;
        size_t m_line_pos;
    };

    struct l
    {
    private:
        void next();                // Advance, move to the next char
        void skip_line_comment();
        void skip_block_comment();

        aist_lexeme_T lexeme_init(AIST::LEXEME_type type, size_t pos, size_t len);

        aist_lexeme_T collect_string();
        aist_lexeme_T collect_char();
        aist_lexeme_T collect_number();
        aist_lexeme_T collect_id();
        aist_lexeme_T next_with_lexeme(AIST::LEXEME_type type);
    public:
        size_t   m_errorPos;
        char     m_errorStr[128];
        jmp_buf* m_error_jmp_buf;


        aist_mem_T* m_mem;
        const char* m_label_src;

        const char* m_contents;
        size_t      m_length;
        l_state     m_state;
        l_state     m_prev_state;

        aist_lexeme_T get_next_lexeme(); // Get the next lexeme from the lexer
        void  lex_error(const char* str, ...);

        aist_lexeme_T m_prev_lexeme;
        aist_lexeme_T m_cur_lexeme;
    }m_lexer;



    void _unexpected_lexeme_error();
    aist_ast_T*      create_ast(AIST::AST_type type);
    aist_ast_list_T* create_list();
    aist_ast_T* parser_eat(AIST::LEXEME_type lex_type);

    AIST::AST_type parse_type(); // определить тип (return AST_NULL не тип)
    aist_ast_T* parse_float();
    aist_ast_T* parse_string();
    aist_ast_T* parse_uchar();
    aist_ast_T* parse_integer();
    aist_ast_T* parse_boolean();
    aist_ast_T* parse_list();
    aist_ast_T* parse_object();
    aist_ast_T* parse_new();
    aist_ast_T* parse_delete();
    aist_ast_T* parse_return();
    aist_ast_T* parse_if();
    aist_ast_T* parse_while();
    aist_ast_T* parse_for();
    aist_ast_T* parse_function_call(aist_ast_T* variable_name);// вход анализа вызова функции ( 'name(' )
    aist_ast_T* parse_array_access(aist_ast_T* left, bool getOnly);// 'name[index]', 'name[index] ='
    aist_ast_T* parse_variable();   // вход анализа изменение переменной   ( 'name =' 'name +='  'name -=' )
    aist_ast_T* parse_definition(bool setFnParamInit); // вход анализа определения переменной ( 'type name =', 'type name(' )

    aist_ast_T* parse_factor();  // вычисляет переменную для анализа (уровень 0)
    aist_ast_T* parse_term_l1(aist_ast_T* node); // самый высокий уровень анализа. 'вызов функции' '*' '/' '%'  (уровень 1)
    aist_ast_T* parse_term_l2(aist_ast_T* node); // +  - 
    aist_ast_T* parse_term_l3(aist_ast_T* node); // << >>
    aist_ast_T* parse_term_l4(aist_ast_T* node); // + -  == != ...
    aist_ast_T* parse_term_l5(aist_ast_T* node); // самый низкий приоритет операций < <= .. == != 
    aist_ast_T* parse_expression(aist_ast_T* prev_node = NULL);  //вход анализа 1го вычисляемого значение

    aist_ast_T* parse_statement();   // любая строка (определяет тип анализа)
    aist_ast_T* parse_block_statements(); // вход анализа блока  { ... }
    aist_ast_T* parse_block_with_one_statement(); //создает блок с 1 выражением (до ';')

    aist_ast_T* parse_zunpack(zRead* data);

    aist_runtime_T* m_runtime;
public:
    const char* get_error(){ return m_lexer.m_errorStr; }
    size_t      get_errorPos(){ return m_lexer.m_errorPos;  }

    bool        parse(aist_runtime_T* runtime, const char* label_src, const char* contents, size_t len, size_t shift_line_num);
    aist_ast_T* m_result_ast;
};


class aist_runtime_T
{
    friend class aist_T;
    friend class aist_ast_list_T;
    friend class aist_rt_stack_T;
    friend struct aist_parser_T;

    friend aist_ast_T* builtin_dbg_mem(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args);
    friend aist_ast_T* builtin_isDef(aist_runtime_T* runtime, size_t line_pos12, aist_ast_T* obj_this, const aist_ast_list_T* args);
private:
    aist_ast_T* function_call(aist_ast_T* fn_def, aist_ast_list_T* arguments, aist_ast_T* obj_this, unsigned int line_pos12);

    bool boolean_evaluation(aist_ast_T* node);
    aist_ast_T* eval_var_inc(aist_ast_T* node);

    aist_ast_T* eval_variable(aist_ast_T* node);
    aist_ast_T* eval_variable_definition(aist_ast_T* node);
    aist_ast_T* eval_variable_modifier(aist_ast_T* node);
    aist_ast_T* eval_function_definition(aist_ast_T* node);
    aist_ast_T* eval_function_call(aist_ast_T* node);
    aist_ast_T* eval_block(aist_ast_T* node);
    aist_ast_T* eval_attribute_access(aist_ast_T* node);
    aist_ast_T* eval_array_access(aist_ast_T* node);
    aist_ast_T* eval_binop(aist_ast_T* node);
    aist_ast_T* eval_unary(aist_ast_T* node);
    aist_ast_T* eval_break(aist_ast_T* node);//n
    aist_ast_T* eval_continue(aist_ast_T* node);//n
    aist_ast_T* eval_return( aist_ast_T* node); //n
    aist_ast_T* eval_if(aist_ast_T* node);
    aist_ast_T* eval_ternary( aist_ast_T* node);
    aist_ast_T* eval_while(aist_ast_T* node);
    aist_ast_T* eval_for(aist_ast_T* node);
    aist_ast_T* eval_new(aist_ast_T* node);
    aist_ast_T* eval_delete(aist_ast_T* node);
    aist_ast_T* eval_include(aist_ast_T* node);
    aist_ast_T* eval(aist_ast_T* node);

    aist_ast_T* eval_main(aist_ast_T* node);

    aist_ast_T* ast_create(AIST::AST_type type, size_t flags);
    aist_ast_T* rt_ast_copy(aist_ast_T* ast);

    aist_ast_list_T* list_create(size_t initSize, bool useParseMem);
    void             list_free(aist_ast_list_T*& list);

    bool m_object_copy_call_new;

    char m_tmp_str[64];
    aist_rt_stack_T* m_rt_stack;
    jmp_buf*         m_error_jmp_buf;
    aist_T::error_T  m_error;

    aist_parser_T* m_parser;
    aist_ast_T*    m_return_ast;

    aist_mem_T* m_mem_rt;
    aist_mem_T* m_mem_parser;
    aist_ast_T* m_ast_void; // уловитель ошибок
    aist_ast_T* m_ast_null;

    aist_def_load_include m_include_fn;
    aist_def_free_include m_include_free_fn;
    void rt_clear_parser();
public:


    static aist_runtime_T* create(aist_mem_T* mem_rt);
    static aist_mem_T*     destroy(aist_runtime_T* rt);

    void* m_user_data[8];
    void  expect_args(size_t line_pos12, const aist_ast_list_T* in_args, int argc, ... );
    bool  ast_to_str(aist_ast_T* ast, const char*& outStr, size_t& outLen);

    const aist_ast_T* parse(const char* label_src, const char* src_code, size_t src_len = 0, size_t shift_line_num = 0);

    aist_ast_T* rt_find_var(const char* name);
    aist_ast_T* rt_get_null() { return m_ast_null; };
    bool rt_run(const char* label_src, const aist_ast_T* ast);
    void rt_error(aist_ast_T* release, size_t line_pos12, const char* str, ...);

    void* rt_malloc(size_t size);
    void  rt_free(void* ptr, size_t len);
    char* rt_str_clone(const char* value, size_t len = 0);


    inline aist_ast_T* rt_ast_create(AIST::AST_type type) { return ast_create(type, AIST::FLAG_AST_MEM_RT); }
    void ast_release(aist_ast_T* ast);
};


const char* aist_type_name(AIST::AST_type type);
const char* aist_LEXEME_name(AIST::LEXEME_type type);


//string alloc, free
void  aist_str_free (aist_mem_T* mem, char*& value, size_t len = 0);
char* aist_str_clone(aist_mem_T* mem, const char* value, size_t len = 0);

void aist_ast_dump(const char* fileName, const aist_ast_T* ast);

class aist_dynamic_array_T;
bool aist_ast_save_zpak(const aist_ast_T* ast, aist_dynamic_array_T* outData);


#endif //_AIST_RUNTIME_H_
