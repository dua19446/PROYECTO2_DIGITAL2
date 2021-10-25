char com;                         // Variable para la comunicación serial
const int PU = 2;                 // arriba
const int PD = 3;                 // abajo
const int START = 5;                  // start

int FLAG = 0;                         // Banderas para arriba
int FLAG2 = 0;                        // Banderas para abajo
int FLAG7 = 0;


unsigned char INICIO = 1;             // Flag para el Inicio


//******************************
//                                     S E T   U P 
//******************************
void setup() {
  Serial.begin(9600);                 // Inicializar C. serial a 9600 bits per second
  delay(100);
  pinMode(PU, INPUT_PULLUP);      // Hacer el pin un input pull up  
  pinMode(PD, INPUT_PULLUP);          
  pinMode(START, INPUT_PULLUP);       

}

//*******************************
//                               L O O P   P R I N C I P A L
//*******************************
void loop() {
    delay(10);

    if (digitalRead(START) == HIGH){     // Primer botón
        FLAG7 = 1;                       // Bandera para el antirrebote
          }
    else{
        if(FLAG7 == 1){                  // Si la FLAG está encendida entonces apgarla
           FLAG7 = 0;                    // Apagar bandera   
           Serial.write(0);
           INICIO = 0;
           }
       }

//-------------------------------------------boton de inicio----------------------------

    if (INICIO == 0){                   // Si no se precionó START entonces no comienza
    if (Serial.available()){                      
        com = Serial.read();                   // Guardar lo leído en Message
        Serial.write(com);                     // Escribir lo que se recibe
        delay(4);
        if(com == 49){                          // Recibe un 1
        INICIO = 1; 
    }

    }

//----------------------------------arriba----------------------------------
    if (digitalRead(PU) == HIGH){   // Primer botón: primer jugador
        FLAG = 1;                       // Bandera para el antirrebote
          }
    else{
        if(FLAG == 1){                  // Si la FLAG está encendida entonces apgarla
           FLAG = 0;                    // Apagar bandera   
           Serial.write(1);             // Se envía el dato ASCII = 1
           }
        }
//--------------------------------abajo------------------------------------
    if (digitalRead(PD) == HIGH){   // Segundo botón: Botón especial
        FLAG2 = 1;                      // Bandera para el antirrebote
          }
    else{
        if(FLAG2 == 1){                 // Si la FLAG está encendida entonces apgarla
           FLAG2 = 0;                   // Apagar bandera   
           Serial.write(2);             // Se envía el dato ASCII = 2


           }
        }
    }
}
