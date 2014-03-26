#include <Average.h>  //used to perform math operations like average on an array, usueful for stabilizing data
#include <EasyTransfer.h>
#include <AFMotor.h>


EasyTransfer ET;
AF_DCMotor motorLeft(4); 
AF_DCMotor motorRight(3);
int ENCODER_LEFT = 1;  // Argument of 1 means encoder is connected to pin 3
int ENCODER_RIGHT = 0; // Argument of 0 means encode is connected to pin 2
long ticksLeft = 0;
long ticksRight = 0;
double TICK_MULT_STRAIT = .02637581; // Real value is 0.02637581
double TICK_MULT_TURN = TICK_MULT_STRAIT * 6.0311347;
boolean OPEN = true;
boolean CLOSED = false;
boolean ALIGN = true;
boolean NO_ALIGN = false;
boolean FOLLOW = true;
boolean NO_FOLLOW = false;
int DIST_SENSOR_FRONT = A0;
int DIST_SENSOR_LEFT = A1;
int DIST_SENSOR_RIGHT = A2;
float leftarray[5];    //Array used to store several values of left distance sensor
float rightarray[5];   //Array used to store several values of right distance sensor
float leftIRavg;
float rightIRavg;
int MIN_DIST = 550; //Align robot after 25 cm of movement
int DISTANCE_THRESHOLD = 250;
int FOLLOW_DISTANCE_THRESHOLD_LOW = 175;
int FOLLOW_DISTANCE_THRESHOLD_HIGH = 250;



struct RECEIVE_DATA_STRUCTURE{
  char dir;
  boolean condition;
  int dist;
  char side;
  boolean align;
  boolean follow;
};

RECEIVE_DATA_STRUCTURE data;

