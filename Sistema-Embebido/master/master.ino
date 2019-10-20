#include <SoftwareSerial.h>
#include <SD.h>

#define TAM_MAX_READ 9
#define TAM_MAX_WRITE 2

// PUERTOS DE CONEXION CON ESCLAVO
const int PUERTO_RX_SLAVE = 2;
const int PUERTO_TX_SLAVE = 3;

const int PIN_CS_SD = 10;

const int INST_CENSO = 1; // INSTRUCCION PARA RUTINA DE CENSO (INICIO/FIN)
const int INST_RIEGO_Z1 = 2; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 (INICIO/FIN)
const int INST_RIEGO_Z2 = 3; // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 (INICIO/FIN)
const int INST_MANTENIMIENTO = 4; // INSTRUCCION PARA RUTINA DE MANTENIMIENTO (INICIO/FIN)
const int INST_RES_RIEGO_Z1 = 12; //INSTRUCCION PARA LEER EL RESULTADO DEL RIEGO EN LA ZONA 1
const int INST_RES_RIEGO_Z2 = 13; //INSTRUCCION PARA LEER EL RESULTADO DEL RIEGO EN LA ZONA 2

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned long MS_INTERVAL_TO_CENSO = 1000; // 15 seg.

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

unsigned long currentMillis = 0; // grab current time
unsigned long previousMillis = 0;  // millis() returns an unsigned long.

//Podriamos poner directamente los valores para no comer espacio, revisar si es necesario
static const float PER_TEMP = 0.2;
static const float PER_HUM_AMB = 0.2;
static const float PER_HUM_SUE = 0.4;
static const float PER_LUZ = 0.2;
static const int MAX_TEMP = 40;
static const int MAX_HUMEDAD = 100;
static const int MAX_LUZ = 100;

char riegoEnCursoZona1 = 'F';
char riegoEnCursoZona2 = 'F';

//int[30][9] censos;

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.print("Arduino Maestro iniciado...");

  if (!SD.begin(PIN_CS_SD)) {
    Serial.println("No se pudo inicializar la SD");
    return;
  }
}

void loop() {
  currentMillis = millis();
  
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_CENSO) {
    Serial.println("Envio instruccion de censo a el esclavo.");
    String ret = "";
    ret = ret + '<' + INST_CENSO + '>';
    serialSlave.write(ret);
    previousMillis = millis();
    MS_INTERVAL_TO_CENSO = 3000;
  }
  
  int valoresRecibidos[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerEsclavo(valoresRecibidos);
  if(valoresRecibidos[0] != -1) {
    switch(valoresRecibidos[0]){
      case INST_CENSO: {
        int perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
        int perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);      
        guardarEnArchivo(valoresRecibidos,perEfectividadZ1,perEfectividadZ2); //Implentar 
        
        if(determinarRiegoEnZona1()){ //Implementar
          float varZona1 = obtenerVariableRiego("variableRiegoZona1.txt");
          float vol1 = calcularVolumenRiego(valoresRecibidos[2], varZona1); // Implementar
          serialSlave.write(INST_RIEGO_Z1); //CAMBIAR POR <INST, 0000>
          serialSlave.write(vol1); //PROBAR URGENTE
          riegoEnCursoZona1 = 'T';
        }

        if(determinarRiegoEnZona2()){
          float varZona2 = obtenerVariableRiego("variableRiegoZona2.txt");
          float vol2 = calcularVolumenRiego(valoresRecibidos[6], varZona2);
          serialSlave.write(INST_RIEGO_Z2); //CAMBIAR POR <INST, 0000>
          serialSlave.write(vol2);
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
          var1 = obtenerVariableRiego("variableRiegoZona1.txt");
          if(perHumedadSueloZona1 <= 40){
            var1 = var1 + (1/perHumedadSueloZona1);
          }
          else if(perHumedadSueloZona1 => 60){
            var1 = var1 - (1/perHumedadSueloZona1);
          }
          escribirVariableRiego(var1, "variableRiegoZona1.txt");
        } else {
          Serial.println("El caudal de agua es correcto para la zona 1.")
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
          var2 = obtenerVariableRiego("variableRiegoZona2.txt");
          if(perHumedadSueloZona2 <= 40){
            var2 = var2 + (1/perHumedadSueloZona2);
          }
          else if(perHumedadSueloZona2 => 60){
            var2 = var2 - (1/perHumedadSueloZona2);
          }
          escribirVariableRiego(var2, "variableRiegoZona2.txt");
        } else {
          Serial.println("El caudal de agua es correcto para la zona 2.")
        }
        break;
        //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
      }
    }
  }
}

