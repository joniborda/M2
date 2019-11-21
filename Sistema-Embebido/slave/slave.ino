#include <SoftwareSerial.h>
#include <DHT_U.h>
#include <DHT.h>

#define DEBUG  //comentar esto para no usar el debug

#ifdef DEBUG
 #define DEBUG_PRINT(x) Serial.println(x)
#else
 #define DEBUG_PRINT(x)
#endif

#define VALOR_MAX_LUZ 600
#define VALOR_MIN_LUZ 550 // UMBRAL DE 50
#define POND_LUZ_INT 150

#define INST_CENSO                  1 // INSTRUCCION PARA RUTINA DE CENSO INICIO
#define INST_FIN_CENSO              2 // INSTRUCCION PARA RUTINA DE CENSO FIN
#define INST_RIEGO_Z1               3 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 INICIO
#define INST_RIEGO_Z2               4 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 INICIO
#define INST_FIN_RIEGO_Z1           5 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 FIN
#define INST_FIN_RIEGO_Z2           6 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 FIN
#define INST_MANTENIMIENTO          7 // INSTRUCCION PARA INICIO RUTINA DE MANTENIMIENTO
#define INST_RES_MANTENIMIENTO      8 // INSTRUCCION PARA RESPUESTA DE RUTINA DE MANTENIMIENTO
#define INST_DETENER_RIEGO_Z1       9 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 1               @DEPRECATED
#define INST_DETENER_RIEGO_Z2       10 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 2              @DEPRECATED
#define INST_ENCENDER_LUZ_1_MANUAL  11 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_ENCENDER_LUZ_2_MANUAL  12 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_APAGAR_LUZ_1_MANUAL    13 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_APAGAR_LUZ_2_MANUAL    14 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_AUTO_LUZ_1             15 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_AUTO_LUZ_2             16 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_RES_RIEGO_Z1           17 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2           18 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2
#define INST_INICIO_CONEXION_BT     19 // INSTRUCCION QUE ENVIA EL BLUETOOTH AVISANDO QUE SE CONECTO
#define INST_CENSO_MANUAL           20 // INSTRUCCION PARA COMENZAR UN CENSO MANUAL
#define INST_RIEGO_MANUAL           21 // INSTRUCCION PARA COMENZAR UN RIEGO MANUAL
#define INST_TIPO_RIEGO_CONT        22 // INSTRUCCION PARA CAMBIAR EL TIPO DE RIEGO A CONTINUO
#define INST_TIPO_RIEGO_INTER       23 // INSTRUCCION PARA CAMBIAR EL TIPO DE RIEGO A INTERMITENTE
#define INST_FIN_RIEGO_MANUAL       24 // INSTRUCCION QUE INDICA QUE SE FINALIZO EL RIEGO MANUAL
#define INST_DETENER_RIEGO_MANUAL   25 // INSTRUCCION QUE DETIENE EL RIEGO MANUAL                     @DEPRECATED
#define INST_DETENER_RIEGO_GRAL     26 // INSTRUCCION QUE DETENIENE CUALQUIER TIPO DE RIEGO, SEA MANUAL O AUTOMATICO
#define INST_DESCONEXION_BT         27 // INSTRUCCION QUE INDICA LA DESCONEXION DE UN DISPOSITIVO  

#define M_INICIO_ARDUINO_OK         50
#define M_INICIO_RIEGO_Z1           52
#define M_INICIO_RIEGO_Z2           53
#define M_INICIO_RIEGO_M            54
#define M_INICIO_CENSO              55
#define M_INICIO_CENSO_M            56

#define M_INICIO_CONEXION_BT_ER     57
#define M_RIEGO_MANUAL_ER           58
#define M_CENSO_MANUAL_ER           59
#define M_MANT_MANUAL_ER            60
#define M_STOP_RIEGO_GRAL_OK        61  
#define M_STOP_RIEGO_GRAL_ER        62
#define M_CAMBIO_T_RIEGO_CONT       63
#define M_CAMBIO_T_RIEGO_INT        64

#define PIN_SENSOR_LUZ1             A0
#define PIN_SENSOR_LUZ2             A1
#define PIN_SENSOR_HUMEDAD_SUELO2   A2
#define PIN_SENSOR_HUMEDAD_SUELO1   A3

