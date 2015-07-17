#ifndef __ENCODERSLAVE__
#define __ENCODERSLAVE__
#include <EEPROM.h>
#include <arduino.h>
#include "EncoderMod.h"

#define MAX_ENCS 4

//////////////////////////////////////////////////////////////////////////////////

struct settings_t{
  int n; //number of encoders attached on Arduno
  int res; //resolution
  bool read_index;  //working mode
  int a[MAX_ENCS]; //pins a 
  int b[MAX_ENCS]; //pins b
  int x[MAX_ENCS]; //pins x
  byte I2C_address; //slave's address
  byte EEPROM_address;
};

union settings{ 
  settings_t settings;
  byte settings_byte[sizeof(settings)];
};

//////////////////////////////////////////////////////////////////////////////////

struct data_t{
  long angles[4];
  long rounds[4];
};

union data{ 
  data_t data;
  byte data_byte[sizeof(data)];
};

//////////////////////////////////////////////////////////////////////////////////

class EncoderSlave {
  public:
  	EncoderSlave();
    void set(int reset_pin,int mode_pin);
    void save_to_EEPROM();
    void read_from_EEPROM();
    void default_settings();
    void read(int res_mult, int com_mult);
    void info();
    void settings_info();
    settings settings_u;
    data data_u;
    Encoder* encoders = NULL;
};

#endif

//////////////////////////////////////////////////////////////////////////////////
