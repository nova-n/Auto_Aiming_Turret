#include<iostream>
#include <string.h> //allows me to use strtok
#include <stdio.h>

int arrayOfInts[] = {0,1,2,3,4,5}; 
//need to "initialize" the function in the header, and then define them later at the bottom, or another file.
void arrayItemReplacer(int (&intArray)[]);//need the parenthasees to pass array as reference ONLY FOR ARRAYS

int main(){
    arrayItemReplacer(arrayOfInts);
    for(int i=0;i<sizeof(arrayOfInts)/sizeof(arrayOfInts[0]);i++){ //sizeof(arrayOfInts)/sizeof(arrayOfInts[0]) is length of array
            std::cout << arrayOfInts[i]<< ",";
    }
    std::cout << "\n";
    return 0;
}

void arrayItemReplacer(int (&intArray)[]){ //need the parenthasees to pass array as reference ONLY FOR ARRAYS
    intArray[0] = 5;
}
