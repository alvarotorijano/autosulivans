//      ___    __                          ______           _   _                 
//     /   |  / /   ______ __________     /_  __/___  _____(_) (_)___ _____  ____ 
//    / /| | / / | / / __ `/ ___/ __ \     / / / __ \/ ___/ / / / __ `/ __ \/ __ \
//   / ___ |/ /| |/ / /_/ / /  / /_/ /    / / / /_/ / /  / / / / /_/ / / / / /_/ /
//  /_/  |_/_/ |___/\__,_/_/   \____/    /_/  \____/_/  /_/_/ /\__,_/_/ /_/\____/ 
//                                                       /___/                    
//alvarotorijano@gmail.com

/*
           _                                                                                                                              
           \`*-.                                                                                                                          
            )  _`-.                                                                                                                                        .-.   .-.
           .  : `. .                       _         _          ____        _ _ _                 _       _____             _                             /   \ /   \ 
           : _   '  \                     / \  _   _| |_ ___   / ___| _   _| (_) |__   __ _ _ __ ( )___  |  ___|__  ___  __| | ___ _ __               .-. |    |    | .-.
           ; *` _.   `*-._               / _ \| | | | __/ _ \  \___ \| | | | | | '_ \ / _` | '_ \|// __| | |_ / _ \/ _ \/ _` |/ _ \ '__|             /   \ \  / \  / /   \
           `-.-'          `-.           / ___ \ |_| | || (_) |  ___) | |_| | | | |_) | (_| | | | | \__ \ |  _|  __/  __/ (_| |  __/ |                |   |  '`.-.`'  |   |
             ;       `       `.        /_/   \_\__,_|\__\___/  |____/ \__,_|_|_|_.__/ \__,_|_| |_| |___/ |_|  \___|\___|\__,_|\___|_|                 \_.' .-`   `-. '._/
             :.       .        \                                                                                                                        .-'         '-.
             . \  .   :   .-'   .                                                                                                                      /               \
             '  `+.;  ;  '      :                                                                                                                      |               |
             :  '  |    ;       ;-.                                                                                                                     \             /
             ; '   : :`-:     _.`* ;                                                                                                                     '.___...___.'
   [Suli] .*' /  .*' ; .*`- +'  `*'                                                                                                       
          `*-*   `*-*  `*-*'                               
*/


#define VERSION_DEL_COMEDERO "V-1.6"
#define SPLASH                2.0      //tiempo en segundos que se mostrara el mensaje de bienvenida
#define TIEMPO_BRILLO         5.0      //Tiempo en segundos que se mantendra la pantalle encendida al mostrar un mensaje
#define TIEMPO_MENSAJE        3.5      //Tiempo que se mantendra en la pantalla el mensaje
#define MILLIS_DIARIOS        86400000 // cantidad de milisegundos que tiene un dia
#define espera 5

//COMPILACION CONDICIONAL PARA DEBUG DESDE LA INTERFAZ SERIE//
//----------------------------------------------------------//
//#define DEBUG //si esta definido se usa la interfaz serial como entrada de comandos, si no lo esta, se usara el bluetooth.
//----------------------------------------------------------//


#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

//Estas son las definiciones de las direcciones de memoria

#define ADDR_A_SECOND     0x00
#define ADDR_A_MINUTE     0x01
#define ADDR_A_HOUR       0x02
#define ADDR_A_dayOfMonth 0x03
#define ADDR_A_MONTH      0x04
#define ADDR_A_YEAR       0x05
#define ADDR_A_TEMP       0x06
#define ADDR_A_TEMP_DEC   0x07
#define ADDR_S_HORA       0x08
#define ADDR_S_MINUT      0x09
#define ADDR_A_DOSIS      0x0A
#define ADDR_S_DOSIS      0x0B
#define ADDR_A_GRAMOS     0x0C
#define ADDR_S_GRAMOS     0x0D

//Estas son las definiciones del hardware

#define DS3231_I2C_ADDRESS  0x68
#define LCD16X2_I2C_ADDRESS 0x3f

//Led que indicara cuando esta girando la tolva

#define LED_PIN 13

//Estas son las macros para la retroiluminacion del LCD
#define ON  1
#define OFF 0

//Estas son las definiciones del motor paso a paso

#define STEPPER_ENABLE 4
#define STEPPER_STEP   3 //Los pines van cambiados en la placa perforada
#define STEPPER_DIR    2
#define PASOS_POR_VUELTA 200

//Estas son las macros para el motor paso a paso
#define VUELTA true
#define CONTRAVUELTA false

//Estas son las declaraciones del hardware

//Bluetooth
#define BT_RX A2
#define BT_TX A3
  SoftwareSerial BT(BT_RX, BT_TX); // RX, TX recorder que se cruzan

//Lcd 16X2
  //                     Addr,                En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity
  LiquidCrystal_I2C lcd( LCD16X2_I2C_ADDRESS,  2,  1,  0,  4,  5,  6,  7,           3, POSITIVE );


//PROTOTIPOS//
//----------//

  void resetearComedero();
  void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
  void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
  void displayTime();
  byte decToBcd(byte val);
  byte bcdToDec(byte val);
  void encenderLCD();
  void mostrarMensaje();
  float getTemperature();
  byte comparaOpcion(char *, String *, byte *, byte, byte *);
  void girarTolva (int vueltas, bool sentido);
  void dar_de_comer();
  
