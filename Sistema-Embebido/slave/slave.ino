#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>

#define RUTINA_SIN_ERROR 0
#define RUTINA_CON_ERROR 1

const int INST_CENSO = 1; // INSTRUCCION PARA RUTINA DE CENSO
const int INST_RIEGO_Z1 = 2; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1
const int INST_RIEGO_Z2 = 3; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2
const int INST_MANTENIMIENTO = 4; // INSTRUCCION PARA RUTINA DE MANTENIMIENTO

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

// PUERTOS DE CONEXION CON MAESTRO
const int PUERTO_RX_MASTER = 2;
const int PUERTO_TX_MASTER = 3;

// INTERVALO PARA ACCION EN MS
const unsigned long INTERVAL_TO_DOING = 6000; 

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

void setup() {
  serialMaster.begin(9600); //Velocidad comunicacion maestro
  Serial.begin(9600); //Velocidad comunicacion monitor serial
  sensorDHT1.begin();
  sensorDHT2.begin();
  Serial.print("Arduino Esclavo inciado, esperando instrucciones...");
}

void loop() {
  if (serialMaster.available() > 0 ) {
    int instruccionRecibida = 0;
    // Recibiendo informacion del maestro
    Serial.println("Se recibio una instruccion del maestro. x`");
    instruccionRecibida = (int)serialMaster.read();
    Serial.print("VALOR: ");
    Serial.println(instruccionRecibida);
    
    switch (instruccionRecibida) {
      case INST_CENSO: {
        Serial.println("COMIENZA RUTINA DE CENSO.");
        int valorSensores[] = {INST_CENSO, RUTINA_SIN_ERROR, -1, -1, -1, -1, -1, -1, -1, -1};
        sensarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
        sensarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
        enviarResultadoCensoAMaestro(valorSensores);
        break;
      }

      case INST_MANTENIMIENTO: {
        Serial.println("COMIENZA RUTINA DE MANTENIMIENTO.");
        mantenimiento();
        break;
      }

      case INST_RIEGO_Z1: {
        break;
      }

      case INST_RIEGO_Z2: {
        break;
      }

      default:{
        Serial.println("No se encontro rutina para ese valor.");
        break;
      }
    }
  }
}

void sensarZona1(int* vec) {
  vec[2] = sensorDHT1.readTemperature();
  vec[3] = sensorDHT1.readHumidity();
  vec[4] = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  vec[5] = analogRead(PIN_SENSOR_LUZ1);
  String ret = "";
  ret = ret + "Temperatura1: " + vec[2] + ", HumedadAmbiente1: " + vec[3] + ", HumedadSuelo1: " + vec[4] + ", SensorLuz1: " + vec[5];
  Serial.println(ret);
}

void sensarZona2(int* vec) {
  vec[6] = sensorDHT2.readTemperature();
  vec[7] = sensorDHT2.readHumidity();
  vec[8] = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  vec[9] = analogRead(PIN_SENSOR_LUZ2);
  String ret = "";
  ret = ret + "Temperatura2: " + vec[6] + ", HumedadAmbiente2: " + vec[7] + ", HumedadSuelo2: " + vec[8] + ", SensorLuz2: " + vec[9];
  Serial.println(ret);
}

void mantenimiento() {
  int valorSensores[] = {INST_MANTENIMIENTO, RUTINA_SIN_ERROR, -1, -1, -1, -1, -1, -1, -1, -1};
  
  sensarZona1(valorSensores);
  sensarZona2(valorSensores);

  if (abs(valorSensores[2] - valorSensores[6]) > 10) {
    Serial.println("Sensor de temperatura con fallas.");
  }

  if (abs(valorSensores[3] - valorSensores[7]) > 10) {
    Serial.println("Sensor de humedad atmosferica con fallas.");
  }

  //DEBERIA SER DE NOCHE PORQUE SI NO NUNCA LO VA A DETECTAR
  int valorLuzAnteriorZona1 = valorSensores[5];
  digitalWrite(PIN_LED1, HIGH);
  delay(10);
  int valorLuzActualZona1 = analogRead(PIN_SENSOR_LUZ1);
  if(valorLuzAnteriorZona1 >= valorLuzActualZona1) {
    Serial.print("Se encendio la luz de la zona 1 y el sensor LDR1 no lo detecto.");
  }
  digitalWrite(PIN_LED1, LOW);
}

void enviarResultadoCensoAMaestro(int* vec) {
  /*
  * Formato de envio:
  * <instruccion, respuesta, temperatura1, humedadAmbiente1, humedadSuelo1, sensorLuz1, temperatura2, humedadAmbiente2, humedadSuelo2, sensorLuz2>
  * La instruccion es la que inicio la rutina
  * La respuesta es un codigo de respuesta. Si es 0 es que está ok. Si es 1 o mas es porque surgio un error.
  */
  String ret = "";
  ret = ret + "<" + INST_CENSO + ",0," + vec[2] + "," + vec[3] + "," + vec[4] + "," + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + vec[9] + ">";
  serialMaster.print(ret);
  Serial.println(ret);
}

void enviarResultadoMantenimientoAMaestro(){

}
