//*********************************************
#include <SPI.h>
#include <SD.h>

const int chipSelect = PA_3; //cs PIN
File archivo;

#include "melodias.h"

#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

extern uint8_t aves[];
extern uint8_t trex_c[];
extern uint8_t cactus1[];
int yataque, yataque2;
int ataque_activo, ataque_activo2;
int colision1, colision2;
uint8_t game_over = 0;  //bandera para que el juego funcione
char  up2, down2, start2, com, com2, start1, up1, down1; 
//estas variables sirven para controlar el movimiento de los ataques del jugador 2
int a = 0; //
int c = 0; //
//---------------------------------------------------------------------------------
struct Sprite { // estructura para sprites
  int x; // posicion x
  int y; // posicion y
  int width; // ancho de bitmap
  int height; // altura de bitmap
  int columns; // columna sprite sheet
  int index; // indice sprite sheet
  //int flip; // voltear imagen
  int offset; // desfase
} trex, ave, cactus;

bool rectUp = false; // dirección rectángulo
bool collision = false; // detección de colisión

int arriba = 0;
int saltando = 1;

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
int start = 0;
// notes in the melody:
int melody[] = {
    NOTE_A4, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_E4,
    NOTE_G4, NOTE_C4, NOTE_F4, NOTE_E4, NOTE_G4, NOTE_C4, NOTE_F4, NOTE_E4, NOTE_D4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int DuracionNotas[] = {2, 2, 4, 4, 2, 2, 4, 4, 1, 2, 2, 4, 4, 2, 2, 4, 4, 1};
uint8_t mov = 0;
uint8_t movstate = 0;
uint8_t altura_trx = 160;
uint8_t tierra = 1;
uint8_t bajada1 = 0;
float bajada = 0;
uint8_t salto1 = 0;
uint8_t velocidad = 0;
uint8_t estadosalto = 0;
uint8_t suelo = 1;
int comenzar = 0;
//*********************************************
// PROTOTIPOS DE FUNCIONES 
//*********************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

int AS_HE(int a);
void mapeo(char documento[]);
void movimiento();
void salto();

unsigned long previousMillis = 0;
const long interval = 42;
extern uint8_t fondo_trex[];
extern uint8_t logo[];

uint8_t estadobotonviejo = 0;
uint8_t estadoboton = 0;

const int boton = PA_6;

//*********************************************
// INICIALIZACION 
//*********************************************
void setup() 
{
  //pinMode(PF_1, INPUT_PULLUP);
  //pinMode(PF_3, INPUT_PULLUP);
  pinMode(boton, INPUT_PULLUP);
  pinMode(PA_7, INPUT_PULLUP);
  //pinMode(PUSH1, INPUT_PULLUP);
  //pinMode(PD_6, INPUT_PULLUP);
  
  pinMode(PF_2, OUTPUT);
  Serial.begin(9600);
  SPI.setModule(0);

  pinMode(PA_3, OUTPUT);     // change this to 53 on a mega
  
   if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");

  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);                          // Inicializar C. serial a 9600 bits per second
  Serial2.begin(9600);
  
  Serial2.setTimeout(50);                       // T para la transf. de datos
  delay(100);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Start");
  LCD_Init();
  LCD_Clear(0x00);

  //dimensiones de Trex
  trex.x = 0;
  trex.y = 160;
  trex.width = 44;
  trex.height = 56;
  trex.columns = 2;
  trex.index = 0;
  trex.offset = 0;

  //dimensiones de cactus
  cactus.x = 290;
  cactus.y = 165;
  cactus.width = 48;
  cactus.height = 50;

  //dimensiones de ave
  ave.x = 290;
  ave.y = 70;
  ave.width = 52;
  ave.height = 45;
  ave.columns = 2;
  ave.index = 0;
  ave.offset = 0;

}

