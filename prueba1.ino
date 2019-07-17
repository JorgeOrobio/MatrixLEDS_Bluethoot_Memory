#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <Wire.h>
#define BASE_ADDRESS 0x50
#include <SoftwareSerial.h>
SoftwareSerial BT1(3, 4); // RX | TX
//De acuerdo al protocolo se debe conectar los RX y TX inversos, es decir, RX-TX ; TX-RX


//Vcc - Vcc
//Gnd - Gnd
//Din - Mosi (Pin 11)
//Cs  - SS (Pin 10)
//Clk - Sck (Pin 13)

const int pinCS = 10;
const int numberOfHorizontalDisplays = 8;
const int numberOfVerticalDisplays = 1;
int maxaddress = 8192 - 1;        //24C64    -> 65536 bit   -> 8192 byte
int address = 0;
char *palabra;
String  blue;//variable qe va a guardar en la memoria que reciba del bt
char value;
String palabraAux="";//variabe que va a guardar el mensaje recibido por bt en caso de que no haya memoria
unsigned int tamayoNewValue = 0;


Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
const int wait = 25; // Velocidad a la que realiza el scroll
const int spacer = 1; 
const int width = 5 + spacer; // Ancho de la fuente a 5 pixeles

void setup() {
  Serial.begin(9600);
  BT1.begin(9600);
  Wire.begin();
  matrix.setIntensity(10); // Ajustar el brillo entre 0 y 15

  // Ajustar segun las necesidades
  matrix.setPosition(0, 0, 0); // El primer display esta en <0, 0>
  matrix.setPosition(1, 1, 0); // El segundo display esta en <1, 0>
  matrix.setPosition(2, 2, 0); // El tercer display esta en <2, 0>
  matrix.setPosition(3, 3, 0); // El cuarto display esta en <3, 0>
  matrix.setPosition(4, 4, 0); // El quinto display esta en <4, 0>
  matrix.setPosition(5, 5, 0); // El sexto display esta en <5, 0>
  matrix.setPosition(6, 6, 0); // El séptimo display esta en <6, 0>
  matrix.setPosition(7, 7, 0); // El octavo display esta en <7, 0>
  matrix.setPosition(8, 8, 0); // El noveno display esta en <8, 0>

  matrix.setRotation(0, 1);    // Posición del display
  matrix.setRotation(1, 1);    // Posición del display
  matrix.setRotation(2, 1);    // Posición del display
  matrix.setRotation(3, 1);    // Posición del display
  matrix.setRotation(4, 1);    // Posición del display
  matrix.setRotation(5, 1);    // Posición del display
  matrix.setRotation(6, 1);    // Posición del display
  matrix.setRotation(7, 1);    // Posición del display
  matrix.setRotation(8, 1);    // Posición del display
}


void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data )
{
  if (maxaddress >= 511)
  {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(data);
    Wire.endTransmission();
  }

  delay(5);
}


byte readEEPROM(int deviceaddress, unsigned int eeaddress )
{
  byte rdata = 0xFF;
  if (maxaddress >= 511)
  {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
  }

  Wire.requestFrom(deviceaddress, 1);

  if (Wire.available()) rdata = Wire.read();

  return rdata;
}

void writeWordToEEPROM(char cadena[], int tamayoPalabra) {
  for (address = 0; address < tamayoPalabra; address++) {
    value = cadena[address];
    writeEEPROM(BASE_ADDRESS, address, value);
  }
  address = 0; //para no afectar address que es una variable global
}

void clearMemory() {
  char eraser;
  int limiteLimpieza=100;
  for (address = 0 ; address <= limiteLimpieza; address++) {
    eraser = '~'; //"borra" la memoria llenandola de virgulillas
    writeEEPROM(BASE_ADDRESS, address, value);
  }

}

void readWordFromEEPROM(char* palabra1, int maximiliano) {
  for (address = 0 ; address <= maximiliano; address++) {
    if(palabra1[address]!='~'){
    palabra1[address] = readEEPROM(BASE_ADDRESS, address);  
    }else{
      return;  //hfjimenez at utp edu dot co  IMPORTANTE!!!!!!
    }
  }
}

void loop() {

  long int time = millis();
  int tam;
  if (BT1.available())
  {
    clearMemory();
    palabraAux=""; //limpiamos las dos palabras que se van a recibir por bt
    blue ="";
    tam = 0;
    while (BT1.available()) {
    char c = char(BT1.read());
    if((byte)c != NULL){ //esta condicion se puede omitir, pero eso se deja a gusto del programador
      writeEEPROM(BASE_ADDRESS,tam,c);
      palabraAux += c; 
      }    
    blue +=  (String)c;  
    Serial.println(blue);
    tam++;
  }
}else{
  tam=50; //Si no hay memoria el tamaño maximo de la palabra es 50 se puede cambiar a gusto del programador
}  
char palabra1[tam + 1]; //tam + 1 para el caso de que llegue un mensaje con memoria conectada 
  readWordFromEEPROM(palabra1, tam + 1);
  for (int i=0 ; i<= tam; i++) {
    Serial.print((byte)palabra1[i]);//ciclo para mostrar la palabra en el monitor serie
    Serial.print(" ");
  }
  String cadena;
  String mensajeBasico="Inserte una memoria";
  if(((byte)palabra1[0] != 255 )&&((byte)palabra1[1]!= 255)){//condicion para saber si hay conectada una memoria
   cadena = (String)palabra1; //si esta se manda la palabra que se recogio de la memoria
  }else{
    if(palabraAux.length()>0){//Si no hay memoria revisa si ha recibido un mensaje por bt
    cadena = palabraAux;      
    }else{                    // Si ha recibido un mensaje, entonces muestrelo, sino entonces muestre un mensaje base
    cadena = mensajeBasico;  
    }
  }
  for (int i = 0; i < width * cadena.length() + matrix.width() - 1 - spacer; i++) {//ciclo que imprime la palabra guardada en cadena en la matriz de LEDS 

    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // Centrar el texto

    while (x + width - spacer >= 0 && letter >= 0) {
      if (letter < cadena.length()) {
        matrix.drawChar(x, y, cadena[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }
    matrix.write(); // Muestra los caracteres

    delay(wait);
  }
}
