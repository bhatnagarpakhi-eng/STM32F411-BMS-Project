STM32F411 Battery Management System (BMS) 

A multi-sensor Battery Management System built on the STM32F411CEU6(Black Pill), featuring real time voltage/current/temperature monitoring, USB CDC serial reporting and fault detection - developed as a project demonstrating embedded systems design from bring-up to sensor integration. 

Project Status 

Module                                                            Status

Clock configuration (96MHz HCLK, 48MHz USB)                       Complete
USB CDC virtual COM port                                          Complete 
LED blinking indicator                                            Complete 
Cell voltage reporting(simulated)                                 Complete 
ADC voltage sensing                                               In Progress 
DS18B20 temperature sensing                                       Planned 
INA219 current sensing                                            Planned
SSD1306 OLED display                                              Planned 
FSM (Idle/Charging/Full/Fault)                                    Planned 
SOC(State of Charge) calculation                                  Planned


Hardware

MCU: STM32F411CEU6(Black Pill board)
Sensors(planned): DS18B20(temperature x2 ), INA219(current/voltage), SSD1306(OLED display) 
Programming interface: USB DFU bootloader (no external programmer required) 


Key learnings

STM32 USB DFU bootloader enables firmware flashing without external hardware.
USB CDC lets the MCU present itself as a virtual COM port- no CH340/FTDI chip required.
PLL clock math must exactly match the physical crystal frequency; USB enumeration is far less tolerant of clock error than GPIO/delay timing, making USB failures a reliable diagnostic for clock misconfiguration.
USB interrupt priority must be lower than SysTick to avoid preemption conflicts with HAL_Delay().

Author 
Pakhi Bhatnagar


