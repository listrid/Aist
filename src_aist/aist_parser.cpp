/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif
#include "aist_runtime.h"

static size_t aist_parser_char_code(const char* inStr, size_t len, char& outChar)
{
    if(len < 2 && inStr[0] != '\\')
        return 0;
    char c = inStr[1];
    if(c >= '0' && c <= '9')
    {
        outChar = c - '0';
        return 2;
    }
    if(c == 't' ){ outChar = '\t'; return 2; }
    if(c == 'n' ){ outChar = '\n'; return 2; }
    if(c == 'r' ){ outChar = '\r'; return 2; }
    if(c == '\\'){ outChar = '\\'; return 2; }
    if(c == '\''){ outChar = '\''; return 2; }
    if(c == '"') { outChar = '"';  return 2; }
    if(len < 4 || c != 'x')
        return 0;
    c = inStr[2];
    if(c >= '0' && c <= '9')
    {
        outChar = c - '0';
    }else if(c >= 'a' && c <= 'f')
    {
        outChar = c - 87;
    }else if(c >= 'A' && c <= 'F')
    {
        outChar = c - 55;
    }else{
        return 0;
    }
    outChar <<= 4;
    c = inStr[3];
    if(c >= '0' && c <= '9')
    {
        outChar |= c - '0';
    }else if(c >= 'a' && c <= 'f')
    {
        outChar |= c - 87;
    }else if(c >= 'A' && c <= 'F')
    {
        outChar |= c - 55;
    }else{
        return 0;
    }
    return 4;//  \xHH
}


AIST::AST_type aist_parser_T::parse_type()
{
//контроль что тип идет в is_data_type
    aist_lexeme_T lex = m_lexer.m_cur_lexeme;
    AIST::AST_type type = AIST::AST_type::AST_NULL;
    if(lex.m_len == 4 && memcmp(lex.m_value, "void", 4) == 0)
        type = AIST::AST_type::AST_VOID;
    else if(lex.m_len == 6 && memcmp(lex.m_value, "string", 6) == 0)
        type = AIST::AST_type::AST_STRING;
    else if(lex.m_len == 4 && memcmp(lex.m_value, "char", 4) == 0)
        type = AIST::AST_type::AST_UCHAR;
    else if(lex.m_len == 3 && memcmp(lex.m_value, "int", 3) == 0)
        type = AIST::AST_type::AST_INTEGER;
    else if(lex.m_len == 5 && memcmp(lex.m_value, "float", 4) == 0)
        type = AIST::AST_type::AST_FLOAT;
    else if(lex.m_len == 4 && memcmp(lex.m_value, "bool", 4) == 0)
        type = AIST::AST_type::AST_BOOLEAN;
    else if(lex.m_len == 6 && memcmp(lex.m_value, "object", 6) == 0)
        type = AIST::AST_type::AST_OBJECT;
    else if(lex.m_len == 4 && memcmp(lex.m_value, "list", 4) == 0)
        type = AIST::AST_type::AST_LIST;
    else if(lex.m_len == 3 && memcmp(lex.m_value, "any", 3) == 0)
        type = AIST::AST_type::AST_ANY;
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    return type;
}


static bool is_data_type(aist_lexeme_T lex)
{
    if(lex.m_len == 3)
        return (memcmp(lex.m_value, "int", 3) == 0) || (memcmp(lex.m_value, "any", 3) == 0);
    if(lex.m_len == 4)
    {
        return (
            memcmp(lex.m_value, "void", 4) == 0 ||
            memcmp(lex.m_value, "char", 4) == 0 ||
            memcmp(lex.m_value, "bool", 4) == 0 ||
            memcmp(lex.m_value, "list", 4) == 0);
    }
    if(lex.m_len == 5)
        return (memcmp(lex.m_value, "float", 5) == 0) || (memcmp(lex.m_value, "const", 5) == 0);
    if(lex.m_len == 6)
        return (memcmp(lex.m_value, "string", 6) == 0) || (memcmp(lex.m_value, "object", 6) == 0);
    return false;
}


static bool is_correct_name(aist_lexeme_T lex)
{
    if(is_data_type(lex))
        return false;
    if(lex.m_len == 8 && memcmp(lex.m_value, "continue", 8) == 0)
        return false;
    if(lex.m_len == 7 && memcmp(lex.m_value, "include", 7) == 0)
        return false;
    if(lex.m_len == 6)
    {
        if(memcmp(lex.m_value, "delete", 6) == 0)
            return false;
        if(memcmp(lex.m_value, "return", 6) == 0)
            return false;
    }
    if(lex.m_len == 5)
    {
        if(memcmp(lex.m_value, "false", 5) == 0)
            return false;
        if(memcmp(lex.m_value, "while", 5) == 0)
            return false;
        if(memcmp(lex.m_value, "break", 5) == 0)
            return false;
    }
    if(lex.m_len == 4)
    {
        if(memcmp(lex.m_value, "true", 4) == 0)
            return false;
        if(memcmp(lex.m_value, "NULL", 4) == 0)
            return false;
        if(memcmp(lex.m_value, "else", 4) == 0)
            return false;
    }
    if(lex.m_len == 3)
    {
        if(memcmp(lex.m_value, "new", 3) == 0)
            return false;
        if(memcmp(lex.m_value, "for", 3) == 0)
            return false;
    }
    if(lex.m_len == 2 && memcmp(lex.m_value, "if", 2) == 0)
        return false;
    return true;
}


void aist_parser_T::_unexpected_lexeme_error()
{
    char tmpStr[64];
    size_t len = m_lexer.m_cur_lexeme.m_len;
    if(len > 63)
        len = 63;
    aist_os_memcpy(tmpStr, m_lexer.m_cur_lexeme.m_value, len);
    tmpStr[len] = 0;
    m_lexer.lex_error("Unexpected lexeme '%s'.", tmpStr);
}


inline aist_ast_T* aist_parser_T::create_ast(AIST::AST_type type)
{
    aist_ast_T* node = m_runtime->ast_create(type, 0);
    if(AIST::AST_type::AST_STRING != type)
    {
        int p = int(m_lexer.m_prev_lexeme.m_pos - m_lexer.m_state.m_line_pos);
        if(p < 0)
            p = 0;
        if(p > 0xFFF)
            p = 0xFFF;
        node->m_line_pos12  = (unsigned int)((m_lexer.m_state.m_line<<12)|p);
    }
    return node;
}


inline aist_ast_list_T* aist_parser_T::create_list()
{
    return m_runtime->list_create(4, true);
}


aist_ast_T* aist_parser_T::parser_eat(AIST::LEXEME_type lex_type)
{
    if(m_lexer.m_cur_lexeme.m_lex_type != lex_type &&
       !(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EOF && lex_type ==  AIST::LEXEME_type::LEXEME_SEMI))
        _unexpected_lexeme_error();
    if(m_lexer.m_cur_lexeme.m_lex_type == lex_type)
    {
        m_lexer.m_prev_lexeme = m_lexer.m_cur_lexeme;
        m_lexer.m_prev_state  = m_lexer.m_state;
        m_lexer.m_cur_lexeme  = m_lexer.get_next_lexeme();
    }
    return NULL;
}


