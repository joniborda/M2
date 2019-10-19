#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>

const int INST_SENSOR = 1; // instruccion para censar
const int INST_MANTENIMIENTO = 2; // instruccion para mantenimiento

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

DHT sensorDHT1 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

bool toIrrigate = false;
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
int temperatura1 = 0;
int temperatura2 = 0;
int humedadAmbiente1 = 0;
int humedadAmbiente2 = 0;
int humedadSuelo1 = 0;
int humedadSuelo2 = 0;
int sensorLuz1 = 0;
int sensorLuz2 = 0;
int valorRecibido = 0;

void setup() {
  serialMaster.begin(9600);
  Serial.begin(9600);
  sensorDHT1.begin();
  sensorDHT2.begin();
  Serial.print("iniciado");
}

void loop() {
  if (serialMaster.available() > 0 ) {
    // leyendo lo que me envia el master
    Serial.println("recibio un valor");
    valorRecibido = (int)serialMaster.read();
    Serial.print("valor = ");
    Serial.println(valorRecibido);
    
    switch (valorRecibido) {
      case INST_MANTENIMIENTO: {
        Serial.println("comienza cens1o");
        int censoPrueba = 1;
        mantenimiento();
        break;
      }
      case INST_SENSOR: {
        Serial.println("comienza cens1o");
        int valoresCenso[] = {1, 0, -1, -1, -1, -1, -1, -1, -1, -1};
        //Serial.println(censoPrueba);
        Serial.println("comienza censo");
        censo1(valoresCenso);
        censo2();
        enviarCenso(valoresCenso);
        break;
      }
      default:
        Serial.println("orden enviada fuera de rango");
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

void enviarCenso(int *arrayCenso) {
  /*
  * Formato de envio:
  *
  * <instruccion,respuesta,temperatura1,humedadAmbiente1,humedadSuelo1,sensorLuz1,temperatura2,humedadAmbiente2,humedadSuelo2,sensorLuz2>
  *
  * La instruccion es la instruccion a la que responde
  * La respuesta es el codigo de respuesta. Si es 0 es que está todo bien. Si es 1 o mas es porque hay errores
  */
  String ret = "";
  ret = ret + "<1,0," + arrayCenso[2] + "," + arrayCenso[3] + "," + arrayCenso[4] + "," + arrayCenso[5] + "," + temperatura2 + "," + humedadAmbiente2 + "," + humedadSuelo2 + "," + sensorLuz2 + ">";
  serialMaster.print(ret);
  Serial.println(ret);
}

void censo1(int *arrayCenso) {
  Serial.print("arrayCenso[0]=");
  arrayCenso[2] = sensorDHT1.readTemperature();
  arrayCenso[3] = sensorDHT1.readHumidity();
  arrayCenso[4] = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  arrayCenso[5] = analogRead(PIN_SENSOR_LUZ1);
  String ret = "";
  ret = ret + "temperatura1 " + arrayCenso[2] + ", humedadAmbiente1 " + arrayCenso[3] + ", humedadSuelo1  " + arrayCenso[4] + ", sensorLuz1" + arrayCenso[5];
  Serial.println(ret);
}

void censo2() {
  temperatura2 = sensorDHT2.readTemperature();
  humedadAmbiente2 = sensorDHT2.readHumidity();
  humedadSuelo2 = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  censarLuz2();
  String ret = "";
  ret = ret + "temperatura2 " + temperatura2 + ", humedadAmbiente2 " + humedadAmbiente2 + ", humedadSuelo2  " + humedadSuelo2 + ", sensorLuz2" + sensorLuz2;
  Serial.println(ret);
}

void mantenimiento() {
  //censo1();
  censo2();
  if (abs(temperatura1 - temperatura2) > 10) {
    Serial.println("sensor de temperatura fallando");
  }

  if (abs(humedadAmbiente1 - humedadAmbiente2) > 10) {
    Serial.println("sensor de humedad ambiente fallando");
  }

  if (abs(humedadSuelo1 - humedadSuelo2) > 10) {
    Serial.println("sensor de humedad suelo fallando");
  }

  if (abs(sensorLuz1 - sensorLuz2) > 10) {
    Serial.println("hay diferencia entre los sensores de luz");
    int prev_luz = sensorLuz1;
    encenderLuz1();
    censarLuz1(); //censarlo inmediatamente no se si prueba que anda

    if (prev_luz >= sensorLuz1) {
      Serial.println('encendimos la luz1 y el ldr1 no se dio cuenta, entonces no funciona');
    }
    apagarLuz1();

    prev_luz = sensorLuz2;
    encenderLuz2();
    censarLuz2(); //censarlo inmediatamente no se si prueba que anda

    if (prev_luz >= sensorLuz2) {
      Serial.println('encendimos la luz2 y el ldr2 no se dio cuenta, entonces no funciona');
    }
    apagarLuz2();
  }
}

void encenderLuz1() {
  digitalWrite(PIN_LED1, HIGH);
}

void apagarLuz1() {
  digitalWrite(PIN_LED1, LOW);
}

void censarLuz1() {
  sensorLuz1 = analogRead(PIN_SENSOR_LUZ1);
}

void encenderLuz2() {
  digitalWrite(PIN_LED2, HIGH);
}

void apagarLuz2() {
  digitalWrite(PIN_LED2, LOW);
}

void censarLuz2() {
  sensorLuz2 = analogRead(PIN_SENSOR_LUZ2);
}
