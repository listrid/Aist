/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#include "../aist_runtime.h"
#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER < 1900 
#define snprintf _snprintf
#endif

#ifdef AIST_DEBUG_
size_t gDbg_Init_num = 0;
size_t gDbg_Free_num = 0;

size_t gDbg_prev_Init_num = 0;
size_t gDbg_prev_Free_num = 0;
size_t gDbg_prev_Alloc_rt = 0;


void aist_debug_print_info(aist_runtime_T* rt)
{
    printf("======================= start block debug info ==================================\r\n");
    printf("initAst: %u (%i)  freeAst: %u (%i)  active: (%u)!!!\r\n",
           gDbg_Init_num, gDbg_Init_num - gDbg_prev_Init_num,
           gDbg_Free_num, gDbg_Free_num - gDbg_prev_Free_num,
           gDbg_Init_num - gDbg_Free_num
           );
    gDbg_prev_Free_num = gDbg_Free_num;
    printf("rt_memAllocSize: %u (%i) rt_fulMem: %u\r\n", rt->m_mem_rt->AllocSize(), rt->m_mem_rt->AllocSize() - gDbg_prev_Alloc_rt, rt->m_mem_rt->StorageSize());
//    if(rt->m_mem_parser)
//        printf("parser_memAllocSize: %u  parser_fulMem: %u\r\n", rt->m_mem_parser->AllocSize(), rt->m_mem_parser->StorageSize());
    gDbg_prev_Alloc_rt = rt->m_mem_rt->AllocSize();
    printf("======================== end block debug info ===================================\r\n");

}
#endif

static size_t gLevel;
static FILE* gF;
static char level_spase[2000];

static void print_level(int c, const char* ast)
{
    fwrite(level_spase, 1, gLevel*2+2, gF);
    fwrite(ast, 1, strlen(ast), gF);
}

static void print_f(const char* str, ...)
{
    char tmp[128];
    va_list arg_ptr;
    va_start(arg_ptr, str);
    size_t len = aist_os_vsnprintf(tmp, 127, str, arg_ptr);
    va_end(arg_ptr);
    fwrite(tmp, 1, len, gF);
}


