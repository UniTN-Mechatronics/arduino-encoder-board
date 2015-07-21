#include "EncoderSlave.h"
#include <arduino.h>
#include <EEPROM.h>
#include "EncoderMod.h"

#define ENCS_MAX 4

//////////////////////////////////////////////////////////////////////////////////

EncoderSlave::EncoderSlave(){
   default_settings();
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::set(int reset_pin, int mode_pin) {
  pinMode(reset_pin, INPUT_PULLUP); //RESET_PIN and MODE_PIN work with pullup resistence
  pinMode(mode_pin, INPUT_PULLUP);
  for(int i = 0; i < settings_u.settings.n; i++) { //sets at 0 four arrays 
    data_u.data.angles[i] = 0;
    data_u.data.rounds[i] = 0;
  }
  encoders = new Encoder[settings_u.settings.n];
  lost_pulses = new long[settings_u.settings.n];
  lost_pulses_b = new long[settings_u.settings.n];
  for(int i = 0; i < settings_u.settings.n; i++) {
    encoders[i].init(settings_u.settings.a[i], settings_u.settings.b[i]);
    lost_pulses[i] = 0;
    lost_pulses_b[i] = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::default_settings() {
   settings_u.settings.n = 1;
   settings_u.settings.res = 2048;

   settings_u.settings.a[0] = 3;
   settings_u.settings.b[0] = 4;

   settings_u.settings.a[1] = 0;
   settings_u.settings.b[1] = 0;

   settings_u.settings.a[2] = 0;
   settings_u.settings.b[2] = 0;

   settings_u.settings.a[3] = 0;
   settings_u.settings.b[3] = 0;

   settings_u.settings.x[0] = 0;
   settings_u.settings.x[1] = 0;
   settings_u.settings.x[2] = 0;
   settings_u.settings.x[3] = 0;

   settings_u.settings.I2C_address = 0x03;
   settings_u.settings.EEPROM_address = 0x10;
   settings_u.settings.read_index = false;
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::save_to_EEPROM() {
  EEPROM.put(settings_u.settings.EEPROM_address, settings_u.settings_byte); 
  //data are saved on EEPROM in order to rimember them till the user changes them 
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::read_from_EEPROM() {
  EEPROM.get(settings_u.settings.EEPROM_address, settings_u.settings_byte);
  //sets data remembered in the EEPROM
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::read(int res_mult, int com_mult) {  
  for(int i = 0; i < settings_u.settings.n; i++) {
    data_u.data.angles[i] = long(encoders[i].read() * 2 * PI * com_mult / float(settings_u.settings.res * res_mult)); 
    //the values are changed so as to make easier the comunication
  }
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::info() {
/* this function shows the information and values
of every variable in the settings_t structure */
    Serial.println("INFO:");
    Serial.print("ENCODERS: ");
    Serial.println(settings_u.settings.n);
    Serial.print("RESOLUTION: ");
    Serial.println(settings_u.settings.res);
    Serial.print("SLAVE ADDRESS: ");
    Serial.println(settings_u.settings.I2C_address, HEX);
    Serial.print("ENC1 PIN_A: ");
    Serial.print(settings_u.settings.a[0]);
    Serial.print(" ENC1 PIN_B: ");
    Serial.print(settings_u.settings.b[0]);
    Serial.print(" ENC1 PIN_X: ");
    Serial.println(settings_u.settings.x[0]);
    Serial.print("ENC2 PIN_A: ");
    Serial.print(settings_u.settings.a[1]); 
    Serial.print(" ENC2 PIN_B: ");
    Serial.print(settings_u.settings.b[1]);
    Serial.print(" ENC2 PIN_X: ");
    Serial.println(settings_u.settings.x[1]);
    Serial.print("ENC3 PIN_A: ");
    Serial.print(settings_u.settings.a[2]);
    Serial.print(" ENC3 PIN_B: ");
    Serial.print(settings_u.settings.b[2]);
    Serial.print(" ENC3 PIN_X: ");
    Serial.println(settings_u.settings.x[2]); 
    Serial.print("ENC4 PIN_A: ");
    Serial.print(settings_u.settings.b[3]);  
    Serial.print(" ENC4 PIN_B: ");
    Serial.print(settings_u.settings.b[3]);
    Serial.print(" ENC4 PIN_X: ");
    Serial.println(settings_u.settings.x[3]);
    Serial.print("READ_INDEX ");
    Serial.println(settings_u.settings.read_index);               
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::settings_info() { //prints on the serial port a legen so as to help the user
    Serial.println("LEGEND:");
    Serial.println("'n' at the end in order to set the number of encoders ");
    Serial.println("'a' to set the slave's address");
    Serial.println("'r' to set the resolution of every encoders ");
    Serial.println("'h' to see the values ");
    Serial.println("A ... D in order to set encoder's a pins");
    Serial.println("E ... H in order to set encoder's b pins");
    Serial.println("I ... N in order to set encoder's x pins");
    Serial.println("Z or z to consider or not the index signal");
    Serial.println("s to save the settings");
}

//////////////////////////////////////////////////////////////////////////////////