//***************************************************************************************************************************************
/* Código de ejemplo de videojuego:
 * Detección de colisiones con objetos rectangulares.
 * Emplea estructuras para definir clases e instanciar objetos.
 * Incorpora actualización de cuadros cada 42ms ( aprox. 24fps).
 * Movimiento de sprite utilizando botones integrados.
 * Basado en librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Autor: Diego Morales
 * IE3027: Electrónica Digital 2 - 2021
 */
//***************************************************************************************************************************************
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
#include "font.h"
#include "lcd_registers.h"
#include "pantalla.h"

extern uint8_t marioBitmap[]; // cargar bitmap desde memoria flash
extern uint8_t aves[];
extern uint8_t trex_c[];
extern uint8_t cactus1[];
uint8_t game_over;  //bandera para que el juego funcione
int yataque, yataque2;
int ataque_activo, ataque_activo2;
int colision1, colision2;
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

//struct obstaculos{ // estructura para rectangulos
//  int x; // posicion x
//  int y; // posicion y
//  int width; // ancho de rectángulo
//  int height; // altura de rectángulo
//} cactus;

bool rectUp = false; // dirección rectángulo
bool collision = false; // detección de colisión

unsigned long previousMillis = 0;  
const long interval = 42;
//***************************************************************************************************************************************
// Initialization
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Start");
  LCD_Init();
  LCD_Clear(0x00);
  
  pinMode(PUSH1, INPUT_PULLUP); // botones integrados con como entrada pull-up
  pinMode(PUSH2, INPUT_PULLUP);

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
  cactus.y = 120;
  cactus.width = 24;
  cactus.height = 50;

  //dimensiones de ave
  ave.x = 290;
  ave.y = 50;
  ave.width = 52;
  ave.height = 45;
  ave.columns = 2;
  ave.index = 0;
  ave.offset = 0;

  FillRect(0, 0, 320, 290, 0xFFFF);
}
//***************************************************************************************************************************************
// Loop
//***************************************************************************************************************************************
void loop() {
  unsigned long currentMillis = millis();
  
  // actualización de frame cada 42ms = 24fps
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    bool ataque_ave = !digitalRead(PUSH1); // lectura de entradas
    bool ataque_cactus = !digitalRead(PUSH2);

//---------------------------------------------------------------------------------------------------------------------------------------
//-----Control del ataque por medio de los botones-----------
//---------------------------------------------------------------------------------------------------------------------------------------
    //Para cuando J2 ataca con ave
    char ataque1 = ataque_ave;
    if (ataque1 == 1) {//Los botones estan config. como Pull-Ups
      yataque = ave.y; 
      ataque_activo = 1;
      colision1 = 1;
      a = ave.x;
      }  
    //se usa un ciclo para que el ataque vaya en linea recta desde la posicion de disparo
    if (ataque_activo == 1){
      int anim = (a / 32) % 3;
      LCD_Sprite(a, yataque, ave.width, ave.height, aves, ave.columns, anim, 0, 0);
      FillRect(a + 5, yataque, ave.width, ave.height, 0xFFFF); //con esto se borra el rastro que deja el ataque poniendo un cuadrado
      //con esto se hace avanzar al ataque
      a -= 5;  //esto es lo que permite que el sprite se vaya moviendo.
      if (a > trex.width){ //mientras el ataque sea mayor al ancho del Trex va a seguir avanzando
        ataque_activo = 1;
        }
      else {
        ataque_activo = 0;
        }
      }
    //---------------------------------------------------------------------------------------------------------------------------- 
    //----------------------------------Para cuando J2 ataca con cactus-----------------------------------------------------------
    //Es la misma logica que en ataque de ave
    char ataque2 = ataque_cactus;
    if (ataque2 == 1) {
      int yataque2 = cactus.y;
      ataque_activo2 = 1;
      colision2 = 1;
      c = cactus.x;
      }  

    if (ataque_activo2 == 1){
      int anim2 = (a / 32) % 3;
      LCD_Sprite(c, yataque2, cactus.width, cactus.height, cactus1, 1, anim2, 0, 0);
      FillRect(c + 5, yataque2, cactus.width, cactus.height, 0xFFFF); 
      c -= 5;  
      if (c > trex.width){ 
        ataque_activo2 = 1;
        }
      else {
        ataque_activo2 = 0;
        }
    } 
    
    //----------------------------------------------------------------------------------------
    //--------------Rutinas para comprobar que el ataque acerto-------------------------------
    //Para el ataque del J2
    if (colision1 == 1 ) {
      FillRect(a, yataque, ave.width, ave.height, 0xFFFF);
      int h = trex.y - yataque;
      int d = trex.x - trex.y;
      int r = a - trex.x;
      int golpe = 0;
      if ((((h < 39)& (d >= 0) & (h >= 0)) | (h <= 0) & (h > -24)) & (golpe == 1));
        game_over = 1; //si el golpe acierta, acaba el juego

      if (golpe == 1)colision1 = 0;
    }
    
  //----------------------------------------------------------------------------------------
  }
  
}
