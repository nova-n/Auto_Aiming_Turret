#include <AccelStepper.h>
#include <elapsedMillis.h>

//Serial stuff------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const byte numChars = 32; //assumes that serial buffer will give a max of 32 chars
char recievedChars[numChars]; //collects the serial buffer data into one a char array, or cstring 
char tempChars[numChars]; //temporarily holds the recievedChars for parsing. Neccesary to protect original data
const char coordAxes[] = {'X','Y'};
float coords[sizeof(coordAxes)] = {}; //initializes the array with 0s
const char *delimiter = ","; //two spaces allocates, one for ',', the other for '/0' which is hidden
bool coordsReady = false;

void recvStartEndMarkers();
void coordAssigner(char fromSerial[], float (&coordsGiven)[sizeof(coordAxes)], const char *delim = delimiter, const char coordAxesGiven[] = coordAxes);
//passing coords array by reference, to save space
//just need to define default arguments once, not in the definition below.
//need to specify size of coords array, or will get error "reference to array of unkown bound int"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



//Stepper Motor stuff-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Motor Connections (constant current, step/direction bipolar motor driver)
const int dirPin = 4;
const int stepPin = 5;
AccelStepper horizStepper(AccelStepper::DRIVER, stepPin, dirPin);// works for a4988 (Bipolar, constant current, step/direction driver)
int desiredMoveHoriz;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//Camera stuff------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
double cameraConeAngle = 75; //in degrees
double stepAngleSize = 1.8; //in degrees
int actualSteps = round(cameraConeAngle/stepAngleSize);
int cameraResolution[2]; //width, height 
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(9600);
  Serial.println("Arduino is ready!");
  //Serial.println(100.00/320.00);
  while(Serial.available() >0){
    Serial.read(); //clears the serial buffer in the setup
  }
  horizStepper.setMaxSpeed(1000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizStepper.setAcceleration(200.0);   // try other acceleration rates.
  horizStepper.move(-21);
  while(horizStepper.currentPosition() != -21){
    horizStepper.run(); //keeps the motor initialized before starting the main loop
  }
  recvStartEndMarkers();
  Serial.println("trying to get resolution");
  if(coordsReady == true){ //using coordsReady to see if the message (vidwidth,vidheight) has been sent. Not actual coords
    //X##Y##, where the value after X is width, and the value after Y is height of the video. Just easier than using W and H
    cameraResolution[0] = coords[0];
    cameraResolution[1] = coords[1];
    Serial.print("Camera Resolution: W:");
    Serial.print(cameraResolution[0]);
    Serial.print(", H:");
    Serial.println(cameraResolution[1]);
    coords[sizeof(coordAxes)] = {};
  }
}

void loop() {
  recvStartEndMarkers();
  if(coordsReady == true){
    coordAssigner(tempChars,coords); 
    coordsReady = false;
    //desiredMoveHoriz = -1 * coords[0];
    desiredMoveHoriz = -1.00 * round( coords[0]/320.00 * actualSteps);
    Serial.println(desiredMoveHoriz);
  }
  //horizStepper.moveTo(-1*(round(desiredMoveHoriz/320) * 21));
  horizStepper.moveTo(desiredMoveHoriz);
  horizStepper.run(); // must be called continuously to make the motor run
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
