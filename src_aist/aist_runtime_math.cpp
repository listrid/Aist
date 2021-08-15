/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include "aist_runtime.h"
#include <string.h>
#include <stdarg.h>


aist_ast_T* aist_runtime_T::eval_binop(aist_ast_T* node)
{
    aist_ast_T* left, * right;
    
    if((node->m_flags & AIST::FLAG_RESTORE_THIS) != 0)
    {
        aist_ast_T* obj = m_rt_stack->get_this_one();
        left = eval(node->m_binop->m_left);
        m_rt_stack->set_this_one(obj);
    }else{
        left = eval(node->m_binop->m_left);// !! сначало левая часть потом правая
    }
    right = eval(node->m_binop->m_right);

    AIST::AST_type left_ast_type  = left->m_ast_type;
    AIST::AST_type right_ast_type = right->m_ast_type;

    aist_ast_T* return_value = ast_create(AIST::AST_type::AST_NULL, AIST::FLAG_AST_MEM_RT);
    return_value->m_countUse = 1;
    if(node->m_binop->m_operator == AIST::LEXEME_type::LEXEME_PLUS)
    {
        if(right_ast_type == AIST::AST_type::AST_STRING)
        {
            const char* add_str;
            size_t add_str_len;
            if(this->ast_to_str(left, add_str, add_str_len))
            {
                char* new_str = (char*)this->rt_malloc(right->m_string_value_len + add_str_len + 1);
                aist_os_memcpy(new_str, add_str, add_str_len);
                aist_os_memcpy(new_str+add_str_len, right->m_string_value, right->m_string_value_len);
                new_str[add_str_len + right->m_string_value_len] = 0;
                return_value->m_ast_type = AIST::AST_type::AST_STRING;
                return_value->m_string_value = new_str;
                return_value->m_string_value_len = (unsigned int)(right->m_string_value_len+add_str_len);
                ast_release(left);
                ast_release(right);
                return return_value;
            }
        }
        if(left_ast_type == AIST::AST_type::AST_STRING)
        {
            const char* add_str;
            size_t add_str_len;
            if(this->ast_to_str(right, add_str, add_str_len))
            {
                char* new_str = (char*)this->rt_malloc(left->m_string_value_len + add_str_len + 1);
                aist_os_memcpy(new_str, left->m_string_value, left->m_string_value_len);
                aist_os_memcpy(new_str+left->m_string_value_len, add_str, add_str_len);
                new_str[add_str_len + left->m_string_value_len] = 0;
                return_value->m_ast_type = AIST::AST_type::AST_STRING;
                return_value->m_string_value = new_str;
                return_value->m_string_value_len = (unsigned int)(left->m_string_value_len+add_str_len);
                ast_release(left);
                ast_release(right);
                return return_value;
            }
        }
    }
    if(left_ast_type == AIST::AST_type::AST_BOOLEAN)
    {
        left_ast_type = AIST::AST_type::AST_INTEGER;
        left->m_int_value = left->m_int_value != 0;
    }else if(left_ast_type == AIST::AST_type::AST_UCHAR)
    {
        left_ast_type = AIST::AST_type::AST_INTEGER;
        left->m_int_value &= 0xFF;
    }
    if(right_ast_type == AIST::AST_type::AST_BOOLEAN)
    {
        right_ast_type = AIST::AST_type::AST_INTEGER;
        right->m_int_value = right->m_int_value != 0;

    }else if(right_ast_type == AIST::AST_type::AST_UCHAR)
    {
        right_ast_type = AIST::AST_type::AST_INTEGER;
        right->m_int_value &= 0xFF;
    }
    switch(node->m_binop->m_operator)
    {
        case AIST::LEXEME_type::LEXEME_PLUS: // '+'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_int_value + right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type  = AIST::AST_type::AST_INTEGER;
                    return_value->m_int_value = left->m_int_value + right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value + right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value + right->m_int_value;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_MINUS: // '-'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_int_value - right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_INTEGER;
                    return_value->m_int_value = left->m_int_value - right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value - right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value - right->m_int_value;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_DIV: // '/'
        {
            if(right_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right->m_int_value == 0)
                {
                    ast_release(right);
                    rt_error(left, node->m_line_pos12, "Div 0");
                }
                if(left_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type  = AIST::AST_type::AST_INTEGER;
                    return_value->m_int_value = left->m_int_value / right->m_int_value;
                }else if(left_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value / right->m_int_value;
                }
                break;
            }
            if(right_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right->m_float_value == 0)
                {
                    ast_release(right);
                    rt_error(left, node->m_line_pos12, "Div 0");
                }
                if(left_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_int_value / right->m_float_value;
                }else if(left_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value / right->m_float_value;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_STAR: // '*'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_int_value * right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type  = AIST::AST_type::AST_INTEGER;
                    return_value->m_int_value = left->m_int_value * right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value * right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type    = AIST::AST_type::AST_FLOAT;
                    return_value->m_float_value = left->m_float_value * right->m_int_value;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_BOOLEAN_AND: // '&&'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER || left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                bool lb = left->m_boolean_value != 0;
                if(left_ast_type == AIST::AST_type::AST_INTEGER)
                    lb = left->m_int_value != 0;
                if(left_ast_type == AIST::AST_type::AST_FLOAT)
                    lb = left->m_float_value != 0.;
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = lb && right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = lb && (right->m_float_value != 0.);
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_BOOLEAN_OR: // '||'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER || left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                bool lb = left->m_boolean_value != 0;
                if(left_ast_type == AIST::AST_type::AST_INTEGER)
                    lb = left->m_int_value != 0;
                if(left_ast_type == AIST::AST_type::AST_FLOAT)
                    lb = left->m_float_value != 0.;
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = lb || right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = lb || (right->m_float_value != 0.);
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_LEQUAL: // '<='
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value <= right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value <= right->m_float_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value <= right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value <= right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                if(left->m_string_value_len <= right->m_string_value_len)
                {
                    int r = memcmp(left->m_string_value, right->m_string_value, left->m_string_value_len);
                    return_value->m_boolean_value = r <= 0;
                }else{
                    int r = memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len);
                    return_value->m_boolean_value = r < 0;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_LESS: // '<'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value < right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value < right->m_float_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value < right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value < right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                if(left->m_string_value_len <= right->m_string_value_len)
                {
                    int r = memcmp(left->m_string_value, right->m_string_value, left->m_string_value_len);
                    if(left->m_string_value_len < right->m_string_value_len)
                    {
                        return_value->m_boolean_value = r <= 0;
                    }else{
                        return_value->m_boolean_value = r < 0;
                    }
                }else{
                    int r = memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len);
                    return_value->m_boolean_value = r < 0;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_LARGER: // '>'
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value > right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value > right->m_float_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value > right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value > right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                if(left->m_string_value_len <= right->m_string_value_len)
                {
                    int r = memcmp(left->m_string_value, right->m_string_value, left->m_string_value_len);
                    return_value->m_boolean_value = r > 0;
                }else{
                    int r = memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len);
                    return_value->m_boolean_value = r >= 0;
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_LAQUAL: // '>='
        {
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value >= right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value >= right->m_float_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value >= right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value >= right->m_int_value;
                }
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                if(left->m_string_value_len <= right->m_string_value_len)
                {
                    int r = memcmp(left->m_string_value, right->m_string_value, left->m_string_value_len);
                    if(left->m_string_value_len < right->m_string_value_len)
                    {
                        return_value->m_boolean_value = (r > 0);
                    }else{
                        return_value->m_boolean_value = (r >= 0);
                    }
                }else{
                    int r = memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len);
                    return_value->m_boolean_value = (r >= 0);
                }
            }
        }break;
        case AIST::LEXEME_type::LEXEME_EQUALS_EQUALS: // '=='
        {

            if(left_ast_type == AIST::AST_type::AST_NULL)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (right_ast_type == AIST::AST_type::AST_NULL);
            }else if(right_ast_type == AIST::AST_type::AST_NULL)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (left_ast_type == AIST::AST_type::AST_NULL);
            }
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value == right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value == right->m_float_value;
                }
                break;
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value == right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value == right->m_int_value;
                }
                break;
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (left->m_string_value_len == right->m_string_value_len && memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len) == 0);
            } 
        }break;
        case AIST::LEXEME_type::LEXEME_NOT_EQUALS: // '!='
        {
            if(left_ast_type == AIST::AST_type::AST_NULL)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (right_ast_type != AIST::AST_type::AST_NULL);
            }else if(right_ast_type == AIST::AST_type::AST_NULL)
            {
                return_value->m_ast_type = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (left_ast_type != AIST::AST_type::AST_NULL);
            }
            if(left_ast_type == AIST::AST_type::AST_INTEGER)
            {
                if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value != right->m_int_value;
                }else if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_int_value != right->m_float_value;
                }
                break;
            }
            if(left_ast_type == AIST::AST_type::AST_FLOAT)
            {
                if(right_ast_type == AIST::AST_type::AST_FLOAT)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value != right->m_float_value;
                }else if(right_ast_type == AIST::AST_type::AST_INTEGER)
                {
                    return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                    return_value->m_boolean_value = left->m_float_value != right->m_int_value;
                }
                break;
            }
            if(left_ast_type == AIST::AST_type::AST_STRING && right_ast_type == AIST::AST_type::AST_STRING)
            {
                return_value->m_ast_type      = AIST::AST_type::AST_BOOLEAN;
                return_value->m_boolean_value = (left->m_string_value_len != right->m_string_value_len || memcmp(left->m_string_value, right->m_string_value, right->m_string_value_len) != 0);
            }
        }break;
        case AIST::LEXEME_type::LEXEME_PLUS_PLUS:   // '++'
        case AIST::LEXEME_type::LEXEME_MINUS_MINUS: // '--'
        {
            return_value->m_ast_type    = left->m_ast_type;
            return_value->m_float_value = left->m_float_value;//как более больший размер
        }break;
    }
    if(return_value->m_ast_type != AIST::AST_type::AST_NULL)
    {
        ast_release(left);
        ast_release(right);
        return return_value;
    }

    if(left_ast_type == AIST::AST_type::AST_INTEGER && right_ast_type == AIST::AST_type::AST_INTEGER)
    {
        aist_int left_v  = left->m_int_value;
        aist_int right_v = right->m_int_value;
        switch(node->m_binop->m_operator)
        {//+ - div  *  делается раньше
            case AIST::LEXEME_type::LEXEME_BINARY_OR:  left_v |= right_v; break; // '|'
            case AIST::LEXEME_type::LEXEME_BINARY_AND: left_v &= right_v; break; // '&'
            case AIST::LEXEME_type::LEXEME_PERCENTAGE: left_v %= right_v; break; // '%'
            case AIST::LEXEME_type::LEXEME_BINARY_XOR: left_v ^= right_v; break; // '^'
            case AIST::LEXEME_type::LEXEME_SHIFT_LEFT:  left_v <<= right_v;break; // '<<'
            case AIST::LEXEME_type::LEXEME_SHIFT_RIGHT:left_v >>= right_v; break; // '>>'
            default:
            {
                ast_release(right);
                rt_error(left, node->m_line_pos12, "'INT %s INT' is not a valid operator", aist_LEXEME_name(node->m_binop->m_operator));
            }
        }
        return_value->m_ast_type = AIST::AST_type::AST_INTEGER;
        return_value->m_int_value = left_v;
        ast_release(left);
        ast_release(right);
        return return_value;
    }
    right_ast_type = right->m_ast_type;
    ast_release(right);
    rt_error(left, node->m_line_pos12, "'%s %s %s' is not a valid operator", aist_type_name(left->m_ast_type), aist_LEXEME_name(node->m_binop->m_operator), aist_type_name(right_ast_type));
    return NULL;
}


