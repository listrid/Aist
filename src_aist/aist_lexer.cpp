/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include "aist_runtime.h"


inline aist_lexeme_T aist_parser_T::l::lexeme_init(AIST::LEXEME_type type, size_t pos, size_t len)
{
    aist_lexeme_T lex;
    lex.m_lex_type = type;
    lex.m_value = &m_contents[pos];
    lex.m_pos   = pos;
    lex.m_len   = (unsigned short)len;
    return lex;
}


void aist_parser_T::l::next()
{
    if(m_state.m_cur_char == '\n')
    {
        m_state.m_line_pos = m_state.m_cur_index + 1;
        m_state.m_line ++;
    }
    if(m_state.m_cur_char == '\r')
        m_state.m_line_pos = m_state.m_cur_index + 1;

    if(m_state.m_cur_char != '\0' && m_state.m_cur_index < m_length)
    {
        m_state.m_cur_index ++;
        m_state.m_cur_char = m_contents[m_state.m_cur_index];
    }else{
        m_state.m_cur_char = '\0';
    }
}


void aist_parser_T::l::skip_line_comment()
{
    while(m_state.m_cur_char != '\n' && m_state.m_cur_char != '\0')
        next();
}


void aist_parser_T::l::skip_block_comment()
{
    while(m_state.m_cur_char != '\0')
    {
        next();
        if(m_state.m_cur_char == '*')
        {
            next();
            if(m_state.m_cur_char == '/')
            {
                next();
                return;
            }
        }
    }
    lex_error("Missing closing '*/' block");
}


aist_lexeme_T aist_parser_T::l::collect_string()
{
    next();
    size_t index = m_state.m_cur_index;
    while(m_state.m_cur_char != '"')
    {
        if(m_state.m_cur_char == '\0')
            lex_error("Missing closing quotation mark");
        next();
    }
    next();
    return lexeme_init(AIST::LEXEME_type::LEXEME_STRING_VALUE, index, m_state.m_cur_index - index - 1);
}


aist_lexeme_T aist_parser_T::l::collect_char()
{
    next();
    size_t index = m_state.m_cur_index;
    int c = 0;
    if(m_state.m_cur_char == '\\')
    {
        c += 2;
        next();
        next();
    }
    while(m_state.m_cur_char != '\'')
    {
        next();
        c++;
    }
    next();
    return lexeme_init(AIST::LEXEME_type::LEXEME_CHAR_VALUE,  index, c);
}


