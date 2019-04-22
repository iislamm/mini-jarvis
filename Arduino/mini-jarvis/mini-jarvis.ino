#include <dht.h>
#include <Servo.h>

#define DOOR_SERVO_PIN 0
#define HANGINGS_SERVO_PIN 1
#define PUMP_PIN 2
#define LED_PIN 3
#define FLAME_PIN 3
#define MOTION1_PIN 4
#define MOTION2_PIN 5
#define BUZZER_PIN 7
#define LDR_PIN A0
#define TEMPRETURE_PIN A1
boolean motion1State = false; // sensor 1 detection for motion
boolean motion2State = false; // sensor 2 detection for motion
boolean currentLDRStatus = true; // true = lights outside; false = no lights outside
int lighting = 0;
int temp; // current temperature
bool door_open_state = false, hangings_open_state = false; // door and hanings are closed by default


dht DHT;
Servo doorServo;
Servo hangingsServo;

void setup() {
  // begin serial communication // 115200 used to communicate best with the esp8265 microcontroller
  Serial.begin(115200);

  doorServo.attach(8); // write to the door motor using pin 8
  hangingsServo.attach(9); // write to the hangings motor using pin 9
  
  DDRD |= (1<<LED_PIN) | (1<<BUZZER_PIN); // set pin 3 (D3) and 7 (D7) to be used as output pins // the rest will be input by default
  DDRB = 0b00000111; // set pin 8 (B0) and 9 (B1) and 10 (B2) to be used as output // the rest will be input by default
}

void loop() {
  // First, check if there is any fire detected and react on the result
  checkFire();

  // recieve the data from the esp8266 wifi microcontroller and update the data
  updateData();
  delay(1000);
  
  // check if its lighting outside or not
  checkLDR();
  delay(500);

  //update the temperature
  checkTemp();
  delay(500);

  // check if there is any one left or entered the house
  checkMotion();
  delay(500);
}

void checkFire() {
  int fireStatus = (PINB >> 4)&0x01;
 
  if (fireStatus == 1) { // the sensor the detected fire
    PORTD = PORTD | (1 << 7); // turn on the buzzer
  }
  else {
    PORTD = PORTD & ~(1 << 7); // turn off the buzzer if it's on
  }
}

void checkTemp() {
  int chck = DHT.read11(TEMPRETURE_PIN );
  int newTemp = DHT.temperature;
  if (newTemp != temp) {
    // if the temperature has changed, send it to the esp8266 microcontroller to send it to the server
    temp = newTemp;
    Serial.write('t'); // 't' is the code that makes the esp8266 microcontroller that the value after it is the new temperature
    Serial.write(temp);
  }
}

void checkLDR() {
  int LDRStatus = analogRead(LDR_PIN); // how is the light outside
  char state;
  if(LDRStatus < 1) { // no lights outside
    if (currentLDRStatus == true) { // if there was lights outside before, change the currents light state
        currentLDRStatus = false;
        state = 1;
        // send the esp8266 microcontroller that there is no lights outside
        Serial.write('l'); // 'l' is the code for the esp8266 microcontroller to understand that the value after it is for switching the lights either on or off
        Serial.write(state); // switch on
    }
  } 
  else {
    if (currentLDRStatus == false) { // if there was no lights outside, but now not, change the current state
        currentLDRStatus = true;
        state = 0;
        Serial.write('l'); // 'l' is the code for the esp8266 microcontroller to understand that the value after it is for switching the lights either on or off
        Serial.write(state); // switch off  
    }
  }
}

/*
  This funciton checks if there is any new motion detected around the doors and checks
  the direction of the motion to send to the server if there is any one entered or left
*/

void checkMotion() {
  bool tempMotion1State = motion1State;
  bool tempMotion2State = motion2State;
  if ((PINB&(1<<MOTION1_PIN)) != tempMotion1State) { // if the state of the motion detection changed
    // set the motion state to the oppisite
    tempMotion1State = !tempMotion1State;
    
    if (tempMotion1State == true && tempMotion1State == tempMotion2State) { // if sensor one detected new motion and sensor 2 have already detected one, then someone just left
      Serial.write('m'); // 'm' is the code for the esp8266 microcontroller to understand that te value after it will be a number telling if there is any one left or enterd
      Serial.write(2); // 1 entered; 2 left
    }
    
  }
  if ((PINB&(1<<MOTION2_PIN)) != tempMotion2State) {
    tempMotion2State = !tempMotion2State;
    if (tempMotion2State == true && tempMotion1State == tempMotion2State) { // if sensor two detected motion and sensor 1 have already detected on, then someone just entered
      Serial.write('m'); // 'm' is the code for the esp8266 microcontroller to understand that te value after it will be a number telling if there is any one left or enterd
      Serial.write(1); // 1 entered; 2 left
    }
  }

  motion1State = tempMotion1State;
  motion2State = tempMotion2State;
}

/*
  This function recieves the data form the esp8266 microcontroller and update the outputs
*/

void updateData() {
  char flag; // every time a data sent, a flag is sent first to tell what is the data will be sent after it
  boolean temp_door_state, temp_hangings_state;
  for (int i = 0; i < 3; i++) {
    if(Serial.available()) {
     flag = Serial.read();
     
    delay(100);
    if(flag == 'l'){ // 'l' means that the value sent after it is the lighting value
      lighting = Serial.read();
      delay(100);
    }
    else if(flag == 'd'){ // 'd' means that the value sent after it is the door open state 
      temp_door_state = Serial.read();
      delay(100);
    }
    else if(flag == 'h'){ // 'h' means that the value sent after it is the hangings open state
      temp_hangings_state = Serial.read();
      delay(100);
    }
  }

    if (flag == 'l') { // output the new value of the lighting
      lighting = map(lighting,0,100,0,255);
      analogWrite(LED_PIN, lighting);
    }

     else if (flag == 'd') {
      if (temp_door_state != door_open_state) { // if the door is state is not the same 
        
        door_open_state = temp_door_state;
        if (door_open_state == true) { // open the door
          for (int angle = 10; angle < 180; angle++) {
            doorServo.write(angle);
            delay(10);
          }
        }
        else { // close the door
          for (int angle = 180; angle >= 10; angle--) {
            doorServo.write(angle);
            delay(10);
          }
        }
      }
    }
     else if (flag == 'h') {
      if (temp_hangings_state != hangings_open_state) {
        hangings_open_state = temp_hangings_state;
        if (hangings_open_state == true) { // open the hangings
          for (int angle = 180; angle >= 10; angle--) {
            hangingsServo.write(angle);
            delay(10);
          }
        }
        else { // close the hangings
          for (int angle = 10; angle < 180; angle++) {
            hangingsServo.write(angle);
            delay(10);
          }
        }
      }
      
    } 
  }
}
