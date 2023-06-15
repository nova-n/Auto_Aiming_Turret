#include <stdio.h>

int digit = 42;
int *digitAdress = &digit;
char *mystring = "abcdef";

int main(){
    printf("hi \n");
    mystring++;
    printf("string: %s \n", mystring);
    return 0;
}