void setup(){
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  ET.begin(details(data), &Serial);
  attachInterrupt(ENCODER_LEFT, countLeft, RISING);
  attachInterrupt(ENCODER_RIGHT, countRight, RISING);
  motorLeft.setSpeed(255);
  motorRight.setSpeed(255);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

void loop(){
  if (ET.receiveData()){
    processData(); // Motors stay dormant until signal recieved, then process and move
  }
}

void processData(){
  int distance = data.dist;
  if (data.dist == 0 && data.follow){
    followWallUntilOpen(data.side, true);
  } else if (data.follow){
    followWall(data.side, data.dist, true);
  } else if (distance > 0){
    if (data.dir == 'r'){
      right(distance, true);
    } else if (data.dir == 'l'){
      left(distance, true);
    } else if (data.dir == 'f'){
      forward(distance, true);
    } else if (data.dir == 'b'){
      backward(distance, true);
    }
  } else {
    if (data.dir == 'r'){
      rightCondition(data.condition, data.side, true);
    } else if (data.dir == 'l'){
      leftCondition(data.condition, data.side, true);
    } else if (data.dir == 'f'){
      forwardCondition(data.condition, data.side, true);
    } else if (data.dir == 'b'){
      backwardCondition(data.condition, data.side, true);
    }
  }
}

void done(){
  data.dir = -1;
  data.condition = 0;
  data.dist = 0;
  data.side = 0;
  ET.sendData();
}

void align(){
    senseLR();
    while(((abs(leftIRavg - rightIRavg))/((leftIRavg+rightIRavg)/2)) > .50){
     if (leftIRavg < .9*rightIRavg){   // This will be if it is left in the hall
       left(30.0, false);
       forward(10, false);
       right(30.0, false);
     }
     if (rightIRavg < .9*leftIRavg){
       right(30.0, false);
       forward(10, false);
       left(30.0, false);
     }
    senseLR();
   }
 }
 
void senseLR(){ //This function returns an 2 length array with a left and right IR sensor value.  These two values are composed of the average of 10 of their values.
  leftarray[0] = analogRead(DIST_SENSOR_LEFT);
  rightarray[0] = analogRead(DIST_SENSOR_RIGHT);
  delay(3);
  leftarray[1] = analogRead(DIST_SENSOR_LEFT);
  rightarray[1] = analogRead(DIST_SENSOR_RIGHT);
  delay(3);
  leftarray[2] = analogRead(DIST_SENSOR_LEFT);
  rightarray[2] = analogRead(DIST_SENSOR_RIGHT);
  delay(3);
  leftarray[3] = analogRead(DIST_SENSOR_LEFT);
  rightarray[3] = analogRead(DIST_SENSOR_RIGHT);
  delay(3);
  leftarray[4] = analogRead(DIST_SENSOR_LEFT);
  rightarray[4] = analogRead(DIST_SENSOR_RIGHT);
  delay(3);
  leftIRavg = mean(leftarray, 5);
  rightIRavg = mean(rightarray, 5);
}

void followWallUntilOpen(char side, boolean final){
  int distanceSensor = DIST_SENSOR_RIGHT;
  int leftWheelDirection = FORWARD;
  int rightWheelDirection = BACKWARD;
  if (side == 'l'){
    distanceSensor = DIST_SENSOR_LEFT;
    leftWheelDirection = BACKWARD;
    rightWheelDirection = FORWARD;
  }
  
  int distance = analogRead(distanceSensor);
  while (distance > 75) {
    motorLeft.run(FORWARD);
    motorRight.run(FORWARD);
   
    if (distance < FOLLOW_DISTANCE_THRESHOLD_LOW){
      motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(150);
      motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);
    }
    
    if (distance > FOLLOW_DISTANCE_THRESHOLD_HIGH){
      motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(100);
      motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);
    }
    distance = analogRead(distanceSensor);
  }  
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void followWall(char side, int dist, boolean final){
  int distanceSensor = DIST_SENSOR_RIGHT;
  int leftWheelDirection = FORWARD;
  int rightWheelDirection = BACKWARD;
  if (side == 'l'){
    distanceSensor = DIST_SENSOR_LEFT;
    leftWheelDirection = BACKWARD;
    rightWheelDirection = FORWARD;
  }
  do {
    int distance = analogRead(distanceSensor);
    motorLeft.run(FORWARD);
    motorRight.run(FORWARD);
    
    int ticksLeftTemp = ticksLeft;
    int ticksRightTemp = ticksRight;
    ticksLeft = 0;
    ticksRight = 0;

    if (distance < 75){
      forward(15, false);
      if (side == 'l'){
        left(90, false);
      }
      else{
        right(90, false);
      }
      forward(30, false);
    }
    
    if (analogRead(DIST_SENSOR_FRONT) > 400){
      if (side == 'l'){
        right(90, false);
      } else {
        left(90, false);
      }
    }
    
    ticksLeft = ticksLeftTemp;
    ticksRight = ticksRightTemp;
        
    if (distance < FOLLOW_DISTANCE_THRESHOLD_LOW){
      motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(150);
      motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);
    }
    
    if (distance > FOLLOW_DISTANCE_THRESHOLD_HIGH){
      motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(100);
      motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);
    }
    
  } while (ticksLeft * TICK_MULT_STRAIT < dist || ticksRight * TICK_MULT_STRAIT < dist);
  
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void forwardCondition(char condition, char side, boolean final){
  int distSensor = DIST_SENSOR_FRONT;
  if (side == 'l'){
    distSensor = DIST_SENSOR_LEFT;
  } else if (side == 'r'){
    distSensor = DIST_SENSOR_RIGHT;
  }
  boolean conditionMet = false;
  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
  while (!conditionMet){
    if (data.align){
      senseLR();
      if (leftIRavg < rightIRavg){   // This will be if it is left in the hall
        motorRight.run(BACKWARD);
        delay(20);
        motorRight.run(FORWARD);
        delay(50);
      }
      if (rightIRavg < leftIRavg){  // This will be if it is right in the hall
        motorLeft.run(BACKWARD);
        delay(20);
        motorLeft.run(FORWARD);
        delay(50);
      }
      senseLR();
    } else {
      while (ticksLeft > ticksRight) {
        motorLeft.run(RELEASE);
      }
      motorLeft.run(FORWARD);
      while (ticksRight > ticksLeft){
        motorRight.run(RELEASE);
      }
      motorRight.run(FORWARD);
    }
    
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > DISTANCE_THRESHOLD);
    } else {
      conditionMet = (analogRead(distSensor) <= DISTANCE_THRESHOLD);
    }
  }
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void forward(int dist, boolean final){
  int tickMult = TICK_MULT_STRAIT;
  if (dist < 50 && dist > 25){
    tickMult -= .0035;
  } else if (dist < 25){
    dist -= 2;
  }
  do{
   if (data.align){
      senseLR();
      if (leftIRavg < rightIRavg){   // This will be if it is left in the hall
        motorRight.run(BACKWARD);
        delay(20);
        motorRight.run(FORWARD);
        delay(50);
      }
      if (rightIRavg < leftIRavg){  // This will be if it is right in the hall
        motorLeft.run(BACKWARD);
        delay(20);
        motorLeft.run(FORWARD);
        delay(50);
      }
      senseLR();
    } else {
      while (ticksLeft > ticksRight) {
        motorLeft.run(RELEASE);
      }
      motorLeft.run(FORWARD);
      while (ticksRight > ticksLeft){
        motorRight.run(RELEASE);
      }
      motorRight.run(FORWARD);
    }
        
  } while (ticksLeft * TICK_MULT_STRAIT < dist || ticksRight * TICK_MULT_STRAIT < dist);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void backwardCondition(char condition, char side, boolean final){
  int distSensor = DIST_SENSOR_FRONT;
  if (side == 'l'){
    distSensor = DIST_SENSOR_LEFT;
  } else if (side == 'r'){
    distSensor = DIST_SENSOR_RIGHT;
  }
  boolean conditionMet = false;
  motorLeft.run(BACKWARD);
  motorRight.run(BACKWARD);
  while (!conditionMet){
    if (data.align){
      senseLR();
      if (leftIRavg < rightIRavg){   // This will be if it is left in the hall
        motorRight.run(FORWARD);
        delay(20);
        motorRight.run(BACKWARD);
        delay(50);
      }
      if (rightIRavg < leftIRavg){  // This will be if it is right in the hall
        motorLeft.run(FORWARD);
        delay(20);
        motorLeft.run(BACKWARD);
        delay(50);
      }
      senseLR();
    } else {
      while (ticksLeft > ticksRight) {
        motorLeft.run(RELEASE);
      }
      motorLeft.run(FORWARD);
      while (ticksRight > ticksLeft){
        motorRight.run(RELEASE);
      }
      motorRight.run(FORWARD);
    }
  }
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void backward(double dist, boolean final){
  if (dist < 25){
    dist -= 2;
  }
  do{
    motorLeft.run(BACKWARD);
    motorRight.run(BACKWARD);
    
    while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }
    motorLeft.run(BACKWARD);
    while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }
    motorRight.run(BACKWARD);
    
  } while (ticksLeft * TICK_MULT_STRAIT < dist || ticksRight * TICK_MULT_STRAIT < dist);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(FORWARD);
  motorRight.run(FORWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void leftCondition(char condition, char side, boolean final){
  int distSensor = DIST_SENSOR_FRONT;
  if (side == 'l'){
    distSensor = DIST_SENSOR_LEFT;
  } else if (side == 'r'){
    distSensor = DIST_SENSOR_RIGHT;
  }
  boolean conditionMet = false;
  while (!conditionMet){
    motorLeft.run(BACKWARD);
    motorRight.run(FORWARD);
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > DISTANCE_THRESHOLD);
    } else {
      conditionMet = (analogRead(distSensor) <= DISTANCE_THRESHOLD);
    }
  }
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(FORWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}


void left(double deg, boolean final){
  if (deg > 30){
    deg -= 5;
  } else if (deg > 15){
    deg -= 3;
  }
  do{
    motorLeft.run(BACKWARD);
    motorRight.run(FORWARD);
    
  } while (ticksLeft * TICK_MULT_TURN < deg & ticksRight * TICK_MULT_TURN < deg);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);

  motorLeft.run(FORWARD);
  motorRight.run(BACKWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);

  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}