aist_ast_T* aist_parser_T::parse_array_access(aist_ast_T* left, bool getOnly)
{
    aist_ast_T* ast_list_access = create_ast(AIST::AST_type::AST_ARRAY_ACCESS);
    ast_list_access->m_arr_access->m_left = left;
    parser_eat(AIST::LEXEME_type::LEXEME_LBRACKET);
    ast_list_access->m_arr_access->m_pointer = parse_expression();
    parser_eat(AIST::LEXEME_type::LEXEME_RBRACKET);
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EQUALS) // '='
    {
        if(getOnly)
            _unexpected_lexeme_error();
        parser_eat(AIST::LEXEME_type::LEXEME_EQUALS);
        ast_list_access->m_arr_access->m_right = parse_expression();
    }else if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN) // '('
    {
        aist_ast_T*  n = create_ast(AIST::AST_type::AST_VARIABLE);
#ifdef AIST_USE_HASH_NAME
        n->m_variable_name_hash = 0;
#else
        n->m_variable_name = aist_str_clone(m_lexer.m_mem, 0, 0);
#endif
        ast_list_access->m_arr_access->m_right = parse_function_call(n);
    }else{
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS_EQUALS ||
           m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS_EQUALS)
        {
            AIST::LEXEME_type oper_type;
            if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS_EQUALS)
            {
                oper_type = AIST::LEXEME_type::LEXEME_PLUS;
            }else{
                oper_type = AIST::LEXEME_type::LEXEME_MINUS;
            }
            parser_eat(m_lexer.m_cur_lexeme.m_lex_type);

            aist_ast_T* ast_list_access_2 = create_ast(AIST::AST_type::AST_ARRAY_ACCESS);
            ast_list_access_2->m_arr_access->m_left = ast_list_access->m_arr_access->m_left;
            ast_list_access_2->m_arr_access->m_pointer = ast_list_access->m_arr_access->m_pointer;

             aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
             ast_binop->m_binop->m_left = ast_list_access_2;
             ast_binop->m_binop->m_operator = oper_type;
             ast_binop->m_binop->m_right = parse_expression();
             ast_list_access->m_arr_access->m_right = ast_binop;
        }
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS_PLUS ||
           m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS_MINUS)
        {
            AIST::LEXEME_type oper_type;
            AIST::LEXEME_type oper = m_lexer.m_cur_lexeme.m_lex_type;
            if(oper == AIST::LEXEME_type::LEXEME_PLUS_PLUS)
            {
                oper_type = AIST::LEXEME_type::LEXEME_PLUS;
            } else{
                oper_type = AIST::LEXEME_type::LEXEME_MINUS;
            }
            parser_eat(oper);
            aist_ast_T* ast_list_access_2 = create_ast(AIST::AST_type::AST_ARRAY_ACCESS);
            ast_list_access_2->m_arr_access->m_left    = ast_list_access->m_arr_access->m_left;
            ast_list_access_2->m_arr_access->m_pointer = ast_list_access->m_arr_access->m_pointer;
            //x = x+1
            aist_ast_T* ast_binop2 = create_ast(AIST::AST_type::AST_BINOP);
            ast_binop2->m_binop->m_left = ast_list_access;
            ast_binop2->m_binop->m_operator = oper_type;
            ast_binop2->m_binop->m_right = create_ast(AIST::AST_type::AST_INTEGER);
            ast_binop2->m_binop->m_right->m_int_value = 1;
            ast_list_access_2->m_arr_access->m_right = ast_binop2;
            // ret x
            aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
            ast_binop->m_binop->m_right = ast_list_access_2;
            ast_binop->m_binop->m_operator = oper;
            ast_binop->m_binop->m_left = ast_list_access;
            return ast_binop;
        }
    }


    return ast_list_access;
}


aist_ast_T* aist_parser_T::parse_block_with_one_statement()
{
    aist_ast_T* block  = create_ast(AIST::AST_type::AST_BLOCK);
    aist_ast_T* statement = parse_statement();
    if(m_lexer.m_cur_lexeme.m_lex_type >= AIST::LEXEME_type::LEXEME_PLUS  && //all oper
       m_lexer.m_cur_lexeme.m_lex_type <= AIST::LEXEME_type::LEXEME_MINUS)
    {
        statement = parse_expression(statement);
    }
   // parser_eat(AIST::LEXEME_type::LEXEME_SEMI);
    block->m_block_list->push(statement);
    return block;
}


aist_ast_T* aist_parser_T::parse_expression(aist_ast_T* node)
{
    node = parse_term_l5(node);
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BOOLEAN_AND || // '&&'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BOOLEAN_OR)    // '||'
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);
        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_term_l5(NULL);
        ast_binop->m_binop->m_operator = binop_operator;
        node = ast_binop;
    }
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_QUESTION) // '?'   //Example:  x > 2 ? 5 : 3
    {
        aist_ast_T* ternary = create_ast(AIST::AST_type::AST_TERNARY);
        ternary->m_ternary->m_expr = node;
        parser_eat(AIST::LEXEME_type::LEXEME_QUESTION); // '?'
        ternary->m_ternary->m_body = parse_expression(NULL);
        parser_eat(AIST::LEXEME_type::LEXEME_COLON);    // ':'
        ternary->m_ternary->m_else_body = parse_expression(NULL);
        return ternary;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_block_statements()
{
    aist_ast_T* block  = create_ast(AIST::AST_type::AST_BLOCK);
    aist_ast_T* statement = parse_statement();
    block->m_block_list->push(statement);

    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_SEMI || statement->m_ast_type != AIST::AST_type::AST_NULL)
    {
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EOF)
            break;
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_RBRACE) // '}'
            break;
        if(m_lexer.m_prev_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_RBRACE || m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_SEMI)
        {
            if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_SEMI)
            {
                char tmpStr[64];
                size_t len = m_lexer.m_cur_lexeme.m_len;
                if(len > 63)
                    len = 63;
                aist_os_memcpy(tmpStr, m_lexer.m_cur_lexeme.m_value, len);
                tmpStr[len] = 0;
                m_lexer.lex_error("Unexpected lexeme '%s'. (expected ';')", tmpStr);
            }
            parser_eat(AIST::LEXEME_type::LEXEME_SEMI);
        }
        statement = parse_statement();
        if(m_lexer.m_cur_lexeme.m_lex_type >= AIST::LEXEME_type::LEXEME_PLUS  &&
           m_lexer.m_cur_lexeme.m_lex_type <= AIST::LEXEME_type::LEXEME_MINUS)
        {
            statement = parse_expression(statement);
        }
        if(statement->m_ast_type != AIST::AST_type::AST_NULL)
            block->m_block_list->push(statement);
    }
    return block;
}


aist_ast_T* aist_parser_T::parse_float()
{
    aist_ast_T* ast_float = create_ast(AIST::AST_type::AST_FLOAT);
    char tmp[64];
    size_t len = m_lexer.m_cur_lexeme.m_len;
    if(len > 63)
        len = 63;
    aist_os_memcpy(tmp, m_lexer.m_cur_lexeme.m_value, len);
    tmp[len] = 0;
    ast_float->m_float_value = (aist_float)atof(tmp);
    parser_eat(AIST::LEXEME_type::LEXEME_FLOAT_VALUE);
    return ast_float;
}


aist_ast_T* aist_parser_T::parse_string()
{
    aist_ast_T* ast_string = create_ast(AIST::AST_type::AST_STRING);
    size_t test_len = m_lexer.m_cur_lexeme.m_len;
    const char* test_str = m_lexer.m_cur_lexeme.m_value;
    char temp_c = 0;

    for(size_t i = 0; i < m_lexer.m_cur_lexeme.m_len; i++)
    {
        if(test_str[i] == '\\')
        {
            if(test_str[i+1] == 'x')
            {
                if(!aist_parser_char_code(test_str+i, 4, temp_c))
                    m_lexer.lex_error("Chars can only contain one character");
                test_len -= 3;
            }else{
                if(!aist_parser_char_code(test_str+i, 2, temp_c))
                    m_lexer.lex_error("Chars can only contain one character");
                test_len -= 1;
            }
        }
    }
    char* new_str = (char*)m_lexer.m_mem->Alloc(test_len+1);
    if(test_len ==  m_lexer.m_cur_lexeme.m_len)
    {
        aist_os_memcpy(new_str, test_str, test_len);
    }else{
        test_len = 0;
        for(size_t i = 0; i < m_lexer.m_cur_lexeme.m_len; i++)
        {
            if(test_str[i] == '\\')
            {
                if(test_str[i+1] == 'x')
                {
                    if(!aist_parser_char_code(test_str+i, 4, temp_c))
                        m_lexer.lex_error("Chars can only contain one character");
                    i += 3;
                    new_str[test_len ++] = temp_c;
                }else{
                    if(!aist_parser_char_code(test_str+i, 2, temp_c))
                        m_lexer.lex_error("Chars can only contain one character");
                    i += 1;
                    new_str[test_len ++] = temp_c;
                }
                continue;
            }
            new_str[test_len ++] = test_str[i];
        }
    }
    new_str[test_len] = 0;
    ast_string->m_string_value_len = (unsigned int)test_len;
    ast_string->m_string_value     = new_str;

    parser_eat(AIST::LEXEME_type::LEXEME_STRING_VALUE);
    return ast_string;
}