//VARIABLES GLOBLALES//
//-------------------//
  char nombre[32] = "Sulivan";
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year; //Fecha y hora actuales
  byte A_second, A_minute, A_hour, A_dayOfMonth, A_month, A_year; //Fecha y hora de la ultima vez que se alimento
  byte S_hora, S_minuto, S_dosis, A_dosis, S_gram, A_gram; //Hora a la que esta marcada la alimentacion, dosis, gramos que dispensa el comedero
  float A_temperatura;
  byte sec_anterior = 0;
  unsigned long iluminacion = millis();
  unsigned long mensaje = millis();
  bool flag_luz = true;

//TEXTOS//
//------//
  const PROGMEM char cad_separador[]       = {"\n-----------------------------------------------\n"};
  const PROGMEM char manual[]              = {"Este es el manual del comedero.\n----------------------------------------\nResponde a los siguientes comandos:\nUltimo\nComida\nHora\nDosis\nGramos\nPeso\nReset\nLuz\nTest\nVelocidad\nHistorico\nMan\n\nPara mostrar el uso de los comandos\nescribir: Man [comando]. P.Ej.: Man Luz\n**No distingue mayusculas."};
  const PROGMEM char man_ultimo[]          = {"NOMBRE\n\tUltimo: muestra la fecha, la hora y la\ntemperatura a la que se le dio de comer\npor ultima vez al gato\n\nSinopsis:\n\t No necesita argumentos."};  
  const PROGMEM char man_comida[]          = {"NOMBRE:\n\tDefine la hora a la que se suminsitra la\ncomida al gato.\n\nSinopsis:\n\tcomida hh:mm\n\nhh = hora en formato 24h y dos cifras.\nmm = minuto en formato de dos cifras.\nP.ej.: comida 14:05"};                                                                                                     
  const PROGMEM char man_hora[]            = {"NOMBRE:\n\tHora: ajusta la hora y fecha \n\tdel reloj.\n\nSinopsis:\n\thora hh:mm:ss dd:mm:aaaa w\n\n\thh: hora en formato 24h y dos cifras.\n\tmm: minutos en formato de dos cifras.\n\tss: segundos en formato de dos cifras.\n\tdd: dias en formato de dos cifras.\n\tmm: mes en formato de dos cifras.\n\taa: a"};
  const PROGMEM char man_hora2[]           = {"o en formato de dos cifras.\n\tw: dia de la semana. 1-7\n\nP.ej.: hora 14:30:00 01/04/2016 5"};
  const PROGMEM char man_dosis[]           = {"NOMBRE:\n\tDosis: permite establecer manualmente el\n\tnumero de dosis que se suministraran\n\nSinopsis:\n\tDosis xx\n\tP.ej.:Dosis 5\n\t-Valores aceptados: 0 - 50"};
  const PROGMEM char man_gramos[]          = {"NOMBRE:\n\tGramos: especifica el numero de gramos\n\tque suministra el comedero con cada\n\tdosis en su actual configuracion.\n\nSinopsis:\n\tgramos xxx\n\tP.ej.: Gramos 15\n\t-Valores aceptados: 10 - 250"};
  const PROGMEM char man_peso[]            = {"NOMBRE:\n\tPeso: permite especificar el peso en\n\tgramos del gato, de tal forma que el\n\tcomedero pueda calcular el numero de  \n\tdosis que hay quesuministrar para que \n\tel gato coma.\n\nSinopsis:\n\tPeso xxxxx\n\t-Valores aceptados: 001 - 15000\n\t-Por debajo de 1000gr se considera que\n\tel gato pesa un kilo y por encima de \n\t15000gr que el animal sufre sobrepeso.\n\tP.ej.: Gramos 10600"};
  const PROGMEM char man_reset[]           = {"NOMBRE:\n\tReset: permite reiniciar el comedero\n\nSinopsis:\n\tReset\n\tNo necesita argumentos"};
  const PROGMEM char man_luz[]             = {"NOMBRE:\n\tLuz: permite mantener la luz encendida\n\tCambia entre el modo autogestionado de\n\tla luz o la mantiene encendida.\n\nSinopsis:\n\tLuz\n\tNo necesita argumentos"};
  const PROGMEM char man_test[]            = {"NOMBRE:\n\tTest: funcion implementada con el fin de\n\tprobar el correcto funcionamiento.\n\nSinopsis:\n\tTest []\n\tVuelta: Hace girar la tolva una vez.\n\tContravuelta: Gira la tolva en sentido \n\t\t      contrario.\n\t<Vacio>: Suministra una comida completa\n\nEjemplo:\nTest Contravuelta"};
  const PROGMEM char man_velocidad[]       = {"NOMBRE:\n\tVelocidad: permite definir la velocidad\n\ta la que girara la tolva, pues hay que\n\ttener en cuenta que puesto que la comida\n\testa formada por granos, la velocidad\n\tinfluira en la cantidad de comida que se\n\tsuministre, de tal forma que cuanto\n\tmayor sea la velodad menos comida caera.\n\tSe recomienda hacer varias mediciones.\n\nSinopsis:\n\tvelocidad xxx%%\n\txxx = porcentaje de velocidad\n\tValores aceptados 1 - 100.\n\tEl simbolo %% es opcional.\n\tP.ej.: Velocidad 60%%"};
  const PROGMEM char man_man[]             = {"NOMBRE:\n\tMan: muestra el manual del comedero asi \n\tcomo de los comandos disponibles.\n\nSinopsis:\n\tman [comando]\n\tvacio muestra los comandos aceptados.\n\tSeguido de un comando muestra la\n\tdescripcion y uso de ese comando.\n\tP.ej.: Man Peso"};
  const PROGMEM char man_historico[]       = {"NOMBRE:\n\tHistorico: mustra el registro de las\n\tveces que se le ha dado de comer al gato\n\tasi como permite borrar dichos registros\n\nSinopsis:\n\tHistorico (Borrar)\n\tSi se utiliza sin argumento muestra los\n\tregistros\n\tSi se utiliza seguido de la palabra \n\t \"borrar\" borrara los registros\n\tP.ej.: Historico Borrar"};
  const PROGMEM char bienvenida[]          = {"\nAlvaro Torijano Garcia\n---Consola de depuracion del comedero---"};
  const PROGMEM char sobrepeso[]           = {"\nEste gato sufre sobrepeso, no se dispensara ninguna cantidad.\nPor favor visita a un veterinario"};
  const PROGMEM char comando_invalido[]    = {"\nNo se ha detectado un comando valido"};
  const PROGMEM char l_auto[]              = {"\nLuz Autogestionada"};
  const PROGMEM char l_fija[]              = {"\nLuz fijada en encendido"};
  const PROGMEM char ultima_alimentacion[] = {"\nUltima fecha de alimentacion autonoma"};
  const PROGMEM char hora_invalido[]       = {"\nFormato de hora INVALIDO"};
  const PROGMEM char hora_comida[]         = {"\nLa nueva hora de la comida son las:"};
  const PROGMEM char dosis_invalida[]      = {"\nNo es una dosis valida"};
  const PROGMEM char reloj_actualizado[]   = {"\nSe ha actualizado el reloj con los siguientes datos:\n"};
  const PROGMEM char demasiado_peso[]      = {"\nEso es demasiado peso para un gato"};
  const PROGMEM char fech_o_invalid[]      = {"\nFormato de hora o fecha incorrecto"};
  const PROGMEM char suministraran[]       = {"\nSe suministraran: "};
  const PROGMEM char cantidad_invalida[]   = {"\nNo es una cantidad valida"};
  const PROGMEM char suministran[]         = {"\nSe suministran: "};
  const PROGMEM char c_dosis[]             = {" dosis"};
  const PROGMEM char gramos_dosis[]        = {" gramos por dosis"};
  const PROGMEM char p_invalido[]          = {"\nNo es un peso valido"};
  const PROGMEM char suministraron[]       = {"\nSe suministraron: "};
  const PROGMEM char cad_gramos[]          = {" Gramos"};
  