void loop() {
  unsigned long currentMillis = millis();
   
   if (Serial2.available()){                      
    com2 = Serial2.read();                   // Guardar lo leído en Message
    Serial.write(com2);                     // Escribir lo que se recibe
 
  delay(4);
  if(com2 == 0){                          // Recibe un 0
      start2 = 1;                 //Start2
  }
    if(com2 == 1){                          // Recibe un 1
      up2 = 1;                 //Up2

  }

    if(com2 == 2){                          // Recibe un 2
      down2 = 1;                  //Down2
  }

   }
//if (game_over == 1){   
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

  if(start == 0)
  {
    //LCD_Bitmap(0, 0, 320, 240, fondo_trex);
    FillRect(0, 0, 320, 240, 0xffff);
    LCD_Bitmap(175, 125, 85, 56, logo);
    for(int x = 0; x <319; x++){
     LCD_Bitmap(x, 216, 16, 16, tile);
     x += 15;
    }
    String text1 = "DINO JUMP";
    String text2 = "Pablo Moreno";
    String text3 = "Alejandro Duarte";
    String text4 = "APACHURRALE 'START'";
    String text5 = "PARA COMENZAR";
    LCD_Print(text1, 90, 30, 2, 0xffff, 0x0000);
    LCD_Print(text2, 10, 100, 1, 0xffff, 0x0000);
    LCD_Print(text3, 10, 120, 1 , 0xffff, 0x0000);
    LCD_Print(text4, 10, 190, 2, 0xffff, 0x0000);
    LCD_Print(text5, 60, 210, 2, 0xffff, 0x0000);
    if (start1 == 1){
        start = 1;
        comenzar = 1;
        return;
     }
    for (int Nota = 0; Nota < 18; Nota++) 
    {
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
     int Duracion = 1000 / DuracionNotas[Nota];
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
     tone(PF_2, melody[Nota], Duracion); 
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
     int pausa = Duracion * 1.40;
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
        delay(pausa);
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
     noTone(PF_2);
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
    }
     if (digitalRead(PA_6) == 0){
        start = 1;
        comenzar = 1;
        return;
     }
     
  }    
  if (comenzar == 1){
    
    FillRect(0, 0, 320, 240, 0xffff);
    FillRect(0, 216, 320, 30, 0x6b04);
    comenzar = 0;
  }
    
    if(altura_trx >= 160){
      suelo = 1;
    }
    else if(altura_trx < 160){
      suelo = 0;
    }
    estadoboton = digitalRead(boton);
    
    if (digitalRead(PA_7) == 1 && digitalRead(PA_6) == 1){
        LCD_Sprite(0, 160, 44, 56, trex_c, 2, mov, 0, 0);
        FillRect(44, 160, 44, 56, 0xffff);
        FillRect(0, 155, 44, 5, 0xffff);
      //}
    }
    
    if (digitalRead(PA_7) == 0){
        LCD_Sprite(0, 183, 80, 32, trex_agachado, 2, mov, 0, 0); //para el dinosaurio corriendo parado
        FillRect(0, 160, 44, 23, 0xffff);
    }
    if (digitalRead(PA_6) == 0){
    LCD_Sprite(0, altura_trx, 44, 56, trex_c, 2, mov, 0, 0);
    salto();
    }
   
    estadobotonviejo = estadoboton;
    movimiento();

    //---------------------------------------------------------------------------------------------------------------------------------------
//-----Control del ataque por medio de los botones-----------
//---------------------------------------------------------------------------------------------------------------------------------------
    //Para cuando J2 ataca con ave
//    if (up2 == 1) {//Los botones estan config. como Pull-Ups
//      yataque = ave.y; 
//      ataque_activo = 1;
//      colision1 = 1;
//      a = ave.x;
//      up2 = 0;  //esta condicion permite que salga del loop del ataque
//      }  
//    //se usa un ciclo para que el ataque vaya en linea recta desde la posicion de disparo
//    if (ataque_activo == 1){
//      int anim = (a / 32) % 3;
//      LCD_Sprite(a , ave.y, ave.width, ave.height, aves, ave.columns, anim, 0, 0);
//      FillRect(a+ ave.width , ave.y, ave.width, ave.height, 0xFFFF); //con esto se borra el rastro que deja el ataque poniendo un cuadrado
//      //con esto se hace avanzar al ataque
//      a -= 5;  //esto es lo que permite que el sprite se vaya moviendo.
//      if (a > 0 -35){ //mientras el ataque sea mayor al ancho del Trex va a seguir avanzando
//        ataque_activo = 1;
//        }
//      else {
//        ataque_activo = 0;
//        }
//      }

   //---------------------------------------------------------------------------------------------------------------------------- 
    //----------------------------------Para cuando J2 ataca con cactus-----------------------------------------------------------
    //Es la misma logica que en ataque de ave
    if (down2 == 1) {
      //int yataque2 = cactus.y;
      ataque_activo2 = 1;
      colision2 = 1;
      c = cactus.x;
      down2 = 0;        //esta condicion permite salair del loop de ataque 
      }  

    if (ataque_activo2 == 1){
      int anim2 = (a / 32) % 3;
      LCD_Sprite(c, cactus.y, cactus.width, cactus.height, cactus1, 1, anim2, 0, 0);
      FillRect(c +cactus.width , cactus.y, cactus.width , cactus.height, 0xFFFF); 
      c -= 5;  
      if (c > 0 - cactus.width){ 
        ataque_activo2 = 1;
        }
      else {
        ataque_activo2 = 0;
        }
    }

          if (colision2  == 1 ) {
        int h = altura_trx - cactus.y; //se obtiene la posicion en el eje Y de dinosaurio respecto a ataque cactus
        int d = altura_trx - cactus.x; //se obtiene la posicion en el eje X del dinosaurio respecto a cactus
        int r = c - trex.width ; //se obtiene la posicion en el eje X del dinosaurio + su ancho respecto al Ataque de cactus
        int hit2 = 0; //sirve para saber si el ataque impacto o no al dinosaurio
        if ((r <= 0) & (d <= 0)) {
          hit2++; //para acertar el ataque el jugador viendo hacia la izquierda
        }
        //ambas distancias tienen que ser negativas.
        if ((((h < cactus.height) & (h >= 0)) | (h <= 0) & (h > -21)) & (hit2 == 1)); //para acertar el personaje atacado
        //tiene que estar como máximo 29 unidades arriba del ataque y 21 por debajo del ataque
        //game_over = 1;
  
        if (hit2 == 1) {
          colision2 = 0;
        }
      }
 }
