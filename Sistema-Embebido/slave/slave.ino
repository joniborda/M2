#include <SoftwareSerial.h>
#include <DHT_U.h>
#include <DHT.h>

#define TAM_MAX_WRITE 9
#define VALOR_LIMITE_LUZ 500

#define INST_CENSO                  1 // INSTRUCCION PARA RUTINA DE CENSO INICIO
#define INST_FIN_CENSO              2 // INSTRUCCION PARA RUTINA DE CENSO FIN
#define INST_RIEGO_Z1               3 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 INICIO
#define INST_RIEGO_Z2               4 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 INICIO
#define INST_FIN_RIEGO_Z1           5 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 FIN
#define INST_FIN_RIEGO_Z2           6 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 FIN
#define INST_MANTENIMIENTO          7 // INSTRUCCION PARA INICIO RUTINA DE MANTENIMIENTO
#define INST_RES_MANTENIMIENTO      8 // INSTRUCCION PARA RESPUESTA DE RUTINA DE MANTENIMIENTO
#define INST_DETENER_RIEGO_Z1       9 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 1
#define INST_DETENER_RIEGO_Z2       10 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 2
#define INST_ENCENDER_LUZ_1_MANUAL  11 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_ENCENDER_LUZ_2_MANUAL  12 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_APAGAR_LUZ_1_MANUAL    13 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_APAGAR_LUZ_2_MANUAL    14 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_AUTO_LUZ_1             15 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_AUTO_LUZ_2             16 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_RES_RIEGO_Z1           17 //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2           18 //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2

#define PIN_SENSOR_HUMEDAD_AMBIENTE1 4
#define PIN_SENSOR_HUMEDAD_AMBIENTE2 12
#define PIN_SENSOR_HUMEDAD_SUELO1 A2
#define PIN_SENSOR_HUMEDAD_SUELO2 A3
#define PIN_SENSOR_LUZ1 A0
#define PIN_SENSOR_LUZ2 A1
#define PIN_BOMBA1 9
#define PIN_BOMBA2 6
#define PIN_LED1 7
#define PIN_LED2 8

// PUERTOS DE CONEXION CON MAESTRO
#define PUERTO_RX_MASTER 2
#define PUERTO_TX_MASTER 3

// INTERVALO PARA ACCION EN MS
const unsigned long TIEMPO_RES_RIEGO = 3000;
const unsigned long TIEMPO_RES_MANTENIMIENTO = 1000;

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

unsigned long tiempoActual = millis();
unsigned long tiempoComienzoRiegoZona1 = 0;
unsigned long tiempoDespuesRiegoZona1 = 0;
unsigned long tiempoComienzoRiegoZona2 = 0;
unsigned long tiempoDespuesRiegoZona2 = 0;
unsigned long tiempoMantenimiento = 0;

const unsigned long TIEMPO_RIEGO = 10000;
int prenderLuz1 = 0; // 0 es automatica, 1 es encendido manual, distinto de 0 y de 1 es apagado manual
int prenderLuz2 = 0; // 0 es automatica, 1 es encendido manual, distinto de 0 y de 1 es apagado manual
int valoresMantenimiento[5] = {1, 1, 1, 1, 1};
/*
0 significa que tiene errores
1 significa que funciona correctamente
[0] = temperatura de zona 1 y zona 2
[1] = humedadAmbiente de zona 1 y zona 2
[2] = humedadSuelo de zona 1 y zona 2
[3] = SensorLuz de zona 1
[4] = SensorLuz de zona 2
*/

