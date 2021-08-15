#include <string.h>
#include <stdlib.h>

#include "../src_aist/other/aist_parser_xml.hpp"
#include "../src_aist/other/aist_parser_http.hpp"
#include "test.h"
#include <time.h>



static char* get_stdin(const char* printstr)
{
    char* str;
    int c;
    int i;
    int size = 10;
    str = (char*)malloc(size * sizeof(char)+1);
    printf(">: ");
    for(i = 0; (c = getchar()) != '\n' && c != EOF; ++i)
    {
        if(i == size)
        {
            size = 2*size;
            str = (char*)realloc(str, size*sizeof(char)+1);
            if(str == NULL)
            {
                printf("Cannot reallic string.\n");
                exit(-1);
            }
        }
        str[i] = c;
        str[i+1] = 0;
    }
    if(i == size)
    {
        str = (char*)realloc(str, (size+1)*sizeof(char));
        if(str == NULL)
        {
            printf("Cannot realloc string.\n");
            exit(-1);
        }
    }
    return str;
}


void main_Shell(aist_T* aist)
{
    printf("---- * Interactive Aist Shell * ----\n");
    while(true)
    {
        char* str = get_stdin(">: ");
        if(!aist->run(str, 0, "shell"))
        {
            const aist_T::error_T* err = aist->get_error();
            printf("%s '%s'[line %u:%u] %s\n", err->m_label_src, err->m_isParser?"ErrorParser":"ErrorRt", err->m_line, err->m_pos, err->m_errorStr);
        } else{
            const char* ret = aist->get_return();
            if(ret[0])
                printf("%s\n", ret);

        }
        free(str);
    }
}



void Test_xml()
{
    size_t xml_dataLen;
    char* xml_data = read_file_text("./temp_xml/test.txt", xml_dataLen);
    aist_parser_xml_T parser;


    parser.Init(xml_data, xml_dataLen);
    parser.Sàve("./temp_xml/test_.txt", true);

}



int main(int argc, char* argv[])
{
    main_test_obj();

    //main_test();


    //Test_xml(); return 0;
    
    //main_test1(); return 0;
    //main_test2(); return 0;
	return 0;
}
