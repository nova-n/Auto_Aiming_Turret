const byte numChars = 32; //assumes that serial buffer will give a max of 32 chars
char recievedChars[numChars]; //collects the serial buffer data into one a char array, or cstring 
char tempChars[numChars]; //temporarily holds the recievedChars for parsing. Neccesary to protect original data
const char coordAxes[] = {'X','Y'};
float coords[sizeof(coordAxes)] = {}; //initializes the array with 0s
const char *delimiter = ","; //two spaces allocates, one for ',', the other for '/0' which is hidden
bool coordsReady = false;
int cameraResolution[2]; //width, height


void recvStartEndMarkers();
void coordAssigner(char fromSerial[], float (&coordsGiven)[sizeof(coordAxes)], const char *delim = delimiter, const char coordAxesGiven[] = coordAxes);
 
void setup() {
  //declaring within scope of setup, so they are not global
  //once setup is exited, these get deleted, and RAM is freed up
  //cannot delete global variables
  char initializeGo;
  const int ledPin = 13;
  const int readyLedPin = 12;

  while(!Serial){
    //just wait for the port to be ready?
    delay(100);//can actually leave blank, but is a better way to wait for the pi to connect
  }
  Serial.begin(9600);
  delay(1000);//need to give time for the python program to start before anything is sent, otherwise, will write to nothing
  pinMode(ledPin,OUTPUT);
  pinMode(readyLedPin,OUTPUT);
  digitalWrite(ledPin,LOW);
  digitalWrite(readyLedPin,LOW);
  while(Serial.available() >0){
    Serial.read(); //clears the serial buffer in the setup
  }
  Serial.flush();
  Serial.write("Buffer Cleared, Ready To Start!\0"); //properly terminate with end of line char so that pyserial doesn't need # of bytes 
  //Serial.println("Buffer Cleared, Ready To Start!");
  //digitalWrite(ledPin,HIGH);
  //now, it is finally recieving some input, this time, the resolution
  while(coordsReady == false){  
    recvStartEndMarkers();
    if(digitalRead(ledPin) == LOW){
      digitalWrite(ledPin,HIGH);
    }else{
      digitalWrite(ledPin,LOW);
    }
    delay(100);
  }
  digitalWrite(ledPin,LOW);
  //It should be done receiving anything for now
  if(coordsReady == true){
    coordAssigner(tempChars,coords);
    cameraResolution[0] = coords[0];
    cameraResolution[1] = coords[1];
    //Serial.println(coords[0]); 
    //resetting coordinates and clearing char array tempChars
    coords[0] = 0.00;coords[1] = 0.00;
    memset(tempChars,0,numChars);
  }
  digitalWrite(readyLedPin,HIGH);
  String resolutionMessage = "The resolution is: ";
  resolutionMessage += cameraResolution[0];
  resolutionMessage += "x";
  resolutionMessage += cameraResolution[1];
  resolutionMessage += "\0";
  char charArrayResMsg[64]; // the buffer to put the string as a bunch of chars into
  resolutionMessage.toCharArray(charArrayResMsg, sizeof(charArrayResMsg) );
  Serial.write(charArrayResMsg);
  coordsReady = false;
}


void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(ledPin,HIGH);
}


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

  while(Serial.available() >0){
    rc = Serial.read(); //picks up one at a time
    if(recvInProgress == true){
      if(rc != endMarker){ //while the chars aren't '>', keep going.
        //Serial.println(rc);
        recievedChars[ndx] = rc; // ndx increments with the loop, and is the char position of the new recieved char
        //Serial.println(recievedChars);
        ndx++;
        if(ndx >= numChars){ //if ndx is position 32 or greater, ndx becomes position 31
          ndx = numChars - 1; 
          //How many characters can be received?
          //In the examples I have assumed that you will not need to receive more than 32 bytes. That can easily be altered by changing the value in the constant numChars.
          //Note that the 64 byte size of the Arduino serial input buffer does not limit the number of characters that you can receive because the code in the examples can empty the buffer faster than new data arrives.
        } 
      }else{
        //Serial.println(recievedChars);
        recvInProgress = false;
        recievedChars[ndx] = '\0'; //terminates the string, since will increment one more times than the # of chars
        strcpy(tempChars,recievedChars);//copies receivedChars onto tempChars
        ndx = 0;
        coordsReady = true;
        //digitalWrite(readyLedPin,HIGH);
      }
    }else if(rc == startMarker){
      recvInProgress = true; //if rc finds '<', then starts the if statement above
    }
  }
  //Serial.println(recievedChars);
  return;
}

void coordAssigner(char fromSerial[],float (&coordsGiven)[sizeof(coordAxes)],const char *delim,const char coordAxesGiven[]){
  //don't need to redefine default argument in function definition, just need to do it once 
  //need to specify size of coords array, or will get error "reference to array of unkown bound int"
  char *token;
  token = strtok(fromSerial, delim); //breaks out the tokens X100 Y200 and Z400 out
    //but if you check token, it only displays 1 at a time. Must call strtok again, but with NULL to get the next token
  int iteration = 0; 
  while(token != NULL){
    if(token[0] == coordAxesGiven[iteration]){ //compares the first char from the token to the char of the coordAxes, so compares char to char
      token ++; //just moves it past the letter X, Y or Z to the number
      coordsGiven[iteration] = atoi(token);
    }
    iteration ++;
    token = strtok(NULL,delimiter); //gets to the next token
  }
  for(int j = 0; j<sizeof(coordAxes);j++){
    //Serial.print(coords[j]);
    //Serial.print(",");
  }
  //Serial.println();
  return;
}
