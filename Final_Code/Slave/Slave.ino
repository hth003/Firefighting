#include <Average.h>  //used to perform math operations like average on an array, usueful for stabilizing data
#include <EasyTransfer.h>
#include <AFMotor.h>


EasyTransfer ETin, ETout;
AF_DCMotor motorLeft(3); 
AF_DCMotor motorRight(1);  // Have to use 2 separate bridges, otherwise we'll burn out L293D's easily.
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
int DISTANCE_THRESHOLD = 200;
int FOLLOW_DISTANCE_THRESHOLD_LOW = 125;
int FOLLOW_DISTANCE_THRESHOLD_HIGH = 225;


struct RECEIVE_DATA_STRUCTURE{
  char dir;
  boolean condition;
  int dist;
  char side;
  boolean align;
  boolean follow;
};

struct SEND_DATA_STRUCTURE{
  boolean done;
  int front;
  int left;
  int right;
};

SEND_DATA_STRUCTURE dataOut;
RECEIVE_DATA_STRUCTURE dataIn;

void setup(){
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  ETout.begin(details(dataOut), &Serial);
  ETin.begin(details(dataIn), &Serial);
  attachInterrupt(ENCODER_LEFT, countLeft, RISING);
  attachInterrupt(ENCODER_RIGHT, countRight, RISING);
  motorLeft.setSpeed(255);
  motorRight.setSpeed(255);
  motorLeft.run(RELEASE);
  motorRight.run(RELEASE);
}

void loop(){
  if (ETin.receiveData()){
    processData(); // Motors stay dormant until signal recieved, then process and move
  }
}

void processData(){
  dataOut.done = false;
  motorLeft.run(FORWARD);
  int distance = dataIn.dist;
  if (dataIn.dist == 0 && dataIn.follow){
    followWallUntilOpen(dataIn.side);
  } else if (dataIn.follow){
    followWall(dataIn.side, dataIn.dist);
  } else if (distance > 0){
    if (dataIn.dir == 'r'){
      right(distance);
    } else if (dataIn.dir == 'l'){
      left(distance);
    } else if (dataIn.dir == 'f'){
      forward(distance);
    } else if (dataIn.dir == 'b'){
      backward(distance);
    }
  } else {
    if (dataIn.dir == 'r'){
      rightCondition(dataIn.condition, dataIn.side);
    } else if (dataIn.dir == 'l'){
      leftCondition(dataIn.condition, dataIn.side);
    } else if (dataIn.dir == 'f'){
      forwardCondition(dataIn.condition, dataIn.side);
    } else {
      backwardCondition(dataIn.condition, dataIn.side);
    }
  }
  dataOut.front = analogRead(DIST_SENSOR_FRONT);
  dataOut.left = analogRead(DIST_SENSOR_LEFT);
  dataOut.right = analogRead(DIST_SENSOR_RIGHT);
  dataOut.done = true;
  ETout.sendData();
}

