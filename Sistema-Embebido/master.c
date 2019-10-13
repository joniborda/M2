#include <SoftwareSerial.h>

// puerto hacia el esclavo
const int PUERTO_RX_SLAVE = 2;
const int PUERTO_TX_SLAVE = 3;

// instrucciones que envio al esclavo
const int INST_SENSOR = 1; // instruccion para censar
const int INST_IRRIGATE = 2; // instruccion para regar

const unsigned long INTERVAL_TO_SENSOR = 1000*60; // milisegundos para comenzar censor


SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);

  unsigned long currentMillis = millis(); // grab current time
  unsigned long previousMillis = 0;  // millis() returns an unsigned long.
}

void loop() {

  if (serialSlave.available() > 0 ) {
  	// leyendo lo que me envia el esclavo
    Serial.print((char)serialSlave.read());
  }

  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= INTERVAL_TO_SENSOR) {
 	// to do anything
 	serialSlave.write(INST_SENSOR);
    previousMillis = millis();
  }



}