aist_lexeme_T aist_parser_T::l::collect_number()
{
    AIST::LEXEME_type type = AIST::LEXEME_type::LEXEME_INTEGER_VALUE;
    size_t index = m_state.m_cur_index;
    char prev_char = m_state.m_cur_char;
    while(m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9')
    {
        next();
    }
    if(m_state.m_cur_char == 'x' && prev_char == '0' && (m_state.m_cur_index-index) == 1) //hex
    {
        next();
        while((m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9') ||
              (m_state.m_cur_char >= 'a' && m_state.m_cur_char <= 'f') ||
              (m_state.m_cur_char >= 'A' && m_state.m_cur_char <= 'F'))
        {
            next();
        }
    }else if(m_state.m_cur_char == '.')
    {
        next();
        type = AIST::LEXEME_type::LEXEME_FLOAT_VALUE;
        while(m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9')
        {
            next();
        }
    }
    if((m_state.m_cur_char >= 'a' && m_state.m_cur_char <= 'z') ||
       (m_state.m_cur_char >= 'A' && m_state.m_cur_char <= 'Z') || 
       (m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9') ||
        m_state.m_cur_char == '_')
        lex_error("Unexpected '%c' (incorrect number)", m_state.m_cur_char);

    return lexeme_init(type, index, m_state.m_cur_index - index);
}


aist_lexeme_T aist_parser_T::l::collect_id()
{
    size_t index = m_state.m_cur_index;
    while((m_state.m_cur_char >= 'a' && m_state.m_cur_char <= 'z') ||
          (m_state.m_cur_char >= 'A' && m_state.m_cur_char <= 'Z') || 
          (m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9') ||
           m_state.m_cur_char == '_')
    {
        next();
    }
    if(m_state.m_cur_index - index > AIST::MAX_VARIABLE_NAME_SIZE)
        lex_error("Too long token (max %u)", (int)AIST::MAX_VARIABLE_NAME_SIZE);
    return lexeme_init(AIST::LEXEME_type::LEXEME_NAME, index, m_state.m_cur_index - index);
}


aist_lexeme_T aist_parser_T::l::next_with_lexeme(AIST::LEXEME_type type)
{
    aist_lexeme_T ret = lexeme_init(type, m_state.m_cur_index, 1);
    next();
    return ret;
}


aist_lexeme_T aist_parser_T::l::get_next_lexeme()
{
    while(m_state.m_cur_char != '\0' && m_state.m_cur_index < m_length)
    {
        //skip whitespace
        while(m_state.m_cur_char == ' '  || m_state.m_cur_char == '\r' || m_state.m_cur_char == '\n' || m_state.m_cur_char == '\t')
        {
            next();
        }

        if(m_state.m_cur_char >= '0' && m_state.m_cur_char <= '9')
            return collect_number();

        if((m_state.m_cur_char >= 'a' && m_state.m_cur_char <= 'z') || (m_state.m_cur_char >= 'A' && m_state.m_cur_char <= 'Z') || m_state.m_cur_char == '_')
            return collect_id();

        size_t index = m_state.m_cur_index;
        if(m_state.m_cur_char == '+')
        {
            next();
            if(m_state.m_cur_char == '=')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_PLUS_EQUALS, index, 2);
            }
            if(m_state.m_cur_char == '+')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_PLUS_PLUS, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_PLUS, index, 1);
        }
        if(m_state.m_cur_char == '-')
        {
            next();
            if(m_state.m_cur_char == '=')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_MINUS_EQUALS, index, 2);
            }
            if(m_state.m_cur_char == '-')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_MINUS_MINUS, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_MINUS, index, 1);
        }
        if(m_state.m_cur_char == '*')
        {
            next();
            return lexeme_init(AIST::LEXEME_type::LEXEME_STAR, index, 1);
        }
        if(m_state.m_cur_char == '/')
        {
            next();
            if(m_state.m_cur_char == '/') // '//'
            {
                next();
                skip_line_comment();
                continue;
            }
            if(m_state.m_cur_char == '*') // '/ *'
            {
                next();
                skip_block_comment();
                continue;
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_DIV, index, 1);
        }
        if(m_state.m_cur_char == '&')
        {
            next();
            if(m_state.m_cur_char == '&')  // &&
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_BOOLEAN_AND, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_BINARY_AND, index, 1);
        }
        if(m_state.m_cur_char == '|')
        {
            next();
            if(m_state.m_cur_char == '|')  // ||
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_BOOLEAN_OR, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_BINARY_OR, index, 1);
        }
        if(m_state.m_cur_char == '=')
        {
            next();
            if(m_state.m_cur_char == '=')  // ==
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_EQUALS_EQUALS, index, 2);
            }else{
                return lexeme_init(AIST::LEXEME_type::LEXEME_EQUALS, index, 1);
            }
        }
        if(m_state.m_cur_char == '!')
        {
            next();
            if(m_state.m_cur_char == '=')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_NOT_EQUALS, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_NOT, index, 1);
        }
        if(m_state.m_cur_char == '<')
        {
            next();
            if(m_state.m_cur_char == '=')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_LEQUAL, index, 2);
            }
            if(m_state.m_cur_char == '<')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_SHIFT_LEFT, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_LESS, index, 1);
        }
        if(m_state.m_cur_char == '>')
        {
            next();
            if(m_state.m_cur_char == '=')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_LAQUAL, index, 2);
            }
            if(m_state.m_cur_char == '>')
            {
                next();
                return lexeme_init(AIST::LEXEME_type::LEXEME_SHIFT_RIGHT, index, 2);
            }
            return lexeme_init(AIST::LEXEME_type::LEXEME_LARGER, index, 1);
        }
        if(m_state.m_cur_char == '#')
        {
            next();
            skip_line_comment();
            continue;
        }
        switch(m_state.m_cur_char)
        {
            case '"':  return collect_string();
            case '\'': return collect_char();
            case '{': return next_with_lexeme(AIST::LEXEME_type::LEXEME_LBRACE);
            case '}': return next_with_lexeme(AIST::LEXEME_type::LEXEME_RBRACE);
            case '[': return next_with_lexeme(AIST::LEXEME_type::LEXEME_LBRACKET);
            case ']': return next_with_lexeme(AIST::LEXEME_type::LEXEME_RBRACKET);
            case '(': return next_with_lexeme(AIST::LEXEME_type::LEXEME_LPAREN);
            case ')': return next_with_lexeme(AIST::LEXEME_type::LEXEME_RPAREN);
            case ';': return next_with_lexeme(AIST::LEXEME_type::LEXEME_SEMI);
            case ',': return next_with_lexeme(AIST::LEXEME_type::LEXEME_COMMA);
            case '%': return next_with_lexeme(AIST::LEXEME_type::LEXEME_PERCENTAGE);
            case '.': return next_with_lexeme(AIST::LEXEME_type::LEXEME_DOT);
            case '~': return next_with_lexeme(AIST::LEXEME_type::LEXEME_BINARY_NOT);
            case '?': return next_with_lexeme(AIST::LEXEME_type::LEXEME_QUESTION);
            case ':': return next_with_lexeme(AIST::LEXEME_type::LEXEME_COLON);
            case '^': return next_with_lexeme(AIST::LEXEME_type::LEXEME_BINARY_XOR);
            case '\0': return lexeme_init(AIST::LEXEME_type::LEXEME_EOF, m_state.m_cur_index, 0);
            default: lex_error("Unexpected %c", m_state.m_cur_char);
        }
    }
    return lexeme_init(AIST::LEXEME_type::LEXEME_EOF, m_state.m_cur_index, 0);
}


void aist_parser_T::l::lex_error(const char* str, ...)
{
    int p = int(m_state.m_cur_index - m_state.m_line_pos);
    if(p > 0xFFF)
        p = 0xFFF;
    m_errorPos = (m_state.m_line<<12)|p;
    va_list arg_ptr;
    va_start(arg_ptr, str);
    aist_os_vsnprintf(m_errorStr, 127, str, arg_ptr);
    va_end(arg_ptr); 
    longjmp(*m_error_jmp_buf, 1);
}

