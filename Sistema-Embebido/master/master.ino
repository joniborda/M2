#include <SD.h>
#include <SoftwareSerial.h>

#define DEBUG // Comentar para no hacer debug

#ifdef DEBUG
 #define DEBUG_PRINT(x) Serial.println(x)
#else
 #define DEBUG_PRINT(x)
#endif

// PUERTOS DE CONEXION CON ESCLAVO
#define PUERTO_RX_SLAVE 2
#define PUERTO_TX_SLAVE 3

#define PIN_CS_SD 4

#define INST_CENSO                  1 // INSTRUCCION PARA RUTINA DE CENSO INICIO
#define INST_FIN_CENSO              2 // INSTRUCCION PARA RUTINA DE CENSO FIN
#define INST_RIEGO_Z1               3 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 INICIO
#define INST_RIEGO_Z2               4 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 INICIO
#define INST_FIN_RIEGO_Z1           5 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 1 FIN
#define INST_FIN_RIEGO_Z2           6 // INSTRUCCION PARA RUTINA DE RIEGO ZONA 2 FIN
#define INST_RES_RIEGO_Z1           17 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2           18 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2
#define M_INICIO_ARDUINO_OK         50
#define M_INICIO_CENSO              55

#define PRIORIDAD_TEMP            0.05
#define PRIORIDAD_HUM_AMB         0.05
#define PRIORIDAD_HUM_SUELO       0.35
#define PRIORIDAD_LUZ             0.55
#define MAX_TEMP                  50
#define MAX_HUMEDAD_SUELO         1023
#define MAX_LUZ                   1023
#define MAX_HUMEDAD               100

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned int MS_INTERVAL_TO_CENSO = 10000;

static unsigned long currentMillis = 0UL; // Tiempo actual
static unsigned long msParaNuevoCenso = 0UL;  // Tiempo que falta para enviar el censo

//La luz se comporta asi: 1 totalmente iluminado, 1023 totalmente oscuro.
//La humedad del suelo se comporta asi: 1 totalmente humedo, 1023 totalmente seco.

static int valoresCensoAnterior[] = { -1, -1, -1, -1}; //Necesito que sea global, se guarda luego de censar y determinar si censo
/*  Valores de censo anterior 
 *  0 HumedadAmbiente1
 *  1 Luz1
 *  2 HumedadAmbiente2
 *  3 Luz2
 *  Para determinar que valor corresponde a cada zona usar la formula [4 * (ZONA - 1) + INDICE]
 */