aist_ast_T* aist_runtime_T::eval_unary(aist_ast_T* node)//  (-num) (+num) (!bool)
{
    aist_ast_T* right_p = eval(node->m_unop->m_right);
    aist_ast_T right = *right_p;
    ast_release(right_p);
    switch(node->m_unop->m_operator)
    {
        case AIST::LEXEME_type::LEXEME_MINUS:
        {
            if(right.m_ast_type == AIST::AST_type::AST_INTEGER)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_INTEGER, AIST::FLAG_AST_MEM_RT);
                return_value->m_int_value = -right.m_int_value;
                return return_value;
            }
            if(right.m_ast_type == AIST::AST_type::AST_FLOAT)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_FLOAT, AIST::FLAG_AST_MEM_RT);
                return_value->m_float_value = -right.m_float_value;
                return return_value;
            }
        }break;
        case AIST::LEXEME_type::LEXEME_PLUS:
        {
            if(right.m_ast_type == AIST::AST_type::AST_INTEGER)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_INTEGER, AIST::FLAG_AST_MEM_RT);
                return_value->m_int_value = +right.m_int_value;
                return return_value;
            }
            if(right.m_ast_type == AIST::AST_type::AST_FLOAT)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_FLOAT, AIST::FLAG_AST_MEM_RT);
                return_value->m_float_value = +right.m_float_value;
                return return_value;
            }
        }break;
        case AIST::LEXEME_type::LEXEME_NOT:
        {
            if(right.m_ast_type == AIST::AST_type::AST_INTEGER)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_BOOLEAN, AIST::FLAG_AST_MEM_RT);
                return_value->m_boolean_value = !right.m_int_value;
                return return_value;
            }
            if(right.m_ast_type == AIST::AST_type::AST_FLOAT)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_BOOLEAN, AIST::FLAG_AST_MEM_RT);
                return_value->m_boolean_value = !right.m_float_value;
                return return_value;
            }
            if(right.m_ast_type == AIST::AST_type::AST_BOOLEAN)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_BOOLEAN, AIST::FLAG_AST_MEM_RT);
                return_value->m_boolean_value = !right.m_boolean_value;
                return return_value;
            }
        }break;
        case AIST::LEXEME_type::LEXEME_BINARY_NOT:
        {
            if(right.m_ast_type == AIST::AST_type::AST_INTEGER)
            {
                aist_ast_T* return_value = ast_create(AIST::AST_type::AST_INTEGER, AIST::FLAG_AST_MEM_RT);
                return_value->m_int_value = ~right.m_int_value;
                return return_value;
            }
        }
    }
    rt_error(NULL, node->m_line_pos12, "'%s' is not a valid operator", aist_LEXEME_name(node->m_unop->m_operator));
    return NULL;
}


