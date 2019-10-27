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
#define INST_RIEGO_Z2 3 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 (INICIO/FIN)
#define INST_MANTENIMIENTO 4 // INSTRUCCION PARA RUTINA DE MANTENIMIENTO (INICIO/FIN)
#define INST_DETENER_RIEGO_Z1 5 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 1
#define INST_DETENER_RIEGO_Z2 6 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 2
#define INST_ENCENDER_LUZ_1_MANUAL 7 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_ENCENDER_LUZ_2_MANUAL 8 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_APAGAR_LUZ_1_MANUAL 9 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_APAGAR_LUZ_2_MANUAL 10 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_AUTO_LUZ_1 11 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_AUTO_LUZ_2 15 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_RES_RIEGO_Z1 12 //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2 13 //INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned long MS_INTERVAL_TO_CENSO = 10000; // 15 seg.

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

unsigned long currentMillis = 0; // grab current time
unsigned long previousMillis = 0;  // millis() returns an unsigned long.

#define PRIORIDAD_TEMP 0.05
#define PRIORIDAD_HUM_AMB 0.05
#define PRIORIDAD_HUM_SUELO 0.4
#define PRIORIDAD_LUZ 0.4
#define MAX_TEMP 50
#define MAX_HUMEDAD_SUELO 1023 //1 Completamente humedo - 1023 Completamente seco
#define MAX_LUZ 1023 //1  Completamente oscuro - 1023 Completamente iluminado
#define MAX_HUMEDAD 100

char riegoEnCursoZona1 = 'F';
char riegoEnCursoZona2 = 'F';
int valoresCensoAnterior[] = {-1, -1, -1, -1}; //necesito que sea global, se guarda luego de censar y determinar si censo

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.println("M_OK"); //Maestro iniciado correctamente

  if (!SD.begin(PIN_CS_SD)) {
    Serial.println("E_SD_001"); //Error al inicializar la tarjeta SD
  }else{
    Serial.println("SD_002"); //Tarjeta SD incializada correctamente.
    inicializarArchivosDeCensos();
  }
  
}

void loop() {
    
  currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= MS_INTERVAL_TO_CENSO) {
    
    Serial.println("M_C"); //Maestro envia orden de censar al esclavo
    enviarInstruccionAlEsclavo(INST_CENSO);
    previousMillis = millis();
    MS_INTERVAL_TO_CENSO = (unsigned long)10000;
  }
  
  int valoresRecibidos[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerInstruccion(valoresRecibidos);
  evaluarInstruccion(valoresRecibidos);
  
  /*
  for (int i = 0; i < 9; i++) {
    valoresRecibidos[i] = -1;  
  }
  
  leerBluetooth(valoresRecibidos);
  evaluarInstruccion(valoresRecibidos);
  */
}

void leerInstruccion(int* vec) {
  byte charIndex = 0;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }
  
  if (serialSlave.available() > 0) {
    serialSlave.readBytesUntil('>', entrada, 59);
    Serial.println("L_E"); //Leyendo del esclavo
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
    entrada[0] = '@'; //Indica que el dato leido proviene del esclavo
    Serial.println(entrada);
  }
}

void leerBluetooth(int* vec) {
  byte charIndex = 0;
  char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
  int fieldIndex = 0;
  char entrada[60];
  for (int i = 0; i < 60; i++) {
    entrada[i] = '\0';
  }
  
  if (Serial.available() > 0) {
    Serial.println("L_B_001"); //Leyendo datos del modulo bluetooth
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
  }
}

float obtenerVariableRiego(const char* archivo) {
  File fp = SD.open(archivo);
  char caracter;
  float varRiego = -1;
  char input[5];

  if (fp) {
    if (fp.available()) {
      fp.read(input, sizeof(input));
      varRiego = atof(input);
    } else {
      Serial.println("E_L_V"); //Error al leer el archivo de variable
    }
  } else {
    String ret = "";
    ret = ret + "E_A_V " + archivo;
    Serial.println(ret);
    varRiego = 33.33; //Si falla la apertura de la SD, se envia una variable fija de manera que el arduino pueda seguir regando.
  }
  fp.close();
  return varRiego;
}

void escribirVariableRiego(float var, const char* archivo) {
  SD.remove(archivo);
  File fp = SD.open(archivo, FILE_WRITE);
  if (fp) {
    fp.print(var);
  } else {
    Serial.println("E_E_V");
    Serial.println(archivo);
  }
  fp.close();
}

