#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>

const int INST_SENSOR = 1; // instruccion para censar

const unsigned int PIN_SENSOR_HUMEDAD_AMBIENTE1 = 4;
const unsigned int PIN_SENSOR_HUMEDAD_AMBIENTE2 = 12;
const unsigned int PIN_SENSOR_HUMEDAD_SUELO1 = A2;
const unsigned int PIN_SENSOR_HUMEDAD_SUELO2 = A3; // no tengo
const unsigned int PIN_SENSOR_LUZ1 = A0;
const unsigned int PIN_SENSOR_LUZ2 = A1;
const unsigned int PIN_BOMBA1 = 9;
const unsigned int PIN_BOMBA2 = 6;
const unsigned int PIN_LED1 = 7;
const unsigned int PIN_LED2 = 8;

// puerto hacia el maestro longaniza
const int PUERTO_RX_MASTER = 2;
const int PUERTO_TX_MASTER = 3;


// milisegundos para hacer algo
const unsigned long INTERVAL_TO_DOING = 6000; 

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

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
  Serial.print("iniciado");
}

void loop() {
  if (serialMaster.available() > 0 ) {
    // leyendo lo que me envia el master
    Serial.println("recibio un valor");
    valorRecibido = (int)serialMaster.read();
    Serial.print("valor = ");
    Serial.println(valorRecibido);
    if (valorRecibido == INST_SENSOR) {
      Serial.println("comienza censo");
      censo1();
      censo2();
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

void enviarCenso() {
  String ret = "<"; // si no tiene un valor nulo concatena con basura
  ret = ret + temperatura1 + "," + humedadAmbiente1 + "," + humedadSuelo1 + "," + luz1 + ",";
  ret = ret + temperatura2 + "," + humedadAmbiente2 + "," + humedadSuelo2 + "," + luz2 + ">";
  serialMaster.print(ret);
  Serial.print("<");
  Serial.print(temperatura1);
  Serial.print(",");
  Serial.print(humedadAmbiente1);
  Serial.print(",");
  Serial.print(humedadSuelo1);
  Serial.print(",");
  Serial.print(luz1);
  Serial.print(",");
  Serial.print(temperatura2);
  Serial.print(",");
  Serial.print(humedadAmbiente2);
  Serial.print(",");
  Serial.print(humedadSuelo2);
  Serial.print(",");
  Serial.print(luz2);
  Serial.print(">");
}

void censo1() {
  temperatura1 = sensorDHT1.readTemperature();
  humedadAmbiente1 = sensorDHT1.readHumidity();
  humedadSuelo1 = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  luz1 = analogRead(PIN_SENSOR_LUZ1);
  String ret = ""; // si no tiene un valor nulo concatena con basura
  ret = ret + "temperatura1 " + temperatura1 + ", humedadAmbiente1 " + humedadAmbiente1 + ", humedadSuelo1  " + humedadSuelo1 + ", luz1" + luz1;
  Serial.println(ret);
}

void censo2() {
  temperatura2 = sensorDHT2.readTemperature();
  humedadAmbiente2 = sensorDHT2.readHumidity();
  humedadSuelo2 = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  luz2 = analogRead(PIN_SENSOR_LUZ2);
  String ret = ""; // si no tiene un valor nulo concatena con basura
  ret = ret + "temperatura2 " + temperatura2 + ", humedadAmbiente2 " + humedadAmbiente2 + ", humedadSuelo2  " + humedadSuelo2 + ", luz2" + luz2;
  Serial.println(ret);
}
