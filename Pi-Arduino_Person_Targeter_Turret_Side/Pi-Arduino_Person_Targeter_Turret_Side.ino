//Used Serial Communications Basics from Arduino Forum by Robin2
//https://forum.arduino.cc/t/serial-input-basics-updated/382007

#include <AccelStepper.h>
#include <elapsedMillis.h>
#include <Servo.h> 

//Serial stuff------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const byte numChars = 32; //assumes that serial buffer will give a max of 32 chars
char recievedChars[numChars]; //collects the serial buffer data into one a char array, or cstring 
char tempChars[numChars]; //temporarily holds the recievedChars for parsing. Neccesary to protect original data
const char coordAxes[] = {'X','Y'};
float coords[sizeof(coordAxes)] = {}; //initializes the array with 0s
const char *delimiter = ","; //two spaces allocated, one for ',', the other for '/0' which is hidden
bool coordsReady = false;
float finalHorizGearRatio;

void recvStartEndMarkers();
void coordAssigner(char fromSerial[], float (&coordsGiven)[sizeof(coordAxes)], const char *delim = delimiter, const char coordAxesGiven[] = coordAxes);
//passing coords array by reference, to save space
//just need to define default arguments once, not in the definition below.
//need to specify size of coords array, or will get error "reference to array of unkown bound int"