//ICONO DE LA TEMPERATURA//
//------//
byte grados[8] = {
    B00110, // este es el icono de º  //
    B01001,                           //
    B01001,                           //
    B00110,
    B00000,
    B00000,
    B00000,
    B00000,

    /*
    B01000,  //Este es el icono de ºc
    B10100,
    B01000,
    B00011,
    B00100,
    B00100,
    B00100,
    B00011,*/
    
    /*
    B00100,  //Este es el icono del termometro
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110,*/
};


//EL SETUP//
//--------//

void setup()
   {
       //Se configura los pines que controlan el motor
        pinMode( STEPPER_ENABLE, OUTPUT);
        pinMode( STEPPER_STEP, OUTPUT);
        pinMode( STEPPER_DIR, OUTPUT);
        pinMode( LED_PIN, OUTPUT);
        digitalWrite( STEPPER_ENABLE, HIGH);

       //Configuro el resto de pines como salidas para aliviar la estatica
       //for(i=0; i<14; i++){
       // if(!(i== STEPPER_ENABLE) || (i==STEPPER_STEP) || (i==STEPPER_DIR) || (i== LED_PIN))
       //   pinMode(i, OUTPUT);
       //}
       
       //Arranco las interfaces serie
       #ifdef DEBUG
       Serial.begin(115200);
       while(!Serial);
       #else
       while(!BT);
       BT.begin(9600);
       #endif

       //Inicializo la pantalla
       lcd.begin(16,2);
       lcd.backlight();
       lcd.createChar(1,grados);
       Wire.beginTransmission(DS3231_I2C_ADDRESS);
       Wire.write(0x07); // move pointer to SQW address
       Wire.write(0x10); // sends 0x10 (hex) 00010000 (binary) to control register - turns on square wave
       Wire.endTransmission();

       //Se muestran los mensajes de bienvenida en las consolas
       
       mostrarCadena(bienvenida);

       //Ahora se carga la ultima vez que se alimento al gato

       A_second      = EEPROM.read(ADDR_A_SECOND);
       A_minute      = EEPROM.read(ADDR_A_MINUTE);
       A_hour        = EEPROM.read(ADDR_A_HOUR);
       A_dayOfMonth  = EEPROM.read(ADDR_A_dayOfMonth);
       A_month       = EEPROM.read(ADDR_A_MONTH);
       A_year        = EEPROM.read(ADDR_A_YEAR);
       A_temperatura = EEPROM.read(ADDR_A_TEMP_DEC);
       A_temperatura = A_temperatura / 100;
       A_temperatura = A_temperatura + EEPROM.read(ADDR_A_TEMP);
       A_dosis = EEPROM.read(ADDR_A_DOSIS);
       A_gram = EEPROM.read(ADDR_A_GRAMOS);

       //Ahora se lee la hora a la que esta programado dar de comer al gato

       S_hora   = EEPROM.read(ADDR_S_HORA);
       S_minuto = EEPROM.read(ADDR_S_MINUT);

       //Ahora se lee la dosis que va a suministrar el comedero

       S_dosis =  EEPROM.read(ADDR_S_DOSIS);

       //Ahora se lee la cantidad de comida que dispensa el comedero en cada dosis
       //EEPROM.write(ADDR_GRsAMOS, 15);

       S_gram = EEPROM.read(ADDR_S_GRAMOS);

       //Mensaje de bienvenida
       lcd.clear();
       //lcd.setBacklight(ON);
       lcd.print(" Auto Sulivan's");
       lcd.setCursor(0,1);
       lcd.print("  Feeder ");
       lcd.setCursor(9,1);
       lcd.print(VERSION_DEL_COMEDERO);
       readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year); // Lo primero es leer la fecha y la hora del reloj
       
       //Esperamos para ver el Splash
       mensaje = millis() + (SPLASH *1000);
       encenderLCD();
   }

