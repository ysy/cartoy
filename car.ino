/**
 4WD car with IR receiver. 

 coded by YuanSuyi @ 20230530

*/
#include <HCSR04.h>
#include <IRremote.h>
static int ledPin = 13;
static int delayPeriod = 500;
static bool _running = false;
static bool _hc_enable = true;

static const int IR_RECEIVE_PIN = 12;
static const int LEFT_FRONT_A = 4;
static const int LEFT_FRONT_B = 5;
static const int RIGHT_FRONT_A = 2;
static const int RIGHT_FRONT_B = 3;
static const int REAR_A = 6;
static const int REAR_B = 7;


static const int HC_TRIGGER_PIN = 8;
static const int HC_ECHO_PIN = 9;

typedef enum {
  DIR_NONE, 
  DIR_FORWARD,
  DIR_RIGHT,
  DIR_LEFT,
  DIR_BACKWARD,
} direction_t; 

static direction_t _dir = DIR_NONE;


static void motor_setup() {
  pinMode(LEFT_FRONT_A, OUTPUT);
  pinMode(LEFT_FRONT_B, OUTPUT);
  pinMode(RIGHT_FRONT_A, OUTPUT);
  pinMode(RIGHT_FRONT_A, OUTPUT);
  pinMode(REAR_A, OUTPUT);
  pinMode(REAR_B, OUTPUT);
}

static void motor_stop() {
  Serial.println("MOTOR_STOP");
  digitalWrite(LEFT_FRONT_A, LOW);
  digitalWrite(LEFT_FRONT_B, LOW);

  digitalWrite(RIGHT_FRONT_A, LOW);
  digitalWrite(RIGHT_FRONT_B, LOW);

  digitalWrite(REAR_A, LOW);
  digitalWrite(REAR_B, LOW);

  _running = false;
  _dir = DIR_NONE;
}

static void motor_forward() {
  Serial.println("MOTOR_FORWARD");
  digitalWrite(LEFT_FRONT_A, HIGH);
  digitalWrite(LEFT_FRONT_B, LOW);

  digitalWrite(RIGHT_FRONT_A, HIGH);
  digitalWrite(RIGHT_FRONT_B, LOW);

  digitalWrite(REAR_A, HIGH);
  digitalWrite(REAR_B, LOW);
  _running = true;
  _dir = DIR_FORWARD;
}

static void motor_backward() {
  Serial.println("MOTOR_BACKWARD");
  digitalWrite(LEFT_FRONT_A, LOW);
  digitalWrite(LEFT_FRONT_B, HIGH);

  digitalWrite(RIGHT_FRONT_A, LOW);
  digitalWrite(RIGHT_FRONT_B, HIGH);


  digitalWrite(REAR_A, LOW);
  digitalWrite(REAR_B, HIGH);
  _running = true;
  _dir = DIR_BACKWARD;
}

static void motor_right() {
  motor_forward();
  Serial.println("MOTOR_LEFT");
  digitalWrite(LEFT_FRONT_A, LOW);
  digitalWrite(LEFT_FRONT_B, LOW);

  digitalWrite(RIGHT_FRONT_A, HIGH);
  digitalWrite(RIGHT_FRONT_B, LOW);

  _running = true;
  _dir = DIR_RIGHT;
}

static void motor_left() {
  motor_forward();
  Serial.println("MOTOR_RIGHT");
  digitalWrite(LEFT_FRONT_A, HIGH);
  digitalWrite(LEFT_FRONT_B, LOW);

  digitalWrite(RIGHT_FRONT_A, LOW);
  digitalWrite(RIGHT_FRONT_B, LOW);

  _running = true;
  _dir = DIR_LEFT;
}


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  motor_setup();
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  HCSR04.begin(HC_TRIGGER_PIN, HC_ECHO_PIN);

  //pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Start");
  randomSeed(analogRead(0));

 // motor_forward();
}


//红外控制
static ir_control() {
  if (IrReceiver.decode()) {
      Serial.println("loop decode");
      Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
      //Serial.println(IrReceiver.decodedIRData.command, HEX);
      // USE NEW 3.x FUNCTIONS
      IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
      //IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
      IrReceiver.resume(); // Enable receiving of the next value


      uint16_t address = IrReceiver.decodedIRData.address;
      uint16_t command = IrReceiver.decodedIRData.command;

      if (address != 0x40)
        return;
      
      switch(command) {
        case 0x19:
          motor_forward();
          //motor_backward();
          break;
        case 0x1D:
          motor_backward();
          break;
        case 0x46:
          motor_left();
          break;
        case 0x47:
          motor_right();
          break;
        case 0x0A:
          motor_stop();
          break;
        case 0x1E:
          _hc_enable = false;
          break;
        case 0x1A:
          _hc_enable = true;
          break;
      }
  } else 
  {
    delay(50);
  }
}

typedef enum {
   HC_STATE_NONE, 
   HC_STATE_FORWARD,
   HC_STATE_BACKWARD,
   HC_STATE_TURN_LEFT_OR_RIGHT,
} hc_state_t;



//超声波控制
static hc_control() {
  static hc_state_t state = HC_STATE_NONE;

  if (state == HC_STATE_NONE)
  {
    motor_forward();
    state = HC_STATE_FORWARD;
  }

  
  double* distances = HCSR04.measureDistanceCm();
  Serial.println(distances[0]);
  double dist = distances[0];
  
  if (dist < 20.0 ) {
    motor_stop(); //检测到距离小于20cm，先停车，再左转，再前进
    delay(1000);
    motor_left();
    delay(1000); //时间待定，转90度的时间
    motor_stop();
    delay(200);
    motor_forward();
  }

  delay(50);
}


// the loop function runs over and over again forever
void loop() {
  //motor_forward();
  ir_control();
  //hc_control();

  //double* distances = HCSR04.measureDistanceCm();
  //Serial.println(distances[0]);

  //Serial.println("loop end");
  
  if (_running && _hc_enable && _dir != DIR_BACKWARD)
  {
    double* distances = HCSR04.measureDistanceCm();
    Serial.println(distances[0]);
    double dist = distances[0];

    if (dist < 20.0  && dist > 0 ) {
     // motor_backward();
      //delay(500);
      motor_stop();
    }
  }
}
