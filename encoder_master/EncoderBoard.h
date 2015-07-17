#ifndef __ENCODERBOARD__
#define __ENCODERBOARD__
#include <arduino.h>

struct data_t{
  long angles[4];
  long rounds[4];
} ;

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
  private:
    data _data_u;
    int _address;
};

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