// PUERTOS DE CONEXION CON MAESTRO
#define PUERTO_RX_MASTER              2
#define PUERTO_TX_MASTER              3
#define PIN_SENSOR_HUMEDAD_AMBIENTE1  4
#define PIN_SENSOR_HUMEDAD_AMBIENTE2  12
#define PIN_BOMBA1                    9
#define PIN_BOMBA2                    6
#define PIN_LED1                      7
#define PIN_LED2                      8

// INTERVALO PARA ACCION EN MS
const unsigned long TIEMPO_RES_RIEGO = 10000UL;          // MS Para dar la respuesta de humedad despues de regar
const unsigned long TIEMPO_RES_MANTENIMIENTO = 10000UL;  // MS Para dar la respuesta de mantenimiento despues de encender luces
const unsigned long TIEMPO_INTERMITENCIA = 1000UL;      // MS En el cual prende y apaga la bomba de riego

unsigned long TIEMPO_RIEGO = 10000UL;
unsigned long TIEMPO_RIEGO_MANUAL = 0UL;

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

unsigned long tiempoActual = millis();
unsigned long tiempoComienzoRiegoZona1 = 0UL;     // Tiempo que falta para avisar que termino el riego
unsigned long tiempoDespuesRiegoZona1 = 0UL;      // Tiempo que falta para dar la respuesta de riego
unsigned long tiempoComienzoRiegoZona2 = 0UL;     // Tiempo que falta para avisar que termino el riego
unsigned long tiempoDespuesRiegoZona2 = 0UL;      // Tiempo que falta para dar la respuesta de riego
unsigned long tiempoMantenimiento = 0UL;          // Tiempo que falta para terminar el mantenimiento
unsigned long tiempoComienzoRiegoManual = 0UL;    // Tiempo que debe transcurrir el riego manual
unsigned long tiempoComienzoIntermitencia1 = 0UL; // Tiempo que apaga y prende la zona 1
unsigned long tiempoComienzoIntermitencia2 = 0UL; // Tiempo que apaga y prende la zona 2

//Tipo de riego: 0 continuo, 1 intermitente
static int tipoRiego = 0; //Por defecto es continuo

static bool riegoManualEnCurso = false;
static bool riegoZona1AutomaticoEnCurso = false;
static bool riegoZona2AutomaticoEnCurso = false;
static bool censoManualEnCurso = false;
static bool censoAutomaticoEnCurso = false;
static bool mantenimientoManualEnCurso = false;
static bool mantenimientoAutomaticoEnCurso = false; //Ready to development

static int prenderLuz1 = 0; // 0 Es automatica, 1 es encendido manual, distinto de 0 y de 1 es apagado manual
static int prenderLuz2 = 0; // 0 Es automatica, 1 es encendido manual, distinto de 0 y de 1 es apagado manual
static float intensidadRiegoZona1 = 0;
static float intensidadRiegoZona2 = 0;

static int vecMant [5];

