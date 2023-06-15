#include <string> //allows for array of strings
#include <iostream>
#include <string.h> //allows me to use strtok
#include <stdio.h>
#include <vector>

char *token;
char mystring[80] = {"X100,Y200,Z400"}; //what will come in from the serial buffer
//making string array
//std::string coords[3];
const char *coordAxes[3] = {"X","Y","Z"}; //can't use chars for const chars, since string literals for pointers are immutable 
int coords[3] = {0,0,0}; //array length is size of whole array divided by size of each element 
//(string literals are not char arrays)

//also, pointers are faster than other variables
//string literals are immutable with pointers!!!
//but strtok returns the pointer of the token it is on! which is why char *token is valid

//const char *delimiter =",";
const char delimiter[2] = ","; //there are two spaces allocated: one for ",", and one for /0 at the end of the char array

int main(){
    printf("starting... \n");

    int iteration = 0;
    token = strtok(mystring, delimiter); //breaks out the tokens X100 Y200 and Z400 out
    //but if you check token, it only displays 1 at a time. Must call strtok again, but with NULL to get the next token
    
    while(token != NULL){

      printf("%c \n",token[0]); //%c for char datatype
      printf("%c \n",*coordAxes[iteration]); //seems like to print the out the string "X" as a char, must use a pointer?
      if(token[0] == *coordAxes[iteration]){ //compares the first char from the token to the char of the coordAxes
      //so compares char to char
        printf("it worked \n");
        token ++;
        coords[iteration] = atoi(token);
      }
      iteration ++;
      //token ++; //just moves it past the letter X, Y or Z to the number (for displaying here only)
      //xcoord = std::stoi( token );
      printf("%s \n",token);
      //printf("%s \n",xcoord);
      token = strtok(NULL,delimiter); //gets to the next token
    }
    //std:: allows to use functions from the standard library, std
    std::cout <<"(" << coords[0] << "," <<coords[1] << "," <<coords[2] << ")\n"; //<< are insertion operators
    //return 0;
}