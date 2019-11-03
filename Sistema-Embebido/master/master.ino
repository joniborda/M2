#include <SD.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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
#define INST_MANTENIMIENTO          7 // INSTRUCCION PARA INICIO RUTINA DE MANTENIMIENTO
#define INST_RES_MANTENIMIENTO      8 // INSTRUCCION PARA RESPUESTA DE RUTINA DE MANTENIMIENTO
#define INST_DETENER_RIEGO_Z1       9 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 1
#define INST_DETENER_RIEGO_Z2       10 // INSTRUCCION PARA DETENER EL RIEGO DE LA ZONA 2
#define INST_ENCENDER_LUZ_1_MANUAL  11 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_ENCENDER_LUZ_2_MANUAL  12 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_APAGAR_LUZ_1_MANUAL    13 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_APAGAR_LUZ_2_MANUAL    14 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_AUTO_LUZ_1             15 // INSTRUCCION PARA ENCENDER LUZ 1 MANUALMENTE
#define INST_AUTO_LUZ_2             16 // INSTRUCCION PARA ENCENDER LUZ 2 MANUALMENTE
#define INST_RES_RIEGO_Z1           17 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 1
#define INST_RES_RIEGO_Z2           18 // INSTRUCCION PARA ENVIAR EL RESULTADO DEL RIEGO EN LA ZONA 2
#define INST_INICIO_CONEXION_BT     19 // INSTRUCCION QUE ENVIA EL BLUETOOTH AVISANDO QUE SE CONECTO
#define INST_CENSO_MANUAL           20 // INSTRUCCION PARA COMENZAR UN CENSO MANUAL
#define INST_RIEGO_MANUAL           21 // INSTRUCCION PARA COMENZAR UN RIEGO MANUAL
#define INST_TIPO_RIEGO_CONT        22 // INSTRUCCION PARA CAMBIAR EL TIPO DE RIEGO A CONTINUO
#define INST_TIPO_RIEGO_INTER       23 // INSTRUCCION PARA CAMBIAR EL TIPO DE RIEGO A INTERMITENTE
#define INST_FIN_RIEGO_MANUAL       24 // INSTRUCCION QUE INDICA QUE SE FINALIZO EL RIEGO MANUAL
#define INST_DETENER_RIEGO_MANUAL   25 // INSTRUCCION QUE DETIENE EL RIEGO MANUAL

#define M_INICIO_ARDUINO_OK         50
#define M_INICIO_RIEGO_M            54
#define M_STOP_RIEGO_GRAL_OK        61
#define M_CAMBIO_T_RIEGO_CONT       63
#define M_CAMBIO_T_RIEGO_INT        64

#define PRIORIDAD_TEMP 0.05
#define PRIORIDAD_HUM_AMB 0.05
#define PRIORIDAD_HUM_SUELO 0.3
#define PRIORIDAD_LUZ 0.6
#define MAX_TEMP 50
#define MAX_HUMEDAD_SUELO 1023
#define MAX_LUZ 1023
#define MAX_HUMEDAD 100

SoftwareSerial serialSlave(PUERTO_RX_SLAVE, PUERTO_TX_SLAVE);

LiquidCrystal_I2C lcd(0x3F,16,4); 

// INTERVALO DE RUTINA DE CENSO EN MS
static unsigned int MS_INTERVAL_TO_CENSO = 10000;

static unsigned long currentMillis = 0; // tiempo actual
static unsigned long msParaNuevoCenso = 0;  // tiempo que falta para enviar el censo

//La luz se comporta asi: 1 totalmente iluminado, 1023 totalmente oscuro.
//La humedad del suelo se comporta asi: 1 totalmente humedo, 1023 totalmente seco.

static bool mantenimientoEnCurso = false;
static int valoresCensoAnterior[] = { -1, -1, -1, -1}; //Necesito que sea global, se guarda luego de censar y determinar si censo
/*  Valores de censo anterior 
 *  0 Temp1
 *  1 HumAmb1
 *  2 Temp2
 *  3 HumAmb2
 *  Para determinar que valor corresponde a cada zona usar la formula [4 * (zona - 1) + indice]
 */
