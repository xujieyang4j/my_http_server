#include<stdio.h>
#include<string.h>

int main(int argc, char const *argv[])
{
    char buf[1024];
    strcpy(buf, "abcabcbacbab");
    printf("buf:%s\n", buf);
    strcpy(buf, "efg");
    printf("buf:%s\n", buf);
    return 0;
}