void align(){
    senseLR();
    while(((abs(leftIRavg - rightIRavg))/((leftIRavg+rightIRavg)/2)) > .50){
     if (leftIRavg < .9*rightIRavg){   // This will be if it is left in the hall
       left(30.0);
       forward(10);
       right(30.0);
     }
     if (rightIRavg < .9*leftIRavg){
       right(30.0);
       forward(10);
       left(30.0);
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

long avgSensor(int sensor){
  long sum = 0;
  for (int i = 0; i < 100; i ++){
    sum += analogRead(sensor);
  }
  return sum / 100;
}

void followWallUntilOpen(char side){
  int distanceSensor = DIST_SENSOR_RIGHT;
  int deg = 30;
  int fDist = 7;
  //int leftWheelDirection = FORWARD;
  //int rightWheelDirection = BACKWARD;
  if (side == 'l'){
    distanceSensor = DIST_SENSOR_LEFT;
    //leftWheelDirection = BACKWARD;
    //rightWheelDirection = FORWARD;
  }
  
  int distance = avgSensor(distanceSensor);
  if (distance > 75){
    do {
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
     
      if (distance < FOLLOW_DISTANCE_THRESHOLD_LOW){
        /*motorLeft.run(leftWheelDirection);
        motorRight.run(rightWheelDirection);
        delay(50);
        motorLeft.run(FORWARD);
        motorRight.run(FORWARD);
        delay(100);
        motorLeft.run(rightWheelDirection);
        motorRight.run(leftWheelDirection);
        delay(50);*/
        if (side = 'l'){
          left(deg);
          forward(fDist);
          right(deg/2);
        } else {
          right(deg);
          forward(fDist);
          left(deg/2);
        }
      } 
      distance = avgSensor(distanceSensor);
      if (distance < 75)
        break;
      
      if (distance > FOLLOW_DISTANCE_THRESHOLD_HIGH){
        /*motorLeft.run(rightWheelDirection);
        motorRight.run(leftWheelDirection);
        delay(50);
        motorLeft.run(FORWARD);
        motorRight.run(FORWARD);
        delay(100);
        motorLeft.run(leftWheelDirection);
        motorRight.run(rightWheelDirection);
        delay(50);*/
        if (side = 'r'){
          left(deg);
          forward(fDist);
          right(deg/2);
        } else {
          right(deg);
          forward(fDist);
          left(deg/2);
        }
      } 
      distance = avgSensor(distanceSensor);
      if (distance < 75)
        break;
      
      if (analogRead(DIST_SENSOR_FRONT) > 500){
        if (side == 'l'){
          right(90);
        } else {
          left(90);
        }
      }
      
      distance = avgSensor(distanceSensor);
    } while (distance > 75);
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
  
}

void followWall(char side, int dist){
  int distanceSensor = DIST_SENSOR_RIGHT;
  //int leftWheelDirection = FORWARD;
  //int rightWheelDirection = BACKWARD;
  if (side == 'l'){
    distanceSensor = DIST_SENSOR_LEFT;
    //leftWheelDirection = BACKWARD;
    //rightWheelDirection = FORWARD;
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
      forward(15);
      if (side == 'l'){
        left(90);
      }
      else{
        right(90);
      }
      forward(30);
    }
    
    if (analogRead(DIST_SENSOR_FRONT) > 600){
      if (side == 'l'){
        right(90);
      } else {
        left(90);
      }
    }
    
    ticksLeft = ticksLeftTemp;
    ticksRight = ticksRightTemp;
        
    if (distance < FOLLOW_DISTANCE_THRESHOLD_LOW){
      /*motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(100);
      motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);*/
      if (side = 'l'){
        left(20);
        forward(7);
        right(20);
      } else {
        right(20);
        forward(7);
        left(20);
      }
    }
    
    if (distance > FOLLOW_DISTANCE_THRESHOLD_HIGH){
      /*motorLeft.run(rightWheelDirection);
      motorRight.run(leftWheelDirection);
      delay(50);
      motorLeft.run(FORWARD);
      motorRight.run(FORWARD);
      delay(100);
      motorLeft.run(leftWheelDirection);
      motorRight.run(rightWheelDirection);
      delay(50);*/
      if (side = 'r'){
        left(20);
        forward(7);
        right(20);
      } else {
        right(20);
        forward(7);
        left(20);
      }
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

}

void forwardCondition(char condition, char side){
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
    if (dataIn.align){
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
      /*while (ticksLeft > ticksRight) {
        motorLeft.run(RELEASE);
      }*/
      motorLeft.run(FORWARD);
      /*while (ticksRight > ticksLeft){
        motorRight.run(RELEASE);
      }*/
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

}

void forward(int dist){
  int tickMult = TICK_MULT_STRAIT;
  if (dist < 50 && dist > 25){
    tickMult -= .0035;
  } else if (dist < 25){
    dist -= 2;
  }
  do{
   if (dataIn.align){
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
      /*if (ticksLeft - ticksRight > 50) {
        motorLeft.run(RELEASE);
        delay(5);
      }*/
      motorLeft.run(FORWARD);
      /*if (ticksRight - ticksLeft > 50){
        motorRight.run(RELEASE);
        delay(5);
      }*/
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

}

void backwardCondition(char condition, char side){
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
    if (dataIn.align){
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
      /*while (ticksLeft > ticksRight) {
        motorLeft.run(RELEASE);
      }*/
      motorLeft.run(FORWARD);
      /*while (ticksRight > ticksLeft){
        motorRight.run(RELEASE);
      }*/
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

}

void backward(double dist){
  if (dist < 25){
    dist -= 2;
  }
  do{
    /*while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }*/
    motorLeft.run(BACKWARD);
    /*while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }*/
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

}

void leftCondition(char condition, char side){
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

}


void left(double deg){
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

}


void rightCondition(char condition, char side){
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

}

void right(double deg){
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

}

void countLeft(){
  ticksLeft ++;
}

void countRight(){
  ticksRight ++;
}
