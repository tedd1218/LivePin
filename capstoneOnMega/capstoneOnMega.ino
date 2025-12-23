#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>

Adafruit_PWMServoDriver pwm;

#define SERVO_MIN 100
#define SERVO_MAX 600

#define ROWS 1
#define COLUMNS 32

Servo servo16;
Servo servo17;
Servo servo18;
Servo servo19;
Servo servo20;
Servo servo21;
Servo servo22;
Servo servo23;
Servo servo24;
Servo servo25;
Servo servo26;
Servo servo27;
Servo servo28;
Servo servo29;
Servo servo30;
Servo servo31;
Servo servo32;

int servoPins[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

// int servoPins[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
Servo gpioServos[16] = {
  servo16, servo17, servo18, servo19, 
  servo20, servo21, servo22, servo23, 
  servo24, servo25, servo26, servo27, 
  servo28, servo29, servo30, servo31
};

int depthArray[32] = {
  30, 60, 90, 120, 
  150, 180, 30, 60, 
  90, 120, 150, 180, 
  30, 60, 90, 120, 30, 
  60, 90, 120, 150, 180, 
  30, 60, 90, 120, 150, 
  180, 30, 60, 90, 120
};

// left motor
const int stepPin = 5;
const int dirPin = 4;

// right motor
const int stepPin2 = 8;
const int dirPin2 = 7;

volatile int RESET_SIGNAL;
// must be 2 because it is an interrupt pin
const int resetButtonPin = 2;

// reset actuators
const int A_DIR_PIN = 9;
const int A_PWM_PIN = 11;

// reset actuator B pins
const int B_DIR_PIN = 12;
const int B_PWM_PIN = 13;

// reset mechanism speeds
const int extendA_speed = 50;
const int extendB_speed = 150;

const int retractA_speed = 150;
const int retractB_speed = 53;

const unsigned long FULL_TRAVEL_TIME = 26000;

#define LIMIT_SWITCH_PIN 10

int IMAGE_DONE;
int LIMIT_SWITCH;

int angleToPulseWidth(int angle) {
  angle = constrain(angle, 0, 180);
  return map(angle, 0, 180, 1000, 2000);
}

// reset actutors A
void extendA(int speed = extendA_speed) {
  digitalWrite(A_DIR_PIN, LOW);
  analogWrite(A_PWM_PIN, constrain(speed, 0, 255));
  Serial.println("A EXTENDING");
}

void retractA(int speed = retractA_speed) {
  digitalWrite(A_DIR_PIN, HIGH);
  analogWrite(A_PWM_PIN, constrain(speed, 0, 255));
  Serial.println("A RETRACTING");
}

void stopA() {
  analogWrite(A_PWM_PIN, 0);
  Serial.println("A STOPPED");
}

// reset actuators B
void extendB(int speed = extendB_speed) {
  digitalWrite(B_DIR_PIN, HIGH);
  analogWrite(B_PWM_PIN, constrain(speed, 0, 255));
  Serial.println("B EXTENDING");
}

void retractB(int speed = retractB_speed) {
  digitalWrite(B_DIR_PIN, LOW);
  analogWrite(B_PWM_PIN, constrain(speed, 0, 255));
  Serial.println("B RETRACTING");
}

void stopB() {
  analogWrite(B_PWM_PIN, 0);
  Serial.println("B STOPPED");
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.print("begin setup\n");

  RESET_SIGNAL = 0;
  IMAGE_DONE = 0;
  LIMIT_SWITCH = 0;
  pinMode(resetButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(resetButtonPin), resetRoutine, FALLING);

  // // set up stepper motors
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);

  // reest Actuator A pins
  pinMode(A_DIR_PIN, OUTPUT);
  pinMode(A_PWM_PIN, OUTPUT);

  // reset Actuator B pins
  pinMode(B_DIR_PIN, OUTPUT);
  pinMode(B_PWM_PIN, OUTPUT);

  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);

  // Start with both motors off
  digitalWrite(A_DIR_PIN, LOW);
  analogWrite(A_PWM_PIN, 0);

  digitalWrite(B_DIR_PIN, LOW);
  analogWrite(B_PWM_PIN, 0);

  // extend reset pane
  extendA();
  extendB();
  delay(FULL_TRAVEL_TIME);

  stopA();
  stopB();

  int servoPort = 22;
  for (int i = 0; i < 16; i++) {
    gpioServos[i].attach(servoPort);
    servoPort++;
  }


  // pwm.begin();
  // Serial.print("pwm begin passed\n");
  // pwm.setOscillatorFrequency(27000000); 
  // pwm.setPWMFreq(50);
  // Wire.setClock(400000);  

  Serial.print("configured\n");
  delay(10);

  // for (int i = 0; i < 10; i++) {
  //   pwm.writeMicroseconds(servoPins[i], angleToPulseWidth(0));
  // }
  gpioServos[0].write(0);
  delay(1000);

  // pwm.writeMicroseconds(servoPins[9], angleToPulseWidth(0));
  // delay(1000);
  // pwm.writeMicroseconds(servoPins[9], angleToPulseWidth(180));
  // delay(1000);

  Serial.print("reset to 0\n");

  // for(int i = 0; i < 16; i++) {
  //   gpioServos[i].write(0);
  // }
}

void loop() {
  // Serial.print("begin (actual) loop\n");
  if ((!RESET_SIGNAL) && (!IMAGE_DONE)) {
    // for (int j = 0; j < 10; j++) {
    //   pwm.writeMicroseconds(servoPins[j], angleToPulseWidth(depthArray[j]));
    // }
    // gpioServos[0].write(depthArray[11]);

    // delay(1000);

    // for (int i = 0; i < 10; i++) {
    //   pwm.writeMicroseconds(servoPins[i], angleToPulseWidth(0));
    // }
    gpioServos[0].write(0);

    delay(1000);

    // moving up
    digitalWrite(dirPin, HIGH);
    digitalWrite(dirPin2, LOW);

    for (long x = 0; x < 1700; x++) {
      digitalWrite(stepPin, HIGH);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(100);
      digitalWrite(stepPin, LOW);
      digitalWrite(stepPin2, LOW);
      delayMicroseconds(100);
    }

    delay(1000);

  }
    // IMAGE_DONE = 1;
}

void resetRoutine() {
  Serial.print("abset button pressed\n");
  RESET_SIGNAL = 1;

  // retract all pins
  // for(int i = 0; i < 16; i++) {
  //   pwm.setPWM(servoPins[i], 0, SERVO_MIN);
  //   gpioServos[i].write(0);
  // }

  // move gantry down
  digitalWrite(dirPin, LOW);
  digitalWrite(dirPin2, HIGH);
  while(digitalRead(LIMIT_SWITCH_PIN) == LOW) {
    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(100);
    digitalWrite(stepPin, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(100);
  }

  // activate reset mechanism
    retractA();
    retractB();
    delay(FULL_TRAVEL_TIME);

    stopA();
    stopB();
    delay(500);
}