void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  DEBUG_PRINT("M_OK"); //Maestro iniciado correctamente

  if (!SD.begin(PIN_CS_SD)) {
    DEBUG_PRINT("E_SD_1"); //Error al inicializar la tarjeta SD
  } else {
    DEBUG_PRINT("SD_2"); //Tarjeta SD incializada correctamente.
    inicializarArchivosDeCensos();
  }
  DEBUG_PRINT("ARDUINO MAESTRO INICIADO CORRECTAMENTE");
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - msParaNuevoCenso) >= MS_INTERVAL_TO_CENSO) {

    DEBUG_PRINT("M_C"); //Maestro envia orden de censar al esclavo
    enviarInstruccionAlEsclavo(INST_CENSO);
    msParaNuevoCenso = millis();
    MS_INTERVAL_TO_CENSO = (unsigned int)45000;
  }

  // [0] => Instruccion
  // [1] => Temp1
  // [2] => HumAmb1
  // [3] => HumSuelo1
  // [4] => Luz
  // [5] => Temp2
  // [6] => HumAmb2
  // [7] => HumSuelo2
  // [8] => Luz2
  int valoresRecibidos[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1};
  leerInstruccionEsclavo(valoresRecibidos); //Proviene del esclavo
  switch (valoresRecibidos[0]) {
    case INST_FIN_CENSO: {
        // Ocurre cuando el esclavo avisa que termino el censo y me envia los valores de ese censo
        // De la zona 1 y de la zona 2
        float perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
        float perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);
        String msg = "";
        msg = msg + "<" + INST_FIN_CENSO + "," + valoresRecibidos[1] + "," + valoresRecibidos[2] + "," + valoresRecibidos[3] + "," + valoresRecibidos[4] + "," + valoresRecibidos[5] + "," + valoresRecibidos[6] + "," + valoresRecibidos[7] + "," + valoresRecibidos[8] + ">";
        DEBUG_PRINT(msg);
        
        DEBUG_PRINT("%EF1 ");
        DEBUG_PRINT(perEfectividadZ1);
        DEBUG_PRINT("%EF2 ");
        DEBUG_PRINT(perEfectividadZ2);
        
        guardarEnArchivo(valoresRecibidos, perEfectividadZ1, perEfectividadZ2);

        if (determinarRiegoEnZona(perEfectividadZ1, valoresRecibidos[4], valoresRecibidos[2], valoresCensoAnterior[0], valoresCensoAnterior[1])) {
          DEBUG_PRINT("RZ1"); //Es eficiente regar en la zona 1
          float varZona1 = obtenerVariableRiego("V1.TXT");
          float vol1 = calcularVolumenRiego(valoresRecibidos[3], varZona1);
          String ret = "";
          ret = ret + "<" + INST_RIEGO_Z1 + "," + vol1 + ",10000>";
          serialSlave.print(ret);
        }

        if (determinarRiegoEnZona(perEfectividadZ2, valoresRecibidos[8], valoresRecibidos[6], valoresCensoAnterior[2], valoresCensoAnterior[3])) {
          DEBUG_PRINT("RZ2"); //Es eficiente regar en la zona 2
          float varZona2 = obtenerVariableRiego("V2.TXT");
          float vol2 = calcularVolumenRiego(valoresRecibidos[7], varZona2);
          String ret = "";
          ret = ret + "<" + INST_RIEGO_Z2 + "," + vol2 + ",10000>";
          serialSlave.print(ret);
        }
        // Guardo los valores para el determinar el riego
        valoresCensoAnterior[0] = valoresRecibidos[2]; // HumedadAmbiente1
        valoresCensoAnterior[1] = valoresRecibidos[4]; // Luz1
        valoresCensoAnterior[2] = valoresRecibidos[6]; // HumedadAmbiente2
        valoresCensoAnterior[3] = valoresRecibidos[8]; // Luz2
        break;
      }
    case INST_RES_RIEGO_Z1: {
      //Aca se analiza el resultado del riego de la zona 1.
      analizarResultadoRiego(1, valoresRecibidos[1], "V1.TXT");
      break;
    }
    case INST_RES_RIEGO_Z2: {
      //Aca se analiza el resultado del riego de la zona 2.
      analizarResultadoRiego(2, valoresRecibidos[1], "V2.TXT");
      break;
    }
    case M_INICIO_ARDUINO_OK: {
      DEBUG_PRINT("Arduino esclavo iniciado correctamente");
      break;
    }
    case M_INICIO_CENSO: {
      DEBUG_PRINT("Arduino esclavo censando...");
      break;
    }
    default: {
      break;
    }
  }
}

