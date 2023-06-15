#include <string> //allows for array of strings
#include <iostream>
#include <string.h> //allows me to use strtok
#include <stdio.h>
#include <vector>

//const byte numChars = 32;
const int numChars = 32;
char recievedChars[numChars];
char tempChars[numChars]; //temporary array for when parsing

char mystring[80] = {"<X100,Y200,Z400>"}; //what will come in from the serial buffer
//char mystring[80] = {"<X100>"};
//making string array
const char coordAxes[3] = {'X','Y','Z'};
//const char coordAxes[1] = {'X'};
int coords[3] = {0,0,0}; //array length is size of whole array divided by size of each element 
//int coords[1] = {0};
//(string literals are not char arrays)

//also, pointers are faster than other variables
//string literals are immutable with pointers!!!
//but strtok returns the pointer of the token it is on! which is why char *token is valid

//const char *delimiter =",";
const char *delimiter = ","; //there are two spaces allocated: one for ",", and one for /0 at the end of the char array

void recvStartEndMarkers();
void coordAssigner(char fromSerial[],int (&coordsGiven)[],const char *delim = delimiter,const char coordAxesGiven[] = coordAxes);

int main(){
  std::cout << sizeof(coordAxes) << "\n";  
  recvStartEndMarkers();
  printf("starting... \n");
  //std::cout <<"(" << coords[0] << "," <<coords[1] << "," <<coords[2] << ")\n"; //<< are insertion operators
  coordAssigner(recievedChars,coords);
  std::cout <<"(" << coords[0] << "," <<coords[1] << "," <<coords[2] << ")\n"; //<< are insertion operators
  //std::cout <<"(" << coords[0] << ")\n";
  //return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------

void recvStartEndMarkers(){
  //static is a variable specific to a function. When it is called repeatedly, it remembers its value between calls, 
  //despite being defined initially in the function. This happens until the program ends. The variable isn't reinitialized.
  //this is similar to defining a variable globally so that it doesn't reset every time you call the function
  static bool recvInProgress = false;
  //static byte ndx = 0; //byte is an unsigned number from 0 to 255, so takes one byte of memory
  static int ndx = 0;//C++ doesn't recognize byte, so just using int instead. int is signed, so takes 2 bytes.
  char startMarker = '<';
  char endMarker = '>';
  char rc;


  //while(Serial.available() >0){
  while(ndx <=strlen(mystring)){//remove
    //std::cout << "ndx: " << ndx << "\n";
    //rc = Serial.read(); //picks up one at a time
    rc = mystring[ndx];//remove
    if(recvInProgress == true){
      if(rc != endMarker){ //while the chars aren't '>', keep going.
        recievedChars[ndx] = rc; // ndx increments with the loop, and is the char position of the new recieved char
        ndx++;
        if(ndx >= numChars){ //if ndx is position 32 or greater, ndx becomes position 31
          ndx = numChars - 1; 
          //How many characters can be received?
          //In the examples I have assumed that you will not need to receive more than 32 bytes. That can easily be altered by changing the value in the constant numChars.
          //Note that the 64 byte size of the Arduino serial input buffer does not limit the number of characters that you can receive because the code in the examples can empty the buffer faster than new data arrives.
        } 
      }else{
        //std::cout << "reached end \n";
        recvInProgress = false;
        recievedChars[ndx] = '\0'; //terminates the string, since will increment one more times than the # of chars
        ndx = 0;
        break; //remove, since causes infinite loop here, but is fine on arduino
      }
    }else if(rc == startMarker){
      recvInProgress = true; //if rc finds '<', then starts the if statement above
    }
  }
  
  //deletes the first char in char array by shifitng all entries left. NOT NEEDED IN ACTUAL ARDUINO!!!
  memmove(recievedChars,recievedChars+1,strlen(recievedChars)); //remove, since trying to mockup that Serial.read() would have deleted the '<'
  //so when the loop itterates again, index 0, will be 'X'
  //std::cout<<recievedChars<<"\n";
  //}
  return;

}

void coordAssigner(char fromSerial[],int (&coordsGiven)[],const char *delim,const char coordAxesGiven[]){
  //don't need to redefine default argument in function definition, just need to do it once 
  char *token;
  token = strtok(fromSerial, delim); //breaks out the tokens X100 Y200 and Z400 out
    //but if you check token, it only displays 1 at a time. Must call strtok again, but with NULL to get the next token
  int iteration = 0; 
  while(token != NULL){

    //printf("%c \n",token[0]); //%c for char datatype
    //printf("%c \n",coordAxesGiven[iteration]); //seems like to print the out the string "X" as a char, must use a pointer?
    if(token[0] == coordAxesGiven[iteration]){ //compares the first char from the token to the char of the coordAxes
    //so compares char to char
      //printf("it worked \n");
      token ++;
      //std::cout<<"token: " <<token << '\n';
      coordsGiven[iteration] = atoi(token);
    }
    iteration ++;
    //token ++; //just moves it past the letter X, Y or Z to the number (for displaying here only)
    //xcoord = std::stoi( token );
    //printf("%s \n",token);
    //printf("%s \n",xcoord);
    token = strtok(NULL,delimiter); //gets to the next token
  }
  //std:: allows to use functions from the standard library, std
}