# soundTimer
* Small battery powered device with nanoWatt Technology from Microchip.
* By pressing of tactile switch we start 3 min timer
* LED blinks 1 time every second during 1st minute then 1 beep.
*            2 times every second during 2nd minute then 2 beeps.
*            3 times every second during 3rd minute then 9 beeps.

## HW/SW
- PIC12F683
- MPLAB® X IDE v3.30 + MPLAB® XC8 Compiler v1.37

### Pin connections
- GP0 pin7 NC
- GP1 pin6 NC
- GP2 pin5 tactile switch to the Vss
- GP3 pin4 Vdd
- GP4 pin3 to digital transisor -> Buzzer
- GP5 pin2 LED -> resistor to the Vdd

# Enjoy !