void setup() {
  serialMaster.begin(9600); //Velocidad comunicacion maestro
  Serial.begin(9600); //Velocidad comunicacion monitor serial
  sensorDHT1.begin();
  sensorDHT2.begin();
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_BOMBA1, OUTPUT);
  pinMode(PIN_BOMBA2, OUTPUT);
  pinMode(PIN_SENSOR_HUMEDAD_SUELO1, INPUT); // Si no censa la humedad es porque hay que sacar esto
  pinMode(PIN_SENSOR_HUMEDAD_SUELO1, INPUT); // Si no censa la humedad es porque hay que sacar esto
  pinMode(PIN_SENSOR_LUZ1, INPUT);
  pinMode(PIN_SENSOR_LUZ2, INPUT);
  Serial.println("Esclavo iniciado, esperando instrucciones");
}

void loop() {
  if (serialMaster.available() > 0 ) {
    // Recibiendo informacion del maestro.
    Serial.println("Se recibio una instruccion del maestro.");
    int instr_recibida = -1;
    float intesidadRiego = -1;
    leerMaestro(&instr_recibida, &intesidadRiego);
    Serial.print("INSTRUCCION: ");
    Serial.println("" + instr_recibida);
    switch (instr_recibida) {
      case INST_CENSO: {
        Serial.println("RUTINA DE CENSO.");
        int valorSensores[] = {INST_FIN_CENSO, -1, -1, -1, -1, -1, -1, -1, -1};
        sensarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
        sensarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
        enviarResultadoCensoAMaestro(valorSensores);
        break;
      }
      case INST_MANTENIMIENTO: {
        Serial.println("RUTINA DE MANTENIMIENTO.");
        iniciarMantenimiento();
        break;
      }
      case INST_RIEGO_Z1: {
        Serial.println("RIEGO ZONA 1.");
        tiempoComienzoRiegoZona1 = millis();
        float intesidadRiegoZona1 = intesidadRiego;
        analogWrite(PIN_BOMBA1, intesidadRiegoZona1 * 255/100);
        break;
      }
      case INST_RIEGO_Z2: {
        Serial.println("RIEGO ZONA 2.");
        tiempoComienzoRiegoZona2 = millis();
        float intensidadRiegoZona2 = intesidadRiego;
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2 * 255/100);
        break;
      }
      case INST_DETENER_RIEGO_Z1: {
        // PENSAR QUE PASARIA SI SE DETIENE EL RIEGO CON RESPECTO A LA RESPUESTA
        Serial.println("STOP ZONA 1.");
        tiempoComienzoRiegoZona1 = 0;
        analogWrite(PIN_BOMBA1, 0);
        tiempoDespuesRiegoZona1 = 0;
        break;
      }
      case INST_DETENER_RIEGO_Z2: {
        Serial.println("STOP ZONA 2.");
        tiempoComienzoRiegoZona2 = 0;
        analogWrite(PIN_BOMBA2, 0);
        tiempoDespuesRiegoZona2 = 0;
        break;
      }
      case INST_ENCENDER_LUZ_1_MANUAL: {
        Serial.println("PRENDER LUZ 1 MANUAL");
        prenderLuz1 = 1;
        break;
      }
      case INST_ENCENDER_LUZ_2_MANUAL: {
        Serial.println("PRENDER LUZ 2 MANUAL");
        prenderLuz2 = 1;
        break;
      }
      case INST_APAGAR_LUZ_1_MANUAL: {
        Serial.println("APAGAR LUZ 1 MANUAL");
        prenderLuz1 = 2;
        break;
      }
      case INST_APAGAR_LUZ_2_MANUAL: {
        Serial.println("APAGAR LUZ 2 MANUAL");
        prenderLuz2 = 2;
        break;
      }
      case INST_AUTO_LUZ_1: {
        Serial.println("LUZ AUTO 1");
        prenderLuz1 = 0;
        break;
      }
      case INST_AUTO_LUZ_2: {
        Serial.println("LUZ AUTO 2");
        prenderLuz2 = 0;
        break;
      }
      default:{
        Serial.println("No se encontro rutina para ese valor.");
        break;
      }
    }
  }
  
  tiempoActual = millis();
  if (tiempoComienzoRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona1) >= TIEMPO_RIEGO) {
    // Aviso que termino de regar la zona 1.
    analogWrite(PIN_BOMBA1, 0);
    String ret = "";
    ret = ret + "<" + INST_RIEGO_Z1 + ">";
    serialMaster.print(ret);
    tiempoDespuesRiegoZona1 = millis();
    tiempoComienzoRiegoZona1 = 0;
  }

  tiempoActual = millis();
  if (tiempoDespuesRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona1) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro el valor del sensor de humedad del suelo.
    String ret = "";
    ret = ret + "<" + INST_RES_RIEGO_Z1 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO1) + ">"; 
    serialMaster.print(ret);
    tiempoDespuesRiegoZona1 = 0;
  }
  
  tiempoActual = millis();
  if (tiempoComienzoRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona2) >= TIEMPO_RIEGO) {
    // Aviso que termino de regar la zona 2.
    analogWrite(PIN_BOMBA2, 0);
    String ret = "";
    ret = ret + "<" + INST_RIEGO_Z2 + ">";
    serialMaster.print(ret);
    Serial.println(ret);
    tiempoDespuesRiegoZona2 = millis();
    tiempoComienzoRiegoZona2 = 0;
  }

  tiempoActual = millis();
  if (tiempoDespuesRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona2) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro el valor del sensor de humedad del suelo.
    String ret = "";
    ret = ret + "<" + INST_RES_RIEGO_Z2 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO2) + ">";
    serialMaster.print(ret);
    Serial.println(ret);
    tiempoDespuesRiegoZona2 = 0;
  }

  // 0 para prender luz de forma automaticamente
  // 1 para prender luz de forma manual
  // 2 para apagar luz de forma manual (cualquier valor distinto de 1 o 0)
  if (prenderLuz1 == 1 || (prenderLuz1 == 0 && analogRead(PIN_SENSOR_LUZ1) > VALOR_LIMITE_LUZ)) {
    digitalWrite(PIN_LED1, HIGH);
  } else {
    digitalWrite(PIN_LED1, LOW);
  }

  if (prenderLuz2 == 1 || (prenderLuz2 == 0 && analogRead(PIN_SENSOR_LUZ2) > VALOR_LIMITE_LUZ)) {
    digitalWrite(PIN_LED2, HIGH);
  } else {
    digitalWrite(PIN_LED2, LOW);
  }

  tiempoActual = millis();
  if (tiempoMantenimiento > 0 && (unsigned long)(tiempoActual - tiempoMantenimiento) >= TIEMPO_RES_MANTENIMIENTO) {
    finalizarMantenimiento();
  }
}

