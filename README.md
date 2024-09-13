# Important notes
This version is forked from original by David Carrión (https://github.com/dacarsoft/DB9_2_Keyboard). 

Its only for PS/2 and ATMEGA 328P micro (not USB implemented) to be used with ESPectrum emulator. See info/ folder to see information about pinout

# PowaJoy schematic
This is the schematic to make a Powa-joy. It's a conversor circuit from DB9 (joystick ports) to PS/2 for ESPectrum emulator on ESP32 LilyGo board. This project uses the firmware by David Carrion (DB9_2_Keyboard_ESPectrum - See description below) adapted to ATMEGA 328P micro.

See (schematics/esquema_328P.pdf)

![image](https://github.com/user-attachments/assets/fb3b092b-68a8-4e7b-b244-6de542be1621)

This fork aims to translate joysticks/pads button presses to keyboard presses by using specifically designed for generic ATmega 328P boards Arduino Mini, but is possible to integrated on, Arduino UNO, Arduino Micro and Arduino Nano.

## <b>Arduino Mini</b>

See schematic above
![image](https://github.com/user-attachments/assets/932ce9b2-a265-4cb4-aad6-31944a4d0395)

## <b>Arduino Nano</b>
Pin coversion for Arduino Nano, see below.

Arduino Nano --> Arduino Mini

LED
- D13 --> PIN 13

PS/2 (DATA / CLK)
- DATA: D10 --> PIN 10
- CLK:  D12 --> PIN 12
- 
PORT 1
- D8  --> PIN 8
- D9  --> PIN 9
- D11 --> PIN 11
- D14  --> A0
- D15  --> A1
- D16  --> A2
- D17  --> A3

PORT 2
- D0 --> PIN 0
- D1 --> PIN 1
- D2 --> PIN 2
- D3 --> PIN 3
- D4 --> PIN 4
- D6 --> PIN 6
- D7 --> PIN 7

![image](https://github.com/user-attachments/assets/04068531-fd36-4ebb-b3d9-fd1ddf13fa01)


# DB9_2_Keyboard (by David Carrión - Forked by hash6Iron)

Firmware to converts DB9 joysticks button presses to keyboard scancodes through PS/2 port.

This translation is a big help for ESPectrum (official website https://zxespectrum.speccy.org or project repository https://github.com/EremusOne/ZX-ESPectrum-IDF ) with LilyGo board to use conventional 80's DB9 joysticks or compatible DB9 gamepads (wired or not like as 8-bitDO gamepad tested)

The code sends keystrokes when a button is pressed on a joystick. Two simulatenous joysticks are supported. PS/2 keyboard scancodes are sent. The supported joysticks are standard one-two buttons joysticks, Sega three buttons joysticks and Sega 6 buttons joysticks.

The USB detection is done in the first seconds from the boot when the ATmega board is connected to a PS/2 port and powered up. ESPectrum does a negotiation to comunicate with PowaJoy.

The default map for each joystick to execute the translation from joystick buttons is done according to these sequences:


**DB9 PORT 1 (PS/2):**

> CURSOR UP, CURSOR DOWN, CURSOR LEFT, CURSOR RIGHT, RIGHT ALT, ENTER, ESCAPE, F1, Z, Y, X, M

> Q, A, O, P, M, ENTER, ESCAPE, F1, Z, Y, X, C

> CURSOR UP, CURSOR DOWN, CURSOR LEFT, CURSOR RIGHT, ZERO, ENTER, ESCAPE, F5, Z, Y, X, M

> SEVEN, SIX, FIVE, EIGHT, ZERO, ENTER, ESCAPE, F1, Z, Y, X, M


**DB9 PORT 2 (PS/2):**

> Q, A, O, P, M, ENTER, ESCAPE, F1, Z, Y, X, C

> CURSOR UP, CURSOR DOWN, CURSOR LEFT, CURSOR RIGHT, RIGHT ALT, ENTER, ESCAPE, F1, Z, Y, X, M

> CURSOR UP, CURSOR DOWN, CURSOR LEFT, CURSOR RIGHT, ZERO, ENTER, ESCAPE, F5, Z, Y, X, M

> SEVEN, SIX, FIVE, EIGHT, ZERO, ENTER, ESCAPE, F1, Z, Y, X, M


The keymaps can be changed by pressing a direction button (up, down, left, right) while the board is plugged or powered up.


# FLASH INSTRUCTIONS:

Tha Arduino IDE and tools can be used, but to flash easily the firmware on the ATmega 328P this DIY keyboard's updater program can be used also by using the  .HEX file available under the build/PowaJoy folder of the project. 
Flash tools could be downloaded from:

> MAC OS: https://github.com/qmk/qmk_toolbox/releases/download/beta/QMK.Toolbox.app.zip

> WINDOWS: https://github.com/qmk/qmk_toolbox/releases/download/beta/qmk_toolbox.exe

To upload using AVRDUDE - Example:
- avrdude -c arduino -P COM4 -b 115200 -p atmega328p -D -U flash:w:firmware.hex