//}    
}
//*********************************************
// Función para inicializar LCD
//*********************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //**************
  // Secuencia de Inicialización
  //**************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //**************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //**************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //**************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //**************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //**************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //**************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //**************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //**************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //**************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //**************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //**************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //**************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //**************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//*********************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//*********************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//*********************************************
// Función para enviar datos a la LCD - parámetro (dato)
//*********************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//*********************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//*********************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//*********************************************
// Función para borrar la pantalla - parámetros (color)
//*********************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//*********************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//********************************************* 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//*********************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//********************************************* 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//*********************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//*********************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//*********************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//*********************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c); 
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//*********************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//*********************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//*********************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//*********************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//*********************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//*********************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
    for (int j = 0; j < height; j++){
        k = (j*(ancho) + index*width -1 - offset)*2;
        k = k+width*2;
       for (int i = 0; i < width; i++){
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k+1]);
        k = k - 2;
       } 
    }
  }
  else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
int AS_HE(int a){
  switch (a){
    case 48:
      return 0;
    case 49:
      return 1;
    case 50:
      return 2;
    case 51:
      return 3; 
    case 52:
      return 4;
    case 53:
      return 5;
    case 54:
      return 6;
    case 55:
      return 7;
    case 56:
      return 8;
    case 57:
      return 9;
    case 97:
      return 10;
    case 98:
      return 11;
    case 99:
      return 12;
    case 100:
      return 13;
    case 101:
      return 14;
    case 102:
      return 15;                                                                                            
  }
}

void mapeo(char documento[]){
  archivo = SD.open(documento, FILE_READ);
  int hex1 = 0;
  int val1 = 0;
  int val2 = 0;
  int mapear = 0;
  int vert = 0;
  unsigned char maps[640]; 
  if(archivo){
    Serial.println("Leyendo el archivo...");
    while (archivo.available()){
      mapear = 0;
      while (mapear< 640){
        hex1 = archivo.read();
        if (hex1 == 120){
          val1 = archivo.read();
          val2 = archivo.read();
          val1 = AS_HE(val1);
          val2 = AS_HE(val2);
          maps[mapear] = val1 * 16 + val2;
          mapear++; 
        }
      }
      LCD_Bitmap(0, vert, 320, 1, maps);
      vert++;
    }
    archivo.close();
  }
  else{
    Serial.println("No se pudo encontrar el archivo.");
    archivo.close();
  }
}
void movimiento(){
movstate += 50;
mov = (movstate/32)%3;
}
void salto(){
  bajada = 0 ;
  bajada1 = int(bajada);
  altura_trx+=bajada1; 
  if(suelo == 1){// Se revisa que el jugador este en el suelo para poder saltar
  if (estadoboton == 0 && estadobotonviejo == HIGH) {
    salto1 = 1; // Se activa el caso que simula una media parabola hacia arriba
  }
  }
  if(suelo == 0){
    if( altura_trx<160 ){
      bajada = 2;
    }
    velocidad = bajada;
    FillRect(0, altura_trx-velocidad, 44, velocidad, 0xffff);
    //LCD_Sprite(0, altura_trx-velocidad, 44, 56, trex_normal, 1, 0, 0, 0);
    bajada1 = int(bajada);
    altura_trx+=bajada1;
    if(altura_trx > 160){
      altura_trx = 160;
    }
  }
  if (salto1 == 1){ // Subrutina de salto que desplaza a el jugador hacia arriba con desplazamientos grandes y luego graduales para simular una semi parabola
      switch (estadosalto){
        case 0:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 1:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 2:
         bajada = -20;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+20+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+20+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 3:
         bajada = -15;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+15+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+15+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 4:
         bajada = -15;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+15+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+15+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 5:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

      case 6:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 7:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 8:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         estadosalto ++;
        break;

        case 9:
         bajada = -10;
         bajada1= int(bajada);
         altura_trx+=bajada1; 
         FillRect(0, altura_trx+10+20, 44, 32, 0xffff);
         //LCD_Sprite(0, altura_trx+10+16, 44, 56, trex_normal, 1, 0, 0, 0);
         salto1 = 0;
         estadosalto= 0;
        break;

  
        default:
        salto1 = 0;
        estadosalto = 0;
      }
  }   
}