//EL LOOP//
//-------//

void loop(){

    char comando [40] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};   
    float temp;
    int h = 0, aux;
    byte cant, c, i=0, j;
    byte h_leida, m_leida;
    unsigned long millis_actuales = millis();
    
    String pruebas[12]= {"ULTIMO", "COMIDA", "HORA", "DOSIS", "GRAMOS", "PESO", "RESET", "LUZ", "TEST", "VELOCIDAD", "MAN", "HISTORICO" };
    byte longitudes[12]= {6,6,4,5,6,4,5,3,4,9,3,9};

    //Este es el codigo que se ejecuta antes de leer cualquier comando
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year); // Lo primero es leer la fecha y la hora del reloj
    //Serial.println(millis_actuales); // con este comando observo cuanto tiempo le lleva al microcontrolador dar una vuelta al loop
    if ((millis_actuales > iluminacion) && (flag_luz == true)){
      lcd.setBacklight(OFF);
    }

    //Comprobamos si es la hora de comer
    
    if (((A_dayOfMonth != dayOfMonth ) || (A_month != month) || (A_year != year)) 
          && 
          (hour >= S_hora && minute >= S_minuto)){
            dar_de_comer();
          }
          
    
    if (millis_actuales >= MILLIS_DIARIOS){
      resetearComedero();
    }

// Esta porcion de codigo permite comprobar la circuiteria sin necesidad de un ordenador
    //if (millis_actuales >5000 && millis_actuales <5100){
    //  girarTolva (1,VUELTA);
    //}
      
    //mostramos la hora en la pantalla si procede
    if((second != sec_anterior) &&(millis_actuales > mensaje)){
      sec_anterior = second;
      temp = getTemperature();
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print(dayOfMonth);
      lcd.print("/");
      lcd.print(month);
      lcd.print("/");
      lcd.print(2000+year);
      lcd.setCursor(0,1);
      lcd.print(hour);
      lcd.print(":");
      lcd.print(minute);
      lcd.print(":");
      lcd.print(second);
      lcd.print(" ");
      lcd.print(temp);
      lcd.write(1);
      lcd.print("C");     
    }

