#include <Wire.h>
#include <Servo.h>

// Board: Arduino Micro
// Programmer: AVR ISP

int escPin = 11;
int servo2Pin = 10;
int servoPin = 9;
//int tempProbe1Pin = 23;
//int tempProbe2Pin = 24;
//int ledPin = 30;
//int rocketPin = 31;

int i2cAddress = 0x8;

Servo esc;
Servo servo;

int status = 0;

#define ERROR_READING 1
#define INVALID_RANGE 2


void setup() {
  Serial.begin(9600);

  Wire.begin(i2cAddress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  esc.attach(escPin, 1500, 2000); // pin, min pulse width, max pulse width in microseconds
  esc.write(1500);                // initialize the ESC
  delay(3000);

  servo.attach(servoPin, 0, 180); // pin, min pulse width, max pulse width in microseconds

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
      case 1: {
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

        if (range(1000, 2000, speed)) {
          //esc.write(speed);
          motorspeed = speed;
        }
        else {
          status = INVALID_RANGE;
        }
        
        break;
      }

      case 2: {
        byte degrees = Wire.read();
        Serial.print("servo: ");
        Serial.println(degrees);

        //50 max left
        //70 center
        //90 max right
        //degrees = interpolate(degrees, 50, 90, 0, 180);
          
        if (range(0, 180, degrees)) {
          servo.write(degrees);
        }
        else {
          status = INVALID_RANGE;
        }
        
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
