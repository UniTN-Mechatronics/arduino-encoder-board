ENCODER BOARD (ENGLISH)
========================


##PURPOSE


The purpose is read angles and speed from one or more encoders, attached to an Arduino board, which works as Slave, and send them to another Arduino Board, the master, using the I2C connection.

###REQUIREMENT:

* two or more arduino boards;
* jumpers and cables;
* a led with a resistor;
* breadboard;
* one or more encoders;
* two buttons or just two jumpers to simulate the buttons.

###CONNECTIONS:

* connect the Slave's SCL pin with the Master's SCL pin and the Slave's SDA pin with the Master's SDA pin;
* connect the Slave's ground to the Master's ground;
* attach the button, the led and the resistor on the breadboard;
* connect the encoder/s to the Slave board.

![Alt text](https://github.com/DavideDorigoni/arduino-encoder-board/blob/master/electrical_connections.png?raw=true)

##SLAVE BOARD


**CONFIGURATION:**

 to set: number of encoders, resolution, slave's address and pins; the program helps the user doing the setting process on the serial monitor.

Firstly, to access to this mode, the **mode_pin** has to be low.

``` c++
if (digitalRead(MODE_PIN) == LOW) {
Serial.begin(SERIAL_BAUD);
state = Config;
EncS.settings_info(); 
} 
```

Secondly, you have to *write down the settigns* on the serial port:

with those commands you have to *write first the number and then the command letter*.

* 'n' in order to set the **number of encoders**;
* 'a' to set the slave's **address**;
* 'r' to set the **resolution** of every encoders;
* 'l' to set the **speed low threshold**;
* 'k' to set the **speed high threshold**;
* 't' to set the encoder's **lost pulses threshold**;
* 'A' ... 'D' in order to set encoder's **a pins**;
* 'E' ... 'H' in order to set encoder's **b pins**;
* 'I' ... 'N' in order to set encoder's **x pins**.

Instead, the commands below *don't need numbers before the letter*.

* 'z' to consider or not the **index signal**;
* 's' to **save** the settings;
* 'v' to calculate ro not the **angular speed**;
* 'h' to show the values.

To end the configuration process you have to *turn high* the **MODE_PIN** and *reset the board*; thanks to the libray **EEPROM.h** the settings will be rememberd.

**DEFAULT SETTINGS**

*To reset the settings* with the default values you must set low the **RESET_PIN** , saving in to the EEPROM the values firstly setted up by the constructor method. In this way the settings setted up by the user are overlooked and overwritten by the default settings.


**INTERRUPT AND INDEX**

To have an higher precision you can switch on the **index** (`read_index = true`), and connect x channels to the Slave. If the motors have to do lots of rounds, this is the best choice with the lowest possibility of making mistakes. Besides, if the number of pulses lost is more than a threshold, a led will turn on.

The code can afford up to 4 pins connected with the x channels and all of them have to be interrupt pins.

```c++
func_ptr ISRs[] = {index_ISR_0, index_ISR_1, index_ISR_2, index_ISR_3};

```

```c++
if(EncS.settings_u.settings.read_index == true) {
  for( int i = 0; i < EncS.settings_u.settings.n; i++) {
    attachInterrupt(digitalPinToInterrupt(EncS.settings_u.settings.x[i]), ISRs[i], RISING); 
  }
}

```

```c++
void index_ISR_0() {
  encs.lost_pulses[0] = sign(encs.encoders[0].read())*(modulo(encs.encoders[0].read()) - encs.settings_u.settings.res * RES_MULT);
  if((modulo(encs.lost_pulses[0]) > encs.settings_u.settings.lost_pulses_th) && (encs.data_u.data.rounds[0] != 0)) {
    digitalWrite(LED_PIN, HIGH);
  }
  else digitalWrite(LED_PIN, LOW);
  if(encs.data_u.data.rounds[0] == 0) {
    encs.encoders[0].write(0); 
  }
  else {
    // encs.encoders[0].write(0); 
    encs.encoders[0].write(encs.lost_pulses[0] - encs.lost_pulses_b[0]);
    encs.lost_pulses_b[0] = encs.lost_pulses[0];
    }
  encs.data_u.data.rounds[0]++; //every time this function runs,
  // adds a round and reset to 0 the value on the encoder
  encs.speed(0, COM_MULT_SPEED);  
}
```
In this function the speed is calculated every round as the division between 2*pi-greco and the period.

**SENDING TO MASTER**

every time the master requests, every data inside the struct data_t is sent.

```c++
void requestEvent() { 
  Wire.write(EncS.data_u.data_byte, sizeof(data)); 
}
```

**SPEED**

If the read_index is true the speed is calculated in two different ways:

* the speed under the low threshold will be filter:

```c++
if(encs.filters[i].get_speed() < encs.settings_u.settings.speed_th_l) {
    encs.data_u.data.angular_speed[i] = encs.filters[i].get_speed() * COM_MULT_SPEED;
}
```

* up the high threshold by the interrupt:

```c++
else if(encs.filters[i].get_speed() > encs.settings_u.settings.speed_th_h) {
  encs.data_u.data.angular_speed[i] = encs.speed_idx[i] * COM_MULT_SPEED;
}
```

* between the low and high threshold, by a math formula witch combine the two measures:

```c++
else {
  double y = (encs.filters[i].get_speed() - encs.settings_u.settings.speed_th_l) / (encs.settings_u.settings.speed_th_h - encs.settings_u.settings.speed_th_l);
  encs.data_u.data.angular_speed[i] = ((1 - y) * encs.filters[i].get_speed() + y * encs.speed_idx[i]) * COM_MULT_SPEED; 
}
```

if the read_index is false, we use the filter measure of the speed.

##EncoderSlave library
**FUNCTIONS:**

* **EncoderSlave:** it sets the default settigns.

```c++
EncoderSlave::EncoderSlave(){
   default_settings();
}
```

* **set(int reset_pin,int mode_pin):** it sets the encoders with the values found in the struct settings_t and create new arrays. Moreover the data variables, such as angles are setted to 0;

```c++
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
```

* **save_to_EEPROM():** it saves the settings inside the EEPROM.


* **read_from_EEPROM():** it reads the information previously saved in the EEPROM.


* **default_settings():** it sets the default settings.


* **read(int res_mult, int com_mult):** it reads the value from every encoder. The first constant typed in read function, is the resolution multiplier (number of pulses for round / resolution), while the second constant is the comunication multiplier.

```c++
void EncoderSlave::read(int res_mult, int com_mult) {  
  for(int i = 0; i < settings_u.settings.n; i++) {
    data_u.data.angles[i] = long(encoders[i].read() * 2 * PI * com_mult / float(settings_u.settings.res * res_mult)); 
    //the values are changed so as to make easier the comunication
  }
}
```

* **info():** it shows the state of the settings on the serial monitor.


* **settings_info():** it shows the command list.

* **speed(int index, int com_mult): ** it's the function used to calculate the speed in the interrupts.

```c++
double dt = modulo(micros() - t1[index]) / 1000000.0;
t1[index] = micros();
speed_idx[index] = (2.0 * PI / dt);
```

to work, the value of Speed has to be ture.

**FROM THE CLASS DynamicFilter:**

* **DynamicFilter:** it sets as default the values used in the math formula to calculate the filter speed.

* **set_eps(double value):** it allows the user to set the eps parameter.

* **set_hp(double value):** it allows the user to set the hp parameter. 

* **set_hv(double value):** it allows the user to set the hv parameter.

* **update(double dt, double _x):** it contains the formula to calculate the angular speed(rad/s) and the angle(rad). 

```c++
_df_x = _df_x_p + dt*_df_x_dot_p + ((dt*_params_df_hp)/_params_df_eps) *  (_x - _df_x_p);
_df_x_dot = _df_x_dot_p + ((dt*_params_df_hv) / pow(_params_df_eps, 2)) * (_x - _df_x_p);
_df_x_p = _df_x;
_df_x_dot_p = _df_x_dot; 
```

* **get_speed():** it gives back the private value of _df_x_dot, which is the angular speed(rad/s). 

* **get_speed():** it gives back the private value of _df_x, which is the angle(rad).

In this library has been used two libraries, ***EEPROM.h*** and ***EncoderMod.h***, whitch is ***Encoder.h*** libray with few changes.

##MASTER BOARD

First of all, in the master arduino code have to be included the *Wire.h* and **EncoderBoard.h** libraries. 

###EncodrBoard library

In this library has been included *arduino.h* library.

**FUNCTIONS:**

* **EncoderBoard(int address)**: it sets to 0 angles, rounds, angular speed and aquires the address of the slave.

```c++
Wire.begin();
for(int i = 0; i < MAX_ENCS; i++) {
	_data_u.data.angles[i] = 0;
	_data_u.data.rounds[i] = 0;
	_data_u.data.angular_speed[i] = 0;
}
_address = address;
```

* **void update()**: it starts the I2C comunication and puts the data in to the data_t struct. 

```c++
Wire.requestFrom(_address, 128);    // request 128 bytes from slave device 
int i = 0;
while(Wire.available()) { // slave may send less than requested
_data_u.data_byte[i] = (Wire.read()); 
i++;
}
```

* **get(int index)**: it gives back the value of the angle from the encoder specified by the index.

* **angular_speed(int index):** it gives back the value of the angle from the encoder specified by the index.

* **modulo(int value):** it gives back the value always positive.

##LINKS:

**libraries used:**

[encoder library](http://www.pjrc.com/teensy/td_libs_Encoder.html)

[Wire library](https://www.arduino.cc/en/reference/wire)


ENCODER BOARD (ITALIANO)
========================


##SCOPO


Lo scopo è di leggere gli angoli da uno o più encoders collegati ad una scheda Arduino, che funge da Slave, e mandare le misure ad un'altra scheda Arduino, il master, tramite connessione I2C.

###REQUISITI:

* due o più schede arduino;
* jumpers e cavi;
* breadboard;
* un LED e una resistenza;
* uno o più encoders;
* due pulsanti o in alternativa due jumpers che ne simulino il comportamento.

###COLLEGAMENTI:

* connettere il pin SCL dello Slave a quello del master e allo stesso modo connettere il pin SDA dello Slave a quello del Master;
* connettere la massa dello Slave con quella del Master;
* attaccare i pulsanti, il LED e la resistenza alla breadboard;
* connettere il/gli encoder allo Slave.

![Alt text](https://github.com/DavideDorigoni/arduino-encoder-board/blob/master/electrical_connections.png?raw=true)

##SLAVE 

**CONFIGURAZIONE:**

per impostare: numero degli encoders, risoluzione, indirizzo e pins; il programma aiuta l'utente permettendogli di impostare i settaggi tramite monitor seriale.

Come prima cosa, per accedere a questa modalità il **mode_pin** deve essere basso (low).

``` c++
if (digitalRead(MODE_PIN) == LOW) {
Serial.begin(SERIAL_BAUD);
state = Config;
EncS.settings_info(); 
} 
```

Come seconda cosa, bisogna *scrivere le impostazioni* sulla porta seriale:

usando questi comandi bisogna *scrivere prima il numero e poi la lettera* corrispondente al comando.

* 'n' per impostare il**numero di encoders**;
* 'a' per impostare **l'indirizzo** dello Slave;
* 'r' per impostare **la risoluzione** di ogni encoders;
* 'A' ... 'D' per impostare i pins**a**;
* 'E' ... 'H' per impostare i pins**b**;
* 'I' ... 'N' per impostare i pins**x**.

Invece, i comandi sottostanti *non necessitano di nessun numero*.

* 'Z' o 'z' per considerare o no l'indice;
* 's' per **salvare** le impostazioni;
* 'h' per vedere i valori.

Per finire il processo di configurazione, bisogna *mettere alto* il **MODE_PIN** e *resettare la scheda*; grazie alla libreria **EEPROM.h** le impostazioni verranno ricordate.

**IMPOSTAZIONI DI DEFAULT**

*Per ripristinare le impostazioni a quelle di default* bisogna impostare basso il **RESET_PIN** , salvando nella EEPROM i valori impostati precedentemente dal metodo costruttore. In questo modo, le impostazioni impostate dall'utente sono ignorate e sovrascritte da quelle di default.


**INTERRUPT E INDICE**

Per avere una maggiore precisione bisogna impostare l' **indice** (`read_index = true`), e connettere i canali x allo Slave. Se il motore compie molti giri, questa è la migliore opzione con la minima possibilità di fare errori. Inoltre, se il numero di impulsi persi per giro è maggiore di una certa soglia, il led collegato si accende.

Il programma può supportare al massimo 4 canali x, e quest'ultimi devono essere collegati a pins di interrupt.

```c++
func_ptr ISRs[] = {index_ISR_0, index_ISR_1, index_ISR_2, index_ISR_3};

```

```c++
if(EncS.settings_u.settings.read_index == true) {
  for( int i = 0; i < EncS.settings_u.settings.n; i++) {
    attachInterrupt(digitalPinToInterrupt(EncS.settings_u.settings.x[i]), ISRs[i], RISING); 
  }
}

```

```c++
void index_ISR_0() {
  EncS.encoders[0].write(EncS.encoders[0].read() - EncS.settings_u.settings.res * RES_MULT);
  EncS.data_u.data.rounds[0]++;
}
```

**SPEDIRE DATI AL MASTER**

ogni volta che il Master lo richiede, tutti i dati dentro la struct data_t vengono mandati al master.

```c++
void requestEvent() { 
  Wire.write(EncS.data_u.data_byte, sizeof(data)); 
}
```
##EncoderSlave libreria

**FUNZIONI:**

* **EncoderSlave:** imposta i valori di default.

```c++
EncoderSlave::EncoderSlave(){
   default_settings();
}
```

* **set(int reset_pin,int mode_pin):** imposta la resistenza di pullup sui due pin e imposta gli encoders con i valori che trova nella struct settings_t. In più, i valori sono impostati a 0.

```c++
void EncoderSlave::set(int reset_pin, int mode_pin) {
  pinMode(reset_pin, INPUT_PULLUP); 
  pinMode(mode_pin, INPUT_PULLUP);
  for(int i = 0; i < settings_u.settings.n; i++) {  
    data_u.data.angles[i] = 0;
    data_u.data.rounds[i] = 0;
  }
  encoders = new Encoder[settings_u.settings.n];
  for(int i = 0; i < settings_u.settings.n; i++) {
    encoders[i].init(settings_u.settings.a[i], settings_u.settings.b[i]);
  }
}
```

* **save_to_EEPROM():** salva le impostazioni della EEPROM.


* **read_from_EEPROM():** estrapola i valori dalla EEPROM.


* **default_settings():** imposta i valori di default.


* **read(int res_mult, int com_mult):** legge i valori da ogni encoders. la prima costante indica il valore di moltiplicazione che l'encoder ha(impulsi in un giro / risoluzione), mentre la seconda è il motiplicatore che viene usato per facilitare la comunicazione.

```c++
void EncoderSlave::read(int res_mult, int com_mult) {  
  for(int i = 0; i < settings_u.settings.n; i++) {
    data_u.data.angles[i] = long(encoders[i].read() * 2 * PI * com_mult / float(settings_u.settings.res * res_mult)); 
  }
}
```

* **info():** mostra lo stato delle impostazioni sul monitor.



* **settings_info():** mostra la lista dei comandi sul monitor.

In questa libreria sono state usate altre due librerie: ***EEPROM.h*** e ***EncoderMod.h***, la quale è ***Encoder.h*** con qualche modifica.

##MASTER

Prima di tutto, nel codice deve essere inclusa le librerie *Wire.h* ed **EncoderBoard.h**. 

###libreria EncodrBoard 

In questa libreria è stata aggiunta la libreria *arduino.h*.

**FUNZIONI:**

* **EncoderBoard(int address)**: imposta a 0 gli angoli e i giri e acquisisce l'indirizzo dello Slave.

```c++
EncoderBoard::EncoderBoard(int address) {
  Wire.begin();
  for(int i = 0; i < ENCS_MAX; i++) {
    _data_u.data.angles[i] = 0;
    _data_u.data.rounds[i] = 0;
  }
  _address = address;
}
```

* **void update()**: inizia la comunicazione I2C e mette i dati nella struct data_t. 

```c++
void EncoderBoard::update() {
  Wire.requestFrom(_address, 128);    
  int i = 0;
  while(Wire.available()) { 
    _data_u.data_byte[i] = (Wire.read()); 
    i++;
  }
}
```

* **double get(int index)**: restituisce il valore dell'angolo compiuto dall'encoder specificato dall'indice. 


##LINKS:

**librerie usate:**

[encoder library](http://www.pjrc.com/teensy/td_libs_Encoder.html)

[Wire library](https://www.arduino.cc/en/reference/wire)


