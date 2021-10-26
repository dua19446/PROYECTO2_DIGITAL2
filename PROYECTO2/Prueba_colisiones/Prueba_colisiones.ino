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
  Serial3.begin(9600);                          // Inicializar C. serial a 9600 bits per second
  Serial2.begin(9600);
  
  Serial2.setTimeout(50);
  Serial3.setTimeout(50);                        // T para la transf. de datos
  delay(100);
  
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

  FillRect(0, 0, 320, 290, 0xFFFF);
}
//***************************************************************************************************************************************
// Loop
//***************************************************************************************************************************************
void loop() {
  unsigned long currentMillis = millis();
    if (Serial3.available()){                      
    com = Serial3.read();                   // Guardar lo leído en Message
    Serial.write(com);                     // Escribir lo que se recibe
 
  delay(4);
  if(com == 0){                          // Recibe un 1
      start1 = 1;                 // Start1

  }
    if(com == 1){                          // Recibe un 1
      up1 = 1;                 //Up1
  }

    if(com == 2){                          // Recibe un 2
      down1 = 1;                  //Down1
  }

   }
   
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
  // actualización de frame cada 42ms = 24fps
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //bool ataque_ave = !digitalRead(PUSH1); // lectura de entradas
    //bool ataque_cactus = !digitalRead(PUSH2);


//---------------------------------------------------------------------------------------------------------------------------------------
//-----Control del ataque por medio de los botones-----------
//---------------------------------------------------------------------------------------------------------------------------------------
    //Para cuando J2 ataca con ave
    if (up2 == 1) {//Los botones estan config. como Pull-Ups
      yataque = ave.y; 
      ataque_activo = 1;
      colision1 = 1;
      a = ave.x;
      up2 = 0;  //esta condicion permite que salga del loop del ataque
      }  
    //se usa un ciclo para que el ataque vaya en linea recta desde la posicion de disparo
    if (ataque_activo == 1){
      int anim = (a / 32) % 3;
      LCD_Sprite(a , ave.y, ave.width, ave.height, aves, ave.columns, anim, 0, 0);
      FillRect(a+ ave.width , ave.y, ave.width, ave.height, 0xFFFF); //con esto se borra el rastro que deja el ataque poniendo un cuadrado
      //con esto se hace avanzar al ataque
      a -= 5;  //esto es lo que permite que el sprite se vaya moviendo.
      if (a > 0 -35){ //mientras el ataque sea mayor al ancho del Trex va a seguir avanzando
        ataque_activo = 1;
        }
      else {
        ataque_activo = 0;
        }
      }
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
//      if (colision2  == 1 ) {
//        int h = trex.y - cactus.y; //se obtiene la posicion en el eje Y de dinosaurio respecto a ataque cactus
//        int d = trex.x - cactus.x; //se obtiene la posicion en el eje X del dinosaurio respecto a cactus
//        int r = c - trex.width ; //se obtiene la posicion en el eje X del dinosaurio + su ancho respecto al Ataque de cactus
//        int hit2 = 0; //sirve para saber si el ataque impacto o no al dinosaurio
//        if ((r <= 0) & (d <= 0)) {
//          hit2++; //para acertar el ataque el jugador viendo hacia la izquierda
//        }
//        //ambas distancias tienen que ser negativas.
//        if ((((h < cactus.height) & (h >= 0)) | (h <= 0) & (h > -21)) & (hit2 == 1)); //para acertar el personaje atacado
//        //tiene que estar como máximo 29 unidades arriba del ataque y 21 por debajo del ataque
//        
//  
//        if (hit2 == 1) {
//          colision2 = 0;
//        }
//      }
    
  //----------------------------------------------------------------------------------------
  }
  
}
