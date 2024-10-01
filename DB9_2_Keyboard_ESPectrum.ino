/*
Project:  DB9 Joysticks to PS/2 adapter firmware
Author:   David Carrión - 2023
PS2Dev library: https://github.com/Harvie/ps2dev

Folked: Antonio Tamairón - 2024 for PowaJOY
01/10/2024 - Añadido simulacion del típico boton de reset del Spectrum+
*/


//#include <ps2dev.h>
#include "ps2dev.h"
#include "Keyboard.h"


#define HASH6IRON_BOARD

#ifdef HASH6IRON_BOARD
  /*
  ATmega 328P board pinout:

  PORT 1
  ----------------------------------------------------------------
  Arduino A0 -> DB9 pin 1: Up
  Arduino A1 -> DB9 pin 2: Down
  Arduino A2 -> DB9 pin 3: Left
  Arduino A3 -> DB9 pin 4: Right
  Arduino VCC-> DB9 pin 5: VCC 5V
  Arduino 5 ->  DB9 pin 6: A / Primary fire
  Arduino 8  -> DB9 pin 7: Selection signal (allows more buttons)
  Arduino GND-> DB9 pin 8: GND
  Arduino 9  -> DB9 pin 9: SELECT BUTTON / Secondary fire

  PORT 2
  ----------------------------------------------------------------
  Arduino 3 -> DB9 pin 1: Up
  Arduino 2 -> DB9 pin 2: Down
  Arduino 1 -> DB9 pin 3: Left
  Arduino 0 -> DB9 pin 4: Right
  Arduino VCC-> DB9 pin 5: VCC 5V
  Arduino 4 -> DB9 pin 6: A / Primary fire
  Arduino 6  -> DB9 pin 7: Selection signal (allows more buttons)
  Arduino GND-> DB9 pin 8: GND
  Arduino 7  -> DB9 pin 9: SELECT BUTTON / Secondary fire

  */
  const uint8_t PS2_PINS[2] = {12, 10}; // CLK , DATA  

  /* NORMA ATARI - DB9 MACHO
   * PIN 1 - UP              
   * PIN 2 - DOWN            
   * PIN 3 - LEFT
   * PIN 4 - RIGHT
   * PIN 5 - FIRE
   * PIN 6 - FIRE2
   * PIN 7 - VCC
   * PIN 8 - GND
   * PIN 9 - SELECT          
 
 */

  // Entradas
  // PORT 1
  // {UP,DOWN,LEFT,RIGHT,FIRE,FIRE2,SELECT}
  // A0, A1, A2, A3, PB3, PB1 - Exclude pin 7 for Sega controllers
  const uint8_t DB9_1_PINS[6] = {14, 15, 16, 17, 11, 9 }; 

  // PORT 2
  // {UP,DOWN,LEFT,RIGHT,FIRE,FIRE2,SELECT}
  // PD3, PD2, PD1, PD0, PD4, PD7 - Exclude pin 7 for Sega controllers
  const uint8_t DB9_2_PINS[6] = { 3, 2, 1, 0, 4, 7 }; 
  
  
  // Salidas
  // PB0
  const uint8_t DB9_1_SELECT = 8;
  // PD6
  const uint8_t DB9_2_SELECT = 6; 

  // Indicador LED de la placa
  const uint8_t LED_ONBOARD = 13;

  // Botón de RESET para ESPectrum
  const uint8_t KEYRESET = 18; //A4
#endif

PS2dev keyboard(PS2_PINS[0], PS2_PINS[1]);  // clock, data
char lastkeycode; // Keycode to be sent again when something fails
uint8_t lastkeycodestatus; // Keydown or keyrelease status for the last keycode sent
//unsigned char keyboardleddata; // Needed for keyboard reading handle code


const uint8_t DB9_1_TOTALPINS = sizeof(DB9_1_PINS);

const char DB9_1_MAP_PS2[4][12] = { // Keycode PS2 maps for the first controller
{PS2dev::ESP_JOY1UP, PS2dev::ESP_JOY1DOWN, PS2dev::ESP_JOY1LEFT, PS2dev::ESP_JOY1RIGHT, PS2dev::ESP_JOY1B, PS2dev::ESP_JOY1C, PS2dev::ESP_JOY1A, PS2dev::ESP_JOY1START, PS2dev::ESP_JOY1Z, PS2dev::ESP_JOY1Y, PS2dev::ESP_JOY1X, PS2dev::ESP_JOY1MODE},
// {PS2dev::UP_ARROW, PS2dev::DOWN_ARROW, PS2dev::LEFT_ARROW, PS2dev::RIGHT_ARROW, PS2dev::RIGHT_ALT, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M},
{PS2dev::Q, PS2dev::A, PS2dev::O, PS2dev::P, PS2dev::M, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::C},
{PS2dev::UP_ARROW, PS2dev::DOWN_ARROW, PS2dev::LEFT_ARROW, PS2dev::RIGHT_ARROW, PS2dev::ZERO, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F5, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M},
{PS2dev::SEVEN, PS2dev::SIX, PS2dev::FIVE, PS2dev::EIGHT, PS2dev::ZERO, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M}
};

