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
const unsigned long MS_INTERVAL_TO_SENSOR = 4000; // 4 segundos

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

File filePointer;
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
static const float PER_TEMP = 0.2;
static const float PER_HUM_AMB = 0.2;
static const float PER_HUM_SUE = 0.4;
static const float PER_LUZ = 0.2;
static const int MAX_TEMP = 40;
static const int MAX_HUMEDAD = 100;
static const int MAX_LUZ = 100;

boolean newdata = false;
char recvchars[32];

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);

  if (!SD.begin(PIN_CS_SD)) {
    Serial.println("No se pudo inicializar la SD");
    return;
  }
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_SENSOR) {
    Serial.println("envio orden a esclavo");
    serialSlave.write(INST_SENSOR);
    previousMillis = millis();
  }

  Serial.println("leerEsclavo");
  leerEsclavo();
  guardarEnArchivo();

  verificarRiego();
}

void leerEsclavo() {
    static boolean recvinprogress = false;
    static byte ndx = 0;
    static const char startmarker = '<';
    static const char comma = ',';
    static const char endmarker = '>';
    static char charLeido;
    static char input[4];
    int index = 0;
  
    while(serialSlave.available() > 0) {
      charLeido = (char)serialSlave.read();
      if (charLeido == startmarker) {
        recvinprogress = true;
      } else if (charLeido == endmarker) {
        recvinprogress = false;
      }
      
      if(recvinprogress == true && charLeido != startmarker && charLeido != endmarker) {
        if (charLeido != comma) {
          input[ndx] = charLeido;
          Serial.print("c ");
          Serial.println(charLeido);
          ndx++;  
        } else {
          input[ndx] = '\0';
          ndx = 0;
          if (index == 0) { // cambiar por un switch
            temperatura1 = atoi(input);
          } else if (index == 1) {
            humedadAmbiente1 = atoi(input);
          } else if (index == 2) {
            humedadSuelo1 = atoi(input);
          } else if (index == 3) {
            luz1 = atoi(input);
          } else if (index == 4) {
            temperatura2 = atoi(input);
          } else if (index == 5) {
            humedadAmbiente2 = atoi(input);
          } else if (index == 6) {
            humedadSuelo2 = atoi(input);
          } else if (index == 7) {
            luz2 = atoi(input);
          }
          index++;
        }
      }
  }
  // el ultimo no tiene coma
  input[ndx] = '\0';
  ndx = 0;
  luz2 = atoi(input);

  String ret = "";
  ret = ret + temperatura1 + "," + humedadAmbiente1 + "," + luz1 + "," + calcularEfectividad1() + ',';
  ret = ret + temperatura2 + "," + humedadAmbiente2 + "," + luz2 + "," + calcularEfectividad2();
  Serial.println(ret);
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
        Serial.print(atoi(input)); // solo imprimo la lectura
        Serial.println(" ");
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
    String ret = ""; // si no tiene un valor nulo concatena con basura
    ret = ret + temperatura1 + "," + humedadAmbiente1 + "," + luz1 + "," + calcularEfectividad1() + ',';
    ret = ret + temperatura2 + "," + humedadAmbiente2 + "," + luz2 + "," + calcularEfectividad2();
    Serial.print("archivo.txt: ");
    
    filePointer.print(ret);
    
    filePointer.close(); //cerramos el archivo
  } else {
    Serial.println("Error al abrir el archivo");
  }
}

void verificarRiego() {
  if (toIrrigate) {
    // TODO: Verificar que si envio regar haya un tiempo de diferencia con el censo para que no se pisen
    serialSlave.write(INST_IRRIGATE);
    toIrrigate = false;
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
