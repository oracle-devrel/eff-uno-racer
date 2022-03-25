#include <Wire.h>
#include <Servo.h>

// Board: Arduino Micro
// Programmer: AVR ISP

#define CASE_MOTOR 1
#define CASE_SERVO1 2
#define CASE_SET_SERVO1_MIN 20
#define CASE_SET_SERVO1_MAX 21
#define CASE_SERVO2 3
#define CASE_SET_SERVO2_MIN 30
#define CASE_SET_SERVO2_MAX 31
#define CASE_LED 4

int escPin = 11;
int servo1Pin = 10;
int servo2Pin = 9;
//int tempProbe1Pin = 23;
//int tempProbe2Pin = 24;
int ledPin = 12;
//int rocketPin = 31;

int i2cAddress = 0x8;

Servo esc;
Servo servo1;
Servo servo2;

int status = 0;

int motorMin = 1000;
int motorMax = 2000;
int servo1Min = 0;
int servo1Max = 180;
int servo2Min = 0;
int servo2Max = 180;

#define ERROR_READING 1
#define INVALID_RANGE 2

void setup() {
  Serial.begin(9600);

  Wire.begin(i2cAddress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  esc.attach(escPin, motorMin, motorMax); // pin, min pulse width, max pulse width in microseconds
  esc.write(1500);                        // initialize the ESC
  delay(3000);

  servo1.attach(servo1Pin, servo1Min, servo1Max); // pin, min pulse width, max pulse width in microseconds
  
  servo2.attach(servo2Pin, servo2Min, servo2Max); // pin, min pulse width, max pulse width in microseconds

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println("starting");
}

bool step = false;
int motorspeed = 1500;

void loop() {
//  if (step) {
//    step = false;
//    esc.write(motorspeed);
//  }
//  else {
//    step = true;
//    esc.write(1500);
//  }
//  delay(150);

  esc.write(motorspeed);
}

boolean range(int min, int max, int value) {
  return (min <= value) && (value <= max);
}

// linear interpolation
int interpolate(int value, int minIn, int maxIn, int minOut, int maxOut) {
  int temp = (value - minIn) / (maxIn - minIn);
  return minOut + (maxOut - minOut) * temp;
}

void receiveEvent(int byteCount) {
  Serial.print("receiveEvent: "); 
  Serial.println(byteCount);

  if (byteCount > 1) {
    byte message = Wire.read();
 
    switch (message) {
      case CASE_MOTOR: {
        int speed = 0;   
      
        if (byteCount == 3) {
          byte high = Wire.read();
          byte low = Wire.read();
          Serial.println(high);
          Serial.println(low);
          speed = 0;
          speed = (high << 4) | low;
        }
        else {
          speed = Wire.read();
        }
        
        Serial.print("esc: ");
        Serial.println(speed);

        if (range(motorMin, motorMax, speed)) {
          //esc.write(speed); TODO Remove. Moved to loop for linear interpoloation
          motorspeed = speed;
        }
        else {
          status = INVALID_RANGE;
        }
        
        break;
      }

      case CASE_SERVO1: {
        byte degrees = Wire.read();
        Serial.print("servo: ");
        Serial.println(degrees);

        //50 max left
        //70 center
        //90 max right
        //degrees = interpolate(degrees, 50, 90, 0, 180);
          
        if (range(servo1Min, servo1Max, degrees)) {
          servo1.write(degrees);
        }
        else {
          status = INVALID_RANGE;
        }
        
        break;
      }

      case CASE_SET_SERVO1_MIN: {
        byte min = Wire.read();
        Serial.print("set servo1 min: ");
        Serial.println(min);
        servo1Min = min;
        break;                
      }

      case CASE_SET_SERVO1_MAX: {
        byte max = Wire.read();
        Serial.print("set servo1 max: ");
        Serial.println(max);
        servo1Max = max;
        break;  
      }


      case CASE_SERVO2: {
        byte degrees = Wire.read();
        Serial.print("servo: ");
        Serial.println(degrees);

        //50 max left
        //70 center
        //90 max right
        //degrees = interpolate(degrees, 50, 90, 0, 180);
          
        if (range(servo2Min, servo2Max, degrees)) {
          servo2.write(degrees);
        }
        else {
          status = INVALID_RANGE;
        }
        
        break;
      }

      case CASE_SET_SERVO2_MIN: {
        byte min = Wire.read();
        Serial.print("set servo1 min: ");
        Serial.println(min);
        servo2Min = min;
        break;                
      }

      case CASE_SET_SERVO2_MAX: {
        byte max = Wire.read();
        Serial.print("set servo1 max: ");
        Serial.println(max);
        servo2Max = max;
        break;  
      }

      case CASE_LED: {
        byte state = Wire.read();

        if (state == 0)
           digitalWrite(ledPin, LOW);
        else if (state == 1)
           digitalWrite(ledPin, HIGH);
        break;
      }
      
      default: {
        status = ERROR_READING;
      }
    }
  }
}

void requestEvent() {
  Serial.print("requestEvent ");
  Serial.println(status);
  Wire.write(status);
}
