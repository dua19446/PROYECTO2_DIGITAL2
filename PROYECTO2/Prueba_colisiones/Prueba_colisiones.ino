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

struct Sprite { // estructura para sprites
  int x; // posicion x
  int y; // posicion y
  int width; // ancho de bitmap
  int height; // altura de bitmap
  int columns; // columna sprite sheet
  int index; // indice sprite sheet
  int flip; // voltear imagen
  int offset; // desfase
} trex;

struct Rectangle{ // estructura para rectangulos
  int x; // posicion x
  int y; // posicion y
  int width; // ancho de rectángulo
  int height; // altura de rectángulo
  int color; // color de relleno  
} rect;

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

  trex.x = 0;
  trex.y = 100;
  trex.width = 44;
  trex.height = 56;
  trex.columns = 8;
  trex.index = 0;
  //trex.flip = 0;
  trex.offset = 0;

  rect.x = 140;
  rect.y = 80;
  rect.width = 48;
  rect.height = 50;
  rect.color = 0x07E0;
}
//***************************************************************************************************************************************
// Loop
//***************************************************************************************************************************************
void loop() {
  unsigned long currentMillis = millis();
  
  // actualización de frame cada 42ms = 24fps
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    bool left = !digitalRead(PUSH1); // lectura de entradas
    bool right = !digitalRead(PUSH2);
    if (right) { // modificación de atributos de sprite
      trex.x += 4;
      trex.index++;
      trex.index %= 3;
      //trex.flip = 1;
    }
    if (left) {
      mario.x -= 4;
      mario.index++;
      mario.index %= 3;
      mario.flip = 0;
    }
    if (rectUp) { // movimiento de rectángulo
      FillRect(rect.x, rect.y + rect.height - 5, rect.width, 5, 0x0000); // se colorea resto de rectángulo del frame anterior
      rect.y-= 5;
      if (rect.y <= 0) {
        rectUp = false;
      }
    }
    else {
      FillRect(rect.x, rect.y, rect.width, 5, 0x0000);
      rect.y+= 5;
      if (rect.y >= 200) {
        rectUp = true;
      }
    }  
    collision = Collision(mario.x, mario.y, mario.width, mario.height,
                          rect.x, rect.y, rect.width, rect.height); // detección de colisión
    if (collision) { // se reemplaza el color al colisionar
      rect.color = 0xF800;
    }
    else {
      rect.color = 0x07E0;
    }
    FillRect(rect.x, rect.y, rect.width, rect.height, rect.color);
    if (mario.flip == 1){ // dependiendo de la dirección, se colorea resto del sprite del frame anterior
      FillRect(mario.x - 4, mario.y, 4, mario.height, 0x0000);
    }
    else{
      FillRect(mario.x + mario.width, mario.y, 4, mario.height, 0x0000);
    }
    LCD_Sprite(mario.x, mario.y, mario.width, mario.height, marioBitmap, mario.columns, mario.index, mario.flip, mario.offset);
  }
  
}
