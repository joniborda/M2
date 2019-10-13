#include <SoftwareSerial.h>

// puerto hacia el esclavo
const int PUERTO_RX_SLAVE = 2;
const int PUERTO_TX_SLAVE = 3;

// instrucciones que envio al esclavo
const int INST_SENSOR = 1; // instruccion para censar
const int INST_IRRIGATE = 2; // instruccion para regar

// milisegundos para comenzar censor
const unsigned long INTERVAL_TO_SENSOR = 1000*60; 

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);

  bool toIrrigate = false;
  unsigned long currentMillis = millis(); // grab current time
  unsigned long previousMillis = 0;  // millis() returns an unsigned long.
  int temperatura1 = 0;
  int temperatura2 = 0;
  int humedadAmbiente1 = 0;
  int humedadAmbiente2 = 0;
  int humedadSuelo1 = 0;
  int humedadSuelo2 = 0;
  int luz1 = 0;
  int luz2 = 0;
}

void loop() {

  leerEsclavo();

  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= INTERVAL_TO_SENSOR) {
 	// to do anything
 	serialSlave.write(INST_SENSOR);
    previousMillis = millis();
  }

  verificarRiego();
}

void leerEsclavo() {
  if (serialSlave.available() > 0 ) {
  	// leyendo lo que me envia el esclavo
    Serial.print((char)serialSlave.read());
  }
}

void verificarRiego() {
  if (toIrrigate) {
  	// TODO: Verificar que si envio regar haya un tiempo de diferencia con el censo para que no se pisen
  	serialSlave.write(INST_IRRIGATE);
  	toIrrigate = false;
  }
}