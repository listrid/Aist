#include <stdio.h>
#include "test.h"


static const char* src_aist_obj = ""
"object d = \n"
"{\n"
"    any a = \"wwww\";\n"
"    int m_int = 999;\n"
"    string m_s = \"ssss\";\n"
"    object lvl1 ={\n"
"       int m_i = 77;\n"
"    };\n"
"    int m_fn(int v )\n"
"    {\n"
"        return 1000+v;\n"
"    }\n"
"    int m_i = 5;\n"
"};\n"
"list lst = [8,7,\"qqq\"];\n"
""
""
""
"return  d.m_i;";


void main_test_obj()
{
    aist_T* aist = aist_create();
    if(!aist->run(src_aist_obj, 0, "test"))
    {
        const aist_T::error_T* err = aist->get_error();
        printf("%s '%s'[line %u:%u] %s\n", err->m_label_src, err->m_isParser?"ErrorParser":"ErrorRt", (int)err->m_line, (int)err->m_pos, err->m_errorStr);
    }else{
        const char* ret = aist->get_return();
        printf("return '%s'\n", ret);
    }

    aist_int iv, v;
    aist_list_T* list;

    aist->get_list("lst", list, false);
    aist->get_int("d.m_i", iv);
    aist->get_int("lst[1]", v);

    getchar();


    aist_destroy(aist);
}
