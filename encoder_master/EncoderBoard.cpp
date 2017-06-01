#include "EncoderBoard.h"
#include <arduino.h>



//////////////////////////////////////////////////////////////////////////////////////////////////////

EncoderBoard::EncoderBoard(int address) {
	Wire.begin();
	for(int i = 0; i < MAX_ENCS; i++) {
		_data_u.data.angles[i] = 0;
		_data_u.data.rounds[i] = 0;
		_data_u.data.angular_speed[i] = 0;
	}
	_address = address;
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderBoard::update() {
  Wire.requestFrom(_address, sizeof(_data_u));    // request 128 bytes from slave device
  int i = 0;
  while(Wire.available()) { // slave may send less than requested
    _data_u.data_byte[i] = (Wire.read());
    i++;
  }
}

//////////////////////////////////////////////////////////////////////////////////

double EncoderBoard::get(int index) {
  return modulo(_data_u.data.angles[index] / COM_MULT ) + 2 * PI * _data_u.data.rounds[index];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

double EncoderBoard::angular_speed(int index) {
  return _data_u.data.angular_speed[index] / COM_MULT_SPEED;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

double EncoderBoard::modulo(double value) {	
  if(value < 0) return value;
  else return -value;
}