void guardarEnArchivo(int* vec, float perEfectividadZ1, float perEfectividadZ2) {
  File fp = SD.open("ZONA1.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;    
    fp.println(ret);
  } else {
    Serial.println("E_A_Z1"); //Error al abrir archivo ZONA1.TXT
  }
  fp.close();
  
  fp = SD.open("ZONA2.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + perEfectividadZ2;    
    fp.println(ret);
  } else {
    Serial.println("E_A_Z2"); //Error al abrir archivo ZONA2.TXT
  }
  fp.close();
}

float calcularEfectividad(int temp, int humedadAmbiente, int humedadSuelo, int luz) {
  float perTemperatura = (float)temp / MAX_TEMP;
  float perHumedadAmbiente = (float)humedadAmbiente / MAX_HUMEDAD;
  float perHumedadSuelo = (float)humedadSuelo / MAX_HUMEDAD_SUELO;
  float perLuz = (float)luz / MAX_LUZ;
  Serial.print("%T: ");
  Serial.println(perTemperatura);
  Serial.print("%HA: ");
  Serial.println(perHumedadAmbiente);
  Serial.print("%SS: ");
  Serial.println(perHumedadSuelo);
  Serial.print("%NL: ");
  Serial.println(perLuz);
  Serial.println("");

  Serial.println("V_PRIORIZADOS");
  Serial.print("%%T: ");
  Serial.println(perTemperatura * PRIORIDAD_TEMP);
  Serial.print("%%HA: ");
  Serial.println(perHumedadAmbiente * PRIORIDAD_HUM_AMB);
  Serial.print("%%SS: ");
  Serial.println(perHumedadSuelo * PRIORIDAD_HUM_SUELO);
  Serial.print("%%NL: ");
  Serial.println(perLuz * PRIORIDAD_LUZ);
  Serial.println("");
  return (PRIORIDAD_TEMP * (perTemperatura) + PRIORIDAD_HUM_AMB * (1 - perHumedadAmbiente) + PRIORIDAD_HUM_SUELO * (perHumedadSuelo) + PRIORIDAD_LUZ * (perLuz)) * 100;
}

int determinarRiegoEnZona(int zona, float perEfectividad, int luzActual, int humedadActual) {
  if(perEfectividad > 70.00){
    return 1;
  }
  else if(perEfectividad > 40.00){
    int luzAnterior = valoresCensoAnterior[zona - 1];
    int humedadAnterior = valoresCensoAnterior[zona - 1 + 2];
    if(luzAnterior == -1 || humedadAnterior == -1) //Es la primera vez que censa, no hay valores anteriores
      return 0;
    float varLuz = ((float)(luzActual - luzAnterior) / (float)luzAnterior);
    if(varLuz < 0) {
      Serial.println("La luz se encuentra en descenso.");
      return 1;
    } else if(varLuz < 60.00){
      Serial.println("La luz se mantiene estable.");
      return 1;
    } else {
      Serial.println("La luz esta en ascenso, no es conveniente regar.");
    }
  }
  return 0;
}

float calcularVolumenRiego(int riego, float var) {
  return (riego * var) / 1023;
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
    Serial.println("E_A_001"); //Error al crear archivo ZONA1.TXT 
  }
  fp.close();
  
  fp = SD.open("ZONA2.TXT", FILE_WRITE);
  if (!fp) {
    Serial.println("E_A_002"); //Error al crear archivo ZONA2.TXT
  }
  fp.close();

  if(!SD.exists("VAR1.txt")){
    Serial.println("A_V_001"); //El archivo VAR1.TXT no existe en la tarjeta SD
    fp = SD.open("VAR1.txt",FILE_WRITE);
    Serial.println("A_V_002"); //El archivo VAR1.TXT no existia y se acaba de crear
    if(fp){
      fp.println("33.33");
    } else {
      Serial.println("E_A_001"); //Error al escribir el archivo VAR1.txt con el valor por defecto
    }
    fp.close();
  }

  if(!SD.exists("VAR2.txt")){
    Serial.println("A_V_003"); //El archivo VAR2.TXT no existe en la tarjeta SD
    fp = SD.open("VAR2.txt",FILE_WRITE);
    Serial.println("A_V_004"); //El archivo VAR2.TXT no existia y se acaba de crear
    if(fp){
      fp.println("33.33");
    } else {
      Serial.println("E_A_002"); //Error al escribir el archivo VAR2.txt con el valor por defecto      
    }
    fp.close();
  }
}