#ifdef DEBUG
     if(Serial.available()){
      delay (400);
       while (Serial.available()){
          
          comando[i] = Serial.read();
       
#else
     if(BT.available()){
      delay(400);
       while (BT.available()){
          
          comando[i] = BT.read();
     
     
#endif
     
          Serial.print(comando[i]);
          i++;
          
       }
          for(i=0; i<40; i++)

#ifdef DEBUG
          Serial.flush();

#else 
          BT.flush();

#endif

          c = (comparaOpcion(comando, pruebas, longitudes, (byte)sizeof(longitudes),&i));
          //Serial.println("La opcion encontrada es: ");
          //Serial.print(c);
          
          switch (c){

            case 1:
                aux = EEPROM.read(ADDR_A_DOSIS) * EEPROM.read(ADDR_A_GRAMOS);
                mostrarCadena(ultima_alimentacion);
                #ifdef DEBUG
                Serial.print(A_hour);
                Serial.print(":");
                Serial.print(A_minute);
                Serial.print(":");
                Serial.print(A_second);
                Serial.print("   ");
                Serial.print(A_dayOfMonth);
                Serial.print("/");
                Serial.print(A_month);
                Serial.print("/");
                h = 2000 + A_year;
                Serial.print(h);
                Serial.print("   ");
                Serial.print(A_temperatura);
                Serial.print(" C");
                #else
                BT.print(A_hour);
                BT.print(":");
                BT.print(A_minute);
                BT.print(":");
                BT.print(A_second);
                BT.print("   ");
                BT.print(A_dayOfMonth);
                BT.print("/");
                BT.print(A_month);
                BT.print("/");
                h = 2000 + A_year;
                BT.print(h);
                BT.print("   ");
                BT.print(A_temperatura);
                BT.print(" C");
                #endif
                mostrarCadena(suministraron);
                #ifdef DEBUG
                Serial.print(aux);
                #else
                BT.print(aux);
                #endif
                mostrarCadena(cad_gramos);
                
                lcd.clear();
                encenderLCD();
                sprintf(comando, ("%d/%d/%d %dgr\n"), A_dayOfMonth, A_month, h, aux);
                lcd.print(comando);
                lcd.setCursor(0,1);
                sprintf(comando, ("%d:%d:%d "), A_hour, A_minute, A_second);
                lcd.print(comando);
                lcd.print(A_temperatura);
                lcd.write(1);
                mostrarMensaje();

                break;

            case 2:

                  h_leida = ((((byte)(comando[i+ 7])) -48) * 10) + (((byte)(comando[i+ 8]) -48));
                  m_leida = ((((byte)(comando[i+10])) -48) * 10) + (((byte)(comando[i+11]) -48));

                  if((h_leida >= 24) || (m_leida >=60)){

                    mostrarCadena(hora_invalido);
                    lcd.clear();
                    lcd.print("Formato de hora");
                    lcd.setCursor(4,1);
                    lcd.print("INVALIDO");
                    encenderLCD();
                    mostrarMensaje();

                    } 
                    
                  else{
                    EEPROM.write(ADDR_S_HORA, h_leida);
                    EEPROM.write(ADDR_S_MINUT, m_leida);
                    S_hora = h_leida;
                    S_minuto = m_leida;
                    mostrarCadena(hora_comida);
                    #ifdef DEBUG
                    Serial.print(h_leida);
                    Serial.print(":");
                    Serial.print(m_leida);
                    #else
                    BT.print(h_leida);
                    BT.print(":");
                    BT.print(m_leida);
                    #endif
                    lcd.clear();
                    lcd.print(" Hora de comida");
                    lcd.setCursor(5,1);
                    lcd.print(h_leida);
                    lcd.print(":");
                    lcd.print(m_leida);
                    mostrarMensaje();
                    encenderLCD();
                    }

                break;

            case 3:

                hour       = ((((byte)(comando[i + 5])) -48) * 10) + (((byte)(comando[i + 6]) -48));
                minute     = ((((byte)(comando[i + 8])) -48) * 10) + (((byte)(comando[i + 9]) -48));
                second     = ((((byte)(comando[i+ 11])) -48) * 10) + (((byte)(comando[i+ 12]) -48));
                dayOfMonth = ((((byte)(comando[i+ 14])) -48) * 10) + (((byte)(comando[i+ 15]) -48));
                month      = ((((byte)(comando[i+ 17])) -48) * 10) + (((byte)(comando[i+ 18]) -48));
                year       = ((((byte)(comando[i+ 22])) -48) * 10) + (((byte)(comando[i+ 23]) -48));
                dayOfWeek  = ((((byte)(comando[i+ 25])) -48));

              //Se comprueba que los valores esten en los rangos y que los valores tomados provengan de caracteres numericos
                if ((hour >24) || (minute >60) || (second >60) || (dayOfMonth > 31) || (month >12) || (year >50) || (dayOfWeek >7) || !((comando[i +  5] > 47) && (comando[i +  5] <58)) || !((comando[i +  6] > 47) && (comando[i +  6] <58)) || !((comando[i +  8] > 47) && (comando[i +  8] <58)) || !((comando[i +  9] > 47) && (comando[i +  9] <58)) || !((comando[i + 11] > 47) && (comando[i + 11] <58)) || !((comando[i + 12] > 47) && (comando[i + 12] <58)) || !((comando[i + 14] > 47) && (comando[i + 14] <58)) || !((comando[i + 15] > 47) && (comando[i + 15] <58)) || !((comando[i + 17] > 47) && (comando[i + 17] <58)) || !((comando[i + 18] > 47) && (comando[i + 18] <58)) || !((comando[i + 20] > 47) && (comando[i + 20] <58)) || !((comando[i + 21] > 47) && (comando[i + 21] <58)) || !((comando[i + 23] > 47) && (comando[i + 23] <58))){
                  mostrarCadena(fech_o_invalid);
                  lcd.clear();
                  lcd.print("  Hora o fecha");
                  lcd.setCursor(4,1);
                  lcd.print("INVALIDA");
                  mostrarMensaje();
                  encenderLCD();
                  }
                  else{
                    setDS3231time(second,minute,hour,dayOfWeek,dayOfMonth,month,year);
                    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
                    mostrarCadena(reloj_actualizado);
                    #ifdef DEBUG
                    Serial.print(hour);
                    Serial.print(":"); 
                    Serial.print(minute);
                    Serial.print(":");
                    Serial.print(second);
                    Serial.print(" ");
                    Serial.print(dayOfMonth);
                    Serial.print("/");
                    Serial.print(month);
                    Serial.print("/20");
                    Serial.print(year);
                    #else
                    BT.print(hour);
                    BT.print(":"); 
                    BT.print(minute);
                    BT.print(":");
                    BT.print(second);
                    BT.print(" ");
                    BT.print(dayOfMonth);
                    BT.print("/");
                    BT.print(month);
                    BT.print("/20");
                    BT.print(year);
                    #endif
                    lcd.clear();
                    encenderLCD();
                    mostrarMensaje();                    
                    lcd.print("Hora actualiaza");
                    lcd.setCursor(4,1);
                    lcd.print(hour);
                    lcd.print(":");
                    lcd.print(minute);
                    lcd.print(":");
                    lcd.print(second);
                    delay(1000);
                    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
                    lcd.setCursor(11,1);
                    lcd.print(second);
                    delay(1000);
                    lcd.clear();
                    lcd.print(" Fecha cambiada");
                    lcd.setCursor(3,1);
                    lcd.print(dayOfMonth);
                    lcd.print("/");
                    lcd.print(month);
                    lcd.print("/20");
                    lcd.print(year);
                    mensaje = millis() + 2000;
  
                  }

                    break;

                case 4:
                
                    if (!((comando[i + 7] > 47) && (comando[i + 7] <58)))
                    c = (((byte)(comando[i + 6])) -48);
                    else
                    c = ((((byte)(comando[i + 6])) -48) * 10) + (((byte)(comando[i + 7]) -48));
                    if ((c > 50) || ((comando[i + 8] > 47) && (comando[i + 8] <58)) || !((comando[i + 6] > 47) && (comando[i + 6] <58))){
                      mostrarCadena(dosis_invalida);
                      lcd.clear();
                      lcd.print("     Dosis");
                      lcd.setCursor(4,1);
                      lcd.print("INVALIDA");
                      mostrarMensaje();
                      encenderLCD();
                      }
                      
                     else{

                      EEPROM.write (ADDR_S_DOSIS, c);
                      S_dosis = c;
                      mostrarCadena(suministraran);
                      #ifdef DEBUG
                      Serial.print(c);
                      #else
                      BT.print(c);
                      #endif
                      mostrarCadena(c_dosis);
                      lcd.clear();
                      lcd.print("Se suministraran:");
                      lcd.setCursor(0,1);
                      lcd.print(c);
                      lcd.print(" Dosis");
                      mostrarMensaje();
                      encenderLCD();
                      }

                    break;

                case 5:
                
                    if (!((comando[i + 9] > 47) && (comando[i + 9] <58)))
                    h = ((((byte)(comando[i + 7])) -48) * 10) + (((byte)(comando[i + 8]) -48));

                    else
                    h = ((((byte)(comando[i + 7])) -48) * 100) + ((((byte)(comando[i + 8])) -48) * 10) + (((byte)(comando[i + 9]) -48));
                    if ((h > 250) || (h<1) || ((comando[i + 10] > 47) && (comando[i + 10] <58)) || !((comando[i + 7] > 47) && (comando[i + 7] <58)) || !((comando[i + 8] > 47) && (comando[i + 8] <58))){
                      mostrarCadena(cantidad_invalida);
                      lcd.clear();
                      lcd.print("Cantidad");
                      lcd.setCursor(4,1);
                      lcd.print("INVALIDA");
                      mostrarMensaje();
                      encenderLCD();
                    }
                    else{

                      c = h;

                      EEPROM.write(ADDR_S_GRAMOS, c);

                      mostrarCadena(suministran);
                      #ifdef DEBUG
                      Serial.print(c);
                      #else
                      BT.print(c);
                      #endif
                      mostrarCadena(gramos_dosis);
                      lcd.clear();
                      lcd.print(c);
                      lcd.print(" Gramos/Dosis");
                      mostrarMensaje();
                      encenderLCD();
                    }
                
                    break;

                case 6:
                    // Si el primero no es un numero o hay un numero mas del esperado o alguno no es un numero
                    if (((comando[i + 10] > 47) && (comando[i + 10] <58)) || !((comando[i + 5] > 47) && (comando[i + 5] <58)) || !((comando[i + 6] > 47) && (comando[i + 6] <58)) || !((comando[i + 7] > 47) && (comando[i + 7] <58))){ 
                      mostrarCadena(p_invalido);
                      lcd.clear();
                      lcd.print("      Peso");
                      lcd.setCursor(4,1);
                      lcd.print("INVALIDO");
                      mostrarMensaje();
                      encenderLCD();
                    }
                    else{ 
                      h=0;
                      if((comando[i + 9] > 47) && (comando[i + 9] <58))
                         h = ((((byte)(comando[i + 5])) -48) * 10000) + ((((byte)(comando[i + 6])) -48) * 1000) + ((((byte)(comando[i + 7])) -48) * 100) + ((((byte)(comando[i + 8])) -48) * 10) + (((byte)(comando[i + 9]) -48));
                         else 
                           if((comando[i + 8] > 47) && (comando[i + 8] <58))
                           h = ((((byte)(comando[i + 5])) -48) * 1000) + ((((byte)(comando[i + 6])) -48) * 100) + ((((byte)(comando[i + 7])) -48) * 10) + (((byte)(comando[i + 8]) -48));
                           else
                                h = ((((byte)(comando[i + 5])) -48) * 100) + ((((byte)(comando[i + 6])) -48) * 10) + (((byte)(comando[i + 7]) -48));

                       if (h > 25000){
                         mostrarCadena(demasiado_peso);
                         lcd.clear();
                         lcd.print(" Demasiado peso");
                         lcd.setCursor(2,1);
                         lcd.print("para un gato");
                         mostrarMensaje();
                         encenderLCD();
                       }else
                           if(h >= 15000){
                             mostrarCadena(sobrepeso);
                             EEPROM.write(ADDR_S_DOSIS, 0);
                             lcd.clear();
                             lcd.setCursor(3,0);
                             lcd.print("Tu gato");
                             lcd.setCursor(0,1);
                             lcd.print("sufre sobrepeso");
                             encenderLCD();
                             delay(2000);
                             
                             lcd.clear();
                             lcd.print("Por favor visite");
                             lcd.setCursor(0,1);
                             lcd.print("a un veterinario");
                             for (i=0; i<10; i++){
                              lcd.setBacklight(OFF);
                              delay(500);
                              lcd.setBacklight(ON);
                              delay(500);
                             }
                             lcd.clear();
                             lcd.print("No se dispensara");
                             lcd.setCursor(0,1);
                             lcd.print("ninguna cantidad");
                             mostrarMensaje();
                             encenderLCD();
                           }else{
                              h= h/1000;
                              j=0;

                              //Esta es la dieta segun su peso;
                                switch (h){

                                  case 14:
                                    j += 5;
                                  case 13:
                                    j += 5;
                                  case 12:
                                    j += 5;
                                  case 11:
                                    j += 5;
                                  case 10:
                                    j += 5;
                                  case 9:
                                    j += 5;
                                  case 8:
                                    j += 10;
                                  case 7:
                                    j += 10;
                                  case 6:
                                    j += 10;
                                  case 5:
                                    j += 10;
                                  case 4:
                                    j += 10;                                  
                                  case 3:
                                    j += 10;
                                  case 2:
                                    j += 15;
                                  case 1:
                                    j += 15;
                                  case 0:
                                    j += 25;
                                    break;
                                  }

                              S_dosis = j/S_gram;
                                if((j%S_gram)>(S_gram/2))
                                  S_dosis++;
                              if (S_dosis == 0)
                                  S_dosis++;
                              EEPROM.write(ADDR_S_DOSIS, S_dosis);
                              mostrarCadena(suministran);
                              #ifdef DEBUG
                              Serial.print(S_dosis);
                              #else
                              BT.print(S_dosis);
                              #endif
                              mostrarCadena(c_dosis);
                              lcd.clear();
                              lcd.print("Le corresponden");
                              lcd.setCursor(4,1);
                              lcd.print(" dosis");
                              lcd.print(c_dosis);
                              mostrarMensaje();
                              encenderLCD();
                           }
                        }


                break;

            case 7:
                  lcd.clear();
                  encenderLCD();
                  lcd.print("   Reseteando");
                  lcd.setCursor(0,1);
                  lcd.print("Nos vamos abajo!");
                  for(i=3; i>=1; i--){
                    lcd.setCursor(15,0);
                    lcd.print(i);
                    delay(1000);
                  }
                  resetearComedero();
                break;

            case 8:

                  if(flag_luz){
                    mostrarCadena(l_fija);
                    }
                    else{
                     
                      mostrarCadena(l_auto);
                      }
                  
                  flag_luz = !flag_luz;
                  lcd.setBacklight(ON);

                  break;

            case 9:
                if(toupper(comando[i + 5]) == 'C'){
                  lcd.clear();
                  lcd.setCursor(2,0);
                  lcd.print("Contravuelta");
                  lcd.setCursor(3,1);
                  lcd.print("de prueba");
                  encenderLCD();
                  mostrarMensaje();
                  girarTolva(1, CONTRAVUELTA);
                  
                }
                else if (toupper(comando[i + 5]) == 'V'){
                  lcd.clear();
                  lcd.setCursor(5,0);
                  lcd.print("Vuelta");
                  lcd.setCursor(3,1);
                  lcd.print("de prueba");
                  encenderLCD();
                  mostrarMensaje();
                  girarTolva(1, VUELTA);
                }
                
                else{
                  dar_de_comer();
                  
                }
                break;
                
            case 11:
                  for(j=0; j<10; j++)
                      comando[j] = comando [i+4+j];

                      c = comparaOpcion(comando, pruebas, longitudes, (byte)sizeof(longitudes),&i);
                      switch (c){
                        case 0:
                          mostrarCadena(cad_separador);
                          mostrarCadena(manual);
                          break;

                        case 1:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_ultimo);
                          break;

                        case 2:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_comida);
                          break;

                        case 3:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_hora);

                          #ifdef DEBUG
                          Serial.print((char)241);  //Esto sirve para mostrar la ñ (h) 
                          #else 
                          BT.print((char)241);  //Esto sirve para mostrar la ñ (h) 
                          #endif
                          
                          mostrarCadena(man_hora2);
                          break;
                          
                        case 4:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_dosis);
                          break;

                        case 5:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_gramos);
                          break;

                        case 6:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_peso);
                          break;

                        case 7:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_reset);
                          break;

                        case 8:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_luz);
                          break;

                        case 9:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_test);
                          break;

                        case 10:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_velocidad);
                          break;

                        case 11:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_man);
                          break;

                        case 12:
                          mostrarCadena(cad_separador);
                          mostrarCadena(man_historico);
                          break;                      
                      }
                       
                  
                  break;

            default:
                mostrarCadena(comando_invalido);
                lcd.clear();
                lcd.print("    Comando");
                lcd.setCursor(4,1);
                lcd.print("INVALIDO");
                encenderLCD();
                mostrarMensaje();
                break;
          }
      }
}