aist_ast_T* aist_parser_T::parse_uchar()
{
    aist_ast_T* ast_string = create_ast(AIST::AST_type::AST_UCHAR);
    if(m_lexer.m_cur_lexeme.m_len == 1)
    {
        ast_string->m_char_value = m_lexer.m_cur_lexeme.m_value[0];
    }else{
        char outChar = 0;
        if(m_lexer.m_cur_lexeme.m_len == 0 || aist_parser_char_code(m_lexer.m_cur_lexeme.m_value, m_lexer.m_cur_lexeme.m_len, outChar) != m_lexer.m_cur_lexeme.m_len)
            m_lexer.lex_error("Chars can only contain one character");
         ast_string->m_char_value = outChar;
    }
    parser_eat(AIST::LEXEME_type::LEXEME_CHAR_VALUE);
    return ast_string;
}


aist_ast_T* aist_parser_T::parse_integer()
{
    aist_ast_T* ast_integer = create_ast(AIST::AST_type::AST_INTEGER);
    char tmp[64];
    size_t len = m_lexer.m_cur_lexeme.m_len;
    if(len > 63)
        len = 63;
    aist_os_memcpy(tmp, m_lexer.m_cur_lexeme.m_value, len);
    tmp[len] = 0;

    if(len > 2 && tmp[1] == 'x')
    {
        if(len > 2+sizeof(aist_int)*2)
            m_lexer.lex_error("Value hex %s very big", tmp);
        ast_integer->m_int_value = 0;
        for(size_t i = 2; i < len; i++)
        {
            size_t c = tmp[i] - '0';
            if(c > 9)// A..F
                c -= 7;
            if(c > 15)// a..f
                c -= 32;
            ast_integer->m_int_value <<= 4;
            ast_integer->m_int_value |= c;
        }
    }else{
        for(size_t i = 0; i < len; i++)
        {
            size_t v = tmp[i] - '0';
            ast_integer->m_int_value = ast_integer->m_int_value*10 + v;
        }
    }
    parser_eat(AIST::LEXEME_type::LEXEME_INTEGER_VALUE);
    return ast_integer;
}


aist_ast_T* aist_parser_T::parse_boolean()
{
    aist_ast_T* ast_boolean = create_ast(AIST::AST_type::AST_BOOLEAN);
    
    if(m_lexer.m_cur_lexeme.m_len == 5 && memcmp(m_lexer.m_cur_lexeme.m_value, "false", 5) == 0)
        ast_boolean->m_boolean_value = 0; 
    else if(m_lexer.m_cur_lexeme.m_len == 4 && memcmp(m_lexer.m_cur_lexeme.m_value, "true", 4) == 0)
        ast_boolean->m_boolean_value = 1;
    else{
        char tmpStr[64];
        size_t len = m_lexer.m_cur_lexeme.m_len;
        if(len > 63)
            len = 63;
        aist_os_memcpy(tmpStr, m_lexer.m_cur_lexeme.m_value, len);
        tmpStr[len] = 0;
        m_lexer.lex_error("%s is not a boolean value", tmpStr);
    }
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    return ast_boolean;
}


aist_ast_T* aist_parser_T::parse_variable()
{
    aist_ast_T* ast_variable = create_ast(AIST::AST_type::AST_VARIABLE);
#ifdef AIST_USE_HASH_NAME
    ast_variable->m_variable_name_hash = aist_str_hash(m_lexer.m_prev_lexeme.m_value, m_lexer.m_prev_lexeme.m_len);
#else
    ast_variable->m_variable_name = aist_str_clone(m_lexer.m_mem, m_lexer.m_prev_lexeme.m_value, m_lexer.m_prev_lexeme.m_len);
#endif
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EQUALS) // '='
    {
        parser_eat(AIST::LEXEME_type::LEXEME_EQUALS);
        aist_ast_T* ast_modifier = create_ast(AIST::AST_type::AST_VARIABLE_MODIFIER);
        ast_modifier->m_var_modif->m_left  = ast_variable;
        ast_modifier->m_var_modif->m_right = parse_expression();
        return ast_modifier;
    }
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS_PLUS ||
       m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS_MINUS ) // '++'  '--'
    {
        AIST::LEXEME_type oper = m_lexer.m_cur_lexeme.m_lex_type;
        AIST::LEXEME_type oper2 = AIST::LEXEME_type::LEXEME_PLUS;
        if(oper == AIST::LEXEME_type::LEXEME_MINUS_MINUS)
            oper2 = AIST::LEXEME_type::LEXEME_MINUS;
        parser_eat(m_lexer.m_cur_lexeme.m_lex_type);
        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_left = ast_variable;
        ast_binop->m_binop->m_operator = oper;
        ast_binop->m_flags |= AIST::FLAG_RESTORE_THIS;
        // x=x+1
        aist_ast_T* ast_modifier = create_ast(AIST::AST_type::AST_VARIABLE_MODIFIER);
        ast_modifier->m_var_modif->m_left = ast_variable;
        ast_variable->m_countUse++;
        aist_ast_T* ast_binop2 = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop2->m_binop->m_left = ast_variable;
        ast_binop2->m_binop->m_operator = oper2;

        ast_binop2->m_binop->m_right = create_ast(AIST::AST_type::AST_INTEGER);
        ast_binop2->m_binop->m_right->m_int_value = 1;

        ast_modifier->m_var_modif->m_right = ast_binop2;
        ast_modifier->m_flags |= AIST::FLAG_RESTORE_THIS;
        ast_binop->m_binop->m_right = ast_modifier;
        return ast_binop;
    }
    AIST::LEXEME_type oper_type = AIST::LEXEME_type::LEXEME_EOF;
    switch(m_lexer.m_cur_lexeme.m_lex_type)
    {
        case AIST::LEXEME_type::LEXEME_PLUS_EQUALS:  oper_type = AIST::LEXEME_type::LEXEME_PLUS; break;  // '+='
        case AIST::LEXEME_type::LEXEME_MINUS_EQUALS: oper_type = AIST::LEXEME_type::LEXEME_MINUS; break; // '-='
    }

    if(oper_type != AIST::LEXEME_type::LEXEME_EOF)
    {
        parser_eat(m_lexer.m_cur_lexeme.m_lex_type);
        aist_ast_T* ast_modifier = create_ast(AIST::AST_type::AST_VARIABLE_MODIFIER); // '='
        ast_modifier->m_var_modif->m_left = ast_variable;
        ast_variable->m_countUse++;
        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_left = ast_variable;
        ast_binop->m_binop->m_operator = oper_type;
        ast_binop->m_binop->m_right = parse_expression();
        ast_modifier->m_var_modif->m_right = ast_binop;
        ast_modifier->m_flags |= AIST::FLAG_RESTORE_THIS;
        return ast_modifier;
    }
    return ast_variable;
}


aist_ast_T* aist_parser_T::parse_object()
{
    aist_ast_T* ast_object = create_ast(AIST::AST_type::AST_OBJECT);
    ast_object->m_object_children = create_list();
    parser_eat(AIST::LEXEME_type::LEXEME_LBRACE);

    if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_RBRACE) // '}'
    {
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME)
            ast_object->m_object_children->push(parse_definition(true));
        while(m_lexer.m_prev_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_RBRACE &&
              m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME) // '}' без ';' на конце
            ast_object->m_object_children->push(parse_definition(true));
        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_SEMI)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_SEMI);

            if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME)
                ast_object->m_object_children->push(parse_definition(true));
            while(m_lexer.m_prev_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_RBRACE && 
                  m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME)// '}' без ';' на конце
                ast_object->m_object_children->push(parse_definition(true));
        }
    }
    parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); //' }'
    return ast_object;
}


aist_ast_T* aist_parser_T::parse_list()
{
    parser_eat(AIST::LEXEME_type::LEXEME_LBRACKET); // '['
    aist_ast_T* ast_list = create_ast(AIST::AST_type::AST_LIST);
    ast_list->m_list_children = create_list();

    if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_RBRACKET) // ']'
        ast_list->m_list_children->push(parse_expression());

    // Parsing list items
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_COMMA) // ','
    {
        parser_eat(AIST::LEXEME_type::LEXEME_COMMA);
        ast_list->m_list_children->push(parse_expression());
    }
    parser_eat(AIST::LEXEME_type::LEXEME_RBRACKET); // ']'
    return ast_list;
}


