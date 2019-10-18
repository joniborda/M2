#include <SoftwareSerial.h>
#include <SD.h>

// puerto hacia el esclavo
const int PUERTO_RX_SLAVE = 2;
const int PUERTO_TX_SLAVE = 3;
const int PIN_CS_SD = 10;

// instrucciones que envio al esclavo
const int INST_SENSOR = 1; // instruccion para censar
const int INST_IRRIGATE = 2; // instruccion para regar

// milisegundos para comenzar censor
static unsigned long MS_INTERVAL_TO_SENSOR = 1000; // 15 segundos

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

File filePointer;
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
static const float PER_TEMP = 0.2;
static const float PER_HUM_AMB = 0.2;
static const float PER_HUM_SUE = 0.4;
static const float PER_LUZ = 0.2;
static const int MAX_TEMP = 40;
static const int MAX_HUMEDAD = 100;
static const int MAX_LUZ = 100;

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.print("iniciado");

  /*if (!SD.begin(PIN_CS_SD)) {
    Serial.println("No se pudo inicializar la SD");
    return;
  }*/
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_SENSOR) {
    Serial.println("envio orden a esclavo");
    serialSlave.write(INST_SENSOR);
    previousMillis = millis();
    MS_INTERVAL_TO_SENSOR = 3000;
  }

  leerEsclavo();
  //guardarEnArchivo();

}

void leerEsclavo() {
    static boolean recvinprogress = false; // se mantiene estatico porque si no llego al caracter de corte tiene que seguir leyendo la cardena
    static byte charIndex = 0; // es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo
    static const char START_MARKER = '<';
    static const char COMMA = ',';
    static const char END_MARKER = '>';
    static char charLeido;
    static char input[4]; // el dato que este entre comas no puede ser mayor a 4
    static int fieldIndex = 0;
    static int instructionCode = 0;
    static int errorCode = 0;
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
          String a = "";
          a = a +"c " + charLeido + " fieldIndex " + fieldIndex + " ";
          Serial.println(a);
          charIndex++;  
        } else {
          input[charIndex] = '\0';
          charIndex = 0;
          switch (fieldIndex) {
            case 0:
              instructionCode = atoi(input);
              break;
            case 1:
              errorCode = atoi(input);
              break;
            case 2:
              temperatura1 = atoi(input);
              break;
            case 3:
              humedadAmbiente1 = atoi(input);
              break;
            case 4:
              humedadSuelo1 = atoi(input);
              break;
            case 5:
              luz1 = atoi(input);
              break;
            case 6:
              temperatura2 = atoi(input);
              break;
            case 7:
              humedadAmbiente2 = atoi(input);
              break;
            case 8:
              humedadSuelo2 = atoi(input);
              break;
            case 9:
              luz2 = atoi(input);
              break;
          }
          fieldIndex++;
        }
      }
  }

  if (charLeido == END_MARKER) {
    recvinprogress = false;
    // el ultimo no tiene coma
    input[charIndex] = '\0';
    luz2 = atoi(input);

    if (errorCode > 0) {
      Serial.println("Recibo un error");
    }

    String ret = "";
    ret = ret + "temp1 " + temperatura1 + ", humedadAmbiente1 " + humedadAmbiente1 + ", humedadSuelo1 " + humedadSuelo1 + ", luz1 " + luz1 + ", efectividad1 " + calcularEfectividad1() + 
                ", temp2 " + temperatura2 + ", humedadAmbiente2 " + humedadAmbiente2 + ", humedadSuelo2 " + humedadSuelo2 + ", luz2 " + luz2 + ", efectividad2 " + calcularEfectividad2();
    Serial.println(ret);

    errorCode = -1;
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

void guardarEnArchivo() {
  filePointer = SD.open("archivo.txt", FILE_WRITE);
  if (filePointer) {
    String ret = "";
    ret = ret + temperatura1 + "," + humedadAmbiente1 + "," + humedadSuelo1 + "," + luz1 + "," + calcularEfectividad1() + "," +
                 temperatura2 + "," + humedadAmbiente2 + humedadSuelo2 + "," + "," + luz2 + "," + calcularEfectividad2();
    Serial.print("archivo.txt: ");
    
    filePointer.print(ret);
    
    filePointer.close(); //cerramos el archivo
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
