#include <stdlib.h>
#include <stdio.h>

#include "../src_aist/aist_runtime.h"
#include "../src_aist/aist.h"
char* get_stdin(const char* printstr);

const char* t1_str = "\
   aa =aa*10; \n\
   int aaa = (a+a)*2; \n\
   s = s+ss; \n\
    \n\
    \n\
   a=+aa; \n\
return \"ww3\";  \n\
";



const char* t2_str = "\n\
 //int fn(int ii){\n\
 //return 10+20%ii;} \n\
 /*string qs = \"->\\n<-\x0A->\";//;a-2;*/ \n\
1+(-0xff);//true + 0 != 1";


void main_test1()
{

    aist_T* aist = aist_create();

    aist->set_int("a", 2);
    aist->set_int("aa", 4);
    aist->set_string("s", "s\0s", 3);
    aist->set_string("ss", "_\002\0s", 4);

    if(!aist->run(t2_str, 0, "test"))
    {
      //  const char* err = aist.get_error();
      //  printf("%s\n", err);
        getchar();
    }
    aist_int iv;
    aist->get_int("a", iv);
    aist->get_int("aa", iv);
    aist->get_int("aaa", iv);
    const char* sv;
    size_t sv_len;
    aist->get_string("s", sv, &sv_len);

const char* xxx = aist->get_return();

    printf("ret: '%s'\n", xxx);


}


void main_test2()
{
    
    aist_T* aist = aist_create();


    if(!aist->run("int fn(int x1, int x2){"
"int r = x1+x2;"
"if(r > 4){"
"return -4;}print(r);"   
"return 0;for(int i = 0+10; i < 10; i+= 1 ){ if(i==2){continue;}print(i); }}"   
"for(int i = 0; i < 10; i+=1 ){ if(i==2){continue;}print(i); }print(\"stop\");return fn(4,4);", 0, "test"))
    {
//        const char* err = aist.get_error();
//        printf("%s\n", err);
        getchar();
    }else{
        const char* ret = aist->get_return();
        printf("return '%s'\n", ret);
    }
    getchar();
}





