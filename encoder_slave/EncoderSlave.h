#ifndef __ENCODERSLAVE__
#define __ENCODERSLAVE__
#include <EEPROM.h>
#include <arduino.h>
#include <Wire.h>
#include "EncoderMod.h"

#define MAX_ENCS 4

//////////////////////////////////////////////////////////////////////////////////

class DynamicFilter {
  public:
    double _params_df_hp;
    double _params_df_hv;
    double _params_df_eps;

    DynamicFilter();
    void set_eps(double value);
    void set_hp(double value);
    void set_hv(double value);
    void update(double dt, double _x);
    double get_angle();
    double get_speed();

  private:
    double _df_x;
    double _df_x_dot;
    double _df_x_p;
    double _df_x_dot_p;
};

//////////////////////////////////////////////////////////////////////////////////

struct settings_t{
  int n; //number of encoders attached on Arduno
  int res; //resolution
  bool read_index;  //working mode
  int a[4]; //pins a 
  int b[4]; //pins b
  int x[4]; //pins x
  byte I2C_address; //slave's address
  byte EEPROM_address;
  int lost_pulses_th;
  bool Speed;
  int speed_th_l;
  int speed_th_h;
};

union settings{ 
  settings_t settings;
  byte settings_byte[sizeof(settings)];
};

//////////////////////////////////////////////////////////////////////////////////

#if BUFFER_LENGTH == 32  
struct data_t{
  long angles[MAX_ENCS];
  int rounds[MAX_ENCS];
  int angular_speed[MAX_ENCS];
};
#define COM_MULT_SPEED 10.0
#else
struct data_t{
  long angles[MAX_ENCS];
  long rounds[MAX_ENCS];
  long angular_speed[MAX_ENCS];
};
#define COM_MULT_SPEED 10000.0
#endif

// struct data_t{
//   long angles[MAX_ENCS];
//   int rounds[MAX_ENCS];
//   int angular_speed[MAX_ENCS];
// };

union data{ 
  data_t data;
  byte data_byte[sizeof(data)];
};

//////////////////////////////////////////////////////////////////////////////////

class EncoderSlave {
  public:
  	EncoderSlave();
    void set();
    void save_to_EEPROM();
    void read_from_EEPROM();
    void default_settings();
    void read(int res_mult, int com_mult);
    void info();
    void settings_info();
    void speed(int index, int com_mult);

    settings settings_u;
    data data_u;
    Encoder* encoders = NULL;
    DynamicFilter* filters = NULL;
    long* lost_pulses = NULL;
    long* lost_pulses_b = NULL;
    long t1[MAX_ENCS];
    double* speed_idx = NULL;
  private: 
    long modulo(double value);
};

//////////////////////////////////////////////////////////////////////////////////


#endif
