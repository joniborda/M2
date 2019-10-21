#include <SD.h>

#include <SoftwareSerial.h>

#define TAM_MAX_READ 9
#define TAM_MAX_WRITE 2

// PUERTOS DE CONEXION CON ESCLAVO
#define PUERTO_RX_SLAVE 2
#define PUERTO_TX_SLAVE 3

#define PIN_CS_SD 4

#define INST_CENSO 1 // INSTRUCCION PARA RUTINA DE CENSO (INICIO/FIN)
#define INST_RIEGO_Z1 2 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 (INICIO/FIN)
#define INST_RIEGO_Z2 3 //INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 (INICIO/FIN)
#define INST_MANTENIMIENTO 4 // INSTRUCCION PARA RUTINA DE MANTENIMIENTO (INICIO/FIN)
#define INST_RES_RIEGO_Z1 12 //INSTRUCCION PARA LEER EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2 13 //INSTRUCCION PARA LEER EL RESULTADO DEL RIEGO EN LA ZONA 2

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned long MS_INTERVAL_TO_CENSO = 10000; // 15 seg.

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

unsigned long currentMillis = 0; // grab current time
unsigned long previousMillis = 0;  // millis() returns an unsigned long.

#define PRIORIDAD_TEMP 0.1
#define PRIORIDAD_HUM_AMB 0.2
#define PRIORIDAD_HUM_SUELO 0.3
#define PRIORIDAD_LUZ 0.4
#define MAX_TEMP 50
#define MAX_HUMEDAD_SUELO 1023
#define MAX_LUZ 100
#define MAX_HUMEDAD 100

char riegoEnCursoZona1 = 'F';
char riegoEnCursoZona2 = 'F';

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.println("Arduino Maestro iniciado...");

  if (!SD.begin(4)) {
    Serial.println("No se pudo inicializar la SD");
    return;
  }

  inicializarArchivosDeCensos();
}

void loop() {
  currentMillis = millis();
  //Serial.println((unsigned long)(currentMillis - previousMillis));
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_CENSO) {
    
    Serial.println("Envio instruccion de censo a el esclavo.");
    String ret = "";
    ret = ret + '<' + INST_CENSO + '>';
    serialSlave.print(ret);
    previousMillis = millis();
    MS_INTERVAL_TO_CENSO = (unsigned long)10000;
  }
  
  int valoresRecibidos[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerEsclavo(valoresRecibidos);
  if(valoresRecibidos[0] != -1) {
    switch(valoresRecibidos[0]){
      case INST_CENSO: {
        int perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
        int perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);      
        guardarEnArchivo(valoresRecibidos,perEfectividadZ1,perEfectividadZ2);
        
        if(determinarRiegoEnZona1()){ //Implementar
          Serial.println("entra en zona");
          float varZona1 = obtenerVariableRiego("var1.txt");
          float vol1 = calcularVolumenRiego(valoresRecibidos[2], varZona1);
          String ret = "";
          ret = ret + "<" + INST_RIEGO_Z1 + "," + vol1 + ">";
          serialSlave.print(ret);
          riegoEnCursoZona1 = 'T';
        }

        if(determinarRiegoEnZona2()){ //Implementar
          Serial.println("entra en as");
          float varZona2 = obtenerVariableRiego("var2.txt");
          float vol2 = calcularVolumenRiego(valoresRecibidos[6], varZona2);
          String ret = "";
          ret = ret + "<" + INST_RIEGO_Z2 + "," + vol2 + ">";
          serialSlave.print(ret);
          riegoEnCursoZona2 = 'T';
        }
        break;
      }
      case INST_MANTENIMIENTO: {
        //ANALIZAR ERRORES E INFORMAR
        break;
      }
      case INST_RIEGO_Z1: {
        riegoEnCursoZona1 = 'F';
        break;
      }
      case INST_RIEGO_Z2: {
        riegoEnCursoZona2 = 'F';
        break;
      }
      case INST_RES_RIEGO_Z1: {
        //Aca se analiza el resultado del riego de la zona 1.
        int var1 = 0;
        int humedadSueloZona1 = valoresRecibidos[2];
        int perHumedadSueloZona1 = humedadSueloZona1 / 1023;
        if(perHumedadSueloZona1 < 40 || perHumedadSueloZona1 > 60){
          var1 = obtenerVariableRiego("var1.txt");
          if(perHumedadSueloZona1 <= 40){
            var1 = var1 + (1/perHumedadSueloZona1);
          }
          else if(perHumedadSueloZona1 >= 60){
            var1 = var1 - (1/perHumedadSueloZona1);
          }
          escribirVariableRiego(var1, "var1.txt");
        } else {
          Serial.println("El caudal de agua es correcto para la zona 1.");
        }
        break;
        //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
      }
      case INST_RES_RIEGO_Z2: {
        //Aca se analiza el resultado del riego de la zona 2.
        int var2 = 0;
        int humedadSueloZona2 = valoresRecibidos[6];
        int perHumedadSueloZona2 = humedadSueloZona2 / 1023;
        if(perHumedadSueloZona2 < 40 || perHumedadSueloZona2 > 60){
          var2 = obtenerVariableRiego("var2.txt");
          if(perHumedadSueloZona2 <= 40){
            var2 = var2 + (1/perHumedadSueloZona2);
          }
          else if(perHumedadSueloZona2 >= 60){
            var2 = var2 - (1/perHumedadSueloZona2);
          }
          escribirVariableRiego(var2, "var2.txt");
        } else {
          Serial.println("El caudal de agua es correcto para la zona 2.");
        }
        break;
        //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
      }
    }
  }
}

