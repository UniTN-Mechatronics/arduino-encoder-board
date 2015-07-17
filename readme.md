Package Control Messages
========================

OmniMarkupPreviewer:
-------------------

  OmniMarkupPreviewer
  ===================
  
  OmniMarkupPreviewer is jfhdsfhdsjfh a plugin for both [Sublime Text 2] and [Sublime Text 3]
  that preview markups in web browsers. OmniMarkupPreviewer renders markups into
  htmls and send it to web browser in the backgound, which enables a live preview.
  Besides, OmniMarkupPreviewer provide support for exporting result to
  html file as well.
  
  [Sublime Text 2]: http://www.sublimetext.com/2
  [Sublime Text 3]: http://www.sublimetext.com/3
  
  OmniMarkupPreviewer has builtin support following markups:
  
  * [Markdown](http://daringfireball.net/projects/markdown/)
  * [reStructuredText](http://docutils.sourceforge.net/rst.html)
  * [WikiCreole](http://wikicreole.org/)
  * [Textile](http://www.textism.com/tools/textile/)
  * [Pod](http://search.cpan.org/dist/perl/pod/perlpod.pod) (Requires Perl >= `5.10`
    and can be found in `PATH`, if the perl version < `5.10`, `Pod::Simple` should be
    installed from `CPAN`.)
  * [RDoc](http://rdoc.sourceforge.net/) (Requires ruby in your `PATH`)
  
ENCODER BOARD
========================


##PURPOSE


The purpose of the program is to connect two Arduino boards thanks to I2C connection; with the master as the reciver and the slave as sender and measurer of encoders angles and rounds.

SLAVE BOARD
========================


**CONFIGURATION:**

In order to set the values, address of the slave and pins; the program helps the user, doing the setting process with the serial monitor.

Firstly, to access to this mode, the mode_pin has to be low.

```
if (digitalRead(MODE_PIN) == LOW) {
Serial.begin(SERIAL_BAUD);
state = Config;
EncS.settings_info(); 
} 
```

Secondly, you have to **write down the settigns** on the serial port:

with those commands you have to **write first the number and then the command letter**.

* 'n' in order to set the *number of encoders*;
* 'a' to set the slave's *address*;
* 'r' to set the *resolution* of every encoders;
* 'A' ... 'D' in order to set encoder's *a pins*;
* 'E' ... 'H' in order to set encoder's *b pins*;
* 'I' ... 'N' in order to set encoder's *x pins*.

Instead, the commands below **don't need any number befor the letter**.

* 'Z' or 'z' to consider or not the index signal;
* 's' to *save* the settings;
* 'h' to see the values.

**To end** the configuration process you have to **turn high** the **MODE_PIN** and **reset the board**; thanks to the libray **EEPROM** the settings will be rememberd as long as they have been saved.

**DEFAULT SETTINGS**

**To reset the settings** with the default values, you must **set low the RESET_PIN** , saving in to the EEPROM the values firstly setted up by the constructor method. In this way the settings setted up by the user are overlooked and overwritten with the default settings.

**from the library "EncoderSlave"**
```
EncoderSlave::EncoderSlave(){
   default_settings();
}
```

```
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
```
**INTERRUPT AND INDEX**

In order to have an higher precision you can switch on the **index** (`read_index = true`), and connect x channels to arduino. If the engine has to do lots of rounds, this is the best option with the lower possibility of making mistakes.

The code can afford up to 4 pins connected with the x channels and all of them need to be interrupt pins.

```
func_ptr ISRs[] = {index_ISR_0, index_ISR_1, index_ISR_2, index_ISR_3};

```

```
if(EncS.settings_u.settings.read_index == true) {
  for( int i = 0; i < EncS.settings_u.settings.n; i++) {
    attachInterrupt(digitalPinToInterrupt(EncS.settings_u.settings.x[i]), ISRs[i], RISING); 
  }
}

```

```
void index_ISR_0() {
  EncS.encoders[0].write(EncS.encoders[0].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[0]++;
}
```

**SENDING TO MASTER**

every time the master request, every data inside the struct data_t are sent whitch is inside the library and pubblic.

```
void requestEvent() { 
  Wire.write(EncS.data_u.data_byte, sizeof(data)); //the data are sent to the master
}
```
##EncoderSlave library
**FUNCTIONS:**

* **EncoderSlave:** it sets the default settigns.

```
EncoderSlave::EncoderSlave(){
   default_settings();
}
```

* **set(int reset_pin,int mode_pin):** it sets pullup resistence on the two pins and sets the encoders;

```
void EncoderSlave::set(int reset_pin, int mode_pin) {
  pinMode(reset_pin, INPUT_PULLUP); //RESET_PIN and MODE_PIN work with pullup resistence
  pinMode(mode_pin, INPUT_PULLUP);
  for(int i = 0; i < settings_u.settings.n; i++) { //sets at 0 four arrays 
    data_u.data.angles[i] = 0;
    data_u.data.rounds[i] = 0;
  }
  encoders = new Encoder[settings_u.settings.n];
  for(int i = 0; i < settings_u.settings.n; i++) {
    encoders[i].init(settings_u.settings.a[i], settings_u.settings.b[i]);
  }
}
```

* **save_to_EEPROM():** it saves the settings inside the EEPROM;

```
void EncoderSlave::save_to_EEPROM() {
  EEPROM.put(settings_u.settings.EEPROM_address, settings_u.settings_byte); 
}
```

* **read_from_EEPROM():** it reads the information previously saved in the EEPROM;

```
void EncoderSlave::read_from_EEPROM() {
  EEPROM.get(settings_u.settings.EEPROM_address, settings_u.settings_byte);
}
```

* **default_settings():** it sets the default settings

```
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
```

* **read(int res_mult, int com_mult):** it reads the value from every encoder and write it in the appropriate variables;

```
void EncoderSlave::read(int res_mult, int com_mult) {  
  for(int i = 0; i < settings_u.settings.n; i++) {
    data_u.data.angles[i] = long(encoders[i].read() * 2 * PI * com_mult / float(settings_u.settings.res * res_mult)); 
    //the values are changed so as to make easier the comunication
  }
}
```

* **info():** it shows the command list;

```
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
```

* **settings_info():** it shows the state of the settings.

```
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
```

In this library has been used two libraries, ***EEPROM*** and ***EncoderMod***, whitch is ***Encoder*** libray with few changes.

MASTER BOARD
========================
In the second arduino code the *Wire.h* library have to be included
##EncodrBoard library

In this library has been included the *arduino.h* library

**FUNCTIONS:**

* **EncoderBoard(int address)**: it sets to 0 angles and rounds and aquires the address of the slave.

```
EncoderBoard::EncoderBoard(int address) {
  Wire.begin();
  for(int i = 0; i < ENCS_MAX; i++) {
    _data_u.data.angles[i] = 0;
    _data_u.data.rounds[i] = 0;
  }
  _address = address;
}
```

* **void update()**: it starts the I2C comunication and puts the data in to the data_t struct. 

```
void EncoderBoard::update() {
  Wire.requestFrom(_address, 128);    // request 128 bytes from slave device 
  int i = 0;
  while(Wire.available()) { // slave may send less than requested
    _data_u.data_byte[i] = (Wire.read()); 
    i++;
  }
}
```

* **double get(int index)**: it gives back the value of the angle from the encoder specified by the index. 

```
double EncoderBoard::get(int index) { 
 return (_data_u.data.angles[index] / 10000.0 ) + 2 * PI * _data_u.data.rounds[index];
}
```

##LINKS:

**libraries:**