void sensarZona1(int* vec) {
  vec[1] = sensorDHT1.readTemperature();
  vec[2] = sensorDHT1.readHumidity();
  vec[3] = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  vec[4] = analogRead(PIN_SENSOR_LUZ1);
  String ret = "";
  ret = ret + "TEMP1: " + vec[1] + ", HUMAMB1: " + vec[2] + ", HUMSUE1: " + vec[3] + ", SENLUZ1: " + vec[4];
  Serial.println(ret);
}

void sensarZona2(int* vec) {
  vec[5] = sensorDHT2.readTemperature();
  vec[6] = sensorDHT2.readHumidity();
  vec[7] = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  vec[8] = analogRead(PIN_SENSOR_LUZ2);
  String ret = "";
  ret = ret + "TEMP2: " + vec[5] + ", HUMAMB2: " + vec[6] + ", HUMSUE2: " + vec[7] + ", SENLUZ2: " + vec[8];
  Serial.println(ret);
}

void iniciarMantenimiento(valoresMantenimiento) {
  tiempoMantenimiento = millis();

  int valorSensores[] = {INST_MANTENIMIENTO, -1, -1, -1, -1, -1, -1, -1, -1};
  sensarZona1(valorSensores);
  sensarZona2(valorSensores);
  for (int i = 0; i < sizeof(valoresMantenimiento); i++) {
    valoresMantenimiento[i] = 1;
  }
  

  if (abs(valorSensores[1] - valorSensores[5]) > 10) {
    valoresMantenimiento[0] = 0;
    Serial.println("E_S_T");//Sensor de temperatura con fallas
  }

  if (abs(valorSensores[2] - valorSensores[6]) > 10) {
    valoresMantenimiento[1] = 0;
    Serial.println("E_S_H");//Sensor de humedad atmosferica con fallas
  }

  //DEBERIA SER DE NOCHE PORQUE SI NO NUNCA LO VA A DETECTAR
  int valorLuzAnteriorZona1 = valorSensores[4];
  digitalWrite(PIN_LED1, HIGH);
  int valorLuzAnteriorZona2 = valorSensores[8];
  digitalWrite(PIN_LED2, HIGH);
}