void leerEsclavo(int* vec) {
  byte charIndex = 0; // Es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo.
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }
  if (serialSlave.available() > 0) {
    serialSlave.readBytesUntil('>', entrada, 59);
    Serial.println(entrada);
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
        vec[fieldIndex] = atoi(input);
        fieldIndex++;
      }
      i++;
    }
    input[charIndex] = '\0';
    vec[fieldIndex] = atoi(input);
    String ret = "";
    ret = ret + "t1: " + vec[2] + ", ha1: " + vec[2] + ", hs1: " + vec[3] + ", l1: " + vec[4];
                
    Serial.print(ret);
    ret = "";
    ret = ret + ", t2: " + vec[5] + ", ha2: " + vec[6] + ", hs2: " + vec[7] + ", l2: " + vec[8];
    Serial.println(ret);
    
  }
}

float obtenerVariableRiego(char* archivo) {
  File fp = SD.open(archivo);
  char caracter;
  float ret = -1;
  char input[5];

  if (fp) {
    if (fp.available()) {
      fp.read(input, sizeof(input));
      ret = atof(input);
    }
    fp.close();
  } else {
    String ret = "";
    ret = ret + "Error al obtener variable de archivo:  " + archivo;
    Serial.println(ret);
  }
  return ret;
}

void escribirVariableRiego(float var, char* archivo) {
  SD.remove(archivo);
  File fp = SD.open(archivo, FILE_WRITE);
  if (fp) {
    fp.print(var);
    fp.close();
  } else {
    Serial.println("Error al escribir variable de riego.");
  }
}

void guardarEnArchivo(int* vec, int perEfectividadZ1, int perEfectividadZ2) {
  File fp = SD.open("zona1.txt", FILE_WRITE); //VER SI LO ESCRIBE AL FINAL O AL PRINCIPIO
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;    
    fp.println(ret);
    fp.close();
  } else {
    Serial.println("Error al abrir el archivo de zona 1.");
  }

  fp = SD.open("zona2.txt", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + perEfectividadZ2;    
    fp.println(ret);
    fp.close();
  } else {
    Serial.println("Error al abrir el archivo de zona 2.");
  }
}

float calcularEfectividad(int temp, int humedadAmbiente, int humedadSuelo, int luz) {
  float perTemperatura = temp / MAX_TEMP;
  float perHumedadAmbiente = humedadAmbiente / MAX_HUMEDAD;
  float perHumedadSuelo = humedadSuelo / MAX_HUMEDAD_SUELO;
  float perLuz = luz / MAX_LUZ;
  return PRIORIDAD_TEMP * (perTemperatura) + PRIORIDAD_HUM_AMB * (MAX_HUMEDAD - perHumedadAmbiente) + PRIORIDAD_HUM_SUELO * (MAX_HUMEDAD - perHumedadSuelo) - PRIORIDAD_LUZ * (MAX_LUZ - perLuz);
}

int determinarRiegoEnZona1() {
  // ser si esta muy seco
  // si esta muy seco ver si tiene datos anteriores
  // si tiene datos anteriores deberia ver si la luz esta en aumento
  // si tiene datos anteriores deberia ver si la humedad ambiente esta en aumento
  return 0;
}

int determinarRiegoEnZona2() {
  return 0;
}

float calcularVolumenRiego(int riego, float var) {
  return riego * var;
}

void inicializarArchivosDeCensos() {
  File fp;
  if(SD.exists("zona1.txt")) {
    SD.remove("zona1.txt");  
  }

  if(SD.exists("zona2.txt")) {
    SD.remove("zona2.txt");  
  }
  fp = SD.open("zona1.txt", FILE_WRITE);
  if (!fp) {
    Serial.println("no pudo crear zona1.txt");
  }
  fp.close();
  fp = SD.open("zona2.txt", FILE_WRITE);
  if (!fp) {
    Serial.println("no pudo crear zona2.txt");
  }
  fp.close();
}