// FUNCIONES //
//-----------//

//Funcion que ejecuta la operacion "dar de comer"
//---------------------------------------------------------------------------------------//
void dar_de_comer(void){
  float temp = getTemperature();
  
  lcd.clear();
  lcd.print(" Alimentando a:");
  lcd.setCursor(0,1);
  lcd.print(nombre);
  encenderLCD();
  mostrarMensaje();
  
  girarTolva(S_dosis, VUELTA);

    if(A_second != second){
      A_second = second;
      EEPROM.write(ADDR_A_SECOND, second);}
    
    if(A_minute != minute){
      A_minute = minute;
      EEPROM.write(ADDR_A_MINUTE, minute);}
    
    if(A_hour != hour){
      A_hour = hour;
      EEPROM.write(ADDR_A_HOUR, hour);}
    
    if(A_dayOfMonth != dayOfMonth){
      A_dayOfMonth = dayOfMonth;
      EEPROM.write(ADDR_A_dayOfMonth, dayOfMonth);}
    
    if(A_month != month){
      A_month != month;
      EEPROM.write(ADDR_A_MONTH, month);}
    
    if(A_year != year){
      A_year != year;
      EEPROM.write(ADDR_A_YEAR, year);}
    
    if((int)A_temperatura != (int) temp){
      EEPROM.write(ADDR_A_TEMP, (byte)temp);}
    
    if(A_temperatura - (int)temp != temp - (int)temp){
      EEPROM.write(ADDR_A_TEMP_DEC, (byte)((temp - (int)temp)*100));}

    if(A_temperatura != temp)
      A_temperatura = temp;
    
    if(A_dosis != S_dosis){
      EEPROM.write(ADDR_A_DOSIS, S_dosis);}
    
    if(A_gram != S_gram){
      EEPROM.write(ADDR_A_GRAMOS, S_gram);}
    

/*
    #define ADDR_A_SECOND     0x00
    #define ADDR_A_MINUTE     0x01
    #define ADDR_A_HOUR       0x02
    #define ADDR_A_dayOfMonth 0x03
    #define ADDR_A_MONTH      0x04
    #define ADDR_A_YEAR       0x05
    #define ADDR_A_TEMP       0x06
    #define ADDR_A_TEMP_DEC   0x07
    #define ADDR_S_HORA       0x08
    #define ADDR_S_MINUT      0x09
    #define ADDR_A_DOSIS      0x0A
    #define ADDR_DOSIS        0x0B
    #define ADDR_A_GRAMOS     0x0C
    */
    
}