aist_ast_T* aist_runtime_T::eval_break(aist_ast_T* node)
{
    return node->AddRef();
}


aist_ast_T* aist_runtime_T::eval_continue(aist_ast_T* node)
{
    return node->AddRef();
}


aist_ast_T* aist_runtime_T::eval_return(aist_ast_T* node)
{
    if(!node->m_return->m_value)
    {
        aist_ast_T* ret = ast_create(AIST::AST_type::AST_RETURN, AIST::FLAG_AST_MEM_RT);
        if(node->m_return->m_init_value->m_ast_type == AIST::AST_type::AST_FUNCTION_DEFINITION)
        {
            if((node->m_return->m_init_value->m_flags & AIST::FLAG_AST_MEM_RT) == 0)
            {
                ret->m_return->m_value = rt_ast_copy(node->m_return->m_init_value);
            }else{
                ret->m_return->m_value = node->m_return->m_init_value->AddRef();
            }
        }else if(node->m_return->m_init_value->m_ast_type == AIST::AST_type::AST_OBJECT)
        {
            m_object_copy_call_new = true;
            ret->m_return->m_value = rt_ast_copy(node->m_return->m_init_value);
            m_object_copy_call_new = false;
        }else{
            ret->m_return->m_value = eval(node->m_return->m_init_value);
        }
        return ret;
    }
    return node->AddRef();
}