void leerInstruccionEsclavo(int* vec) {
  if (serialSlave.available() > 0) {
    char entrada[60];
    for (int i = 0; i < 60; i++) {
      entrada[i] = '\0';
    }
    serialSlave.readBytesUntil('>', entrada, 59);
    DEBUG_PRINT("L_E"); //Leyendo del esclavo
    
    int charIndex = 0;
    char input[4]; // El dato que este entre comas no puede tener una longitud mayor a 4.
    int fieldIndex = 0;
    int i = 0;
    
    while (entrada[i] != '\0' && i < 59) {
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
    entrada[0] = '@'; //Indica que el dato leido proviene del esclavo o del bluetooth
    DEBUG_PRINT(entrada);
  }
}

float obtenerVariableRiego(const char* archivo) {
  File fp = SD.open(archivo);
  char caracter;
  float varRiego = 33.33;
  char input[5];

  if (fp) {
    if (fp.available()) {
      fp.read(input, sizeof(input));
      varRiego = atof(input);
    } else {
      DEBUG_PRINT("E_L_V"); //Error al leer el archivo de variable
    }
  } else {
    String ret = "";
    ret = ret + "E_A_V " + archivo;
    DEBUG_PRINT(ret);
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
    DEBUG_PRINT("E_E_V");
    DEBUG_PRINT(archivo);
  }
  fp.close();
}

void guardarEnArchivo(int* vec, float perEfectividadZ1, float perEfectividadZ2) {
  File fp = SD.open("Z1.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;
    fp.println(ret);
  } else {
    DEBUG_PRINT("E_A_Z1"); //Error al abrir archivo Z1.TXT
  }
  fp.close();

  fp = SD.open("Z2.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + perEfectividadZ2;
    fp.println(ret);
  } else {
    DEBUG_PRINT("E_A_Z2"); //Error al abrir archivo Z2.TXT
  }
  fp.close();
}

float calcularEfectividad(int temp, int humedadAmbiente, int humedadSuelo, int luz) {
  float perTemperatura = (float)temp / MAX_TEMP;
  float perHumedadAmbiente = (float)humedadAmbiente / MAX_HUMEDAD;
  float perHumedadSuelo = (float)humedadSuelo / MAX_HUMEDAD_SUELO;
  float perLuz = (float)luz / MAX_LUZ;
  DEBUG_PRINT("T:");
  DEBUG_PRINT(perTemperatura);
  DEBUG_PRINT("HA:");
  DEBUG_PRINT(perHumedadAmbiente);
  DEBUG_PRINT("SS:");
  DEBUG_PRINT(perHumedadSuelo);
  DEBUG_PRINT("NL:");
  DEBUG_PRINT(perLuz);

  DEBUG_PRINT("V_PRIORIZADOS");
  DEBUG_PRINT("T:");
  DEBUG_PRINT(perTemperatura * PRIORIDAD_TEMP);
  DEBUG_PRINT("HA:");
  DEBUG_PRINT(perHumedadAmbiente * PRIORIDAD_HUM_AMB);
  DEBUG_PRINT("SS:");
  DEBUG_PRINT(perHumedadSuelo * PRIORIDAD_HUM_SUELO);
  DEBUG_PRINT("NL:");
  DEBUG_PRINT(perLuz * PRIORIDAD_LUZ);
  return (PRIORIDAD_TEMP * (perTemperatura) + PRIORIDAD_HUM_AMB * (1 - perHumedadAmbiente) + PRIORIDAD_HUM_SUELO * (perHumedadSuelo) + PRIORIDAD_LUZ * (perLuz)) * 100;
}

int determinarRiegoEnZona(float perEfectividad, int luzActual, int humedadActual, int humedadAnterior, int luzAnterior) {
  if (luzAnterior == -1 || humedadAnterior == -1){
    //Es la primera vez que censa, no hay valores anteriores
    DEBUG_PRINT("NO_E_DATOS_PREV");
    return 0;
  }

  if (perEfectividad > 70.00) {
    DEBUG_PRINT("PER_EFE_MUY_ALTO");
    return 1;
  }
  else if (perEfectividad > 50.00) {
    float varLuz = ((float)(luzActual - luzAnterior) / (float)luzAnterior);
    if (varLuz < 0) {
      DEBUG_PRINT("L_DESC"); //La luz se encuentra en descenso
    } else if (varLuz < 60.00) {
      DEBUG_PRINT("L_EST"); //La luz se mantiene estable
    } else {
      DEBUG_PRINT("L_ASC"); //La luz esta en ascenso, no es conveniente regar
      return 0;
    }

    float varHum =  ((float)(humedadActual - humedadAnterior) / (float)humedadAnterior);
    if (varHum < 0) {
      DEBUG_PRINT("H_DESC"); //La humedad se encuentra en descenso
    } else if (varHum < 60.00) {
      DEBUG_PRINT("H_EST"); //La humedad se mantiene estable
    } else {
      DEBUG_PRINT("H_ASC"); //La humedad esta en ascenso, no es conveniente regar
      return 0;
    }
  } else {
    DEBUG_PRINT("PER_EFE_MUY_BAJO"); //Porcentaje de efectividad por debajo del minimo
    return 0;
  }

  return 1; //En esta instancia, la luz y la humedad son propicias para regar
}

float calcularVolumenRiego(int riego, float var) {
  DEBUG_PRINT("INTESIDAD DE RIEGO: ");
  DEBUG_PRINT((riego * var) / 1023);
  return (riego * var) / 1023;
}

void inicializarArchivosDeCensos() {
  File fp;
  if (SD.exists("Z1.TXT")) {
    SD.remove("Z1.TXT");
  }

  if (SD.exists("Z2.TXT")) {
    SD.remove("Z2.TXT");
  }
  fp = SD.open("Z1.TXT", FILE_WRITE);
  if (!fp) {
    DEBUG_PRINT("E_A_1"); //Error al crear archivo Z1.TXT
  }
  fp.close();

  fp = SD.open("Z2.TXT", FILE_WRITE);
  if (!fp) {
    DEBUG_PRINT("E_A_2"); //Error al crear archivo Z2.TXT
  }
  fp.close();

  if (!SD.exists("V1.txt")) {
    DEBUG_PRINT("A_V_1"); //El archivo V1.TXT no existe en la tarjeta SD
    fp = SD.open("V1.txt", FILE_WRITE);
    DEBUG_PRINT("A_V_2"); //El archivo V1.TXT no existia y se acaba de crear
    if (fp) {
      fp.println("33.33");
    } else {
      DEBUG_PRINT("E_A_1"); //Error al escribir el archivo V1.txt con el valor por defecto
    }
    fp.close();
  }

  if (!SD.exists("V2.txt")) {
    DEBUG_PRINT("A_V_3"); //El archivo V2.TXT no existe en la tarjeta SD
    fp = SD.open("V2.txt", FILE_WRITE);
    DEBUG_PRINT("A_V_4"); //El archivo V2.TXT no existia y se acaba de crear
    if (fp) {
      fp.println("33.33");
    } else {
      DEBUG_PRINT("E_A_2"); //Error al escribir el archivo V2.txt con el valor por defecto
    }
    fp.close();
  }
}

void analizarResultadoRiego(int zona, int humedadSuelo, const char* archivo) {
  float var = 0.0;
  float humedadSueloZona = humedadSuelo;
  String ret = "";
  ret = ret + "I_R_" + zona;
  DEBUG_PRINT(ret); //Se va a analizar el resultado del riego de la ZONA N
  ret = "";
  ret = ret + "HUMSUELO RESULTANTE: " + humedadSueloZona;
  DEBUG_PRINT(ret);
  float perHumedadSueloZona = (100 - (humedadSueloZona * 100) / 1023);
  if (perHumedadSueloZona < 40 || perHumedadSueloZona > 60) {
    ret = "";
    ret = ret + "PH_Z" + zona;
    DEBUG_PRINT(ret); //Porcentaje de humedad resultado del ultimo riego ZONA N
    ret = "";
    ret = ret + "%HUMSUELO RESULTANTE: " + perHumedadSueloZona;
    DEBUG_PRINT(ret);
    var = obtenerVariableRiego(archivo);
    ret = "";
    ret = ret + "V_PH_Z" + zona;
    DEBUG_PRINT(ret); //Variable de riego con la que se rego ZONA N
    DEBUG_PRINT(var);
    float ajuste = (50 - perHumedadSueloZona) / 2;
    var += ajuste;
    if (var < 0.0) {
      var = 1.00;
    }
    if (var > 100.00) {
      var = 100.00;
    }
    ret = "";
    ret = ret + "N_V_PH_Z" + zona;
    DEBUG_PRINT(ret); //Nueva variable de riego a almacenar ZONA 1
    DEBUG_PRINT(var);
    escribirVariableRiego(var, archivo);
  } else {
    ret = "";
    ret = ret + "R_C_Z" + zona;
    DEBUG_PRINT(ret); //Porcentaje de humedad en ZONA 1 correcto
  }
}

void enviarInstruccionAlEsclavo(const int instruccion) {
  String orden = "";
  orden = orden + '<' + instruccion + '>';
  serialSlave.print(orden);
  DEBUG_PRINT(orden);
}
