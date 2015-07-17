#include "EncoderBoard.h"
#include <Wire.h>
#include <arduino.h>

#define ENCS_MAX 4

//////////////////////////////////////////////////////////////////////////////////////////////////////

EncoderBoard::EncoderBoard(int address) {
	Wire.begin();
	for(int i = 0; i < ENCS_MAX; i++) {
		_data_u.data.angles[i] = 0;
		_data_u.data.rounds[i] = 0;
	}
	_address = address;
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderBoard::update() {
  Wire.requestFrom(_address, 128);    // request 128 bytes from slave device 
  int i = 0;
  while(Wire.available()) { // slave may send less than requested
    _data_u.data_byte[i] = (Wire.read()); 
    i++;
  }
}

//////////////////////////////////////////////////////////////////////////////////

double EncoderBoard::get(int index) {	
 return (_data_u.data.angles[index] / 10000.0 ) + 2 * PI * _data_u.data.rounds[index];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