bool aist_runtime_T::boolean_evaluation(aist_ast_T* node)
{
    bool ret;
    switch(node->m_ast_type)
    {
        case AIST::AST_type::AST_UCHAR:   ret = node->m_char_value    != 0;  break;
        case AIST::AST_type::AST_INTEGER: ret = node->m_int_value     != 0;  break;
        case AIST::AST_type::AST_FLOAT:   ret = node->m_float_value   != 0.; break;
        case AIST::AST_type::AST_BOOLEAN: ret = node->m_boolean_value != 0;  break;
        case AIST::AST_type::AST_STRING:  ret = node->m_string_value_len > 0;break;
        default:
            rt_error(node, node->m_line_pos12, "boolean evaluation");
    }
    ast_release(node);
    return ret;
}


aist_ast_T* aist_runtime_T::eval_if(aist_ast_T* node)
{
    m_rt_stack->level_up();
    if(!node->m_if->m_if_expr)
    {
        aist_ast_T* ret = eval(node->m_if->m_if_body);
        m_rt_stack->level_down();
        return ret;
    }
    if(boolean_evaluation(eval(node->m_if->m_if_expr)))
    {
        aist_ast_T* ret = eval(node->m_if->m_if_body);
        m_rt_stack->level_down();
        return ret;
    }
    if(node->m_if->m_if_otherwise)
    {
        aist_ast_T* ret = eval(node->m_if->m_if_otherwise);
        m_rt_stack->level_down();
        return ret;
    }
    if(node->m_if->m_else_body)
    {
        aist_ast_T* ret = eval(node->m_if->m_else_body);
        m_rt_stack->level_down();
        return ret;
    }
    m_rt_stack->level_down();
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_ternary(aist_ast_T* node)
{
    if(boolean_evaluation(eval(node->m_ternary->m_expr)))
        return eval(node->m_ternary->m_body);
    return eval(node->m_ternary->m_else_body);
}


aist_ast_T* aist_runtime_T::eval_while(aist_ast_T* node)
{
    while(boolean_evaluation(eval(node->m_while->m_test_expr)))
    {
        m_rt_stack->level_up();
        aist_ast_T* result = eval(node->m_while->m_body);
        m_rt_stack->level_down();
        AIST::AST_type result_type = result->m_ast_type;
        ast_release(result);
        if(result_type == AIST::AST_type::AST_BREAK)
            break;
        if(result_type == AIST::AST_type::AST_CONTINUE)
            continue;
    }
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_for(aist_ast_T* node)
{
    m_rt_stack->level_up();
    ast_release(eval(node->m_for->m_init_statement));
    while(boolean_evaluation(eval(node->m_for->m_test_expr)))
    {
        m_rt_stack->level_up();
        aist_ast_T* result = eval(node->m_for->m_body);
        m_rt_stack->level_down();
        if(result->m_ast_type == AIST::AST_type::AST_BREAK)
        {
            ast_release(result);
            break;
        }
        if(result->m_ast_type == AIST::AST_type::AST_RETURN)
        {
            m_rt_stack->level_down();
            return result;
        }
        ast_release(result);
        ast_release(eval(node->m_for->m_update_statement));
    }
    m_rt_stack->level_down();
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_block(aist_ast_T* node)
{
    for(size_t i = 0; i < node->m_block_list->m_ast_count; i++)
    {
        aist_ast_T* eval_ret = eval(node->m_block_list->m_ast_items[i]);
        if(eval_ret->m_ast_type == AIST::AST_type::AST_BREAK || eval_ret->m_ast_type == AIST::AST_type::AST_CONTINUE)
            return eval_ret;
        if(eval_ret->m_ast_type == AIST::AST_type::AST_RETURN)
        {
            if((node->m_flags&AIST::FLAG_BLOCK_IS_FN) == 0)//спустимся по блокам
                return eval_ret;
            aist_ast_T* ret_value = eval_ret->m_return->m_value->AddRef();
            ast_release(eval_ret);
            return ret_value;
        }
        if(i+1 == node->m_block_list->m_ast_count)
            return eval_ret;
        ast_release(eval_ret);
    }
    return m_ast_void;
}


aist_ast_T* aist_runtime_T::eval_new(aist_ast_T* node)
{
    aist_ast_T* new_copy = eval(node->m_new_del_incl_right);
    m_object_copy_call_new = true;
    aist_ast_T* ret = rt_ast_copy(new_copy);
    m_object_copy_call_new = false;
    ast_release(new_copy);
    return ret;
}


aist_ast_T* aist_runtime_T::eval_delete(aist_ast_T* node)
{
    if(node->m_new_del_incl_right && node->m_new_del_incl_right->m_ast_type == AIST::AST_type::AST_VARIABLE)
    {
        m_rt_stack->delete_def(AIST_NAME_HASH(node->m_new_del_incl_right->m_variable_name));
    }
    return m_ast_void;
}


