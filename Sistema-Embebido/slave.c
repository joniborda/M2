#include <SoftwareSerial.h>

// puerto hacia el maestro longaniza
const int PUERTO_RX_MASTER = 2;
const int PUERTO_TX_MASTER = 3;

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

void setup() {
  serialMaster.begin(9600);
  Serial.begin(9600);
}

void loop() {

  if (serialMaster.available() > 0 ) {
  	// leyendo lo que me envia el master
    Serial.print((char)serialMaster.read());
    
  }

}