static void dump_ast(const aist_ast_T* ast)
{
    switch(ast->m_ast_type)
    {
        case AIST::AST_type::AST_NULL:
        {
            print_level(ast->m_countUse, "AST_NULL");
        }break;
        case AIST::AST_type::AST_UCHAR:
        {
            char t[2]; t[0] = ast->m_char_value; t[1] = 0;
            print_level(ast->m_countUse, "AST_UCHAR");
            print_f("  =  %s", t);
        }break;
        case AIST::AST_type::AST_BOOLEAN:
        {
            print_level(ast->m_countUse, "AST_BOOLEAN");
            print_f("  =  %s", ast->m_boolean_value?"true":"false");
        }break;
        case AIST::AST_type::AST_INTEGER:
        {
            print_level(ast->m_countUse, "AST_INTEGER");
            print_f("  =  %i", (int)ast->m_int_value);
        }break;
        case AIST::AST_type::AST_FLOAT:
        {
            print_level(ast->m_countUse, "AST_FLOAT");
            print_f("  =  %f", ast->m_float_value);
        }break;
        case AIST::AST_type::AST_STRING:
        {
            print_level(ast->m_countUse, "AST_STRING");
            if(!ast->m_string_value_len)
            {
                print_f("  =  \"\"");
            }else  print_f("  =  \"%s\"", (int)ast->m_string_value);
        }break;
        case AIST::AST_type::AST_OBJECT:
        {
            print_level(ast->m_countUse, "AST_OBJECT");
            print_f("  =  %i", (int)ast->m_object_children->m_ast_count);
            print_level(-1, "{");
            gLevel++;
            for(size_t i = 0; i < ast->m_object_children->m_ast_count; i++)
                dump_ast(ast->m_object_children->m_ast_items[i]);
            gLevel--;
            print_level(-1, "}");
        }break;
        case AIST::AST_type::AST_LIST:
        {
            print_level(ast->m_countUse, "AST_LIST");
            print_f("  =  %i", (int)ast->m_object_children->m_ast_count);
            print_level(-1, "{");
            gLevel++;
            for(size_t i = 0; i < ast->m_object_children->m_ast_count; i++)
                dump_ast(ast->m_object_children->m_ast_items[i]);
            gLevel--;
            print_level(-1, "}");
        }break;
        case AIST::AST_type::AST_ANY:
        {
            print_level(-1,"\r\n!!!!!! error   AST_ANY  !!!!!!!!!!!!!!");
        }break;

        case AIST::AST_type::AST_VARIABLE:
        {
            print_level(ast->m_countUse, "AST_VARIABLE");
            print_f("  " AIST_PR_NAME_HASH, AIST_NAME_HASH(ast->m_variable_name));
        }break;
        case AIST::AST_type::AST_VARIABLE_DEFINITION:
        {
            print_level(ast->m_countUse, "AST_VARIABLE_DEFINITION");
            print_f("  %s " AIST_PR_NAME_HASH " = ", aist_type_name(ast->m_var_def->m_type), AIST_NAME_HASH(ast->m_var_def->m_name));
            gLevel++;
            dump_ast(ast->m_var_def->m_init_value);
            gLevel--;
        }break;
        case AIST::AST_type::AST_VARIABLE_MODIFIER:
        {
            print_level(ast->m_countUse, "AST_VARIABLE_MODIFIER");
            gLevel++;
            dump_ast(ast->m_var_modif->m_left);
            print_level(-1, "=");
            dump_ast(ast->m_var_modif->m_right);
            gLevel--;
        }break;
        case AIST::AST_type::AST_FUNCTION_DEFINITION:
        {
            print_level(ast->m_countUse, "AST_FUNCTION_DEFINITION  ");
            print_f(" %s " AIST_PR_NAME_HASH "(", aist_type_name(ast->m_fn_def->m_return_type), AIST_NAME_HASH(ast->m_fn_def->m_function_name));
            gLevel++;
            for(size_t i = 0; i < ast->m_fn_def->m_arguments->m_ast_count; i++)
            {
                aist_ast_T* var = ast->m_fn_def->m_arguments->m_ast_items[i];
                if(i) print_f(", ");
                print_f("%s " AIST_PR_NAME_HASH , aist_type_name(var->m_var_def->m_type), AIST_NAME_HASH(var->m_var_def->m_name));
            }
            print_f(")");
            dump_ast(ast->m_fn_def->m_body);
            gLevel--;
        }break;
        case AIST::AST_type::AST_FUNCTION_CALL:
        {
            print_level(ast->m_countUse, "AST_FUNCTION_CALL");
            gLevel++;
            dump_ast(ast->m_fn_call->m_name);
            print_level(-1, "(");
            gLevel++;
            for(size_t i = 0; i < ast->m_fn_call->m_arguments->m_ast_count; i++)
            {
                char txt[50];
                aist_os_snprintf(txt, 50, "============== param %i ==============", i);
                print_level(-1, txt);
                aist_ast_T* var = ast->m_fn_call->m_arguments->m_ast_items[i];
                dump_ast(var);
            }
            gLevel--;
            print_level(-1, ")");
            gLevel--;

        }break;
        case AIST::AST_type::AST_BLOCK:
        {
            print_level(ast->m_countUse, "AST_BLOCK");
            print_f("  =  %i", (int)ast->m_object_children->m_ast_count);
            print_level(-1, "{");
            gLevel++;
            for(size_t i = 0; i < ast->m_object_children->m_ast_count; i++)
                dump_ast(ast->m_object_children->m_ast_items[i]);
            gLevel--;
            print_level(-1, "}");
        }break;
        case AIST::AST_type::AST_BINOP:
        {
            print_level(ast->m_countUse, "AST_BINOP");
            gLevel++;
            dump_ast(ast->m_binop->m_left);
            print_level(-1, aist_LEXEME_name(ast->m_binop->m_operator));
            dump_ast(ast->m_binop->m_right);
            gLevel--;
        }break;
        case AIST::AST_type::AST_UNOP:
        {
            print_level(ast->m_countUse, "AST_UNOP");
            gLevel++;
            print_level(-1, aist_LEXEME_name(ast->m_unop->m_operator));
            dump_ast(ast->m_unop->m_right);
            gLevel--;
        }break;
        case AIST::AST_type::AST_CONTINUE:
        {
            print_level(ast->m_countUse, "AST_CONTINUE");
        }break;
        case AIST::AST_type::AST_BREAK:
        {
            print_level(ast->m_countUse, "AST_BREAK");
        }break;
        case AIST::AST_type::AST_RETURN:
        {
            print_level(ast->m_countUse, "AST_RETURN");
            gLevel++;
            if(ast->m_return->m_init_value)
                dump_ast(ast->m_return->m_init_value);
            gLevel--;
        }break;
        case AIST::AST_type::AST_TERNARY:
        {
            print_level(ast->m_countUse, "AST_TERNARY");
        }break;
        case AIST::AST_type::AST_IF:
        {
            print_level(ast->m_countUse, "AST_IF");
        }break;
        case AIST::AST_type::AST_WHILE:
        {
            print_level(ast->m_countUse, "AST_WHILE");
        }break;
        case AIST::AST_type::AST_FOR:
        {
            print_level(ast->m_countUse, "AST_FOR");
        }break;
        case AIST::AST_type::AST_ATTRIBUTE_ACCESS:
        {
            print_level(ast->m_countUse, "AST_ATTRIBUTE_ACCESS");




        }break;
        case AIST::AST_type::AST_ARRAY_ACCESS:
        {
            print_level(ast->m_countUse, "AST_ARRAY_ACCESS");



        }break;
        case AIST::AST_type::AST_NEW:
        {
            print_level(ast->m_countUse, "AST_NEW");
            if(ast->m_new_del_incl_right)
            {
                gLevel++;
                dump_ast(ast->m_new_del_incl_right);
                gLevel--;
            }
        }break;
        case AIST::AST_type::AST_DELETE:
        {
            print_level(ast->m_countUse, "AST_DELETE");
            gLevel++;
            dump_ast(ast->m_new_del_incl_right);
            gLevel--;
        }break;
        case AIST::AST_type::AST_INCLUDE:
        {
            print_level(ast->m_countUse, "AST_INCLUDE");
            gLevel++;
            dump_ast(ast->m_new_del_incl_right);
            gLevel--;
        }break;
    }
}


void aist_ast_dump(const char* fileName, const aist_ast_T* ast)
{
    if(!ast)
        return;
    gLevel = 0;
    gF = fopen(fileName, "w+b");
    if(!gF)
        return;
    aist_os_memset(level_spase, ' ', sizeof(level_spase));
    level_spase[0] = '\r';
    level_spase[1] = '\n';

    dump_ast(ast);
    fclose(gF);
}

