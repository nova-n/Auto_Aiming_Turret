#include <string.h>
#include <stdio.h>
#include <iostream>
using namespace std;

class Rectangle{
    public: //Access Specifier, meaning members can be accessed anywhere in the program, with the . dot operator
    //like Rectangle.height
        int height;
        int width;
};

class Motor{
    
};

int main(){
    Rectangle largeRectangle; //creates an object largeRectangle after the class Rectangle
    largeRectangle.height = 5;
    largeRectangle.width = 7;
    printf("%d \n",largeRectangle.height);
    return 0;
}