uint8_t DB9_1_MAP_ACTIVE = 0;
uint8_t DB9_1_PRESSCOUNT[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t DB9_1_STATUS[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


const uint8_t DB9_2_TOTALPINS = sizeof(DB9_2_PINS);

const char DB9_2_MAP_PS2[4][12] = { // Keycode PS2 maps for the second controller
{PS2dev::ESP_JOY2UP, PS2dev::ESP_JOY2DOWN, PS2dev::ESP_JOY2LEFT, PS2dev::ESP_JOY2RIGHT, PS2dev::ESP_JOY2B, PS2dev::ESP_JOY2C, PS2dev::ESP_JOY2A, PS2dev::ESP_JOY2START, PS2dev::ESP_JOY2Z, PS2dev::ESP_JOY2Y, PS2dev::ESP_JOY2X, PS2dev::ESP_JOY2MODE},
// {PS2dev::Q, PS2dev::A, PS2dev::O, PS2dev::P, PS2dev::M, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::C},
{PS2dev::UP_ARROW, PS2dev::DOWN_ARROW, PS2dev::LEFT_ARROW, PS2dev::RIGHT_ARROW, PS2dev::RIGHT_ALT, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M},
{PS2dev::UP_ARROW, PS2dev::DOWN_ARROW, PS2dev::LEFT_ARROW, PS2dev::RIGHT_ARROW, PS2dev::ZERO, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F5, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M},
{PS2dev::SEVEN, PS2dev::SIX, PS2dev::FIVE, PS2dev::EIGHT, PS2dev::ZERO, PS2dev::ENTER, PS2dev::ESCAPE, PS2dev::F1, PS2dev::Z, PS2dev::Y, PS2dev::X, PS2dev::M}
};


uint8_t DB9_2_MAP_ACTIVE = 0;
uint8_t DB9_2_PRESSCOUNT[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t DB9_2_STATUS[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


const uint8_t DB9_CYCLES_PRESS = 1; // DEBOUNCE CYCLES TO VALIDATE PRESS
const uint8_t DB9_CYCLES_WAIT = 3;  // RELAX CYCLES NEEDED
uint8_t DB9_CYCLES_WAITCOUNT = 0;   // RELAX CYCLES PENDING


bool JOYSTICK_3BUTTON = false;



void setup() {
  
  //pinMode(PS2_PINS[0], INPUT);
  //pinMode(PS2_PINS[1], INPUT);
  //DDRD=0; // Help to disable UART to use pins PD3 and PD2 (1 and 0)

  #ifdef BLUEPILL_BOARD
  pinMode(LED_ONBOARD_1, OUTPUT); // Onboard LED 1 
  digitalWrite(LED_ONBOARD_1, HIGH);
  pinMode(LED_ONBOARD_2, OUTPUT); // Onboard LED 2 
  digitalWrite(LED_ONBOARD_2, HIGH);
  #endif

  #ifdef HASH6IRON_BOARD
    pinMode(LED_ONBOARD, OUTPUT);
    digitalWrite(LED_ONBOARD, HIGH);
  #endif
  
  // PS/2 keyboard initialization
  keyboard.keyboard_init(); // PS2 keyboard init (notice the k in lowercase)

    
  // Joystick in DB9 port 1 initialization
  for ( uint8_t i = 0; i < sizeof(DB9_1_PINS); ++i ) {
    pinMode(DB9_1_PINS[i], INPUT_PULLUP); // Setup joystick 1 press data pins
  }
  pinMode(DB9_1_SELECT, OUTPUT); // Pin to do signal selection for extra buttons in Sega controllers
  //digitalWrite(DB9_1_SELECT, HIGH);
  
  /*
  // Sega 6 button detection
  digitalWrite(DB9_1_SELECT, LOW);  // State 0
  digitalWrite(DB9_1_SELECT, HIGH); // State 1
  digitalWrite(DB9_1_SELECT, LOW);  // State 2
  digitalWrite(DB9_1_SELECT, HIGH); // State 3
  */
  if ((digitalRead(DB9_1_PINS[0]) == LOW) && (digitalRead(DB9_1_PINS[1]) == HIGH)) { // Check press for desired map change
    DB9_1_MAP_ACTIVE = 1;
    digitalWrite(LED_ONBOARD, HIGH);
    delay(2500);
    digitalWrite(LED_ONBOARD, LOW); 
  }
  if ((digitalRead(DB9_1_PINS[1]) == LOW) && (digitalRead(DB9_1_PINS[0]) == HIGH)) { // Check press for desired map change
    DB9_1_MAP_ACTIVE = 2;
    digitalWrite(LED_ONBOARD, HIGH);
    delay(1000);
    digitalWrite(LED_ONBOARD, LOW);
    delay(1000);    
    digitalWrite(LED_ONBOARD, HIGH);      
    delay(500);
    digitalWrite(LED_ONBOARD, LOW);
  }
  if ((digitalRead(DB9_1_PINS[2]) == LOW) && (digitalRead(DB9_1_PINS[3]) == HIGH)) { // Check press for desired map change
    DB9_1_MAP_ACTIVE = 3;
    digitalWrite(LED_ONBOARD, HIGH);
    delay(500);
    digitalWrite(LED_ONBOARD, LOW);
    delay(500);
    digitalWrite(LED_ONBOARD, HIGH);      
    delay(500);
    digitalWrite(LED_ONBOARD, LOW);
    delay(500);
    digitalWrite(LED_ONBOARD, HIGH); 
    delay(500);
    digitalWrite(LED_ONBOARD, LOW);
  }
  
  // Joystick in DB9 port 2 initialization
  for ( uint8_t i = 0; i < sizeof(DB9_2_PINS); ++i ) {
    pinMode(DB9_2_PINS[i], INPUT_PULLUP); // Setup joystick 2 press data pins
  }
  pinMode(DB9_2_SELECT, OUTPUT); // Pin to do signal selection for extra buttons in Sega controllers
  //digitalWrite(DB9_2_SELECT, HIGH);

  /*
  // Sega 6 button detection
  digitalWrite(DB9_2_SELECT, LOW);    // State 0
  digitalWrite(DB9_2_SELECT, HIGH);   // State 1
  digitalWrite(DB9_2_SELECT, LOW);    // State 2
  digitalWrite(DB9_2_SELECT, HIGH);   // State 3
  */
  if ((digitalRead(DB9_2_PINS[0]) == LOW) && (digitalRead(DB9_2_PINS[1]) == HIGH)) { // Check press for desired map change
    DB9_2_MAP_ACTIVE = 1;
  }
  if ((digitalRead(DB9_2_PINS[1]) == LOW) && (digitalRead(DB9_2_PINS[0]) == HIGH)) { // Check press for desired map change
    DB9_2_MAP_ACTIVE = 2;
  }
  if ((digitalRead(DB9_2_PINS[2]) == LOW) && (digitalRead(DB9_2_PINS[3]) == HIGH)) { // Check press for desired map change
    DB9_2_MAP_ACTIVE = 3;
  }

  digitalWrite(LED_ONBOARD, LOW);
}

void resetPressed()
{
  sendPS2keypress(0x78);
  delay(500);
  sendPS2keyrelease(0x78);
}

void loop() {
  /*
  readPS2handle(keyboardleddata);
  */
  joystickProcess(DB9_1_PINS, DB9_1_TOTALPINS, DB9_1_SELECT, DB9_1_STATUS, DB9_1_PRESSCOUNT, DB9_1_MAP_PS2,DB9_1_MAP_ACTIVE, 1);
  joystickProcess(DB9_2_PINS, DB9_2_TOTALPINS, DB9_2_SELECT, DB9_2_STATUS, DB9_2_PRESSCOUNT, DB9_2_MAP_PS2, DB9_2_MAP_ACTIVE, 2);

  if(digitalRead(KEYRESET) == LOW)
  {
    resetPressed();
    digitalWrite(LED_ONBOARD, HIGH);
    digitalWrite(KEYRESET, HIGH);
    delay(2000);
  }
  else
  {
    digitalWrite(LED_ONBOARD, LOW);
  }
  
}



uint8_t sendPS2keypress(char keycode) {
  /*
  lastkeycode = keycode;
  lastkeycodestatus = 1;
  */
  if (keycode == PS2dev::UP_ARROW || keycode == PS2dev::DOWN_ARROW || keycode == PS2dev::LEFT_ARROW || keycode == PS2dev::RIGHT_ARROW || keycode == PS2dev::RIGHT_ALT) {
    //while (digitalRead(PS2_PINS[0]) == LOW || digitalRead(PS2_PINS[1]) == LOW) {}
    return keyboard.keyboard_press_special(keycode);
  } else {
    //while (digitalRead(PS2_PINS[0]) == LOW || digitalRead(PS2_PINS[1]) == LOW) {}
    return keyboard.keyboard_press(keycode);
  }
}

uint8_t sendPS2keyrelease(char keycode) {
  /*
  lastkeycode = keycode;
  lastkeycodestatus = 0;
  */
  if (keycode == PS2dev::UP_ARROW || keycode == PS2dev::DOWN_ARROW || keycode == PS2dev::LEFT_ARROW || keycode == PS2dev::RIGHT_ARROW || keycode == PS2dev::RIGHT_ALT) {
    //while (digitalRead(PS2_PINS[0]) == LOW || digitalRead(PS2_PINS[1]) == LOW) {}
    return keyboard.keyboard_release_special(keycode);
  } else {
    //while (digitalRead(PS2_PINS[0]) == LOW || digitalRead(PS2_PINS[1]) == LOW) {}
    return keyboard.keyboard_release(keycode);
  }
}



void joystickProcess(const uint8_t JOYSTICK_PINS[6], const uint8_t JOYSTICK_TOTALPINS, const uint8_t JOYSTICK_SELECT, uint8_t JOYSTICK_STATUS[12], uint8_t JOYSTICK_PRESSCOUNT[12], const char JOYSTICK_MAP_PS2[4][12], uint8_t JOYSTICK_MAP_ACTIVE, uint8_t JOYSTICK_INDEX) {

  JOYSTICK_3BUTTON = false;
  
  if (DB9_CYCLES_WAITCOUNT > 0) { DB9_CYCLES_WAITCOUNT--; }
  
  // Specific Sega 3 buttons processing
  digitalWrite(JOYSTICK_SELECT, LOW);  // State 2
  
  if ((digitalRead(JOYSTICK_PINS[2]) == LOW) && (digitalRead(JOYSTICK_PINS[3]) == LOW)) 
  { // Detect if LEFT and RIGHT are sent as pressed at the same time to check 3 buttons mode

    JOYSTICK_3BUTTON = true;
    
    for ( uint8_t i = 4; i < JOYSTICK_TOTALPINS; i++ ) 
    {
      if (digitalRead(JOYSTICK_PINS[i]) == LOW) {
        if (JOYSTICK_PRESSCOUNT[i+2] < DB9_CYCLES_PRESS) 
        {
          JOYSTICK_PRESSCOUNT[i+2]++;
        } 
        else if ((JOYSTICK_STATUS[i+2] == 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
        {
          JOYSTICK_STATUS[i+2] = 1;
          DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;
          
          if (JOYSTICK_MAP_ACTIVE) 
          {          
            sendPS2keypress(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+2]);
          } 
          else 
          {
            keyboard.keyboard_press_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+2]);
          }
          
          #ifdef BLUEPILL_BOARD
            if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, LOW); }
            if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, LOW); }
          #endif

          #ifdef HASH6IRON_BOARD
            if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, HIGH); }
            if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, HIGH); }          
          #endif
        }
      } 
      else 
      {
        if ((JOYSTICK_PRESSCOUNT[i+2] > 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
        {
          JOYSTICK_PRESSCOUNT[i+2]--;
        } 
        else if ((JOYSTICK_STATUS[i+2] == 1) && (DB9_CYCLES_WAITCOUNT == 0)) 
        {
          JOYSTICK_STATUS[i+2] = 0;
          DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;
          
          if (JOYSTICK_MAP_ACTIVE) {          
            sendPS2keyrelease(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+2]);
          } 
          else 
          {
            keyboard.keyboard_release_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+2]);
          }
          
          #ifdef BLUEPILL_BOARD
            if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, HIGH); }
            if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, HIGH); }
          #endif

          #ifdef HASH6IRON_BOARD
            if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, LOW); }
            if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, LOW); }          
          #endif
          
        }
      }
    }
    
  }

  // Standard common processing
  digitalWrite(JOYSTICK_SELECT, HIGH); // State 3
  
  for ( uint8_t i = 0; i < JOYSTICK_TOTALPINS; i++ ) 
  {
    if (digitalRead(JOYSTICK_PINS[i]) == LOW) {
      if (JOYSTICK_PRESSCOUNT[i] < DB9_CYCLES_PRESS) 
      {
        JOYSTICK_PRESSCOUNT[i]++;
      } 
      else if ((JOYSTICK_STATUS[i] == 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
      {
        JOYSTICK_STATUS[i] = 1;
        DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;
        
        if (JOYSTICK_MAP_ACTIVE) 
        {          
          sendPS2keypress(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i]);
        } 
        else 
        {
          keyboard.keyboard_press_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i]);
        }
        
        #ifdef BLUEPILL_BOARD
          if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, LOW); }
          if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, LOW); }
        #endif
        
        #ifdef HASH6IRON_BOARD
          if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, HIGH); }
          if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, HIGH); }          
        #endif
      }
    } 
    else 
    {
      if ((JOYSTICK_PRESSCOUNT[i] > 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
      {
        JOYSTICK_PRESSCOUNT[i]--;

      } 
      else if ((JOYSTICK_STATUS[i] == 1) && (DB9_CYCLES_WAITCOUNT == 0)) 
      {
        JOYSTICK_STATUS[i] = 0;
        DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;
        
        if (JOYSTICK_MAP_ACTIVE) 
        {          
          sendPS2keyrelease(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i]);
        } 
        else 
        {
          keyboard.keyboard_release_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i]);
        }
        
        #ifdef BLUEPILL_BOARD
          if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, HIGH); }
          if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, HIGH); }
        #endif

        #ifdef HASH6IRON_BOARD
          if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, LOW); }
          if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, LOW); }          
        #endif
        
      }
    }
  }
  
  // Specific Sega 6 buttons processing
  if (JOYSTICK_3BUTTON == true) 
  {

    digitalWrite(JOYSTICK_SELECT, LOW);  // State 4
    
    if ((digitalRead(JOYSTICK_PINS[0]) == LOW) && (digitalRead(JOYSTICK_PINS[1]) == LOW)) 
    { // Detect if UP and DOWN are sent as pressed at the same time to check 6 buttons mode
      
      digitalWrite(JOYSTICK_SELECT, HIGH); // State 5
      
      // Now we can try to read the buttons sequence: up Z - down Y - left X - right M
      for ( uint8_t i = 0; i < 4; i++ ) 
      {
        if (digitalRead(JOYSTICK_PINS[i]) == LOW) 
        {
          if (JOYSTICK_PRESSCOUNT[i+8] < DB9_CYCLES_PRESS) 
          {
            JOYSTICK_PRESSCOUNT[i+8]++;
          } 
          else if ((JOYSTICK_STATUS[i+8] == 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
          {
            JOYSTICK_STATUS[i+8] = 1;
            DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;

            if (JOYSTICK_MAP_ACTIVE) 
            {          
              sendPS2keypress(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+8]);
            } 
            else 
            {
              keyboard.keyboard_press_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+8]);
            }
            
            #ifdef BLUEPILL_BOARD
              if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, LOW); }
              if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, LOW); }
            #endif
            
            #ifdef HASH6IRON_BOARD
              if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, HIGH); }
              if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, HIGH); }          
            #endif
          }
        } 
        else 
        {
          if ((JOYSTICK_PRESSCOUNT[i+8] > 0) && (DB9_CYCLES_WAITCOUNT == 0)) 
          {
            JOYSTICK_PRESSCOUNT[i+8]--;
          } 
          else if ((JOYSTICK_STATUS[i+8] == 1) && (DB9_CYCLES_WAITCOUNT == 0)) 
          {
            JOYSTICK_STATUS[i+8] = 0;
            DB9_CYCLES_WAITCOUNT = DB9_CYCLES_WAIT;
            
            if (JOYSTICK_MAP_ACTIVE) 
            {          
              sendPS2keyrelease(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+8]);
            }
            else
            {
              keyboard.keyboard_release_ESPectrum_special(JOYSTICK_MAP_PS2[JOYSTICK_MAP_ACTIVE][i+8]);
            }
            
            #ifdef BLUEPILL_BOARD
              if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD_1, HIGH); }
              if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD_2, HIGH); }
            #endif

            #ifdef HASH6IRON_BOARD
              if (JOYSTICK_INDEX == 1) { digitalWrite(LED_ONBOARD, LOW); }
              if (JOYSTICK_INDEX == 2) { digitalWrite(LED_ONBOARD, LOW); }          
            #endif
            
          }
        }
      }
      
      digitalWrite(JOYSTICK_SELECT, LOW);  // State 6
      digitalWrite(JOYSTICK_SELECT, HIGH); // State 7
      delay(20);
      digitalWrite(JOYSTICK_SELECT, LOW);  // State 0
      digitalWrite(JOYSTICK_SELECT, HIGH); // State 1
    }
  }  
}
