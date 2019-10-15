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
const unsigned long INTERVAL_TO_SENSOR = 100*60; 

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

File myFile;
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

  /*currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= INTERVAL_TO_SENSOR) {
 	// to do anything
  
    previousMillis = millis();
  }*/
  delay(4000);
  Serial.println("envio orden a esclavo");
  serialSlave.write(INST_SENSOR);

  delay(4000);
  Serial.println("leerEsclavo");
  leerEsclavo();

  verificarRiego();
}

void leerEsclavo() {
  //if (serialSlave.available() > 0 ) {
  	// leyendo lo que me envia el esclavo
    static boolean recvinprogress=false;
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
          if (index == 0) {
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
  
  Serial.print("temperatura1 ");
  Serial.print(temperatura1);
  Serial.print(" humedadAmbiente1 ");
  Serial.print(humedadAmbiente1);
  Serial.print(" humedadSuelo1 ");
  Serial.print(humedadSuelo1);
  Serial.print(" luz1 ");
  Serial.print(luz1);
  Serial.print("temperatura2 ");
  Serial.print(temperatura2);
  Serial.print(" humedadAmbiente2 ");
  Serial.print(humedadAmbiente2);
  Serial.print(" humedadSuelo2 ");
  Serial.print(humedadSuelo2);
  Serial.print(" luz2 ");
  Serial.println(luz2);

  myFile = SD.open("archivo.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("archivo.txt: ");
    myFile.print(temperatura1);
    myFile.print(",");
    myFile.print(humedadAmbiente1);
    myFile.print(",");
    myFile.print(humedadSuelo1);
    myFile.print(",");
    myFile.print(luz1);
    myFile.print(",");
    myFile.print(temperatura2);
    myFile.print(",");
    myFile.print(humedadAmbiente2);
    myFile.print(",");
    myFile.print(humedadSuelo2);
    myFile.print(",");
    myFile.println(luz2);
    
    myFile.close(); //cerramos el archivo
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
