#include <EEPROM.h>
#include <Wire.h>
#include "EncoderSlave.h"

#define MODE_PIN 8
#define SERIAL_BAUD 115200
#define RESET_PIN 9
#define CLOCK_PIN A5
#define DATA_PIN A4
#define RES_MULT 4
#define COM_MULT 10000
#define LED_PIN 13
#define EPS 0.11
#define HP 5
#define HV 6

typedef void (*func_ptr)(void);
func_ptr state;

long time_i = 0;
double dt = 0.0001;

EncoderSlave encs;
DynamicFilter DF;
func_ptr ISRs[] = {index_ISR_0, index_ISR_1, index_ISR_2, index_ISR_3};

//////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP); //RESET_PIN and MODE_PIN work with pullup resistence
  pinMode(MODE_PIN, INPUT_PULLUP);

  if (digitalRead(RESET_PIN) == LOW) {
    encs.save_to_EEPROM();  //save settings into the EEPROM 
  }
  else {
    encs.read_from_EEPROM(); //set the settings set by the user
  }

  encs.set();

  if (digitalRead(MODE_PIN) == LOW) { //button pressed
    Serial.begin(SERIAL_BAUD);
    state = Config;
    encs.settings_info(); //view the command list
  }
  else {
    state = Run;
  }
    
  if(encs.settings_u.settings.read_index == true) {
    for( int i = 0; i < encs.settings_u.settings.n; i++) {
      attachInterrupt(digitalPinToInterrupt(encs.settings_u.settings.x[i]), ISRs[i], RISING); 
    }
  }
  Wire.begin(encs.settings_u.settings.I2C_address);
  Wire.onRequest(requestEvent);
  for(int i = 0; i < encs.settings_u.settings.n; i++) {
    encs.filters[i].set_eps(EPS);
    encs.filters[i].set_hv(HV);
    encs.filters[i].set_hp(HP);
  }
  /*the comunication is setted between the master Arduino and the slave with an address and whenever the
  * master asks for data, the function "requestEvent" runs
  */ 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  time_i = micros();
  state(); //state can switch between run and config
  dt = (micros() - time_i) / 1000000.0; // tempo trascorso dal precedente
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Run() {
  encs.read(RES_MULT, COM_MULT);  

  if(encs.settings_u.settings.Speed == true) {
    for(int i = 0; i < encs.settings_u.settings.n; i++) {
      encs.filters[i].update(dt, encs.data_u.data.angles[i] / double(COM_MULT) + 2 * PI * encs.data_u.data.rounds[i]);
      if(encs.settings_u.settings.read_index == true) {
        if(encs.filters[i].get_speed() < encs.settings_u.settings.speed_th_l) {
          encs.data_u.data.angular_speed[i] = encs.filters[i].get_speed() * COM_MULT_SPEED;
        }
        else if(encs.filters[i].get_speed() > encs.settings_u.settings.speed_th_h) {
          encs.data_u.data.angular_speed[i] = encs.speed_idx[i] * COM_MULT_SPEED;
        }
        else {
          double y = (encs.filters[i].get_speed() - encs.settings_u.settings.speed_th_l) / (encs.settings_u.settings.speed_th_h - encs.settings_u.settings.speed_th_l);
          encs.data_u.data.angular_speed[i] = ((1 - y) * encs.filters[i].get_speed() + y * encs.speed_idx[i]) * COM_MULT_SPEED; 
        }
      }
      else {
        encs.data_u.data.angular_speed[i] = encs.filters[i].get_speed();
      }
    } 
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Config() {
  static unsigned long buf = 0;
  char ch; 
  if(Serial.available()) {  //comand parsing
    ch = Serial.read();
    switch (ch) {
      case '0'...'9': //amass values
        buf = buf * 10 + ch - '0';
        break;
      case 'n': //set the number of encoders
        encs.settings_u.settings.n = buf;
        buf = 0;
        break;
      case 'r': //set the resolution
        encs.settings_u.settings.res = buf;
        buf = 0;
        break;
      case 't': //set the resolution
        encs.settings_u.settings.lost_pulses_th = buf;
        buf = 0;
        break;

      case 'A': //set pins a
        encs.settings_u.settings.a[0] = buf;
        buf = 0;
        break;
      case 'B':
        encs.settings_u.settings.a[1] = buf;
        buf = 0;
        break;
      case 'C':
        encs.settings_u.settings.a[2] = buf;
        buf = 0;
        break;
      case 'D':
        encs.settings_u.settings.a[3] = buf;
        buf = 0;
        break;

      case 'E': //set pins b 
        encs.settings_u.settings.b[0] = buf;
        buf = 0;
        break;      
      case 'F':
        encs.settings_u.settings.b[1] = buf;
        buf = 0;
        break;          
      case 'G':
        encs.settings_u.settings.b[2] = buf;
        buf = 0;
        break;         
      case 'H':
        encs.settings_u.settings.b[3] = buf;
        buf = 0;
      break; 

      case 'I': //set pins x
        encs.settings_u.settings.x[0] = buf;
        buf = 0;
        break;  
      case 'L':
        encs.settings_u.settings.x[1] = buf;
        buf = 0;
        break; 
      case 'M':
        encs.settings_u.settings.x[2] = buf;
        buf = 0;
        break; 
      case 'N':
        encs.settings_u.settings.x[3] = buf;
        buf = 0;
        break;
      case 's':   
        encs.save_to_EEPROM(); //save data
        buf = 0;
        Serial.println("settings saved ...");
        break;   
      case 'a': //set the address of the Slave
        encs.settings_u.settings.I2C_address = buf;       
        buf = 0;
        break;
      case 'h': 
        encs.info();       
        buf = 0;
        break;     
      case 'z':   
        encs.settings_u.settings.read_index = !encs.settings_u.settings.read_index;
        break; 
      case 'v':    
        encs.settings_u.settings.Speed = !encs.settings_u.settings.Speed;
        break;
      case 'l':    
        encs.settings_u.settings.speed_th_l = buf;
        buf = 0;
        break; 
      case 'k':  
        encs.settings_u.settings.speed_th_h = buf;
        buf = 0;
        break;       
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////    

void index_ISR_0() {
  encs.lost_pulses[0] = sign(encs.encoders[0].read()) * (modulo(encs.encoders[0].read()) - encs.settings_u.settings.res * RES_MULT);
  if((modulo(encs.lost_pulses[0]) > encs.settings_u.settings.lost_pulses_th) && (encs.data_u.data.rounds[0] != 0)) {
    digitalWrite(LED_PIN, HIGH);
  }
  else digitalWrite(LED_PIN, LOW);
  if(encs.data_u.data.rounds[0] == 0) {
    encs.encoders[0].write(0); 
  }
  else { 
    encs.encoders[0].write(encs.lost_pulses[0] - encs.lost_pulses_b[0]);
    encs.lost_pulses_b[0] = encs.lost_pulses[0];
  }
  encs.data_u.data.rounds[0]++; //every time this function runs,
  // it adds a round
  encs.speed(0, COM_MULT_SPEED);  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_1() {
  encs.lost_pulses[1] = sign(encs.encoders[1].read()) * (modulo(encs.encoders[1].read()) - encs.settings_u.settings.res * RES_MULT);
  if((modulo(encs.lost_pulses[1]) > encs.settings_u.settings.lost_pulses_th) && (encs.data_u.data.rounds[1] != 0)) {
    digitalWrite(LED_PIN, HIGH);
  }
  else digitalWrite(LED_PIN, LOW);
  if(encs.data_u.data.rounds[1] == 0) {
    encs.encoders[1].write(0); 
  }
  else { 
    encs.encoders[0].write(encs.lost_pulses[1] - encs.lost_pulses_b[1]);
    encs.lost_pulses_b[1] = encs.lost_pulses[1];
  }
  encs.data_u.data.rounds[1]++; //every time this function runs,
  // it adds a round
  encs.speed(1, COM_MULT); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_2() {
  encs.lost_pulses[2] = sign(encs.encoders[2].read()) * (modulo(encs.encoders[2].read()) - encs.settings_u.settings.res * RES_MULT);
  if((modulo(encs.lost_pulses[2]) > encs.settings_u.settings.lost_pulses_th) && (encs.data_u.data.rounds[2] != 0)) {
    digitalWrite(LED_PIN, HIGH);
  }
  else digitalWrite(LED_PIN, LOW);
  if(encs.data_u.data.rounds[2] == 0) {
    encs.encoders[2].write(0); 
  }
  else {
    encs.encoders[2].write(encs.lost_pulses[2] - encs.lost_pulses_b[2]);
    encs.lost_pulses_b[2] = encs.lost_pulses[2];
  }
  encs.data_u.data.rounds[2]++; //every time this function runs,
  // it adds a round 
  encs.speed(2, COM_MULT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_3() {
  encs.lost_pulses[3] = sign(encs.encoders[3].read()) * (modulo(encs.encoders[3].read()) - encs.settings_u.settings.res * RES_MULT);
  if((modulo(encs.lost_pulses[3]) > encs.settings_u.settings.lost_pulses_th) && (encs.data_u.data.rounds[3] != 0)) {
    digitalWrite(LED_PIN, HIGH);
  }
  else digitalWrite(LED_PIN, LOW);
  if(encs.data_u.data.rounds[3] == 0) {
    encs.encoders[3].write(0); 
  }
  else {
    // encs.encoders[3].write(0); 
    encs.encoders[3].write(encs.lost_pulses[3] - encs.lost_pulses_b[3]);
    encs.lost_pulses_b[3] = encs.lost_pulses[3];
  }
  encs.data_u.data.rounds[3]++; //every time this function runs,
  // it adds a round 
  encs.speed(3,COM_MULT);  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void requestEvent() { 
  Wire.write(encs.data_u.data_byte, sizeof(data)); //the data are sent to the master
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

long modulo(int value) {
  if(value < 0) {
    return(- value);
  }
  else {
    return(value);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

int sign(double value) {
  if(value < 0) {
    return -1;
  }
  else {
    return 1;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
