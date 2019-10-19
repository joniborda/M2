#include <SoftwareSerial.h>
#include <SD.h>

#define TAM_MAX 9

// PUERTOS DE CONEXION CON ESCLAVO
const int PUERTO_RX_SLAVE = 2;
const int PUERTO_TX_SLAVE = 3;

const int PIN_CS_SD = 10;

const int INST_CENSO = 1; // INSTRUCCION PARA RUTINA DE CENSO
const int INST_RIEGO_Z1 = 2; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1
const int INST_RIEGO_Z2 = 3; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2
const int INST_MANTENIMIENTO = 4; // INSTRUCCION PARA RUTINA DE MANTENIMIENTO

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned long MS_INTERVAL_TO_CENSO = 1000; // 15 seg.

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

File filePointer;
unsigned long currentMillis = millis(); // grab current time
unsigned long previousMillis = 0;  // millis() returns an unsigned long.

//Deberia usarlas en un entorno local a la funcion
int temperatura1 = 0;
int temperatura2 = 0;
int humedadAmbiente1 = 0;
int humedadAmbiente2 = 0;
int humedadSuelo1 = 0;
int humedadSuelo2 = 0;
int luz1 = 0;
int luz2 = 0;

//Podriamos poner directamente los valores para no comer espacio, revisar si es necesario
static const float PER_TEMP = 0.2;
static const float PER_HUM_AMB = 0.2;
static const float PER_HUM_SUE = 0.4;
static const float PER_LUZ = 0.2;
static const int MAX_TEMP = 40;
static const int MAX_HUMEDAD = 100;
static const int MAX_LUZ = 100;

int[30][9] censos;

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.print("Arduino Maestro iniciado...");

  /*if (!SD.begin(PIN_CS_SD)) {
    Serial.println("No se pudo inicializar la SD");
    return;
  }*/
}

void loop() {
  currentMillis = millis();
  
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_CENSO) {
    Serial.println("Envio instruccion de censo a el esclavo.");
    serialSlave.write(INST_CENSO);
    previousMillis = millis();
    MS_INTERVAL_TO_CENSO = 3000;
  }
  int[TAM_MAX] valoresRecibidos = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerEsclavo(valoresRecibidos);
  if(valoresRecibidos[0] != -1) {
    switch(valoresRecibidos[0]){
      case INST_CENSO: {
        int perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
        int perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);      
        guardarEnArchivo(valoresRecibidos,perEfectividadZ1,perEfectividadZ2);      
        if(determinarRiegoEnZona1()){
          int vol1 = obtenerVolumenRiegoZona1();
          serialSlave.write(INST_RIEGO_Z1); //CAMBIAR POR <INST, 0000>
          serialSlave.write(vol1);
        }

        if(determinarRiegoEnZona2()){
          int vol2 = obtenerVolumenRiegoZona2();
          serialSlave.write(INST_RIEGO_Z2); //CAMBIAR POR <INST, 0000>
          serialSlave.write(vol2);
        }
        break;
      }
      case INST_MANTENIMIENTO: {
        //ANALIZAR ERRORES E INFORMAR
        break;
      }
      case INST_RIEGO_Z1: {
        break;
      }
      case INST_RIEGO_Z2: {
        break;
      }
    }
  }
}

void leerEsclavo(int* vec) {
    static boolean recvinprogress = false; // Se mantiene estatico porque si no llego al caracter de corte tiene que seguir leyendo la cadena
    static byte charIndex = 0; // es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo
    static const char START_MARKER = '<';
    static const char COMMA = ',';
    static const char END_MARKER = '>';
    static char charLeido;
    static char input[4]; // el dato que este entre comas no puede ser mayor a 4
    static int fieldIndex = 0;
    static int instructionCode = 0;
    if (serialSlave.available() <= 0) {
      return;  
    } else {
      Serial.println("leyendo");
    }
    while(serialSlave.available() > 0) {
      charLeido = (char)serialSlave.read();
      if (charLeido == START_MARKER) {
        recvinprogress = true;
      } else if (charLeido == END_MARKER) {
        recvinprogress = false;
      }
      
      if(recvinprogress == true && charLeido != START_MARKER) {
        if (charLeido != COMMA) {
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

  if (charLeido == END_MARKER) {
    recvinprogress = false;
    // el ultimo no tiene coma
    input[charIndex] = '\0';
    vec[fieldIndex] = atoi(input);

    String ret = "";
    ret = ret + "temp1 " + temperatura1 + ", humedadAmbiente1 " + humedadAmbiente1 + ", humedadSuelo1 " + humedadSuelo1 + ", luz1 " + luz1 + ", efectividad1 " + calcularEfectividad1() + 
                ", temp2 " + temperatura2 + ", humedadAmbiente2 " + humedadAmbiente2 + ", humedadSuelo2 " + humedadSuelo2 + ", luz2 " + luz2 + ", efectividad2 " + calcularEfectividad2();
    Serial.println(ret);

    instructionCode = -1;
    charIndex = 0;
    fieldIndex = 0;
  }
}

void leerArchivo() {
  filePointer = SD.open("archivo.txt");
  static char caracter;
  static char input[4];
  static int i = 0;
  static int finPalabra = 0;

  // if the file is available, write to it:
  if (filePointer) {
    while (filePointer.available()) {
      caracter = filePointer.read();
      if (caracter == -1) { // -1 indica fin de archivo
        finPalabra = 1;
      }
      if (caracter == '\n') {
        finPalabra = 1;
      }

      if (finPalabra == 0) {
          input[i++] = caracter;
      } else {
        input[i] = '\0';
        i = 0;
        //Serial.print(atoi(input)); // solo imprimo la lectura
        //Serial.println(" ");
        finPalabra = 0;
      }
    }

    filePointer.close();
  } else {
    Serial.println("error opening archivo.txt");
  }
  i = 0; // es estatico así que tengo que resetearlo
  finPalabra = 0;
  input[0] = '\0';
}

void guardarEnArchivo(int *vec) {
  //filePointer = SD.open("archivo.txt", FILE_WRITE);
  if (filePointer) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + calcularEfectividad1() + "," +
                vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + calcularEfectividad2();
    Serial.print("archivo.txt: ");
    
    //filePointer.print(ret);
    
    //filePointer.close(); //cerramos el archivo
  } else {
    Serial.println("Error al abrir el archivo");
  }
}

int calcularEfectividad(float temp, float amb, float suelo, float luz) {
  return PER_TEMP * abs(MAX_TEMP - temp) - PER_HUM_AMB * abs(MAX_HUMEDAD - amb) - PER_HUM_SUE * abs(MAX_HUMEDAD - suelo) - PER_LUZ * abs(MAX_LUZ - luz);
}

int calcularEfectividad1() {
  return calcularEfectividad(temperatura1, humedadAmbiente1, humedadSuelo1, luz1);
}

int calcularEfectividad2() {
  return calcularEfectividad(temperatura2, humedadAmbiente2, humedadSuelo2, luz2);
}
