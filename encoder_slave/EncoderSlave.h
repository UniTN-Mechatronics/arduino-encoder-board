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
  int lost_pulses_th;
};

union settings{ 
  settings_t settings;
  byte settings_byte[sizeof(settings)];
};

//////////////////////////////////////////////////////////////////////////////////

struct data_t{
  long angles[MAX_ENCS];
  long rounds[MAX_ENCS];
  double angular_speed[MAX_ENCS];

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
    void speed(int index);

    settings settings_u;
    data data_u;
    Encoder* encoders = NULL;
    long* lost_pulses = NULL;
    long* lost_pulses_b = NULL;
    long t1[MAX_ENCS];
};

#endif

//////////////////////////////////////////////////////////////////////////////////
