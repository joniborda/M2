#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>

const int INST_SENSOR = 1; // instruccion para censar

const unsigned int PIN_SENSOR_HUMEDAD_HANBIENTE1 = 4;
const unsigned int PIN_SENSOR_HUMEDAD_HANBIENTE2 = 5;
const unsigned int PIN_SENSOR_HUMEDAD_SUELO1 = 6;
const unsigned int PIN_SENSOR_HUMEDAD_SUELO2 = 7;
const unsigned int PIN_SENSOR_LUZ1 = 8;
const unsigned int PIN_SENSOR_LUZ2 = 9;
// puerto hacia el maestro longaniza
const int PUERTO_RX_MASTER = 2;
const int PUERTO_TX_MASTER = 3;


// milisegundos para hacer algo
const unsigned long INTERVAL_TO_DOING = 100*60; 

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1(PIN_SENSOR_HUMEDAD_HANBIENTE1, DHT11);
//DHT sensorDHT2(PIN_SENSOR_HUMEDAD_HANBIENTE2, DHT11);

bool toIrrigate = false;
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
int temperatura1 = 0;
int temperatura2 = 0;
int humedadAmbiente1 = 0;
int humedadAmbiente2 = 0;
int humedadSuelo1 = 0;
int humedadSuelo2 = 0;
int luz1 = 0;
int luz2 = 0;
int valorRecibido = 0;

void setup() {
  serialMaster.begin(9600);
  Serial.begin(9600);
  sensorDHT1.begin();
}

void loop() {
  if (serialMaster.available() > 0 ) {
  	// leyendo lo que me envia el master
    Serial.println("recibio un valor");
    valorRecibido = (int)serialMaster.read();
    Serial.print("valor = ");
    Serial.println(valorRecibido);
    if (valorRecibido == INST_SENSOR) {
      Serial.println("comienza riego");
      censo();
      enviarCenso();
    }
  }
/*
  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= INTERVAL_TO_DOING) {
 	// to do anything
  	censo();
  	enviarCenso();
 	
    previousMillis = millis();
  }*/
}

void censo() {
	temperatura1 = sensorDHT1.readTemperature();
  humedadAmbiente1 = sensorDHT1.readHumidity();
//  temperatura2 = sensorDHT2.readTemperature();
//  humedadAmbiente2 = sensorDHT2.readHumidity();

// luz1 = analogRead(PIN_SENSOR_LUZ1);
// luz2 = analogRead(PIN_SENSOR_LUZ2);
  Serial.print("<");
  Serial.print(temperatura1);
  Serial.print(",");
  Serial.print(humedadAmbiente1);
  Serial.println(">");
}

void enviarCenso() {
  serialMaster.print("<");
  serialMaster.print(temperatura1);
  serialMaster.print(",");
  serialMaster.print(humedadAmbiente1);
  serialMaster.print(",");
  serialMaster.print(humedadSuelo1);
  serialMaster.print(",");
  serialMaster.print(luz1);
  serialMaster.print(",");
  serialMaster.print(temperatura2);
  serialMaster.print(",");
  serialMaster.print(humedadAmbiente2);
  serialMaster.print(",");
  serialMaster.print(humedadSuelo2);
  serialMaster.print(",");
  serialMaster.print(luz2);
  serialMaster.print(">");
}
