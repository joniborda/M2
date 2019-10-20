#include <SoftwareSerial.h>
#include <SD.h>

#define TAM_MAX 9

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

File filePointer;
unsigned long currentMillis = millis(); // grab current time
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
        guardarEnArchivo(valoresRecibidos,perEfectividadZ1,perEfectividadZ2); //Implentar 
        
        if(determinarRiegoEnZona1()){ //Implementar
          int varZona1 = obtenerVariableRiegoZona1(); //Implementar
          int vol1 = calcularVolumenRiego(valoresRecibidos[2]); // Implementar
          serialSlave.write(INST_RIEGO_Z1); //CAMBIAR POR <INST, 0000>
          serialSlave.write(vol1); //PROBAR URGENTE
          riegoEnCursoZona1 = 'T';
        }

        if(determinarRiegoEnZona2()){
          int vol2 = obtenerVolumenRiegoZona2();
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
          var1 = obtenerVariableRiegoZona1();
          if(perHumedadSueloZona1 <= 40){
            var1 = var1 + (1/perHumedadSueloZona1);
          }
          else if(perHumedadSueloZona1 => 60){
            var1 = var1 - (1/perHumedadSueloZona1);
          }
          escribirVariableRiegoZona1(var1);
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
          var2 = obtenerVariableRiegoZona2();
          if(perHumedadSueloZona2 <= 40){
            var2 = var2 + (1/perHumedadSueloZona2);
          }
          else if(perHumedadSueloZona2 => 60){
            var2 = var2 - (1/perHumedadSueloZona2);
          }
          escribirVariableRiegoZona1(var2);
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
    static const char START_MARKER = '<';
    static const char COMMA = ',';
    static const char END_MARKER = '>';
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
    ret = ret + "Temperatura1: " + vec[1] + ", HumedadAmbiente1: " + vec[2] + ", HumedadSuelo1: " + vec[3] + ", Luz1: " + vec[4] +
                ", Temperatura2: " + vec[5] + ", HumedadAmbiente2: " + vec[6] + ", HumedadSuelo2: " + vec[7] + ", Luz2: " + vec[8];
    Serial.println(ret);

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

int determinarRiegoEnZona1() {

}

int determinarRiegoEnZona2() {

}

/*int calcularEfectividad1() {
  return calcularEfectividad(temperatura1, humedadAmbiente1, humedadSuelo1, luz1);
}

int calcularEfectividad2() {
  return calcularEfectividad(temperatura2, humedadAmbiente2, humedadSuelo2, luz2);
} */
