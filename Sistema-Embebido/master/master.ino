#include <SD.h>

#include <SoftwareSerial.h>

#define TAM_MAX_READ 9
#define TAM_MAX_WRITE 2

// PUERTOS DE CONEXION CON ESCLAVO
#define PUERTO_RX_SLAVE 2
#define PUERTO_TX_SLAVE 3

#define PIN_CS_SD 4
#define PIN_RX_BLUETOOTH 0
#define PIN_TX_BLUETOOTH 1

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
  Serial.println("Arduino Maestro iniciado");

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
    
    Serial.println("Envio inst censo.");
    String ret = "";
    ret = ret + '<' + INST_CENSO + '>';
    serialSlave.print(ret);
    Serial.println(ret);
    previousMillis = millis();
    MS_INTERVAL_TO_CENSO = (unsigned long)10000;
  }
  
  int valoresRecibidos[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerEsclavo(valoresRecibidos);
  leerBluetooth(valoresRecibidos);
  
  switch(valoresRecibidos[0]){
    case INST_CENSO: {
      int perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
      int perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);      
      guardarEnArchivo(valoresRecibidos,perEfectividadZ1,perEfectividadZ2);
      
      if(determinarRiegoEnZona1(valoresRecibidos[3])){ //Implementar
        Serial.println("entra en zona1");
        float varZona1 = obtenerVariableRiego("VAR1.TXT");
        float vol1 = calcularVolumenRiego(valoresRecibidos[3], varZona1);
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z1 + "," + vol1 + ">";
        Serial.println(ret);
        serialSlave.print(ret);
        riegoEnCursoZona1 = 'T';
      }
      if(determinarRiegoEnZona2()){ //Implementar
        Serial.println("entra en zona2");
        float varZona2 = obtenerVariableRiego("VAR2.TXT");
        float vol2 = calcularVolumenRiego(valoresRecibidos[7], varZona2);
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z2 + "," + vol2 + ">";
        Serial.println(ret);
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
        Serial.println("hum z1 entre");
        var1 = obtenerVariableRiego("VAR1.TXT");
        if(perHumedadSueloZona1 <= 40){
          var1 = var1 + (1/perHumedadSueloZona1);
        }
        else if(perHumedadSueloZona1 >= 60){
          var1 = var1 - (1/perHumedadSueloZona1);
        }
        if (var1 <= 0) {
          // ver porque pasa a negativo
          var1 = 1;
        }
        escribirVariableRiego(var1, "VAR1.TXT");
      } else {
        Serial.println("No rego Z1.");
      }
      break;
      //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
    }
    case INST_RES_RIEGO_Z2: {
      //Aca se analiza el resultado del riego de la zona 2.
      int var2 = 0;
      // TODO ESTO PUEDE SER UNA FUNCION ----
      int humedadSueloZona2 = valoresRecibidos[1];
      int perHumedadSueloZona2 = humedadSueloZona2 / 1023;
      if(perHumedadSueloZona2 < 40 || perHumedadSueloZona2 > 60){
        Serial.println("hum z1 entre");
        var2 = obtenerVariableRiego("VAR2.TXT");
        if(perHumedadSueloZona2 <= 40){
          var2 = var2 + (1/perHumedadSueloZona2);
        }
        else if(perHumedadSueloZona2 >= 60){
          var2 = var2 - (1/perHumedadSueloZona2);
        }
        if (var2 <= 0) {
          // ver porque pasa a negativo
          var2 = 1;
        }
        // HASTA ACA ----
        escribirVariableRiego(var2, "VAR2.TXT");
      } else {
        Serial.println("No rego Z2.");
      }
      break;
      //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
    }
  }

  while (Serial.available() > 0 ) {
    Serial.println((char)Serial.read()); 
  }
}