void setup() {
  serialSlave.begin(9600);
  Serial.begin(9600);
  Serial.println("M_OK"); //Maestro iniciado correctamente

  if (!SD.begin(PIN_CS_SD)) {
    Serial.println("E_SD_1"); //Error al inicializar la tarjeta SD
  } else {
    Serial.println("SD_2"); //Tarjeta SD incializada correctamente.
    inicializarArchivosDeCensos();
  }

  lcd.init();
  lcd.backlight();
}

void loop() {
  currentMillis = millis();
  if ((unsigned long)(currentMillis - msParaNuevoCenso) >= MS_INTERVAL_TO_CENSO) {

    Serial.println("M_C"); //Maestro envia orden de censar al esclavo
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
    case INST_CENSO: {
        // Implementar guardar que el esclavo inicio el censo
        break;
      }
    case INST_FIN_CENSO: {
        // Ocurre cuando el esclavo avisa que termino el censo y me envia los valores de ese censo
        // de la zona 1 y de la zona 2
        float perEfectividadZ1 = calcularEfectividad(valoresRecibidos[1], valoresRecibidos[2], valoresRecibidos[3], valoresRecibidos[4]);
        float perEfectividadZ2 = calcularEfectividad(valoresRecibidos[5], valoresRecibidos[6], valoresRecibidos[7], valoresRecibidos[8]);
        String msg = "";
        msg = msg + "<" + INST_FIN_CENSO + "," + valoresRecibidos[1] + "," + valoresRecibidos[2] + "," + valoresRecibidos[3] + "," + valoresRecibidos[4] + "," + valoresRecibidos[5] + "," + valoresRecibidos[6] + "," + valoresRecibidos[7] + "," + valoresRecibidos[8] + ">";
        Serial.println(msg);
        
        Serial.print("%EF1 ");
        Serial.println(perEfectividadZ1);
        Serial.print("%EF2 ");
        Serial.println(perEfectividadZ2);
        
        guardarEnArchivo(valoresRecibidos, perEfectividadZ1, perEfectividadZ2);

        if (determinarRiegoEnZona(perEfectividadZ1, valoresRecibidos[4], valoresRecibidos[2], valoresCensoAnterior[0], valoresCensoAnterior[1])) {
          Serial.println("RZ1"); //Es eficiente regar en la zona 1
          float varZona1 = obtenerVariableRiego("V1.TXT");
          float vol1 = calcularVolumenRiego(valoresRecibidos[3], varZona1);
          String ret = "";
          // cambiar para pasar el el porcentaje y el tiempo de riego en ms
          // si le envio el tiempo se va a pisar con el que le dijo el bluetooth
          ret = ret + "<" + INST_RIEGO_Z1 + "," + vol1 + ",10000>";
          serialSlave.print(ret);
          Serial.println(ret);
          //Probar si el bluetooth recibe correctamente la orden de empezo a regar
        }

        if (determinarRiegoEnZona(perEfectividadZ2, valoresRecibidos[8], valoresRecibidos[6], valoresCensoAnterior[2], valoresCensoAnterior[3])) {
          Serial.println("RZ2"); //Es eficiente regar en la zona 2
          float varZona2 = obtenerVariableRiego("V2.TXT");
          float vol2 = calcularVolumenRiego(valoresRecibidos[7], varZona2);
          String ret = "";
          // cambiar para pasar el el porcentaje y el tiempo de riego en ms
          // si le envio el tiempo se va a pisar con el que le dijo el bluetooth
          ret = ret + "<" + INST_RIEGO_Z2 + "," + vol2 + ",10000>";
          serialSlave.print(ret);
          Serial.println(ret);
          //Probar si el bluetooth recibe correctamente la orden de empezo a regar
        }
        // Guardo los valores para el proximo censo
        valoresCensoAnterior[0] = valoresRecibidos[1]; // temp1
        valoresCensoAnterior[1] = valoresRecibidos[3]; // suelo1
        valoresCensoAnterior[2] = valoresRecibidos[5]; // temp2
        valoresCensoAnterior[3] = valoresRecibidos[7]; // suelo2
        break;
      }
    case INST_MANTENIMIENTO: {
        // El esclavo le avisa que empezo el mantenimiento
        mantenimientoEnCurso = true;
        break;
      }
    case INST_RES_MANTENIMIENTO: {
        // El esclavo le avisa que termino el mantenimiento
        mantenimientoEnCurso = false;
        break;
      }
    case INST_FIN_RIEGO_Z1: {
        // Ocurre cuando el esclavo me avisa que termino de regar la zona 1
        String ret = "";
        ret = ret + "<" + INST_FIN_RIEGO_Z1 + ">";
        Serial.println(ret);
        break;
      }
    case INST_FIN_RIEGO_Z2: {
        // Ocurre cuando el esclavo me avisa que termino de regar la zona 2
        String ret = "";
        ret = ret + "<" + INST_FIN_RIEGO_Z2 + ">";
        Serial.println(ret);
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
      lcd.setCursor(0,0);
      lcd.print("Hello, world!");
      break;
    }
    case M_INICIO_RIEGO_M: {
      break;
    }
    case M_STOP_RIEGO_GRAL_OK: {
      break;
    }
    case M_CAMBIO_T_RIEGO_CONT: {
      break;
    }
    case M_CAMBIO_T_RIEGO_INT: {
      break;
    }
    case INST_ENCENDER_LUZ_1_MANUAL: {
      break;
    }
    case INST_ENCENDER_LUZ_2_MANUAL: {
      break;
    }
    case INST_APAGAR_LUZ_1_MANUAL: {
      break;
    }
    case INST_APAGAR_LUZ_2_MANUAL: {
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
    Serial.println("L_E"); //Leyendo del esclavo
    
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
    Serial.println(entrada);
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
  File fp = SD.open("Z1.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[1] + "," + vec[2] + "," + vec[3] + "," + vec[4] + "," + perEfectividadZ1;
    fp.println(ret);
  } else {
    Serial.println("E_A_Z1"); //Error al abrir archivo Z1.TXT
  }
  fp.close();

  fp = SD.open("Z2.TXT", FILE_WRITE);
  if (fp) {
    String ret = "";
    ret = ret + vec[5] + "," + vec[6] + "," + vec[7] + "," + vec[8] + "," + perEfectividadZ2;
    fp.println(ret);
  } else {
    Serial.println("E_A_Z2"); //Error al abrir archivo Z2.TXT
  }
  fp.close();
}

float calcularEfectividad(int temp, int humedadAmbiente, int humedadSuelo, int luz) {
  float perTemperatura = (float)temp / MAX_TEMP;
  float perHumedadAmbiente = (float)humedadAmbiente / MAX_HUMEDAD;
  float perHumedadSuelo = (float)humedadSuelo / MAX_HUMEDAD_SUELO;
  float perLuz = (float)luz / MAX_LUZ;
  Serial.print("T:");
  Serial.println(perTemperatura);
  Serial.print("HA:");
  Serial.println(perHumedadAmbiente);
  Serial.print("SS:");
  Serial.println(perHumedadSuelo);
  Serial.print("NL:");
  Serial.println(perLuz);

  Serial.println("V_PRIORIZADOS");
  Serial.print("T:");
  Serial.println(perTemperatura * PRIORIDAD_TEMP);
  Serial.print("HA:");
  Serial.println(perHumedadAmbiente * PRIORIDAD_HUM_AMB);
  Serial.print("SS:");
  Serial.println(perHumedadSuelo * PRIORIDAD_HUM_SUELO);
  Serial.print("NL:");
  Serial.println(perLuz * PRIORIDAD_LUZ);
  return (PRIORIDAD_TEMP * (perTemperatura) + PRIORIDAD_HUM_AMB * (1 - perHumedadAmbiente) + PRIORIDAD_HUM_SUELO * (perHumedadSuelo) + PRIORIDAD_LUZ * (perLuz)) * 100;
}

int determinarRiegoEnZona(float perEfectividad, int luzActual, int humedadActual, int humedadAnterior, int luzAnterior) {
  if (perEfectividad > 70.00) {
    return 1;
  }
  else if (perEfectividad > 40.00) {
 
    if (luzAnterior == -1 || humedadAnterior == -1) //Es la primera vez que censa, no hay valores anteriores
      return 0;
    float varLuz = ((float)(luzActual - luzAnterior) / (float)luzAnterior);
    if (varLuz < 0) {
      Serial.println("L_DESC");//La luz se encuentra en descenso
      return 1;
    } else if (varLuz < 60.00) {
      Serial.println("L_EST");//La luz se mantiene estable
      return 1;
    } else {
      Serial.println("L_ASC");//La luz esta en ascenso, no es conveniente regar
    }
  }
  Serial.println("PE_BAJO");//Porcentaje de efectividad por debajo del minimo
  return 0;
}

float calcularVolumenRiego(int riego, float var) {
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
    Serial.println("E_A_1"); //Error al crear archivo Z1.TXT
  }
  fp.close();

  fp = SD.open("Z2.TXT", FILE_WRITE);
  if (!fp) {
    Serial.println("E_A_2"); //Error al crear archivo Z2.TXT
  }
  fp.close();

  if (!SD.exists("V1.txt")) {
    Serial.println("A_V_1"); //El archivo V1.TXT no existe en la tarjeta SD
    fp = SD.open("V1.txt", FILE_WRITE);
    Serial.println("A_V_2"); //El archivo V1.TXT no existia y se acaba de crear
    if (fp) {
      fp.println("33.33");
    } else {
      Serial.println("E_A_1"); //Error al escribir el archivo V1.txt con el valor por defecto
    }
    fp.close();
  }

  if (!SD.exists("V2.txt")) {
    Serial.println("A_V_3"); //El archivo V2.TXT no existe en la tarjeta SD
    fp = SD.open("V2.txt", FILE_WRITE);
    Serial.println("A_V_4"); //El archivo V2.TXT no existia y se acaba de crear
    if (fp) {
      fp.println("33.33");
    } else {
      Serial.println("E_A_2"); //Error al escribir el archivo V2.txt con el valor por defecto
    }
    fp.close();
  }
}

void analizarResultadoRiego(int zona, int humedadSuelo, const char* archivo) {
  float var = 0.0;
  float humedadSueloZona = humedadSuelo;
  String ret = "";
  ret = ret + "I_R_" + zona;
  Serial.println(ret); //Se va a analizar el resultado del riego de la ZONA N
  ret = "";
  ret = ret + "HSuelo res: " + humedadSueloZona;
  Serial.println(ret);
  float perHumedadSueloZona = (100 - (humedadSueloZona * 100) / 1023);
  if (perHumedadSueloZona < 40 || perHumedadSueloZona > 60) {
    ret = "";
    ret = ret + "PH_Z" + zona;
    Serial.println(ret); //Porcentaje de humedad resultado del ultimo riego ZONA N
    ret = "";
    ret = ret + "%HSuelo res: " + perHumedadSueloZona;
    Serial.println(ret);
    var = obtenerVariableRiego(archivo);
    ret = "";
    ret = ret + "V_PH_Z" + zona;
    Serial.println(ret); //Variable de riego con la que se rego ZONA N
    Serial.println(var);
    float ajuste = (50 - perHumedadSueloZona) / 2;
    var += ajuste;
    if (var < 0)
      var = 1.00;
    if (var > 100)
      var = 100.00;
    ret = "";
    ret = ret + "N_V_PH_Z" + zona;
    Serial.println(ret); //Nueva variable de riego a almacenar ZONA 1
    Serial.println(var);
    escribirVariableRiego(var, archivo);
  } else {
    ret = "";
    ret = ret + "R_C_Z" + zona;
    Serial.println(ret); //Porcentaje de humedad en ZONA 1 correcto
  }
}

void enviarInstruccionAlEsclavo(const int instruccion) {
  String orden = "";
  orden = orden + '<' + instruccion + '>';
  serialSlave.print(orden);
  Serial.println(orden); // Cuando se conecte el modulo bluetooth, esto puede salir por el dispositivo.
}
