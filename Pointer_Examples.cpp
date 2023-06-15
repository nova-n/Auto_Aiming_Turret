#include <string.h>
#include <stdio.h>
#include <iostream>

char *myString[80] = {"Hello World"};


int main(){
    int x = 6;
    int *y;
    y = &x; //seems like can only be done like this inside of a function, like int main(), or void whatever()

    printf("y, the adress of x: %d \n",y); //since y was assigned the adress ofx, will print the adress of x
    printf("what y points to, which is the value of x: %d \n",*y);//prints what y is pointing to
    printf("adress of y itself: %d \n",&y);
    printf("%d \n",*&y); //*&y is the value that &y points to, and because y points to adress of x, it gives adress of x;
    
    printf("\n");
    
    int *z; //declaring pointer without initialization
    int w;
    z = &w;
    *z = 50; //seems like z must be assigned a valid adress before actually being reassigned to point at another int
    std::cout << *z << std::endl;

    printf("\n");

    std::cout << "printing out *myString: " << *myString<< "\n"; 
    printf(*myString); //prints what myString points to
    printf("\n");
    printf(*myString+1); //prints what myString points to, but starting with the next char
    printf("\n");
    printf("%d \n",myString); //prints the adress of the string pointed to
    printf("%d \n",&myString); //is the same thing as the above

    printf("\n");

    char *stringPointer = "initial";
    printf("initial stringPointer points to: ");
    printf(stringPointer);
    printf("\n");
    //char newString[] = {*myString};

    return 0;
}