/**
 * Censa la luz para ver como varia y envia los resultados al esclavo
 */
void finalizarMantenimiento() {
  int valorLuzActualZona = analogRead(PIN_SENSOR_LUZ1);
  if(valorLuzAnteriorZona1 >= valorLuzActualZona) {
    valoresMantenimiento[3] = 0;
    Serial.println("E_L_1");//Se encendio la luz de la zona 1 y el sensor LDR1 no lo detecto
  }
  digitalWrite(PIN_LED1, LOW);

  valorLuzActualZona = analogRead(PIN_SENSOR_LUZ2);

  if(valorLuzAnteriorZona2 >= valorLuzActualZona) {
    valoresMantenimiento[4] = 0;
    Serial.println("E_L_2");//Se encendio la luz de la zona 2 y el sensor LDR2 no lo detecto
  }
  digitalWrite(PIN_LED2, LOW);
  /*
   * Se envia <instruccion, ErrorTemp, ErrorHumAmb, ErrorHumSuelo, ErrorLDR1, ErrrorLDR2>
   */
  String ret = "";
  ret = ret + "<" + INST_RES_MANTENIMIENTO + "," + valoresMantenimiento[0] + "," + valoresMantenimiento[1] + 
  "," + valoresMantenimiento[2] + "," + valoresMantenimiento[3] + "," + valoresMantenimiento[4] + ">";
  serialMaster.print(ret);
  Serial.println(ret);
  tiempoMantenimiento = 0;
}

void enviarResultadoCensoAMaestro(int* vec) {
  /*
   * Formato de envio:
   * <instruccion, temperatura1, humedadAmbiente1, humedadSuelo1, sensorLuz1, temperatura2, humedadAmbiente2, humedadSuelo2, sensorLuz2>
   * La instruccion es la que inicio la rutina
   */
  String ret = "";
  ret = ret + "<" + vec[0] + "," + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + ">";
  serialMaster.print(ret);
  Serial.println(ret);
}

void leerMaestro(int* inst, float* intesidad) {
  byte charIndex = 0; // Es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo.
  char charLeido;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }

  if (serialMaster.available() > 0) {
    serialMaster.readBytesUntil('>', entrada, 59);
    int i = 0;
    while(entrada[i] != '\0') {
      if (entrada[i] == '<') {
        i++;
        continue;
      }
      if (entrada[i] != ',') {
        input[charIndex] = entrada[i];
        charIndex++;  
      } else {
        input[charIndex] = '\0';
        charIndex = 0;
        if (fieldIndex == 0) {
          *inst = atoi(input);
          Serial.println(*inst);
        } else if (fieldIndex == 1) {
          *intesidad = atof(input);
        }
        fieldIndex++;
      }
      i++;
    }

    input[charIndex] = '\0';
    if(fieldIndex == 0){
      *inst = atoi(input);
    } else if(fieldIndex == 1) {
      *intesidad = atof(input);
    }

    String ret = "";
    ret = ret + "INSTRUCCION: " + *inst + ", INTENSIDAD: " + *intesidad;            
    Serial.println(ret);
  }
}