aist_ast_T* aist_parser_T::parse_new()
{
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    aist_ast_T* ast_new = create_ast(AIST::AST_type::AST_NEW);
    ast_new->m_new_del_incl_right = parse_expression();
    return ast_new;
}


aist_ast_T* aist_parser_T::parse_delete()
{
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    aist_ast_T* ast_delete = create_ast(AIST::AST_type::AST_DELETE);
    ast_delete->m_new_del_incl_right = parse_expression();
    return ast_delete;
}


aist_ast_T* aist_parser_T::parse_factor()
{
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS  ||
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS ||
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NOT   ||
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BINARY_NOT)
    {
        aist_lexeme_T lex = m_lexer.m_cur_lexeme;
        parser_eat(lex.m_lex_type);
        aist_ast_T* ast_unop = create_ast(AIST::AST_type::AST_UNOP);
        ast_unop->m_unop->m_operator = lex.m_lex_type;
        ast_unop->m_unop->m_right    = parse_factor();
        return ast_unop;
    }
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME)
    {
        if((m_lexer.m_cur_lexeme.m_len == 5 && memcmp(m_lexer.m_cur_lexeme.m_value, "false", 5) == 0) ||
           (m_lexer.m_cur_lexeme.m_len == 4 && memcmp(m_lexer.m_cur_lexeme.m_value, "true", 4) == 0))
            return parse_boolean();

        if(m_lexer.m_cur_lexeme.m_len == 4 && memcmp(m_lexer.m_cur_lexeme.m_value, "NULL", 4) == 0)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_NAME);
            return m_runtime->m_ast_null;
        }
        if(m_lexer.m_cur_lexeme.m_len == 3 && memcmp(m_lexer.m_cur_lexeme.m_value, "new", 3) == 0)
            return parse_new();

        parser_eat(AIST::LEXEME_type::LEXEME_NAME);
        aist_ast_T* a = parse_variable();
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_DOT) // '.'
        {
            parser_eat(AIST::LEXEME_type::LEXEME_DOT); // '.'
            aist_ast_T* ast = create_ast(AIST::AST_type::AST_ATTRIBUTE_ACCESS);
            ast->m_attribute->m_left = a;
            ast->m_attribute->m_right = parse_factor();
            if(ast->m_attribute->m_right->m_ast_type <= AIST::AST_type::AST_ANY)
                m_lexer.lex_error("Incorrect attribute");
            a = ast;
        }

        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACKET || // '['
              m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN)     // '('
        {
            if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACKET) // '['
            {
                a = parse_array_access(a, true);
            }else{
                a = parse_function_call(a);
            }
        }
        if(a)
            return a;
    }

    // This is to be able to do things like: 1 * (5 + 5) * (5 * 1) + 3
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN) // '('
    {
        parser_eat(AIST::LEXEME_type::LEXEME_LPAREN); // '('
        aist_ast_T* ast_expr = parse_expression();
        parser_eat(AIST::LEXEME_type::LEXEME_RPAREN); // ')'
        return ast_expr;
    } 

    switch(m_lexer.m_cur_lexeme.m_lex_type)
    {
        case AIST::LEXEME_type::LEXEME_INTEGER_VALUE: return parse_integer(); break;
        case AIST::LEXEME_type::LEXEME_FLOAT_VALUE:  return parse_float();  break;
        case AIST::LEXEME_type::LEXEME_STRING_VALUE: return parse_string(); break;
        case AIST::LEXEME_type::LEXEME_CHAR_VALUE:   return parse_uchar();  break;
        case AIST::LEXEME_type::LEXEME_LBRACE:   return parse_object(); break; // '{'
        case AIST::LEXEME_type::LEXEME_LBRACKET: return parse_list();   break; // '['
        default:
            {
                char tmpStr[64];
                size_t len = m_lexer.m_cur_lexeme.m_len;
                if(len > 63)
                    len = 63;
                aist_os_memcpy(tmpStr, m_lexer.m_cur_lexeme.m_value, len);
                tmpStr[len] = 0;
                m_lexer.lex_error("Unexpected '%s'", tmpStr);
            }break;
    }
    return NULL;
}


aist_ast_T* aist_parser_T::parse_term_l1(aist_ast_T* node)//level 1
{
    if(is_data_type(m_lexer.m_cur_lexeme)) //для возможности определения переменных внутри скобок прототипа функции 
        return parse_definition(true);
    if(!node)
        node = parse_factor();
    aist_ast_T* ast_binop = NULL;

    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN)// '('
        node = parse_function_call(node);

    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_DOT) // '.'
    {
        parser_eat(AIST::LEXEME_type::LEXEME_DOT);
        aist_ast_T* ast_new = create_ast(AIST::AST_type::AST_ATTRIBUTE_ACCESS);
        ast_new->m_attribute->m_left = node;
        ast_new->m_attribute->m_right = parse_expression();
        node = ast_new;
    }

    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_STAR       || // '*'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PERCENTAGE || // '%'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_DIV)          // '/'
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);

        ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_operator = binop_operator;
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_factor();
        node = ast_binop;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_term_l2(aist_ast_T* node)//level 2
{
    node = parse_term_l1(node);
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS || // '-'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS)  // '+'
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);
        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_operator = binop_operator;
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_term_l1(NULL);
        node = ast_binop;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_term_l3(aist_ast_T* node)//level 3
{
    node = parse_term_l2(node);
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_SHIFT_LEFT || // '<<'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_SHIFT_RIGHT)  // '>>'
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);

        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_operator = binop_operator;
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_term_l2(NULL);
        node = ast_binop;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_term_l4(aist_ast_T* node)//level 4
{
    node = parse_term_l3(node);
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BINARY_AND || // '&'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BINARY_XOR || // '^'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BINARY_OR)    // '|'
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);

        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_operator = binop_operator;
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_term_l3(NULL);
        node = ast_binop;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_term_l5(aist_ast_T* node)//level 5
{
    node = parse_term_l4(node);
    while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LESS          || // '<' 
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LEQUAL        || // '<='
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LARGER        || // '>'
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LAQUAL        || // '>='
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EQUALS_EQUALS || // '=='
          m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NOT_EQUALS)      // '!='
    {
        AIST::LEXEME_type binop_operator = m_lexer.m_cur_lexeme.m_lex_type;
        parser_eat(binop_operator);
        aist_ast_T* ast_binop = create_ast(AIST::AST_type::AST_BINOP);
        ast_binop->m_binop->m_operator = binop_operator;
        ast_binop->m_binop->m_left  = node;
        ast_binop->m_binop->m_right = parse_term_l4(NULL);
        node = ast_binop;
    }
    return node;
}


aist_ast_T* aist_parser_T::parse_return()
{
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    aist_ast_T* ast_return = create_ast(AIST::AST_type::AST_RETURN);
    if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_SEMI)
        ast_return->m_return->m_init_value = parse_expression();
    return ast_return;
}


aist_ast_T* aist_parser_T::parse_if()
{
    aist_ast_T* ast_if = create_ast(AIST::AST_type::AST_IF);
    
    parser_eat(AIST::LEXEME_type::LEXEME_NAME); // IF lex

    parser_eat(AIST::LEXEME_type::LEXEME_LPAREN); // '('
    ast_if->m_if->m_if_expr = parse_expression();
    parser_eat(AIST::LEXEME_type::LEXEME_RPAREN); // ')'
    
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACE)
    {
        parser_eat(AIST::LEXEME_type::LEXEME_LBRACE); // '{'
        ast_if->m_if->m_if_body = parse_block_statements();
        parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); // '}'
    }else{ // accept if-statement without braces. (will only parse one statement)
        ast_if->m_if->m_if_body = parse_block_with_one_statement();
    }

    if(m_lexer.m_cur_lexeme.m_len == 4 && memcmp(m_lexer.m_cur_lexeme.m_value, "else", 4) == 0)
    {
        parser_eat(AIST::LEXEME_type::LEXEME_NAME); // ELSE lexeme
        if(m_lexer.m_cur_lexeme.m_len == 2 && memcmp(m_lexer.m_cur_lexeme.m_value, "if", 2) == 0)
        {
            ast_if->m_if->m_if_otherwise = parse_if();
        }else{
            if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACE)
            {
                parser_eat(AIST::LEXEME_type::LEXEME_LBRACE); // '{'
                ast_if->m_if->m_else_body = parse_block_statements();
                parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); // '}'
            }else{
                ast_if->m_if->m_else_body = parse_block_with_one_statement();
            }
        }
    }
    return ast_if;
}