void leerEsclavo(int* vec) {
    static boolean recvinprogress = false; // Se mantiene estatico porque si no llego al caracter de corte tiene que seguir leyendo la cadena.
    static byte charIndex = 0; // Es static porque se pudo haber interrupido la lectura y tiene que continuar desde donde quedo.
    static char charLeido;
    static char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
    static int fieldIndex = 0;
    if (serialSlave.available() <= 0) {
      return;  
    } else {
      Serial.println("Informacion disponible del esclavo.");
    }
    while(serialSlave.available() > 0) {
      charLeido = (char)serialSlave.read();
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
    // El ultimo valor no tiene coma.
    input[charIndex] = '\0';
    vec[fieldIndex] = atoi(input);

    String ret = "";
    ret = ret + "Temperatura1: " + vec[1] + ", HumedadAmbiente1: " + vec[2] + ", HumedadSuelo1: " + vec[3] + ", Luz1: " + vec[4] +
                ", Temperatura2: " + vec[5] + ", HumedadAmbiente2: " + vec[6] + ", HumedadSuelo2: " + vec[7] + ", Luz2: " + vec[8];
    Serial.println(ret);

    charIndex = 0;
    fieldIndex = 0;
  }
}

float obtenerVariableRiego(char* archivo) {
  File fp = SD.open(archivo);
  char caracter;
  float ret = -1;

  // if the file is available, write to it:
  if (fp) {
    if (fp.available()) {
      fp.read(input, sizeof(input));
      ret = atof(input);
    }
    fp.close();
  } else {
    String ret = "";
    ret = ret + "Er. " + archivo;
    Serial.println(ret);
  }
  return ret;
}

void escribirVariableRiego(float var, char* archivo) {
  File fp = SD.open(archivo, FILE_WRITE); //VER SI LO PISA
  if (fp) {
    fp.print(var);
    fp.close();
  } else {
    Serial.println("E escribir var riego");
  }
}

void guardarEnArchivo(int* vec, int perEfectividadZ1, int perEfectividadZ2) {
  File fp = SD.open("diarioZona1.txt", FILE_WRITE); //VER SI LO PISA
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;    
    fp.print(ret);
    fp.close(); //Cerramos el archivo
  } else {
    Serial.println("Error al abrir el archivo de zona 1.");
  }

  fp = SD.open("diarioZona2.txt", FILE_WRITE); //VER SI LO PISA
  if (fp) {
    String ret = "";
    ret = ret + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + perEfectividadZ2;    
    fp.print(ret);
    fp.close(); //Cerramos el archivo
  } else {
    Serial.println("Error al abrir el archivo de zona 2.");
  }
}

int calcularEfectividad(float temp, float amb, float suelo, float luz) {
  return PER_TEMP * abs(MAX_TEMP - temp) - PER_HUM_AMB * abs(MAX_HUMEDAD - amb) - PER_HUM_SUE * abs(MAX_HUMEDAD - suelo) - PER_LUZ * abs(MAX_LUZ - luz);
}

int determinarRiegoEnZona1() {
  // ser si esta muy seco
  // si esta muy seco ver si tiene datos anteriores
  // si tiene datos anteriores deberia ver si la luz esta en aumento
  // si tiene datos anteriores deberia ver si la humedad ambiente esta en aumento
}

int determinarRiegoEnZona2() {

}

float calcularVolumenRiego(int riego, float var) {
  return riego * var;
}
/*int calcularEfectividad1() {
  return calcularEfectividad(temperatura1, humedadAmbiente1, humedadSuelo1, luz1);
}

int calcularEfectividad2() {
  return calcularEfectividad(temperatura2, humedadAmbiente2, humedadSuelo2, luz2);
} */
