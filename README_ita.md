ENCODER BOARD
========================


##SCOPO


Lo scopo del programma è di connettere due o più schede arduino e farle comunicare tramite I2C; con il Master da ricevitore e lo slave come sender. Inoltre, lo slave dovrà occuparsi delle misure di angoli e giri che lette da più encoders.

###REQUISITI:

* due o più schede arduino;
* jumpers e cavi;
* breadboard;
* uno o più encoders;
* due pulsanti o in alternativa due jumpers che ne simulino il comportamento.

###COLLEGAMENTI:

* connettere il pin SCL dello Slave a quello del master e allo stesso modo connettere il pin SDA dello Slave a quello del Master;
* connettere la massa dello Slave con quella del Master;
* attaccare i pulsanti alla breadboard;
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

Per avere una maggiore precisione bisogna impostare l' **indice** (`read_index = true`), e connettere i canali x allo Slave. Se il motore compie molti giri, questa è la migliore opzione con la minima possibilità di fare errori.

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

* **set(int reset_pin,int mode_pin):** imposta la resistenza di pullup sui due pin e imposta gli encoders con i valori che trova nella struct settings_t. In più, i valori come l'angolo sono impostati a 0.

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