aist_ast_T* aist_parser_T::parse_while()
{
    parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    parser_eat(AIST::LEXEME_type::LEXEME_LPAREN);  // '('
    aist_ast_T* ast_while = create_ast(AIST::AST_type::AST_WHILE);
    ast_while->m_while->m_test_expr = parse_expression();  // boolean expression
    parser_eat(AIST::LEXEME_type::LEXEME_RPAREN);  // ')'

    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACE)
    {
        parser_eat(AIST::LEXEME_type::LEXEME_LBRACE); // '{'
        ast_while->m_while->m_body = parse_block_statements();
        parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); // '}'
    }else{
        ast_while->m_while->m_body = parse_block_with_one_statement();
    }
    return ast_while;
}


aist_ast_T* aist_parser_T::parse_for()
{
    aist_ast_T* ast_for = create_ast(AIST::AST_type::AST_FOR);

    parser_eat(AIST::LEXEME_type::LEXEME_NAME); // for
    parser_eat(AIST::LEXEME_type::LEXEME_LPAREN); // '('
    ast_for->m_for->m_init_statement = parse_statement();
    parser_eat(AIST::LEXEME_type::LEXEME_SEMI);  // ';'
    ast_for->m_for->m_test_expr = parse_expression();
    parser_eat(AIST::LEXEME_type::LEXEME_SEMI);  // ';'
    ast_for->m_for->m_update_statement = parse_statement();
    parser_eat(AIST::LEXEME_type::LEXEME_RPAREN); // ')'
    
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACE)
    {
        parser_eat(AIST::LEXEME_type::LEXEME_LBRACE); // '{'
        ast_for->m_for->m_body = parse_block_statements();
        parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); // '}'
    }else{
        ast_for->m_for->m_body = parse_block_with_one_statement();
    }
    return ast_for;
}


aist_ast_T* aist_parser_T::parse_function_call(aist_ast_T* variable_name)
{
    aist_ast_T* ast_function_call = create_ast(AIST::AST_type::AST_FUNCTION_CALL);
    ast_function_call->m_fn_call->m_name = variable_name;
    if(!is_correct_name(m_lexer.m_prev_lexeme))
        m_lexer.lex_error("Incorrect variable definition '" AIST_PR_NAME_HASH "'", AIST_NAME_HASH(variable_name->m_variable_name));
    parser_eat(AIST::LEXEME_type::LEXEME_LPAREN); // '('

    if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_RPAREN) // ')'
    {
        aist_ast_T* ast_expr = parse_expression();
        ast_function_call->m_fn_call->m_arguments->push(ast_expr);
        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_COMMA) // ','
        {
            parser_eat(AIST::LEXEME_type::LEXEME_COMMA);
            ast_expr = parse_expression();
            ast_function_call->m_fn_call->m_arguments->push(ast_expr);
        }
    }
    parser_eat(AIST::LEXEME_type::LEXEME_RPAREN); // ')'
    return ast_function_call;
}


aist_ast_T* aist_parser_T::parse_definition(bool setFnParamInit)
{
    size_t addFlag = 0;
    if(m_lexer.m_cur_lexeme.m_len == 5 && memcmp(m_lexer.m_cur_lexeme.m_value, "const", 5) == 0)
    {
        parser_eat(AIST::LEXEME_type::LEXEME_NAME);
        addFlag = AIST::FLAG_CONST_VAR;
    }

    AIST::AST_type data_type = parse_type();
    if(data_type == AIST::AST_type::AST_NULL)
        m_lexer.lex_error("Incorrect variable type");
    size_t name_len = m_lexer.m_cur_lexeme.m_len;
    char* name  = aist_str_clone(m_lexer.m_mem, m_lexer.m_cur_lexeme.m_value, name_len);
    bool isName = false;
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME)
    {
        isName = is_correct_name(m_lexer.m_cur_lexeme);
        parser_eat(AIST::LEXEME_type::LEXEME_NAME);
    }
   
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN)// '('
    {
        if(!setFnParamInit)
            m_lexer.lex_error("Incorrect variable definition '%s'", name);
        aist_ast_T* ast_function_definition = create_ast(AIST::AST_type::AST_FUNCTION_DEFINITION);
        if(m_lexer.m_cur_lexeme.m_len == 1 && name[0] == '(')// анонимный
            name[0] = '\0';
        ast_function_definition->m_fn_def->m_label_src = aist_str_clone(m_lexer.m_mem, m_lexer.m_label_src);
#ifdef AIST_USE_HASH_NAME
        if(name[0] == 0)
        {
            ast_function_definition->m_fn_def->m_function_name_hash = 0;
        }else{
            ast_function_definition->m_fn_def->m_function_name_hash = aist_str_hash(name, name_len);
        }
#else
        ast_function_definition->m_fn_def->m_function_name = name;
#endif
        ast_function_definition->m_fn_def->m_return_type = data_type;
        ast_function_definition->m_fn_def->m_arguments   = create_list();

        parser_eat(AIST::LEXEME_type::LEXEME_LPAREN); // '('
        
        // Parsing function definition arguments.
        if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_RPAREN) // ')'
        {
            aist_ast_T* var_def = parse_definition(false);
            if(var_def->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION || var_def->m_var_def->m_init_value)
                m_lexer.lex_error("Incorrect variable definition '%s'", name);
            ast_function_definition->m_fn_def->m_arguments->push(var_def);
            while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_COMMA)
            {
                parser_eat(AIST::LEXEME_type::LEXEME_COMMA); // ','
                aist_ast_T* var_def = parse_definition(false);
                if(var_def->m_ast_type != AIST::AST_type::AST_VARIABLE_DEFINITION || var_def->m_var_def->m_init_value)
                    m_lexer.lex_error("Incorrect variable definition '%s'", name);
                ast_function_definition->m_fn_def->m_arguments->push(var_def);
            }
        } 
        parser_eat(AIST::LEXEME_type::LEXEME_RPAREN); // ')'
        parser_eat(AIST::LEXEME_type::LEXEME_LBRACE); // '{'

        ast_function_definition->m_fn_def->m_body = parse_block_statements();
        ast_function_definition->m_fn_def->m_body->m_flags |= AIST::FLAG_BLOCK_IS_FN;
        parser_eat(AIST::LEXEME_type::LEXEME_RBRACE); // '}'
#ifdef AIST_USE_HASH_NAME
        aist_str_free(m_lexer.m_mem, name, name_len);
#endif
        return ast_function_definition;
    }else{
        if(!isName)
            m_lexer.lex_error("Incorrect variable name '%s'", name);
        aist_ast_T* ast_variable_definition = create_ast(AIST::AST_type::AST_VARIABLE_DEFINITION);
#ifdef AIST_USE_HASH_NAME
        ast_variable_definition->m_var_def->m_name_hash = aist_str_hash(name, name_len);
#else
        ast_variable_definition->m_var_def->m_name = name;
#endif
        ast_variable_definition->m_var_def->m_type = data_type;
        ast_variable_definition->m_flags |= addFlag;
       
        if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EQUALS) // '='
        {
            if(!setFnParamInit)
                m_lexer.lex_error("Incorrect variable definition '%s'", name);

            parser_eat(AIST::LEXEME_type::LEXEME_EQUALS);
            ast_variable_definition->m_var_def->m_init_value = parse_expression();
            ast_variable_definition->m_var_def->m_init_value->m_flags |= addFlag;
            if(data_type != AIST::AST_type::AST_ANY && ast_variable_definition->m_var_def->m_init_value->m_ast_type < AIST::AST_type::AST_ANY)
            {
                if(ast_variable_definition->m_var_def->m_init_value->m_ast_type != data_type)
                    m_lexer.lex_error("Invalid type for assigned value");
            }
            if(m_lexer.m_cur_lexeme.m_lex_type != AIST::LEXEME_type::LEXEME_SEMI) // ';' отдаем выше. если нет вызываем ошибку
                parser_eat(AIST::LEXEME_type::LEXEME_SEMI);
#ifdef AIST_USE_HASH_NAME
            aist_str_free(m_lexer.m_mem, name, name_len);
#endif
            return ast_variable_definition;
        }
