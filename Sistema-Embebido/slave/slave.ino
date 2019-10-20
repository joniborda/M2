#include <DHT_U.h>
#include <DHT.h>

#include <SoftwareSerial.h>

#define TAM_MAX 9
const int INST_CENSO = 1; // INSTRUCCION PARA RUTINA DE CENSO (INICIO/FIN)
const int INST_RIEGO_Z1 = 2; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 (INICIO/FIN)
const int INST_RIEGO_Z2 = 3; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 (INICIO/FIN)
const int INST_MANTENIMIENTO = 4; // INSTRUCCION PARA RUTINA DE MANTENIMIENTO (INICIO/FIN)
const int INST_RES_RIEGO_Z1 = 12; //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
const int INST_RES_RIEGO_Z2 = 13; //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2

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
const unsigned long TIEMPO_RES_RIEGO = 3000;

SoftwareSerial serialMaster(PUERTO_RX_MASTER, PUERTO_TX_MASTER);

DHT sensorDHT1 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE1, DHT11);
DHT sensorDHT2 = DHT(PIN_SENSOR_HUMEDAD_AMBIENTE2, DHT11);

unsigned long tiempoActual = millis();
unsigned long tiempoComienzoRiegoZona1 = 0;
unsigned long tiempoDespuesRiegoZona1 = 0;
unsigned long tiempoComienzoRiegoZona2 = 0;
unsigned long tiempoDespuesRiegoZona2 = 0;
int tiempoRiegoZona1 = 0; //A que corresponde este tiempo??? No deberia ser fijo para ambas zonas
int tiempoRiegoZona2 = 0;
int intensidadRiegoZona1 = 50;
int intensidadRiegoZona2 = 50;

void setup() {
  serialMaster.begin(9600); //Velocidad comunicacion maestro
  Serial.begin(9600); //Velocidad comunicacion monitor serial
  sensorDHT1.begin();
  sensorDHT2.begin();
  Serial.print("Arduino Esclavo inciado, esperando instrucciones...");
}

void loop() {
  if (serialMaster.available() > 0 ) {
    // Recibiendo informacion del maestro
    Serial.println("Se recibio una instruccion del maestro.");
    char *stringRecibido;
    int valoresRecibidos[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    
    leerMaestro(valoresRecibidos);
    
    switch (valoresRecibidos[0]) {
      case INST_CENSO: {
        Serial.println("COMIENZA RUTINA DE CENSO.");
        int valorSensores[] = {INST_CENSO, -1, -1, -1, -1, -1, -1, -1, -1};
        sensarZona1(valorSensores); //Obtiene los valores de los sensores de la zona 1 
        sensarZona2(valorSensores); //Obtiene los valores de los sensores de la zona 2
        enviarResultadoCensoAMaestro(valorSensores);
        break;
      }
      case INST_MANTENIMIENTO: {
        Serial.println("COMIENZA RUTINA DE MANTENIMIENTO.");
        mantenimiento();
        //Faltaria cargar el vector con los valores y enviarlo al maestro. Determinar si usamos la funcion enviarResultadoCensoAMaestro o una nueva.
        break;
      }
      case INST_RIEGO_Z1: {
        Serial.println("COMIENZA RIEGO ZONA 1.");
        tiempoComienzoRiegoZona1 = millis();
        // leer el tiempo que tengo que regar y guardarlo en una variable
        tiempoRiegoZona1 = 3000;
        // leer la intesidad que manda para regar
        intensidadRiegoZona1 = 50;
        analogWrite(PIN_BOMBA1, intensidadRiegoZona1 * 255/100);
        break;
      }
      case INST_RIEGO_Z2: {
        Serial.println("COMIENZA RIEGO ZONA 2.");
        tiempoComienzoRiegoZona2 = millis();
        // leer el tiempo que tengo que regar y guardarlo en una variable
        tiempoRiegoZona2 = 3000;
        // leer la intesidad que manda para regar 
        intensidadRiegoZona2 = 50;
        analogWrite(PIN_BOMBA2, intensidadRiegoZona2 * 255/100);
        break;
      }
      default:{
        Serial.println("No se encontro rutina para ese valor.");
        break;
      }
    }
  }
  
  tiempoActual = millis();
  if (tiempoComienzoRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona1) >= tiempoRiegoZona1) {
    // Aviso que termino de regar la zona 1.
    analogWrite(PIN_BOMBA1, 0);
    String ret = "";
    //ret = ret + "<" + INST_RIEGO_Z1 + ",0>"; Entiendo que no es necesario manderle el 0
    ret = ret + "<" + INST_RIEGO_Z1 + ">";
    serialMaster.print(ret);// <inst, ....>
    tiempoDespuesRiegoZona1 = millis();
    tiempoComienzoRiegoZona1 = 0;
  }

  tiempoActual = millis();
  if (tiempoDespuesRiegoZona1 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona1) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro el valor del sensor de humedad del suelo.
    String ret = "";
    // ret = ret + "<" + INST_RES_RIEGO_Z1 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO1) + ",0,0,0,0>"; //No es necesario mandarle todos los ceros del fondo, el maestro va a leer hasta que encuentre el endMark
    ret = ret + "<" + INST_RES_RIEGO_Z1 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO1) + ">"; 
    serialMaster.print(ret);
    tiempoDespuesRiegoZona1 = 0;
  }
  
  tiempoActual = millis();
  if (tiempoComienzoRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoComienzoRiegoZona2) >= tiempoRiegoZona2) {
    // Aviso que termino de regar la zona 2.
    analogWrite(PIN_BOMBA2, 0);
    String ret = "";
    ret = ret + "<" + INST_RIEGO_Z2 + ">";
    serialMaster.print(ret);// <inst, ....>
    tiempoDespuesRiegoZona2 = millis();
    tiempoComienzoRiegoZona2 = 0;
  }

  tiempoActual = millis();
  if (tiempoDespuesRiegoZona2 > 0 && (unsigned long)(tiempoActual - tiempoDespuesRiegoZona2) >= TIEMPO_RES_RIEGO) {
    // Paso el tiempo establecido posterior al riego, se envia al maestro el valor del sensor de humedad del suelo.
    String ret = "";
    ret = ret + "<" + INST_RES_RIEGO_Z2 + "," + analogRead(PIN_SENSOR_HUMEDAD_SUELO2) + ">";
    serialMaster.print(ret);
    tiempoDespuesRiegoZona2 = 0;
  }
}

