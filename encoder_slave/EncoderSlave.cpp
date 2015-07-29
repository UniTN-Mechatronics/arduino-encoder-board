#include "EncoderSlave.h"
#include <arduino.h>
#include <EEPROM.h>
#include "EncoderMod.h"

//////////////////////////////////////////////////////////////////////////////////

EncoderSlave::EncoderSlave(){
   default_settings();
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::set() {
  for(int i = 0; i < MAX_ENCS; i++) { //sets at 0 four arrays 
    data_u.data.angles[i] = 0;
    data_u.data.rounds[i] = 0;
    data_u.data.angular_speed[i] = 0;
  }
  encoders = new Encoder[settings_u.settings.n];
  filters = new DynamicFilter[settings_u.settings.n];
  lost_pulses = new long[settings_u.settings.n];
  lost_pulses_b = new long[settings_u.settings.n];
  speed_idx = new double[settings_u.settings.n];
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
   settings_u.settings.lost_pulses_th = 40;
   for(int i = 1; i <= settings_u.settings.n; i++){
   settings_u.settings.a[i] = 0;
   settings_u.settings.b[i] = 0;
   settings_u.settings.x[i] = 0;
   settings_u.settings.a[0] = 3;
   settings_u.settings.b[0] = 4;	
   }
   settings_u.settings.I2C_address = 0x03;
   settings_u.settings.EEPROM_address = 0x10;
   settings_u.settings.read_index = false;
   settings_u.settings.Speed = false;
   settings_u.settings.speed_th_l = 0;
   settings_u.settings.speed_th_h = 1;
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
    Serial.print("LOST PULSES THRESHOLD: ");
    Serial.println(settings_u.settings.lost_pulses_th);    
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
    Serial.print("SPEED ");
    Serial.println(settings_u.settings.Speed);
    Serial.print("SPEED_TH_L: ");
    Serial.print(settings_u.settings.speed_th_l);
    Serial.print("SPEED_TH_H: ");
    Serial.println(settings_u.settings.speed_th_h);
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::settings_info() { //prints on the serial port a legen so as to help the user
    Serial.println("LEGEND:");
    Serial.println("'n' at the end in order to set the number of encoders ");
    Serial.println("'a' to set the slave's address");
    Serial.println("'r' to set the resolution of every encoders ");
    Serial.println("'t' to set the encoder's lost pulses threshold");
    Serial.println("'l' to set the speed low threshold");
    Serial.println("'k' to set the speed high threshold");
    Serial.println("'h' to see the values ");
    Serial.println("A ... D in order to set encoder's a pins");
    Serial.println("E ... H in order to set encoder's b pins");
    Serial.println("I ... N in order to set encoder's x pins");
    Serial.println("z to consider or not the index signal");
    Serial.println("s to save the settings");
    Serial.println("v to calculate or not the angular speed");
}

//////////////////////////////////////////////////////////////////////////////////

void EncoderSlave::speed(int index, int com_mult) {
	if(settings_u.settings.Speed == true) {
		double dt = modulo(micros() - t1[index]) / 1000000.0;
 		t1[index] = micros();
		// Serial.println((2.0 * PI / dt) * com_mult);
 		speed_idx[index] = (2.0 * PI / dt);
	}
}

//////////////////////////////////////////////////////////////////////////////////

DynamicFilter::DynamicFilter() {
	_df_x = 0;
	_df_x_dot = 0;
	_df_x_p = 0;
	_df_x_dot_p = 0;

	_params_df_hp = 5;
	_params_df_hv = 6;
	_params_df_eps = 0.01;
}

//////////////////////////////////////////////////////////////////////////////////

void DynamicFilter::set_eps(double value) {
	_params_df_eps = value;
}

//////////////////////////////////////////////////////////////////////////////////

void DynamicFilter::set_hp(double value) {
	_params_df_hp = value;
}

//////////////////////////////////////////////////////////////////////////////////

void DynamicFilter::set_hv(double value) {
	_params_df_hv = value;
}

//////////////////////////////////////////////////////////////////////////////////

void DynamicFilter::update(double dt, double _x) {
  _df_x = _df_x_p + dt*_df_x_dot_p + ((dt*_params_df_hp)/_params_df_eps) *  (_x - _df_x_p);
  _df_x_dot = _df_x_dot_p + ((dt*_params_df_hv) / pow(_params_df_eps, 2)) * (_x - _df_x_p);
  _df_x_p = _df_x;
  _df_x_dot_p = _df_x_dot;
}

//////////////////////////////////////////////////////////////////////////////////

double DynamicFilter::get_speed() {
	return _df_x_dot;
}

//////////////////////////////////////////////////////////////////////////////////

double DynamicFilter::get_angle() {
	return _df_x;
}

//////////////////////////////////////////////////////////////////////////////////

long EncoderSlave::modulo(double value){
  if(value < 0) {
    return(- value);
  }
  else {
    return(value);
  }
}