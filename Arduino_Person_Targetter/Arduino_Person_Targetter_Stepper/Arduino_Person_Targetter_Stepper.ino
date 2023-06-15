#include<Servo.h>

#include <AccelStepper.h>
#include <elapsedMillis.h>

// Motor Connections (unipolar motor driver)
const int In1 = 8;
const int In2 = 9;
const int In3 = 10;
const int In4 = 11;
// Motor Connections (constant voltage bipolar H-bridge motor driver)
const int AIn1 = 8;
const int AIn2 = 9;
const int BIn1 = 10;
const int BIn2 = 11;
// Motor Connections (constant current, step/direction bipolar motor driver)
const int dirPin = 4;
const int stepPin = 5;

// Creates an instance - Pick the version you want to use and un-comment it. That's the only required change.
//AccelStepper myStepper(AccelStepper::FULL4WIRE, AIn1, AIn2, BIn1, BIn2);  // works for TB6612 (Bipolar, constant voltage, H-Bridge motor driver)
//AccelStepper myStepper(AccelStepper::FULL4WIRE, In1, In3, In2, In4);    // works for ULN2003 (Unipolar motor driver)
AccelStepper horizStepper(AccelStepper::DRIVER, stepPin, dirPin);           // works for a4988 (Bipolar, constant current, step/direction driver)
//AccelStepper::DRIVER (1) means a stepper driver (with Step and Direction pins). If an enable line is also needed, call setEnablePin() after construction. You may also invert the pins using setPinsInverted(). 
//AccelStepper::FULL2WIRE (2) means a 2 wire stepper (2 pins required). 
//AccelStepper::FULL3WIRE (3) means a 3 wire stepper, such as HDD spindle (3 pins required). 
//AccelStepper::FULL4WIRE (4) means a 4 wire stepper (4 pins required). 
//AccelStepper::HALF3WIRE (6) means a 3 wire half stepper, such as HDD spindle (3 pins required)
//AccelStepper::HALF4WIRE (8) means a 4 wire half stepper (4 pins required)

elapsedMillis printTime;


char data; //character to store the data
int initAngle;
int width;
int height;  // total resolution of the video
int xCoord=-1;
int yCoord=-1;
int desiredAngleHoriz;
bool initDone = false;
int centerCoord[2];

void setup() {
  Serial.begin(9600);
  Serial.flush(); //clear serial buffer first
  Serial.read();
  Serial.write("F"); //seems like it already comes encoded in arduino
  while(initDone == false){ //waits until resolution of camera is given before moving onto void loop
    if (Serial.available() > 0){
      if(Serial.read() == 'W'){ //DON'T USE DOUBLE QUOTES FOR CHAR TYPE!!!! CHAR AND STRING ARE DIFFERENT
        width = Serial.parseInt();
        if(Serial.read() == 'H'){ //must have both, not just one dimension before set to true, so if for some reason, just given H, doesn't continue
          height = Serial.parseInt();
          char buffer[400]; //this is just for display in the serial monitor. Sprintf needs a buffer for at leatst that many characters
          sprintf(buffer,"W: %d , H: %d",width,height); //%d for displaying var as signed decimal integer (just signed integer)
          Serial.println(buffer);
          initDone = true;
          centerCoord[1] = width/2;
          centerCoord[2] = height/2;
          sprintf(buffer,"Center Coordinate (X,Y): ( %d , %d )" , centerCoord[1],centerCoord[2]);
          Serial.println(buffer);
        }
      }
    }
    delay(100); //this is how the loop is delayed until the condition above is met
  }
  Serial.begin(9600);
  // set the maximum speed, acceleration factor, and the target position.
  horizStepper.setMaxSpeed(200.0);   // the motor accelerates to this speed exactly without overshoot. Try other values.
  horizStepper.setAcceleration(200.0);   // try other acceleration rates.
  horizStepper.moveTo(1000); 
  // For negative rotation
  //myStepper.moveTo(-10000);    // This will demonstrate a move in the negative direction.
  Serial.println("homing position"); //note, the program will tell the motor to run, and then move onto the next line of code, while the motor is running
  while(horizStepper.currentPosition() != 1000){
    horizStepper.run(); //keeps the motor initialized before starting the main loop
  }
  horizStepper.setCurrentPosition(0);
  initDone = true;
  printTime = 0;
}

int count = 0;    // tracks seconds to trigger an action if desired.
int desiredMoveHoriz = 0;
void loop() {
  float mSpeed;
  if(count == 0){
      Serial.println("in main loop");
      count = 1;  
      Serial.flush();
  }
  if (printTime >= 1000 && initDone == true) {    // reports speed and position each second
    if(Serial.available() > 0){
      if(Serial.read() == 'X'){ //remeber, single quotes for char
        desiredMoveHoriz = Serial.parseInt();// Serial.parseInt() u a blocking function
        char buffer[400];
        sprintf(buffer,"Desired Movement Horiz: %d ",desiredMoveHoriz);
        Serial.println(buffer);
        if(desiredMoveHoriz !=0){ //if a new movement is commanded, it stops immediately, to rediefine its reference point
           horizStepper.setAcceleration(10000.0); //quickly decelerate, and later accelerate after to target
           horizStepper.setMaxSpeed(0.0);
           horizStepper.stop();
           horizStepper.run();
           //also need to make sure that when move = 0, doesn't do it from position when you inputted movement, so it doesn't go backwards
          horizStepper.setMaxSpeed(200.0);
          //note, if speed is way larger than the movement size, it will overshoot, and try moving back
          if(abs(desiredMoveHoriz) * 10 <= horizStepper.maxSpeed()){
            horizStepper.setMaxSpeed(horizStepper.maxSpeed()/10);
          }
          //myStepper.moveTo(100);
          horizStepper.move(desiredMoveHoriz);
        }
        horizStepper.setAcceleration(200.0);//slowly decelerate
        

        //Problem now is that even if using move, if you update it to a lesser value, like 100 to 10, it will move backwards
                
        //myStepper.stop();
        //want it to update quickly, so can't wait for it to complete movement
        //while(myStepper.distanceToGo() !=0){
          //myStepper.run(); //will keep the code from moving on until the movement is complete
        //}
      }
      //printTime = 0; //makes it update too slowly
      mSpeed = horizStepper.speed();
      //Serial.print(mSpeed);
      //Serial.print("  ");
      //Serial.print(myStepper.currentPosition());
      //Serial.print("  ");
      //Serial.println(count);
    }
    count = 1; 
  }
  
  horizStepper.run(); // must be called continuously to make the motor run
}