void analizarResultadoRiego(int zona, int humedadSuelo, char* archivo) {
  float var = 0.0;
  float humedadSueloZona = humedadSuelo;
  String ret = "";
  ret = ret + "I_R_" + zona;
  Serial.println(ret); //Se va a analizar el resultado del riego de la ZONA N
  ret = "";
  ret = ret + "HSuelo resultado: " + humedadSueloZona;
  Serial.println(ret);
  float perHumedadSueloZona = (100 - (humedadSueloZona * 100) / 1023);
  if(perHumedadSueloZona < 40 || perHumedadSueloZona > 60) {
      ret = "";
      ret = ret + "PH_Z" + zona;
      Serial.println(ret); //Porcentaje de humedad resultado del ultimo riego ZONA N
      ret = "";
      ret = ret + "%HSuelo resultado: " + perHumedadSueloZona;
      Serial.println(ret);
      var = obtenerVariableRiego(archivo);
      ret = "";
      ret = ret + "V_PH_Z" + zona;
      Serial.println(ret); //Variable de riego con la que se rego ZONA N
      Serial.println(var);
      float ajuste = (50 - perHumedadSueloZona)/2;
      var += ajuste;
      if(var < 0)
        var = 1.00;
      if(var > 100)
        var = 100.00;   
      ret = "";
      ret = ret + "N_V_PH_Z" + zona;                
      Serial.print(ret); //Nueva variable de riego a almacenar ZONA 1
      Serial.println(var);
      escribirVariableRiego(var, archivo);
  } else {
    ret = "";
    ret = ret + "R_C_Z" + zona;
    Serial.println(ret); //Porcentaje de humedad en ZONA 1 correcto
  }
}