/* 
Valores de Mantenimiento
0 Significa que tiene errores
1 Significa que funciona correctamente
[0] = Temperatura de zona 1 y zona 2
[1] = HumedadAmbiente de zona 1 y zona 2
[2] = HumedadSuelo de zona 1 y zona 2
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
  pinMode(PIN_SENSOR_HUMEDAD_SUELO1, INPUT);
  pinMode(PIN_SENSOR_HUMEDAD_SUELO1, INPUT);
  pinMode(PIN_SENSOR_LUZ1, INPUT);
  pinMode(PIN_SENSOR_LUZ2, INPUT);
  sendMessageToMaster(M_INICIO_ARDUINO_OK);
}

void loop() {
  int instr_recibida = -1;
  float intesidadRiego = -1; //Puede ser de riego manual o automatico
  int tRiegoManual = -1;

  leerMaestro(&instr_recibida, &intesidadRiego);
  leerBluetooth(&instr_recibida, &intesidadRiego, &tRiegoManual); //El bluetooth posee mas prioridad que el maestro
  switch (instr_recibida) {
    case INST_CENSO: {
      sendMessageToMaster(M_INICIO_CENSO);
      if (!evaluaAccionEnProcesoBluetooth()) {
        censoAutomaticoEnCurso = true;
        int valorSensores[] = {INST_FIN_CENSO, -1, -1, -1, -1, -1, -1, -1, -1};
        censarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
        censarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
        enviarResultadoCensoAMaestro(valorSensores);
        censoAutomaticoEnCurso = false;
      }
      break;
    }
    case INST_MANTENIMIENTO: {
      if(!evaluaAccionConjunto()){
        tiempoMantenimiento = millis();
        DEBUG_PRINT(tiempoMantenimiento);
        mantenimientoManualEnCurso = true;
        int valorSensores[] = {INST_MANTENIMIENTO, -1, -1, -1, -1, -1, -1, -1, -1};
        censarZona1(valorSensores);
        censarZona2(valorSensores);
        for (int i = 0; i < sizeof(vecMant); i++) {
          vecMant[i] = 1;
        }
        if (abs(valorSensores[1] - valorSensores[5]) > 10) {
          vecMant[0] = 0;
          //DEBUG_PRINT("E_S_T");
          //Sensor de temperatura con fallas
        }
        if (abs(valorSensores[2] - valorSensores[6]) > 10) {
          vecMant[1] = 0;
          //DEBUG_PRINT("E_S_H");
          //Sensor de humedad atmosferica con fallas
        }
        vecMant[3] = valorSensores[4];
        digitalWrite(PIN_LED1, HIGH);
        vecMant[4] = valorSensores[8];
        digitalWrite(PIN_LED2, HIGH);
      } else {
        sendMessageToBluetooth(M_MANT_MANUAL_ER);
      }
      break;
    }
    case INST_RIEGO_Z1: {
      if (!evaluaAccionEnProcesoBluetooth()) {
        riegoZona1AutomaticoEnCurso = true;
        tiempoComienzoRiegoZona1 = millis();
        tiempoComienzoIntermitencia1 = millis();
        intensidadRiegoZona1 = intesidadRiego;
        intensidadRiegoZona1 = (intensidadRiegoZona1 / 100) * POND_LUZ_INT;
        //analogWrite(PIN_BOMBA1, (intensidadRiegoZona1 * 178/100) + 76);
        analogWrite(PIN_BOMBA1, intensidadRiegoZona1);
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z1 + "," + intensidadRiegoZona1 + "," + TIEMPO_RIEGO + ">";
        Serial.println(ret);
      }
      break;
    }
    case INST_RIEGO_Z2: {
      if (!evaluaAccionEnProcesoBluetooth()) {
        riegoZona2AutomaticoEnCurso = true;
        tiempoComienzoRiegoZona2 = millis();
        tiempoComienzoIntermitencia2 = millis();
        intensidadRiegoZona2 = intesidadRiego;
        intensidadRiegoZona2 = (intensidadRiegoZona2 / 100) * POND_LUZ_INT;
        //analogWrite(PIN_BOMBA2, (intensidadRiegoZona2 * 178/100) + 76);
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2);                
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z2 + "," + intensidadRiegoZona2 + "," + TIEMPO_RIEGO + ">";
        Serial.println(ret);
      }
      break;
    }
    case INST_DETENER_RIEGO_GRAL: {
      bool fDetuvo = false;
      if(riegoZona1AutomaticoEnCurso) {
        riegoZona1AutomaticoEnCurso = false;
        tiempoComienzoRiegoZona1 = 0;
        analogWrite(PIN_BOMBA1, 0);
        tiempoDespuesRiegoZona1 = 0;
        fDetuvo = true;
      }
      if(riegoZona2AutomaticoEnCurso) {
        riegoZona2AutomaticoEnCurso = false;
        tiempoComienzoRiegoZona2 = 0;
        analogWrite(PIN_BOMBA2, 0);
        tiempoDespuesRiegoZona2 = 0;
        fDetuvo = true;
      }
      if(riegoManualEnCurso) {
        riegoManualEnCurso = false;
        tiempoComienzoRiegoManual = 0;
        analogWrite(PIN_BOMBA1, 0);
        analogWrite(PIN_BOMBA2, 0);
        fDetuvo = true;
      }
      if(fDetuvo) {
        sendMessageToBluetooth(M_STOP_RIEGO_GRAL_OK);
      } else {
        sendMessageToBluetooth(M_STOP_RIEGO_GRAL_ER);
      }
      break;
    }
    case INST_ENCENDER_LUZ_1_MANUAL: {
      prenderLuz1 = 1;
      break;
    }
    case INST_ENCENDER_LUZ_2_MANUAL: {
      prenderLuz2 = 1;
      break;
    }
    case INST_APAGAR_LUZ_1_MANUAL: {
      prenderLuz1 = 2;
      break;
    }
    case INST_APAGAR_LUZ_2_MANUAL: {
      prenderLuz2 = 2;
      break;
    }
    case INST_AUTO_LUZ_1: {
      prenderLuz1 = 0;
      break;
    }
    case INST_AUTO_LUZ_2: {
      prenderLuz2 = 0;
      break;
    }
    case INST_TIPO_RIEGO_CONT: {
      tipoRiego = 0;
      sendMessageToMaster(M_CAMBIO_T_RIEGO_CONT);
      break;
    }
    case INST_TIPO_RIEGO_INTER: {
      tipoRiego = 1;
      sendMessageToMaster(M_CAMBIO_T_RIEGO_INT);
      break;
    }
    case INST_INICIO_CONEXION_BT: {
      censoManualEnCurso = true;
      int valorSensores[] = {INST_INICIO_CONEXION_BT, -1, -1, -1, -1, -1, -1, -1, -1};
      censarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
      censarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
      String ret = "";
      ret = ret + "<" + valorSensores[0] + "," 
      + tipoRiego + "," 
      + valorSensores[1] + "," + valorSensores[2] + "," + valorSensores[3] + "," + valorSensores[4] + "," 
      + valorSensores[5] + "," + valorSensores[6] + "," + valorSensores[7] + "," + valorSensores[8] + ">";
      Serial.println(ret);
      censoManualEnCurso = false;
      break;
    }
    case INST_RIEGO_MANUAL: {
      if(!evaluaAccionConjunto()){
        riegoManualEnCurso = true;
        tiempoComienzoRiegoManual = millis();
        TIEMPO_RIEGO_MANUAL = tRiegoManual;
        intensidadRiegoZona1 = intesidadRiego; //Se utiliza para el tipo de riego del tipo intermitente
        intensidadRiegoZona2 = intesidadRiego; //Se utiliza para el tipo de riego del tipo intermitente
        intensidadRiegoZona1 = (intensidadRiegoZona1 / 100) * POND_LUZ_INT;
        intensidadRiegoZona2 = (intensidadRiegoZona2 / 100) * POND_LUZ_INT;
        analogWrite(PIN_BOMBA1, intensidadRiegoZona1);
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2);
      } else {
        sendMessageToBluetooth(M_RIEGO_MANUAL_ER);
      }
      break;
    }
    case INST_CENSO_MANUAL: {
      if (!evaluaAccionConjunto()) {
        censoManualEnCurso = true;
        int valorSensores[] = {INST_CENSO_MANUAL, -1, -1, -1, -1, -1, -1, -1, -1};
        censarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
        censarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
        String ret = "";
        ret = ret + "<" +  valorSensores[0] + "," 
        + valorSensores[1] + "," + valorSensores[2] + "," + valorSensores[3] + "," + valorSensores[4] + "," 
        + valorSensores[5] + "," + valorSensores[6] + "," + valorSensores[7] + "," + valorSensores[8] + ">";
        Serial.println(ret);
        censoManualEnCurso = false;
      } else {
        sendMessageToBluetooth(M_CENSO_MANUAL_ER);
      }
      break;
    }
    case INST_DESCONEXION_BT: {
      sendMessageToBluetooth(INST_DESCONEXION_BT);                                                                           
      break;
    }  
    default:{
      break;
    }
  }
  //DEBUG_PRINT(tiempoMantenimiento);
  tiempoActual = millis();
  if(tipoRiego == 1) {
    // Si es intermitente tengo que ver si está regando la zona y despues tengo que ver si esta dentro
    // del tiempo de intermitencia. 
    if (riegoZona1AutomaticoEnCurso || riegoManualEnCurso) {
      // prendido desde el tiempo 0 hasta el tiempo de intermitencia y apagado desde el tiempo de intermitencia hasta el
      // doble del tiempo de intermitencia. Ejemplo [0, 1) prendido [1, 2] apagado
      unsigned long tiempoDentroIntermitencia = (unsigned long)(tiempoActual - tiempoComienzoIntermitencia1);

      if (tiempoDentroIntermitencia >= TIEMPO_INTERMITENCIA && tiempoDentroIntermitencia < TIEMPO_INTERMITENCIA * 2) {
        analogWrite(PIN_BOMBA1, 0);
      }
      else if (tiempoDentroIntermitencia >= TIEMPO_INTERMITENCIA * 2) {
        analogWrite(PIN_BOMBA1, intensidadRiegoZona1);
        tiempoComienzoIntermitencia1 = millis();
      } else {
        analogWrite(PIN_BOMBA1, intensidadRiegoZona1);
      }
    }
    
    if (riegoZona2AutomaticoEnCurso || riegoManualEnCurso) {
      unsigned long tiempoDentroIntermitencia = (unsigned long)(tiempoActual - tiempoComienzoIntermitencia2);

      if (tiempoDentroIntermitencia >= TIEMPO_INTERMITENCIA && tiempoDentroIntermitencia < TIEMPO_INTERMITENCIA * 2) {
        analogWrite(PIN_BOMBA2, 0);
      }
      else if (tiempoDentroIntermitencia >= TIEMPO_INTERMITENCIA * 2) {
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2);
        tiempoComienzoIntermitencia2 = millis();
      } else {
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2);
      }
    }
  }
  
  //tiempoActual = millis();
  if(tiempoComienzoRiegoManual > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoManual) >= TIEMPO_RIEGO_MANUAL) {
    // Aviso que termino el riego manual.
    analogWrite(PIN_BOMBA1, 0);
    analogWrite(PIN_BOMBA2, 0);
    riegoManualEnCurso = false;
    tiempoComienzoRiegoManual = 0;
    TIEMPO_RIEGO_MANUAL = 0;
    sendMessageToBluetooth(INST_FIN_RIEGO_MANUAL);
  }
  
  //tiempoActual = millis();
  if (tiempoComienzoRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona1) >= TIEMPO_RIEGO) {
    // Aviso que termino de regar la zona 1.
    analogWrite(PIN_BOMBA1, 0);
    sendMessageToBluetooth(INST_FIN_RIEGO_Z1);
    sendMessageToMaster(INST_FIN_RIEGO_Z1);
    tiempoDespuesRiegoZona1 = millis();
    riegoZona1AutomaticoEnCurso = false;
    tiempoComienzoRiegoZona1 = 0;
  }

  //tiempoActual = millis();
  if (tiempoDespuesRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona1) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro el valor del sensor de humedad del suelo.
    String ret = "";
    ret = ret + "<" + INST_RES_RIEGO_Z1 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO1) + ">"; 
    serialMaster.print(ret);
    Serial.println(ret);
    tiempoDespuesRiegoZona1 = 0;
  }
  
  //tiempoActual = millis();
  if (tiempoComienzoRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona2) >= TIEMPO_RIEGO) {
    // Aviso que termino de regar la zona 2.
    analogWrite(PIN_BOMBA2, 0);
    sendMessageToBluetooth(INST_FIN_RIEGO_Z2);
    sendMessageToMaster(INST_FIN_RIEGO_Z2);
    tiempoDespuesRiegoZona2 = millis();
    riegoZona2AutomaticoEnCurso = false;
    tiempoComienzoRiegoZona2 = 0;
  }

  //tiempoActual = millis();
  if (tiempoDespuesRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona2) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro y dispositivo el valor del sensor de humedad del suelo.
    String ret = "";
    ret = ret + "<" + INST_RES_RIEGO_Z2 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO2) + ">";
    serialMaster.print(ret);
    Serial.println(ret);
    tiempoDespuesRiegoZona2 = 0;
  }

  if(!mantenimientoManualEnCurso){
    if (prenderLuz1 == 1 || (prenderLuz1 == 0 && analogRead(PIN_SENSOR_LUZ1) > VALOR_MAX_LUZ)) {
      digitalWrite(PIN_LED1, HIGH);
    } else {
      if (prenderLuz1 == 0 && analogRead(PIN_SENSOR_LUZ1) < VALOR_MIN_LUZ) {
        digitalWrite(PIN_LED1, LOW);
      }
    }

    if (prenderLuz2 == 1 || (prenderLuz2 == 0 && analogRead(PIN_SENSOR_LUZ2) > VALOR_MAX_LUZ)) {
      digitalWrite(PIN_LED2, HIGH);
    } else {
      if (prenderLuz2 == 0 && analogRead(PIN_SENSOR_LUZ2) < VALOR_MIN_LUZ) {
        digitalWrite(PIN_LED2, LOW);
      }
    }
  }

 
  //tiempoActual = millis();
  //Serial.println(tiempoMantenimiento);
  if (tiempoMantenimiento > 0 && (unsigned long)(tiempoActual - tiempoMantenimiento) >= TIEMPO_RES_MANTENIMIENTO) {
    //Censa la luz para ver como varia y envia los resultados al esclavo
    int valorLuzActualZona = analogRead(PIN_SENSOR_LUZ1);
    if(vecMant[3] >= valorLuzActualZona) {
      vecMant[3] = 0;
      DEBUG_PRINT("E_L_1");
      //Se encendio la luz de la zona 1 y el sensor LDR1 no lo detecto
    } else {
      vecMant[3] = 1;
    }
    digitalWrite(PIN_LED1, LOW);
    valorLuzActualZona = analogRead(PIN_SENSOR_LUZ2);
    if(vecMant[4] >= valorLuzActualZona) {
      vecMant[4] = 0;
      DEBUG_PRINT("E_L_2");
      //Se encendio la luz de la zona 2 y el sensor LDR2 no lo detecto
    } else {
      vecMant[4] = 1;
    }
    digitalWrite(PIN_LED2, LOW);
    //Se envia <instruccion, errorTemp, errorHumAmb, errorHumSuelo, errorLDR1, errorLDR2> 
    String ret = "";
    ret = ret + "<" + INST_RES_MANTENIMIENTO + "," + vecMant[0] + "," + vecMant[1] + "," + vecMant[2] + "," + vecMant[3] + "," + vecMant[4] + ">";
    Serial.println(ret);
    tiempoMantenimiento = 0;
    mantenimientoManualEnCurso = false;
  }
}

void censarZona1(int* vec) {
  vec[1] = sensorDHT1.readTemperature();
  vec[2] = sensorDHT1.readHumidity();
  vec[3] = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  vec[4] = analogRead(PIN_SENSOR_LUZ1);
}

void censarZona2(int* vec) {
  vec[5] = sensorDHT2.readTemperature();
  vec[6] = sensorDHT2.readHumidity();
  vec[7] = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  vec[8] = analogRead(PIN_SENSOR_LUZ2);
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
}

void leerMaestro(int* inst, float* intesidad) {
  byte charIndex = 0;
  char charLeido;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  
  if (serialMaster.available() > 0) {
    for (int i = 0; i < 60; i++) {
      entrada[i] = '\0';
    }
    serialMaster.readBytesUntil('>', entrada, 59);
    int i = 0;
    while (entrada[i] != '\0' && i < 59) {
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
        } else if (fieldIndex == 1) {
          *intesidad = atof(input);
        }
        fieldIndex++;
      }
      i++;
    }

    input[charIndex] = '\0';
    if (fieldIndex == 0){
      *inst = atoi(input);
    } else if (fieldIndex == 1) {
      *intesidad = atof(input);
    }
  }
}

void leerBluetooth(int* inst, float* intesidad, int* tiempo) {
  char entrada[60];
  if (Serial.available() > 0) {
    for (int i = 0; i < 60; i++) {
      entrada[i] = '\0';
    }

    Serial.readBytesUntil('>', entrada, 59);
    int charIndex = 0;
    char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
    int fieldIndex = 0;
    int i = 0;
    
    while (entrada[i] != '\0' && i < 59) {
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
        } else if (fieldIndex == 1) {
          *intesidad = atof(input);
        } else if (fieldIndex == 2) {
          *tiempo = atoi(input);
        }
        fieldIndex++;
      }
      i++;
    }

    input[charIndex] = '\0';
    if (fieldIndex == 0) {
      *inst = atoi(input);
    } else if (fieldIndex == 1) {
      *intesidad = atof(input);
    } else if (fieldIndex == 2) {
      *tiempo = atoi(input);
    }
    DEBUG_PRINT(entrada);
  }
}

bool evaluaAccionConjunto(){
  return evaluaAccionEnProcesoBluetooth() || evaluaAccionEnProcesoMaestro();
}

bool evaluaAccionEnProcesoBluetooth() {
  return riegoManualEnCurso || censoManualEnCurso || mantenimientoManualEnCurso;
}

bool evaluaAccionEnProcesoMaestro() {
  return riegoZona1AutomaticoEnCurso || riegoZona2AutomaticoEnCurso || censoAutomaticoEnCurso || mantenimientoAutomaticoEnCurso;
}

void sendMessageToMaster(const int nMsj){
  String msj = "";
  msj = msj + "<" + nMsj + ">";
  serialMaster.print(msj);
}

void sendMessageToBluetooth(const int nMsj){
  String msj = "";
  msj = msj + "<" + nMsj + ">";
  Serial.println(msj);
}
