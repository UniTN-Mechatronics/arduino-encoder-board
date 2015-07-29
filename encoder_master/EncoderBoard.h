#ifndef __ENCODERBOARD__
#define __ENCODERBOARD__
#include <arduino.h>
#include <Wire.h>

#define MAX_ENCS 4
#define COM_MULT 10000.0
#define COM_MULT_SPEED 10.0


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

//////////////////////////////////////////////////////////////////////////////////////////////////////

class EncoderBoard {
  public:
  	EncoderBoard(int address);
    void update();
    double get(int index);
    double angular_speed(int index);
    double modulo(int value);
  private:
    data _data_u;
    int _address;
};

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
