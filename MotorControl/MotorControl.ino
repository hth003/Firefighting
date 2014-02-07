#include <EasyTransfer.h>
#include <AFMotor.h>

EasyTransfer ET;
AF_DCMotor motorLeft(3);
AF_DCMotor motorRight(1);
int ENCODER_LEFT = 1;  // Argument of 1 means encoder is connected to pin 3
int ENCODER_RIGHT = 0; // Argument of 0 means encode is connected to pin 2
long ticksLeft = 0;
long ticksRight = 0;
double TICK_MULT_STRAIT = .02637581; // Real value is 0.02637581
double TICK_MULT_TURN = TICK_MULT_STRAIT * 6.0311347;
boolean OPEN = true;
boolean CLOSED = false;
int DIST_SENSOR_FRONT = A0;
int DIST_SENSOR_LEFT = A1;
int DIST_SENSOR_RIGHT = A2;

struct RECEIVE_DATA_STRUCTURE{
  char dir;
  boolean condition;
  int dist;
  char side;
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
  /* Put in serial code to get move commands here.
     Code should wait for instruction, act, then send return signal. */
  if (ET.receiveData()){
    processData();
  }
}

void processData(){
  int distance = data.dist;
  if (data.dist){
    if (data.dir == 'r'){
      right(distance);
      done();
    } else if (data.dir == 'l'){
      left(distance);
      done();
    } else if (data.dir == 'f'){
      forward(distance);
      done();
    } else if (data.dir == 'b'){
      backward(distance);
      done();
    }
  } else {
    if (data.dir == 'r'){
      rightCondition(data.condition, data.side);
      done();
    } else if (data.dir == 'l'){
      leftCondition(data.condition, data.side);
      done();
    } else if (data.dir == 'f'){
      forwardCondition(data.condition, data.side);
      done();
    } else if (data.dir == 'b'){
      backwardCondition(data.condition, data.side);
      done();
    }
  }
}

void done(){
  data.dir = 0;
  data.condition = 0;
  data.dist = 0;
  data.side = 0;
  ET.sendData();
}

void forwardCondition(char condition, char side){
  int distSensor = DIST_SENSOR_FRONT;
  if (side == 'l'){
    distSensor = DIST_SENSOR_LEFT;
  } else if (side == 'r'){
    distSensor = DIST_SENSOR_RIGHT;
  }
  boolean conditionMet = false;
  while (!conditionMet){
    motorLeft.run(FORWARD);
    motorRight.run(FORWARD);
    
    while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }
    motorLeft.run(FORWARD);
    while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }
    motorRight.run(FORWARD);
    
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > 512);
    } else {
      conditionMet = (analogRead(distSensor) <= 512);
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

void forward(double dist){
  int tickMult = TICK_MULT_STRAIT;
  if (dist < 50 && dist > 25){
    tickMult -= .0035;
  } else if (dist < 25){
    dist -= 2;
  }
  do{
    motorLeft.run(FORWARD);
    motorRight.run(FORWARD);
    
    while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }
    motorLeft.run(FORWARD);
    while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }
    motorRight.run(FORWARD);
    
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
  while (!conditionMet){
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
    
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > 512);
    } else {
      conditionMet = (analogRead(distSensor) <= 512);
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
    
    while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }
    motorLeft.run(BACKWARD);
    while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }
    motorRight.run(FORWARD);
    
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > 512);
    } else {
      conditionMet = (analogRead(distSensor) <= 512);
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
    
    while (ticksLeft > ticksRight) {
      motorLeft.run(RELEASE);
    }
    motorLeft.run(FORWARD);
    while (ticksRight > ticksLeft){
      motorRight.run(RELEASE);
    }
    motorRight.run(BACKWARD);
    
    if (condition == OPEN){
      conditionMet = (analogRead(distSensor) > 512);
    } else {
      conditionMet = (analogRead(distSensor) <= 512);
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