void sensarZona1(int* vec) {
  vec[2] = sensorDHT1.readTemperature();
  vec[3] = sensorDHT1.readHumidity();
  vec[4] = analogRead(PIN_SENSOR_HUMEDAD_SUELO1);
  vec[5] = analogRead(PIN_SENSOR_LUZ1);
  String ret = "";
  ret = ret + "Temp1: " + vec[1] + ", HumAmb1: " + vec[2] + ", HumSu1: " + vec[3] + ", SenLuz1: " + vec[4];
  Serial.println(ret);
}

void sensarZona2(int* vec) {
  vec[6] = sensorDHT2.readTemperature();
  vec[7] = sensorDHT2.readHumidity();
  vec[8] = analogRead(PIN_SENSOR_HUMEDAD_SUELO2);
  vec[9] = analogRead(PIN_SENSOR_LUZ2);
  String ret = "";
  ret = ret + "Temp2: " + vec[5] + ", HumAmb2: " + vec[6] + ", HumSu2: " + vec[7] + ", SenLuz2: " + vec[8];
  Serial.println(ret);
}

void mantenimiento() {
  int valorSensores[] = {INST_MANTENIMIENTO, -1, -1, -1, -1, -1, -1, -1, -1};
  //
  /*Para el envio del mantenimiento, se envia con el siguiente formato:
  *
  * */
  sensarZona1(valorSensores);
  sensarZona2(valorSensores);

  if (abs(valorSensores[1] - valorSensores[5]) > 10) {
    Serial.println("Sensor de temperatura con fallas.");
  }

  if (abs(valorSensores[2] - valorSensores[6]) > 10) {
    Serial.println("Sensor de humedad atmosferica con fallas.");
  }

  //DEBERIA SER DE NOCHE PORQUE SI NO NUNCA LO VA A DETECTAR
  int valorLuzAnteriorZona1 = valorSensores[4];
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
  * <instruccion, temperatura1, humedadAmbiente1, humedadSuelo1, sensorLuz1, temperatura2, humedadAmbiente2, humedadSuelo2, sensorLuz2>
  * La instruccion es la que inicio la rutina
  */
  String ret = "";
  ret = ret + "<" + INST_CENSO + "," + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + ">";
  serialMaster.print(ret);
  Serial.println(ret);
}

void leerMaestro(int* vec) {
    static boolean recvinprogress = false; // Se mantiene estatico porque si no llego al caracter de corte tiene que seguir leyendo la cadena.
    static byte charIndex = 0; // Es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo.
    static char charLeido;
    static char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
    static int fieldIndex = 0;
    if (serialMaster.available() <= 0) {
      return;  
    } else {
      Serial.println("Informacion disponible del maestro.");
    }
    while(serialMaster.available() > 0) {
      charLeido = (char)serialMaster.read();
      if (charLeido == '<') {
        recvinprogress = true;
      } else if (charLeido == '>') {
        recvinprogress = false;
      }
      
      if(recvinprogress == true && charLeido != '<') {
        if (charLeido != ',') {
          input[charIndex] = charLeido;
          charIndex++;  
        } else {
          input[charIndex] = '\0';
          charIndex = 0;
          vec[fieldIndex] = atoi(input);
          fieldIndex++;
        }
      }
  }

  if (charLeido == '>') {
    recvinprogress = false;
    // el ultimo no tiene coma
    input[charIndex] = '\0';
    vec[fieldIndex] = atoi(input);
    charIndex = 0;
    fieldIndex = 0;
  }
}