//Funcion que muestra por la interfaz serie una cadena guardada en la memoria de programa//
//---------------------------------------------------------------------------------------//
void mostrarCadena(const char * entrada){
    // read back a char
    char myChar;
    int k;
  int len = strlen_P(entrada);
  for (k = 0; k < len; k++)
  {
    myChar =  pgm_read_byte_near(entrada + k);
    #ifdef DEBUG
    Serial.print(myChar);
    #else
    BT.print(myChar);
    #endif
  }
}

//FUNCION QUE DEVUELVE LA OPCION//
//------------------------------//
byte comparaOpcion(char * entrada, String * opciones, byte * longitud, byte cant, byte * p){

          
          
          byte i=0, j, k, c;
          

          while((entrada [i] == ' ')||(entrada [i] == '\n'))
          i++;

        *p = i;
        for(k=0; k<cant; k++){
          c=0;
          for(j=i; j<i+longitud[k] ; j++){
            if(toupper(entrada[j]) == opciones[k][j-i])
            c++;
            if( c==longitud[k] )
            return (k+1);
            }
        }
        return 0;
}

//Funcion que mantiene el mensaje en pantalla//
//-------------------------------------------//

void mostrarMensaje(void){
  mensaje = millis() + (TIEMPO_MENSAJE * 1000);
}