void rightCondition(char condition, char side, boolean final){
  int distSensor = DIST_SENSOR_FRONT;
  if (side == 'l'){
    distSensor = DIST_SENSOR_LEFT;
  } else if (side == 'r'){
    distSensor = DIST_SENSOR_RIGHT;
  }
  boolean conditionMet = false;
  while (!conditionMet){
    motorLeft.run(FORWARD);
    motorRight.run(BACKWARD);
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > DISTANCE_THRESHOLD);
    } else {
      conditionMet = (analogRead(distSensor) <= DISTANCE_THRESHOLD);
    }
  }
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  motorLeft.run(BACKWARD);
  motorRight.run(FORWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void right(double deg, boolean final){
  if (deg > 30){
    deg -= 5;
  } else if (deg > 15){
    deg -= 3;
  }
  do{
    motorLeft.run(FORWARD);
    motorRight.run(BACKWARD);
    
  } while (ticksLeft * TICK_MULT_TURN < deg & ticksRight * TICK_MULT_TURN < deg);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);

  motorLeft.run(BACKWARD);
  motorRight.run(FORWARD);
  delay(100);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
  
  ticksLeft = 0;
  ticksRight = 0;
  if (final){
    done();
  }
}

void countLeft(){
  ticksLeft ++;
}

void countRight(){
  ticksRight ++;
}