#ifdef AIST_USE_HASH_NAME
        aist_str_free(m_lexer.m_mem, name, name_len);
#endif
//type name;
        if(!setFnParamInit)
            return ast_variable_definition;
        if(data_type == AIST::AST_type::AST_ANY)
        {
            ast_variable_definition->m_var_def->m_init_value = m_runtime->m_ast_null;
        }else{
            ast_variable_definition->m_var_def->m_init_value = create_ast(data_type);
        }
        return ast_variable_definition;
    }
}


aist_ast_T* aist_parser_T::parse_statement()
{
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NAME )
    {
        aist_lexeme_T lex = m_lexer.m_cur_lexeme;

        if(lex.m_len == 5 && memcmp(lex.m_value, "while", 5) == 0)
            return parse_while();

        if(lex.m_len == 3 && memcmp(lex.m_value, "for", 3) == 0)
            return parse_for();

        if(lex.m_len == 2 && memcmp(lex.m_value, "if", 2) == 0)
            return parse_if();

        if((lex.m_len == 5 && memcmp(lex.m_value, "false", 5) == 0) ||
            (lex.m_len == 4 && memcmp(lex.m_value, "true", 4) == 0))
        {
            aist_ast_T* ast = parse_boolean();
            if(m_lexer.m_cur_lexeme.m_lex_type >= AIST::LEXEME_type::LEXEME_PLUS  &&
                m_lexer.m_cur_lexeme.m_lex_type <= AIST::LEXEME_type::LEXEME_MINUS)
            {
                m_lexer.m_cur_lexeme = m_lexer.m_prev_lexeme;
                m_lexer.m_state = m_lexer.m_prev_state;
                aist_ast_T* ast_binop = parse_expression();
                return ast_binop;
            }
            return ast;
        }
        if(lex.m_len == 4 && memcmp(lex.m_value, "NULL", 4) == 0)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_NAME);
            if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_EQUALS_EQUALS  || // '=='
               m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NOT_EQUALS)       // '!='
            {
                m_lexer.m_cur_lexeme = m_lexer.m_prev_lexeme;
                m_lexer.m_state = m_lexer.m_prev_state;
                aist_ast_T* ast_binop = parse_expression();
                return ast_binop;
            }
            return m_runtime->m_ast_null;
        }
        if(lex.m_len == 6 && memcmp(lex.m_value, "return", 6) == 0)
            return parse_return();

        if(lex.m_len == 5 && memcmp(lex.m_value, "break", 5) == 0)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_NAME);
            return create_ast(AIST::AST_type::AST_BREAK);
        }
        if(lex.m_len == 7 && memcmp(lex.m_value, "include", 7) == 0)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_NAME);
            aist_ast_T* ast = create_ast(AIST::AST_type::AST_INCLUDE);
            ast->m_new_del_incl_right = parse_expression();
            return ast;
        }
        if(lex.m_len == 8 && memcmp(lex.m_value, "continue", 8) == 0)
        {
            parser_eat(AIST::LEXEME_type::LEXEME_NAME);
            return create_ast(AIST::AST_type::AST_CONTINUE);
        }
        if(lex.m_len == 3 && memcmp(lex.m_value, "new", 3) == 0)
            return parse_new();

        if(lex.m_len == 6 && memcmp(lex.m_value, "delete", 6) == 0)
            return parse_delete();

        if(is_data_type(lex))
            return parse_definition(true);

        parser_eat(AIST::LEXEME_type::LEXEME_NAME);

        // пустышки которые некуда не идут, но можно читать если в конце
        if(m_lexer.m_cur_lexeme.m_lex_type >= AIST::LEXEME_type::LEXEME_PLUS  &&
            m_lexer.m_cur_lexeme.m_lex_type <= AIST::LEXEME_type::LEXEME_MINUS)// '+' ... '-'
        {
            m_lexer.m_cur_lexeme = m_lexer.m_prev_lexeme;
            m_lexer.m_state = m_lexer.m_prev_state;
            aist_ast_T* ast_binop = parse_expression();
            return ast_binop;
        }
        aist_ast_T* ast = parse_variable();

        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN) // '('
            ast = parse_function_call(ast);

        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_DOT) // '.'
        {
            parser_eat(AIST::LEXEME_type::LEXEME_DOT);
            aist_ast_T* ast_new = create_ast(AIST::AST_type::AST_ATTRIBUTE_ACCESS);
            ast_new->m_attribute->m_left = ast;
            ast_new->m_attribute->m_right = parse_expression();
            ast = ast_new;
        }
        while(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LBRACKET) // '['
        {
            ast = parse_array_access(ast, false);
        }
        return ast;
    }
    if((m_lexer.m_cur_lexeme.m_lex_type >= AIST::LEXEME_type::LEXEME_INTEGER_VALUE) &&
       (m_lexer.m_cur_lexeme.m_lex_type <= AIST::LEXEME_type::LEXEME_STRING_VALUE))
        return parse_expression();
    if(m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_MINUS  ||  // '-'
       m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_PLUS   ||  // '+'
       m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_NOT    ||  // '!' 
       m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_LPAREN ||  // '('
       m_lexer.m_cur_lexeme.m_lex_type == AIST::LEXEME_type::LEXEME_BINARY_NOT)// '~'
    {
        return parse_expression();
    }
    return m_runtime->m_ast_null;
}


struct zRead
{
    bool m_error;
    size_t m_bufPos;
    size_t m_maxBuf;
    unsigned char* m_buffer;
    void Init(const void* data, size_t len)
    {
        m_bufPos = 0;
        m_error = false;
        m_buffer = (unsigned char*)data;
        m_maxBuf = len;
    }
    void read(void* dst, size_t len)
    {
        if(m_bufPos + len > m_maxBuf)
        {
            aist_os_memset(dst, 0, len);
            m_error = true;
            return;
        }
        aist_os_memcpy(dst, m_buffer+m_bufPos, len);
        m_bufPos += len;
    }
    unsigned long long zReadU64()
    {
        unsigned long long value = 0;
        size_t i = 0;
        if(m_bufPos == m_maxBuf)
        {
            m_error = true;
            return 0;
        }
        while(m_buffer[m_bufPos]&0x80)
        {
             value = value|((m_buffer[m_bufPos++]&0x7F)<<i);
            if(m_bufPos == m_maxBuf)
            {
                m_error = true;
                return 0;
            }
            i += 7;
        }
        value = value|(m_buffer[m_bufPos++]<<i);
        return value;
    }
    long long zReadI64()
    {
        unsigned long long value = zReadU64();
        if(value&1)
        {
            value = ~(value>>1);
        } else{
            value >>= 1;
        }
        return (long long)value;
    }
    unsigned int zReadU32()
    {
        unsigned int value = 0;
        size_t i = 0;
        if(m_bufPos == m_maxBuf)
        {
            m_error = true;
            return 0;
        }
        while(m_buffer[m_bufPos]&0x80)
        {
            value = value|((m_buffer[m_bufPos++]&0x7F)<<i);
            if(m_bufPos == m_maxBuf)
            {
                m_error = true;
                return 0;
            }
            i +=7;
        }
        value = value|(m_buffer[m_bufPos++]<<i);
        return value;
    }
    inline int zReadI32()
    {
        unsigned int value = zReadU32();
        if(value&1)
        {
            value = ~(value>>1);
        } else{
            value >>= 1;
        }
        return (int)value;
    }
};


