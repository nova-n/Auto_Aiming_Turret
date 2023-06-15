#include<Servo.h>

Servo horizServo; //"horizServo" is an object, and has all properties of servo library. Can access any part of that library for "horizServo"
Servo vertServo;
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
  horizServo.attach(9); //servo will be controlled by pin 9
  vertServo.attach(10);
  initAngle = 90;
  horizServo.write(90);
  Serial.write("F"); //seems like it already comes encoded in arduino
  while(initDone == false){ //waits until resolution of camera is given before moving onto void loop
    if (Serial.available() > 0){
      //Serial.read() is a blocking function. It stops the code until it is satasfied. Not good for smooth motion
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
}

void loop() {
  if (Serial.available() > 0 ){ //checks if the bytes coming into the serial port is greater than 0, aka connected
    //Searial.read takes out and reads one char at a time, so first it will read "X", from X11,Y22
    //next, the string becomes 11Y22 
    //next,it will parse 11, the first valid integer, and take it out, and so on and so forth...
    //Serial.println((char)Serial.read());
    if(Serial.read() == 'X'){ //DON'T USE DOUBLE QUOTES FOR CHAR TYPE!!!! CHAR AND STRING ARE DIFFERENT
      xCoord = Serial.parseInt();
      //Serial.print(xCoord); //try not filling the serial
      //Serial.print(" , ");
    }
    if(Serial.read() == 'Y'){
      yCoord = Serial.parseInt();
      //Serial.print(yCoord);
    }
    //Serial.println();
  }
  //map("Num to be scaled", "min input", "max input", "min degrees", "max degrees")
  if(xCoord > 0){
    //*if(xCoord <= centerCoord[1]){
      //desiredAngleHoriz = map( abs(centerCoord[1] - xCoord) , 0, width, 0 , 90);
      //char buffer[400];
      //sprintf(buffer,"Desired Horizontal Angle: %d (turning left)",desiredAngleHoriz);
      //Serial.println(buffer);
      //horizServo.write(desiredAngleHoriz);
    //}
    //if(xCoord > centerCoord[1]){
      //desiredAngleHoriz = map( abs(centerCoord[1] - xCoord) , 0, width, 91 , 179);
      //char buffer[400];
      //sprintf(buffer,"Desired Horizontal Angle: %d (turning right)",desiredAngleHoriz);
      //Serial.println(buffer);
      //horizServo.write(desiredAngleHoriz);
    //}
    desiredAngleHoriz = map( xCoord , 0, width, 0 , 179);
    char buffer[400];
    sprintf(buffer,"Desired Horizontal Angle: %d (turning left)",desiredAngleHoriz);
    //Serial.println(buffer);
    horizServo.write(desiredAngleHoriz);
  }
}