//FUNCION QUE ENCIENDE LA PANTALLA DETERMINADO TIEMPO//
//---------------------------------------------------//
void encenderLCD(void){ 
  lcd.setBacklight(ON);
  iluminacion = millis() + (TIEMPO_BRILLO * 1000);
}

//FUNCION QUE RESETEA LA PLACA//
//----------------------------//
void resetearComedero(void){
  asm volatile ("jmp 0");  
}

//FUNCION QUE ESTABLECE LA HORA//
//-----------------------------//

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

//------------------------------------------------

//FUNCION QUE LEE LA HORA//
//-----------------------//

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

//FUNCION QUE MUESTRA LA HORA POR SERIAL//
//--------------------------------------//
/*
void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Lunes");
    break;
  case 2:
    Serial.println("Martes");
    break;
  case 3:
    Serial.println("Miercoles");
    break;
  case 4:
    Serial.println("Jueves");
    break;
  case 5:
    Serial.println("Viernes");
    break;
  case 6:
    Serial.println("Sabado");
    break;
  case 7:
    Serial.println("Domingo");
    break;
  }
}
*/
//FUNCION QUE CONVIERTE NUMEROS DECIMALES A BINARIOS//
//--------------------------------------------------//

byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}


//FUNCION QUE CONVIERTE NUMEROS BINARIOS EN DECIMALES//
//---------------------------------------------------//

byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

//FUNCION QUE OBTENE LA TEMPERATURA//
//----------------------------------//
float getTemperature()
{
    byte temperature;

    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(uint8_t(0x11));
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
    temperature = Wire.read();

    return float(temperature) + 0.25*(Wire.read()>>6);
}

void girarTolva (int vueltas, bool sentido){
  int i, j;
  if(sentido){
    digitalWrite(STEPPER_DIR, LOW);
  }else{
    digitalWrite(STEPPER_DIR, HIGH);
  }

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(STEPPER_ENABLE, LOW); //Esto enciende el motor

  for(i=0; i<vueltas; i++){
    
    for(j=0; j<PASOS_POR_VUELTA; j++){

      digitalWrite(STEPPER_STEP, HIGH);
      delay(espera);
      digitalWrite(STEPPER_STEP, LOW);
      delay(espera);
      
    }
  }
  
  digitalWrite(STEPPER_ENABLE, HIGH);
  digitalWrite(STEPPER_DIR, LOW);
  digitalWrite(LED_PIN, LOW);
  
  
}