void leerEsclavo(int* vec) {
  byte charIndex = 0;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }
  
  if (serialSlave.available() > 0) {
    Serial.println("entra..");
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
    ret = ret + "t1: " + vec[1] + ", ha1: " + vec[2] + ", hs1: " + vec[3] + ", L1: " + vec[4];          
    Serial.println(ret);
    ret = "";
    ret = ret + ", t2: " + vec[5] + ", ha2: " + vec[6] + ", hs2: " + vec[7] + ", L2: " + vec[8];
    Serial.println(ret);
  }
}
// ES LA MISMA QUE LEER ESCLAVO, VER FORMA DE PASARLE LA FUNCION AVAILABLE Y READBYTEUNTIL
void leerBluetooth(int* vec) {
  byte charIndex = 0;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }
  
  if (Serial.available() > 0) {
    Serial.println("entra..");
    Serial.readBytesUntil('>', entrada, 59);
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
    ret = ret + "t1: " + vec[1] + ", ha1: " + vec[2] + ", hs1: " + vec[3] + ", L1: " + vec[4];          
    Serial.println(ret);
    ret = "";
    ret = ret + ", t2: " + vec[5] + ", ha2: " + vec[6] + ", hs2: " + vec[7] + ", L2: " + vec[8];
    Serial.println(ret);
  }
}

float obtenerVariableRiego(const char* archivo) {
  File fp = SD.open(archivo);
  char caracter;
  float ret = -1;
  char input[5];

  if (fp) {
    if (fp.available()) {
      fp.read(input, sizeof(input));
      ret = atof(input);
    } else {
      Serial.println("no habilitado para leer");
    }
    fp.close();
  } else {
    String ret = "";
    ret = ret + "Error al obtener variable de archivo:  " + archivo;
    Serial.println(ret);
  }
  return ret;
}

void escribirVariableRiego(float var, const char* archivo) {
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
  File fp = SD.open("ZONA1.TXT", FILE_WRITE); //VER SI LO ESCRIBE AL FINAL O AL PRINCIPIO
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;    
    fp.println(ret);
    fp.close();
  } else {
    Serial.println("Error al abrir el archivo de zona 1.");
  }

  fp = SD.open("ZONA2.TXT", FILE_WRITE);
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

int determinarRiegoEnZona1(int humSuelo) {
  Serial.print("hum suelo");
  Serial.println(humSuelo);
  if (humSuelo > 1000) {
    // esta muy seco
    return 1;
  } else if (humSuelo > 500) {
    // seco pero no tanto
    File fp = SD.open("ZONA1.TXT", FILE_READ);
    if (fp) {
      char entrada[60];

      for (int i = 0; i < 60; i++) {
        entrada[i] = '\0';
      }
      if (fp.available() > 0) {
        fp.readBytesUntil('\n', entrada, 59);
        Serial.println(entrada);
        char input[5];
        int charIndex = 0;
        int fieldIndex = 0;
        int vec[9];
        int i = 0;
        while(entrada[i] != '\0') {
          if (entrada[i] != ',') {
            input[charIndex] = entrada[i];
            charIndex++;  
          } else {
            input[charIndex] = '\0';
            charIndex = 0;
            Serial.print("envio ");
            Serial.println(input);
            vec[fieldIndex] = atoi(input);
            fieldIndex++;
          }
          i++;
        }

        input[charIndex] = '\0'; // finalizo el ultimo porque no tiene coma
        charIndex = 0;
        Serial.print("envio1 ");
        Serial.println(input);
        vec[fieldIndex] = atoi(input);
      } else {
        Serial.println("no habilitado para leer");
      }
      fp.close();
    } else {
      Serial.println("error al leer archio zona1");
    }
  }
  // ser si esta muy seco
  // si esta muy seco ver si tiene datos anteriores
  // si tiene datos anteriores deberia ver si la luz esta en aumento
  // si tiene datos anteriores deberia ver si la humedad ambiente esta en aumento
  return millis()%2;
}

int determinarRiegoEnZona2() {
  return millis()%2;
}

float calcularVolumenRiego(int riego, float var) {
  return riego * var;
}

void inicializarArchivosDeCensos() {
  File fp;
  if(SD.exists("ZONA1.TXT")) {
    SD.remove("ZONA1.TXT");  
  }

  if(SD.exists("ZONA2.TXT")) {
    SD.remove("ZONA2.TXT");  
  }
  fp = SD.open("ZONA1.TXT", FILE_WRITE);
  if (!fp) {
    Serial.println("no pudo crear ZONA1.TXT");
  }
  fp.close();
  fp = SD.open("ZONA2.TXT", FILE_WRITE);
  if (!fp) {
    Serial.println("no pudo crear ZONA2.TXT");
  }
  fp.close();
}
