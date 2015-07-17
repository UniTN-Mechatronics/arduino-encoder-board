#include <EEPROM.h>
#include <Wire.h>
#include "EncoderSlave.h"

#define MODE_PIN 8
#define SERIAL_BAUD 9600
#define RESET_PIN 9
#define CLOCK_PIN A5
#define DATA_PIN A4
#define MAX_EncS 4
#define RES_MULT 4
#define COM_MULT 10000

typedef void (*func_ptr)(void);
func_ptr state;

EncoderSlave EncS;
func_ptr ISRs[] = {index_ISR_0, index_ISR_1, index_ISR_2, index_ISR_3};

//////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  if (digitalRead(MODE_PIN) == LOW) { //button pressed
    Serial.begin(SERIAL_BAUD);
    state = Config;
    EncS.settings_info(); //view the command list
  }
  else {
    state = Run;
  }
    
  if (digitalRead(RESET_PIN) == LOW) {
    EncS.save_to_EEPROM();  //save every settings on EEPROM 
  }
  else {
    EncS.read_from_EEPROM(); //set the seggings setted by the user
  }
  EncS.set(RESET_PIN, MODE_PIN);
  if(EncS.settings_u.settings.read_index == true) {
    for( int i = 0; i < EncS.settings_u.settings.n; i++) {
      attachInterrupt(digitalPinToInterrupt(EncS.settings_u.settings.x[i]), ISRs[i], RISING); 
    }
  }
  Wire.begin(EncS.settings_u.settings.I2C_address);
  Wire.onRequest(requestEvent);
  /*the comunication is setted between the master Arduino and the slave with an address and whenever the
  * master asks for data, the function "requestEvent" runs
  */ 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  state(); //state can switch between run and config
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Run() {
  EncS.read(RES_MULT, COM_MULT); //read the values from the encoders
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
        EncS.settings_u.settings.n = buf;
        buf = 0;
        break;
      case 'r': //set the resolution
        EncS.settings_u.settings.res = buf;
        buf = 0;
        break;

      case 'A': //set a pins
        EncS.settings_u.settings.a[0] = buf;
        buf = 0;
        break;
      case 'B':
        EncS.settings_u.settings.a[1] = buf;
        buf = 0;
        break;
      case 'C':
        EncS.settings_u.settings.a[2] = buf;
        buf = 0;
        break;
      case 'D':
        EncS.settings_u.settings.a[3] = buf;
        buf = 0;
        break;

      case 'E': //set b pins
        EncS.settings_u.settings.b[0] = buf;
        buf = 0;
        break;      
      case 'F':
        EncS.settings_u.settings.b[1] = buf;
        buf = 0;
        break;          
      case 'G':
        EncS.settings_u.settings.b[2] = buf;
        buf = 0;
        break;         
      case 'D':
        EncS.settings_u.settings.a[3] = buf;
        buf = 0;
        break; 
      case 'H':
        EncS.settings_u.settings.b[3] = buf;
        buf = 0;
      break; 

      case 'I': //set x pins
        EncS.settings_u.settings.x[0] = buf;
        buf = 0;
        break;  
      case 'L':
        EncS.settings_u.settings.x[1] = buf;
        buf = 0;
        break; 
      case 'M':
        EncS.settings_u.settings.x[2] = buf;
        buf = 0;
        break; 
      case 'N':
        EncS.settings_u.settings.x[3] = buf;
        buf = 0;
        break;

      case 's': //save data 
        EncS.save_to_EEPROM();
        buf = 0;
        Serial.println("settings saved ...");
        break;   
      case 'a': //set the slave's address
        EncS.settings_u.settings.I2C_address = buf;       
        buf = 0;
        break;
      case 'h': //show info
        EncS.info();       
        buf = 0;
        break;
      case 'Z': //turn the mode read_index on  
        EncS.settings_u.settings.read_index = true;
        break;      
      case 'z': //turn the mode read_index off   
        EncS.settings_u.settings.read_index = false;
        break;      
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////    

void index_ISR_0() {
  EncS.encoders[0].write(EncS.encoders[0].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[0]++; //every time this function runs, adds a round and reset to 0 the value on the encoder
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_1() {
  EncS.encoders[1].write(EncS.encoders[1].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[1]++; //every time this function runs, adds a round and reset to 0 the value on the encoder
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_2() {
  EncS.encoders[2].write(EncS.encoders[2].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[2]++; //every time this function runs, adds a round and reset to 0 the value on the encoder
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void index_ISR_3() {
  EncS.encoders[3].write(EncS.encoders[3].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[3]++; //every time this function runs, adds a round and reset to 0 the value on the encoder
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void requestEvent() { 
  Wire.write(EncS.data_u.data_byte, sizeof(data)); //the data are sent to the master
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