void evaluarInstruccion(int valores[]) {
  switch(valores[0]){
    case INST_CENSO: {
      // Ocurre cuando el esclavo avisa que termino el censo y me envia los valores de ese censo
      // de la zona 1 y de la zona 2
      float perEfectividadZ1 = calcularEfectividad(valores[1], valores[2], valores[3], valores[4]);
      float perEfectividadZ2 = calcularEfectividad(valores[5], valores[6], valores[7], valores[8]); 
      
      Serial.print("%EF1 ");
      Serial.println(perEfectividadZ1);
      Serial.print("%EF2 ");
      Serial.println(perEfectividadZ2); 
      guardarEnArchivo(valores,perEfectividadZ1,perEfectividadZ2);
      
      if(determinarRiegoEnZona(1, perEfectividadZ1, valores[4], valores[2])) {
        Serial.println("R_Z_1"); //Es eficiente regar en la zona 1
        float varZona1 = obtenerVariableRiego("VAR1.TXT");
        float vol1 = calcularVolumenRiego(valores[3], varZona1);
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z1 + "," + vol1 + ">";
        Serial.println(ret);
        serialSlave.print(ret);
        riegoEnCursoZona1 = 'T';
      }
      if(determinarRiegoEnZona(2, perEfectividadZ2, valores[8], valores[6])) {
        Serial.println("R_Z_2"); //Es eficiente regar en la zona 2
        float varZona2 = obtenerVariableRiego("VAR2.TXT");
        float vol2 = calcularVolumenRiego(valores[7], varZona2);
        String ret = "";
        ret = ret + "<" + INST_RIEGO_Z2 + "," + vol2 + ">";
        Serial.println(ret);
        serialSlave.print(ret);
        riegoEnCursoZona2 = 'T';
      }
      // Guardo los valores para el proximo censo
      valoresCensoAnterior[0] = valores[4]; // LUZ 1
      valoresCensoAnterior[1] = valores[8]; // LUZ 2
      valoresCensoAnterior[2] = valores[2]; // HUMEDAD AMBIENTE 1
      valoresCensoAnterior[3] = valores[6]; // HUMEDAD AMBIENTE 2
      break;
    }
    case INST_MANTENIMIENTO: {
      //ANALIZAR ERRORES E INFORMAR
      break;
    }
    case INST_RIEGO_Z1: {
      // Ocurre cuando el esclavo me avisa que termino de regar la zona 1
      riegoEnCursoZona1 = 'F';
      break;
    }
    case INST_RIEGO_Z2: {
      // Ocurre cuando el esclavo me avisa que termino de regar la zona 2
      riegoEnCursoZona2 = 'F';
      break;
    }
    case INST_RES_RIEGO_Z1: {
      //Aca se analiza el resultado del riego de la zona 1.
      /*float var1 = 0.0;
      float humedadSueloZona1 = valores[1];
      Serial.println("I_R_1"); //Se va a analizar el resultado del riego en ZONA 1
      Serial.println(humedadSueloZona1);
      float perHumedadSueloZona1 = (100 - (humedadSueloZona1 * 100) / 1023);
      if(perHumedadSueloZona1 < 40 || perHumedadSueloZona1 > 60) {
          Serial.println("PH_Z1"); //Porcentaje de humedad resultado del ultimo riego ZONA 1
          Serial.println(perHumedadSueloZona1);
          var1 = obtenerVariableRiego("VAR1.TXT");
          Serial.println("V_PH_1"); //Variable de riego con la que se rego ZONA 1
          Serial.println(var1);
          float ajuste = (50 - perHumedadSueloZona1)/2;
          var1 += ajuste;
          if(var1 < 0)
            var1 = 1.00;
          if(var1 > 100)
            var1 = 100.00;                   
          Serial.print("N_V_PH_1 "); //Nueva variable de riego a almacenar ZONA 1
          Serial.println(var1);
          escribirVariableRiego(var1, "VAR1.TXT");
      } else {
        Serial.println("R_Z1_C"); //Porcentaje de humedad en ZONA 1 correcto
      }
      */
      analizarResultadoRiego(1, valores[1], "VAR1.TXT");
      break;
      //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
    }
    case INST_RES_RIEGO_Z2: {
      //Aca se analiza el resultado del riego de la zona 2.
      /*int var2 = 0;
      int humedadSueloZona2 = valores[1];
      String ret = "";
      ret = ret + "I_R_2" + " " + humedadSueloZona2; 
      Serial.println("I_R_2"); //Se va a analizar el resultado del riego en ZONA 2
      Serial.println(humedadSueloZona2);
      float perHumedadSueloZona2 = (100 - (humedadSueloZona2 * 100) / 1023);
      if(perHumedadSueloZona2 < 40 || perHumedadSueloZona2 > 60) {
        Serial.println("PH_Z2"); //Porcentaje de humedad resultado del ultimo riego ZONA 2
          Serial.println(perHumedadSueloZona2);
          var2 = obtenerVariableRiego("VAR2.TXT");
          Serial.println("V_PH_2"); //Variable de riego con la que se rego ZONA 1
          Serial.println(var2);
          float ajuste = (50 - perHumedadSueloZona2)/2;
          var2 += ajuste;
          if(var2 < 0)
            var2 = 1.00;
          if(var2 > 100)
            var2 = 100.00;                   
          Serial.print("N_V_PH_2 "); //Nueva variable de riego a almacenar ZONA 1
          Serial.println(var2);
          escribirVariableRiego(var2, "VAR1.TXT");
      } else {
        Serial.println("R_Z2_C"); //Porcentaje de humedad en ZONA 2 correcto
      }*/
      analizarResultadoRiego(2, valores[1], "varZona2.TXT");
      break;
      //Tambien aca podriamos determinar si la bomba esta funcionando correctamente.
    }
    case INST_DETENER_RIEGO_Z1: {
      // El bluetooth envia detener el riego en zona 1
      // verificar que estè regando
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_DETENER_RIEGO_Z1);
      break;
    }
    case INST_DETENER_RIEGO_Z2: {
      // El bluetooth envia detener el riego en zona 1
      // verificar que estè regando
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_DETENER_RIEGO_Z2);
      break;
    }
    case INST_ENCENDER_LUZ_1_MANUAL: {
      // El bluetooth envia encender luz 1 manualmente
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_ENCENDER_LUZ_1_MANUAL);
      break;
    }
    case INST_ENCENDER_LUZ_2_MANUAL: {
      // El bluetooth envia encender luz 2 manualmente
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_ENCENDER_LUZ_2_MANUAL);
      break;
    }
    case INST_APAGAR_LUZ_1_MANUAL: {
      // El bluetooth envia apagar luz 1 manualmente
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_APAGAR_LUZ_1_MANUAL);
      break;
    }
    case INST_APAGAR_LUZ_2_MANUAL: {
      // El bluetooth envia apagar luz 2 manualmente
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_APAGAR_LUZ_2_MANUAL);
      break;
    }
    case INST_AUTO_LUZ_1: {
      // El bluetooth envia orden de dejar en automatica la luz 1
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_AUTO_LUZ_1);
      break;
    }
    case INST_AUTO_LUZ_2: {
      // El bluetooth envia orden de dejar en automatica la luz 2
      // enviar orden al esclavo
      enviarInstruccionAlEsclavo(INST_AUTO_LUZ_2);
      break;
    }
  }
}

void enviarInstruccionAlEsclavo(const int instruccion) {
    String orden = "";
      orden = orden + '<' + instruccion + '>';
      serialSlave.print(orden);
      Serial.println(orden); // Cuando se conecte el modulo bluetooth, esto puede salir por el dispositivo.
  }