aist_ast_T* aist_parser_T::parse_zunpack(zRead* data)
{
    unsigned int ast_type = data->zReadU32();
    if(!ast_type)
        return NULL;
    aist_ast_T* ast = create_ast((AIST::AST_type)(ast_type-1));
    {
        char flags;
        data->read(&flags, 1);
        flags &= ~AIST::FLAG_AST_MEM_RT;
        ast->m_flags |= flags;
    }
    ast->m_line_pos12 = data->zReadI32();
    switch(ast->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:  ast->m_char_value = data->zReadU32(); break;
        case AIST::AST_type::AST_BOOLEAN:ast->m_boolean_value = data->zReadU32(); break;
        case AIST::AST_type::AST_INTEGER:ast->m_int_value = data->zReadI64(); break;
        case AIST::AST_type::AST_FLOAT:  data->read(&ast->m_float_value, sizeof(aist_float)); break;
        case AIST::AST_type::AST_STRING:
        {
            ast->m_string_value_len = data->zReadU32();
            if(ast->m_string_value_len)
            {
                ast->m_string_value = (char*)m_lexer.m_mem->Alloc(ast->m_string_value_len+1);
                data->read(ast->m_string_value, ast->m_string_value_len);
                ast->m_string_value[ast->m_string_value_len] = 0;
            }
        }break;
        case AIST::AST_type::AST_OBJECT:
        {
            size_t ast_count = data->zReadU32();
            for(size_t i = 0; i < ast_count; i++)
            {
                ast->m_object_children->push(parse_zunpack(data));
            }
        }break;
        case AIST::AST_type::AST_LIST:
        {
            size_t ast_count = data->zReadU32();
            for(size_t i = 0; i < ast_count; i++)
            {
                ast->m_list_children->push(parse_zunpack(data));
            }
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            ast->m_var_def->m_type = (AIST::AST_type)data->zReadU32();
#ifdef AIST_USE_HASH_NAME
            data->read(&ast->m_var_def->m_name_hash, sizeof(ast->m_var_def->m_name_hash));
#else
            size_t len = data->zReadU32();
            if(len)
            {
                ast->m_var_def->m_name = (char*)m_lexer.m_mem->Alloc(len+1);
                data->read((void*)ast->m_var_def->m_name, len);
                ((char*)ast->m_var_def->m_name)[len] = 0;
            }
#endif
            ast->m_var_def->m_value      = parse_zunpack(data);
            ast->m_var_def->m_init_value = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_VARIABLE:
        {
#ifdef AIST_USE_HASH_NAME
             data->read(&ast->m_variable_name_hash, sizeof(ast->m_variable_name_hash));
#else
            size_t len = data->zReadU32();
            if(len)
            {
                ast->m_variable_name = (char*)m_lexer.m_mem->Alloc(len+1);
                data->read(ast->m_variable_name, len);
                ast->m_variable_name[len] = 0;
            }
#endif
        }break;
        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            ast->m_var_modif->m_left  = parse_zunpack(data);
            ast->m_var_modif->m_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
            size_t len;
#ifdef AIST_USE_HASH_NAME
            data->read(&ast->m_fn_def->m_function_name_hash, sizeof(ast->m_fn_def->m_function_name_hash));
#else
            len = data->zReadU32();
            if(len)
            {
                ast->m_fn_def->m_function_name = (char*)m_lexer.m_mem->Alloc(len+1);
                data->read((void*)ast->m_fn_def->m_function_name, len);
                ((char*)ast->m_fn_def->m_function_name)[len] = 0;
            }
#endif
            len = data->zReadU32();
            if(len)
            {
                ast->m_fn_def->m_label_src = (char*)m_lexer.m_mem->Alloc(len+1);
                data->read((void*)ast->m_fn_def->m_label_src, len);
                ((char*)ast->m_fn_def->m_label_src)[len] = 0;
            }
            data->read(&ast->m_fn_def->m_return_type, 1);
            ast->m_fn_def->m_body = parse_zunpack(data);

            size_t ast_count = data->zReadU32();
            for(size_t i = 0; i < ast_count; i++)
            {
                ast->m_fn_def->m_arguments->push(parse_zunpack(data));
            }
        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            ast->m_fn_call->m_name = parse_zunpack(data);
            size_t ast_count = data->zReadU32();
            for(size_t i = 0; i < ast_count; i++)
            {
                ast->m_fn_call->m_arguments->push(parse_zunpack(data));
            }
        }break;
        case AIST::AST_type::AST_BLOCK:
        {
            size_t ast_count = data->zReadU32();
            for(size_t i = 0; i < ast_count; i++)
            {
                ast->m_block_list->push(parse_zunpack(data));
            }
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            data->read(&ast->m_binop->m_operator, 1);
            ast->m_binop->m_left  = parse_zunpack(data);
            ast->m_binop->m_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            data->read(&ast->m_unop->m_operator, 1);
            ast->m_unop->m_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_CONTINUE:break;
        case AIST::AST_type::AST_BREAK:break;
        case AIST::AST_type::AST_RETURN:
        {
            ast->m_return->m_init_value = parse_zunpack(data);;
            ast->m_return->m_value      = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            ast->m_ternary->m_expr      = parse_zunpack(data);
            ast->m_ternary->m_body      = parse_zunpack(data);
            ast->m_ternary->m_else_body = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_IF:
        {
            ast->m_if->m_if_expr      = parse_zunpack(data);
            ast->m_if->m_if_body      = parse_zunpack(data);
            ast->m_if->m_if_otherwise = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            ast->m_while->m_test_expr = parse_zunpack(data);
            ast->m_while->m_body      = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_FOR:
        {
            ast->m_for->m_init_statement   = parse_zunpack(data);
            ast->m_for->m_test_expr        = parse_zunpack(data);
            ast->m_for->m_update_statement = parse_zunpack(data);
            ast->m_for->m_body             = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            ast->m_attribute->m_left  = parse_zunpack(data);
            ast->m_attribute->m_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            ast->m_arr_access->m_left    = parse_zunpack(data);
            ast->m_arr_access->m_right   = parse_zunpack(data);
            ast->m_arr_access->m_pointer = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_NEW:
        {
            ast->m_new_del_incl_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_DELETE:
        {
            ast->m_new_del_incl_right = parse_zunpack(data);
        }break;
        case AIST::AST_type::AST_INCLUDE:
        {
            ast->m_new_del_incl_right = parse_zunpack(data);
        }break;
#ifdef AIST_DEBUG_
        default: printf("WARNING zunpack\n");  break;
#endif
    }
    return ast;
}

#include "aist_dynamic_array.hpp"
static void ast_save_zpack(const aist_ast_T* ast, aist_dynamic_array_T* outData)
{
    if(!ast)
    {
        outData->zWriteU32(0);
        return;
    }
    outData->zWriteU32((int)ast->m_ast_type+1);
    outData->write(&ast->m_flags, 1);
    outData->zWriteI32(ast->m_line_pos12);

    switch(ast->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:  outData->zWriteU32(ast->m_char_value); break;
        case AIST::AST_type::AST_BOOLEAN:outData->zWriteU32(ast->m_boolean_value); break;
        case AIST::AST_type::AST_INTEGER:outData->zWriteI64(ast->m_int_value); break;
        case AIST::AST_type::AST_FLOAT:  outData->write(&ast->m_float_value, sizeof(aist_float)); break;
        case AIST::AST_type::AST_STRING:
        {
            outData->zWriteU32(ast->m_string_value_len);
            if(ast->m_string_value_len)
                outData->write(ast->m_string_value, ast->m_string_value_len);
        }break;
        case AIST::AST_type::AST_OBJECT:
        {
            outData->zWriteU32((unsigned int)ast->m_object_children->m_ast_count);
            for(size_t i = 0; i < ast->m_object_children->m_ast_count; i++)
            {
                ast_save_zpack(ast->m_object_children->m_ast_items[i], outData);
            }
        }break;
        case AIST::AST_type::AST_LIST:
        {
            outData->zWriteU32((unsigned int)ast->m_list_children->m_ast_count);
            for(size_t i = 0; i < ast->m_list_children->m_ast_count; i++)
            {
                ast_save_zpack(ast->m_list_children->m_ast_items[i], outData);
            }
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            outData->zWriteU32(ast->m_var_def->m_type);
#ifdef AIST_USE_HASH_NAME
            outData->write(&ast->m_var_def->m_name_hash, sizeof(ast->m_var_def->m_name_hash));
#else
            size_t len = aist_os_strlen(ast->m_var_def->m_name);
            outData->zWriteU32((unsigned int)len);
            if(len)
                outData->write(ast->m_var_def->m_name, len);
#endif
            ast_save_zpack(ast->m_var_def->m_value, outData);
            ast_save_zpack(ast->m_var_def->m_init_value, outData);
        }break;
        case AIST::AST_type::AST_VARIABLE:
        {
#ifdef AIST_USE_HASH_NAME
            outData->write(&ast->m_variable_name_hash, sizeof(ast->m_variable_name_hash));
#else
            size_t len = aist_os_strlen(ast->m_variable_name);
            outData->zWriteU32((unsigned int)len);
            if(len)
                outData->write(ast->m_variable_name, len);
#endif
        }break;
        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            ast_save_zpack(ast->m_var_modif->m_left, outData);
            ast_save_zpack(ast->m_var_modif->m_right, outData);
        }break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
            size_t len;
#ifdef AIST_USE_HASH_NAME
            outData->write(&ast->m_fn_def->m_function_name_hash, sizeof(ast->m_fn_def->m_function_name_hash));
#else
            len = aist_os_strlen(ast->m_fn_def->m_function_name);
            outData->zWriteU32((unsigned int)len);
            if(len)
                outData->write(ast->m_fn_def->m_function_name, len);
#endif
            len = aist_os_strlen(ast->m_fn_def->m_label_src);
            outData->zWriteU32((unsigned int)len);
            if(len)
                outData->write(ast->m_fn_def->m_label_src, len);
            outData->write(&ast->m_fn_def->m_return_type, 1);
            ast_save_zpack(ast->m_fn_def->m_body, outData);

            outData->zWriteU32((unsigned int)ast->m_fn_def->m_arguments->m_ast_count);
            for(size_t i = 0; i < ast->m_fn_def->m_arguments->m_ast_count; i++)
            {
                ast_save_zpack(ast->m_fn_def->m_arguments->m_ast_items[i], outData);
            }
        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            ast_save_zpack(ast->m_fn_call->m_name, outData);
            outData->zWriteU32((unsigned int)ast->m_fn_call->m_arguments->m_ast_count);
            for(size_t i = 0; i < ast->m_fn_call->m_arguments->m_ast_count; i++)
            {
                ast_save_zpack(ast->m_fn_call->m_arguments->m_ast_items[i], outData);
            }
        }break;
        case AIST::AST_type::AST_BLOCK:
        {
            outData->zWriteU32((unsigned int)ast->m_block_list->m_ast_count);
            for(size_t i = 0; i < ast->m_block_list->m_ast_count; i++)
            {
                ast_save_zpack(ast->m_block_list->m_ast_items[i], outData);
            }
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            outData->write(&ast->m_binop->m_operator, 1);
            ast_save_zpack(ast->m_binop->m_left, outData);
            ast_save_zpack(ast->m_binop->m_right, outData);
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            outData->write(&ast->m_unop->m_operator, 1);
            ast_save_zpack(ast->m_unop->m_right, outData);
        }break;
        case AIST::AST_type::AST_CONTINUE:break;
        case AIST::AST_type::AST_BREAK:break;
        case AIST::AST_type::AST_RETURN:
        {
            ast_save_zpack(ast->m_return->m_init_value, outData);
            ast_save_zpack(ast->m_return->m_value, outData);
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            ast_save_zpack(ast->m_ternary->m_expr, outData);
            ast_save_zpack(ast->m_ternary->m_body, outData);
            ast_save_zpack(ast->m_ternary->m_else_body, outData);
        }break;
        case AIST::AST_type::AST_IF:
        {
            ast_save_zpack(ast->m_if->m_if_expr, outData);
            ast_save_zpack(ast->m_if->m_if_body, outData);
            ast_save_zpack(ast->m_if->m_if_otherwise, outData);
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            ast_save_zpack(ast->m_while->m_test_expr, outData);
            ast_save_zpack(ast->m_while->m_body, outData);
        }break;
        case AIST::AST_type::AST_FOR:
        {
            ast_save_zpack(ast->m_for->m_init_statement, outData);
            ast_save_zpack(ast->m_for->m_test_expr, outData);
            ast_save_zpack(ast->m_for->m_update_statement, outData);
            ast_save_zpack(ast->m_for->m_body, outData);
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            ast_save_zpack(ast->m_attribute->m_left, outData);
            ast_save_zpack(ast->m_attribute->m_right, outData);
        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            ast_save_zpack(ast->m_arr_access->m_left, outData);
            ast_save_zpack(ast->m_arr_access->m_right, outData);
            ast_save_zpack(ast->m_arr_access->m_pointer, outData);
        }break;
        case AIST::AST_type::AST_NEW:
        {
            ast_save_zpack(ast->m_new_del_incl_right, outData);
        }break;
        case AIST::AST_type::AST_DELETE:
        {
            ast_save_zpack(ast->m_new_del_incl_right, outData);
        }break;
        case AIST::AST_type::AST_INCLUDE:
        {
            ast_save_zpack(ast->m_new_del_incl_right, outData);
        }break;
#ifdef AIST_DEBUG_
        default: printf("WARNING save\n");  break;
#endif
    }
}


bool aist_ast_save_zpak(const aist_ast_T* ast, aist_dynamic_array_T* outData)
{
    outData->clear();
    if(ast)
    {
        char t[2];
        t[0] = sizeof(aist_float);
        t[1] = sizeof(aist_int);
#ifdef AIST_USE_HASH_NAME
        outData->write("\x02paH", 4);
#else
        outData->write("\x02paS", 4);
#endif
        outData->write(t, 2);
        ast_save_zpack(ast, outData);
    }
    return ast != NULL;
}


bool aist_parser_T::parse(aist_runtime_T* runtime, const char* label_src, const char* contents, size_t len, size_t shift_line_num)
{
    if(!len)
        len = aist_os_strlen(contents);
    m_lexer.m_mem = runtime->m_mem_parser;
    m_runtime = runtime;
    m_lexer.m_errorStr[0] = 0;
#ifdef AIST_USE_HASH_NAME
    if(contents[0] == 0x2 && memcmp("\x02paH", contents, 4) == 0)
#else
    if(contents[0] == 0x2 && memcmp("\x02paS", contents, 4) == 0)
#endif
    {
        if(len < 8 ||contents[4] != sizeof(aist_float) || contents[5] != sizeof(aist_int))
        {
            aist_os_strcpy(m_lexer.m_errorStr, "Incorrect zpack format");
            m_result_ast = NULL;
            return false;
        }
        zRead reader;
        reader.Init(contents+6, len - 6);
        m_result_ast = parse_zunpack(&reader);
        if(reader.m_error)
        {
            aist_os_strcpy(m_lexer.m_errorStr, "Incorrect zpack format");
            m_result_ast = NULL;
            return false;
        }
        return true;
    }

    m_lexer.m_contents = contents;
    m_lexer.m_length   = len;
    m_lexer.m_state.m_cur_index = 0;
    m_lexer.m_state.m_line_pos  = 0;
    m_lexer.m_state.m_line = 1 + shift_line_num;
    m_lexer.m_state.m_cur_char = contents[0];
    m_lexer.m_prev_state = m_lexer.m_state;
    m_lexer.m_label_src  = aist_str_clone(runtime->m_mem_parser, label_src);
    m_lexer.m_error_jmp_buf = (jmp_buf*)(((size_t)m_lexer.m_mem->Alloc(sizeof(jmp_buf)+16)+15)&(~15));
    if(setjmp(*m_lexer.m_error_jmp_buf))
    {
        m_result_ast = NULL;
        return false;
    }else{
        m_lexer.m_cur_lexeme = m_lexer.get_next_lexeme();
        aist_os_memset(&m_lexer.m_prev_lexeme, 0, sizeof(aist_lexeme_T));
        m_result_ast = parse_block_statements();
        if(m_lexer.m_length != m_lexer.m_state.m_cur_index)
            _unexpected_lexeme_error();
    }
    return true;
}