//Note, arduino already has "parseInt()", which will take a string of numbers, such as "100", and turn it into an int 100
//but this is what is known as a "blocking function", meaning, it will stop the rest of the program from running until it is finished doing parseInt
//this makes the motor movement choppy
//the function call coordAssigner() uses "atoi()", which turns strings into integers, and is nonblocking, which means the rest of the program is able
//to run while atoi() is working

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//Stepper Motor stuff-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Motor Connections (constant current, step/direction bipolar motor driver)
const int dirPinX = 12;
const int stepPinX = 11;
AccelStepper horizStepper(AccelStepper::DRIVER, stepPinX, dirPinX);// works for a4988 (Bipolar, constant current, step/direction driver)
int desiredMoveHoriz;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//Temporary Servo Stuff--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int yServoPin = 9;
Servo yServo;
int desiredMoveYServo;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//Mock Fire Stuff------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int firePin = 7;
int buzzerPin = 8;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//Camera stuff------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
double cameraConeAngle = 66; //in degrees
double stepAngleSize = 1.8; //in degrees
int actualSteps = round(cameraConeAngle/stepAngleSize); //Finds how many steps it takes to traverse the full 66 degrees. In this case, 36.6666 --> 37 steps
int cameraResolution[2]; //width, height 
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void setup() {
  pinMode(firePin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  digitalWrite(firePin,LOW);
  digitalWrite(buzzerPin,LOW);
  
  char initializeGo; //declaring within scope of setup, so they are not global
  const int ledPin = 8; //once setup is exited, these get deleted, and RAM is freed up,
  const int readyLedPin = 7; //since cannot delete global variables

  yServo.write(180); //prevent servo from moving to the default position, since will move too low at default position
  yServo.attach(yServoPin);

  float horizStepperGearReduction = 4; //again, these variables are only defined in the setup's scope
  float horizTurretRatio = 3;
  finalHorizGearRatio = horizStepperGearReduction * horizTurretRatio;
  horizStepper.setCurrentPosition(0);
  vertStepper.setCurrentPosition(0);
  horizStepper.setMaxSpeed(1000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizStepper.setAcceleration(2500.0);   // try other acceleration rates.
  vertStepper.setMaxSpeed(1000.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  vertStepper.setAcceleration(2500.0);   // try other acceleration rates.
  
  while(!Serial){
    //just waits for the port to be ready, so that it will only continue when serial communication with the raspberry pi is open
    //This prevents the rest of the code from running when both the raspberry pi and arduino are powered on, but the raspberry pi does
    //not have the face detection program running yet.
    digitalWrite(buzzerPin,LOW);
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
  
  //now, it is finally recieving some input, this time, the resolution
  while(coordsReady == false){ //starts at false, 
    recvStartEndMarkers(); //uses the coordinate reader to receieve the resolution
    //where as coordinates would normally be sent as "<X100,Y100>", where they are the coordinates of the face on the camera's frame
    //Instead, it first recieves something like <X640,Y480>, getting the horizontal width of the camera, and the vertical height of the camera
    //in the function call recvStartEndMarkers() , it once it gets through the last char of the string '>' , it will set coordsReady to true.
    if(digitalRead(ledPin) == LOW){
      digitalWrite(ledPin,HIGH);//just so you can visually test if it received the string, it will normally be blinking.
    }else{
      digitalWrite(ledPin,LOW);
    }
    delay(100);
  }
  digitalWrite(ledPin,LOW);
  //It should be done receiving anything for now

  if(coordsReady == true){
    //once the stream of characters from serial have been nicely put into one char array from recvStartEndMarkers(),
    //it is put into coordsAssigner(a,b) , which turns that string into numbers
    coordAssigner(tempChars,coords);
    //Now, we have the camera resolution, so we know with the max camera angle what the max coordinate should map to.
    //In this case, the camera cone angle is 66 degrees, so the range is from -33 to 33 degrees
    //The horizontal camera resolution is 640px, so the coordinate spans from -320px to 320px
    //This means that -33 degrees maps to -320px, and 33 degrees maps to 320px
    cameraResolution[0] = coords[0];
    cameraResolution[1] = coords[1];
    //resetting coordinates and clearing char array tempChars
    coords[0] = 0.00;coords[1] = 0.00;
    memset(tempChars,0,numChars);
  }
  digitalWrite(readyLedPin,HIGH);
  //this is sent back to the rasperry pi, so that you can see this message in the terminal
  String resolutionMessage = "The resolution is: ";
  resolutionMessage += cameraResolution[0];
  resolutionMessage += "x";
  resolutionMessage += cameraResolution[1];
  resolutionMessage += "\0";
  char charArrayResMsg[64]; // the buffer to put the string as a bunch of chars into
  resolutionMessage.toCharArray(charArrayResMsg, sizeof(charArrayResMsg) );
  Serial.write(charArrayResMsg);
  coordsReady = false;
  horizStepper.move(-100);
  while(horizStepper.currentPosition() != -21){
    horizStepper.run(); //keeps the motor initialized before starting the main loop
  }
  delay(100);
}

void loop() {
  //the raspberry pi, in the code, is told to sent the entire coordinate string in one go "<X100,Y10>"
  //but because it is sent over serial, only one char is sent at a time.
  //use recvStartEndMarkers() to round up and collect characters.
  //starts picking up when the char it recieves is '<'
  recvStartEndMarkers(); //constantly calling this function
  //keeps collecting, until it hits '>'
  //Then, all chars are collected as one, into one charArray (almost like a string)
  //This is sent over to coordAssigner
  float x=horizStepper.currentPosition();
  if(coordsReady == true){
    if(tempChars[0] != '\n'){ // this is a holdover from testing. I used to manually input coords via the serial monitor, then new line
    //then it would print the coords back with new line being '\n'
      coordAssigner(tempChars,coords); 
    }
    coordsReady = false;//resets it to false

    //precision isn't that good, and I don't want it oscilating forever. So there is acceptable error that counts as being  "on target"
    if(abs(coords[0]) < 5.00){
      coords[0] = 0.0; 
    }
    if(abs(coords[1]) < 5.00){
      coords[1] = 0.0; 
    }
    if(coords[1] == NULL || isnan(coords[1]) == true){ 
      // NULL is sent over from the pi if no face is detected, otherwise, the turret will just keep on spinning in a circle
      //digitalWrite(7,HIGH); is a pin light for testing
    }else{
      //digitalWrite(7,LOW);
      //coords[1] controls the servo Y movement
      coords[1] = -1*coords[1];
      desiredMoveYServo = map(coords[1] , -240 , 240 , 57 , 123);
      if(coords[0] == 0.0 && abs(yServo.read() - desiredMoveYServo)<=10.0){
        digitalWrite(firePin,HIGH);
        digitalWrite(buzzerPin,HIGH);
      }else{
        digitalWrite(firePin,LOW);
        digitalWrite(buzzerPin,LOW);
      }
    }
    
  //remember, actual steps is how many steps it takes to traverse the full 66 degrees (37 steps)
  //so when a coordinate is sent over, say X100, then you only need to move a fraction of that 37 steps
  //in this case, 100/640 of the 37 steps
    desiredMoveHoriz = -1.00 * round( coords[0]/cameraResolution[0] * actualSteps);
    desiredMoveVert = -1.00 * round( coords[1]/cameraResolution[1] * actualSteps);
    //Serial.println(desiredMoveHoriz);
  }

  //very poor proportional control. It was a trial and error to see what worked best
  //Too fast a speed, and the camera would overshoot, and would give the camera a blurry feed
  //too slow was unacceptable
  //but one speed did not work well for all distances
  if(abs(coords[0]) > 200){
    horizStepper.setMaxSpeed(450.0);
  }else if(abs(coords[0]) > 160){
    horizStepper.setMaxSpeed(250.0);
  }else if(abs(coords[0]) > 100){
    horizStepper.setMaxSpeed(150.0);
  }else if(abs(coords[0]) > 50){
    horizStepper.setMaxSpeed(75.0);
  }else if(abs(coords[0]) > 20){
    horizStepper.setMaxSpeed(25.0);
  }else if(abs(coords[0]) <= 20){
    horizStepper.setMaxSpeed(5.0);
  }

  //So say, coord X100 was sent, so then the steps that the stepper needs to move is (100/320)*37 = 11.5625 ---> 12 steps
  //If you tell the stepper to move 12 steps, there is a 4:1 reduction on the stepper, so it will actually only move 3 steps
  //Multiply thre steps by the gearbox, so 12*4 = 48 steps
  //move to is not "move", so when the arduino starts, it sets the current location of the stepepr to 0
  //but as the stepper moves, the "position" updates (there is no feedback, as long as it is not skipping steps, it thinks it is at position)
  //so if ther stepper is at position 57, and you need it to move 12*4 steps forward, telling it to move to go 12*4 steps, it will move backwards
  //so, tell it to move to where it currently is PLUS the desired steps*gear ratio
  horizStepper.moveTo(desiredMoveHoriz*finalHorizGearRatio + x);
  horizStepper.run(); // must be called continuously to make the motor run
  //the coordinates keep changing, so as the void loop continues, it will constantly tell the motor to go to the most recently sent coordinate.
  
  if( yServo.read() < desiredMoveYServo){
    yServo.write( yServo.read() + 1);
  }else if( yServo.read() > desiredMoveYServo ){
    yServo.write( yServo.read() - 1);
  }
  
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
  static char rc;
  //is static bool recvInProgress, since entire coordinate is NOT sent all at once over serial

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
