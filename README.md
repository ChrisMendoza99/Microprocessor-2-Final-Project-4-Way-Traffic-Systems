# Microprocessor-2-Final-Project-4-Way-Traffic-Systems

This is my final project for Microprocessors and Systems 2 class, by creating a 4 way Traffic System using the ESP32 and the features of FreeRTOS. Further more, using the the pepherials of the ESP32, DAC, Timers, Task Synchronizations, Queue, 
and GPIO's.

In this system, there are 8 buttons, 4 of them represent buttons for the pedestrians going from North to South, and the other 4 for East and West. If a Pedestrian presses a button, the system will acknowledge it, and raise a *flag*, and cause the pedestrian lights to be enabled (on the breadboard circuit, the blue lights represent Pedestrian Lights). If the pedestrain presses the buttons more than 6 times, they can enable a Pedestrian Assistance Mode for those who are Visually Impaired, by triggering a buzzer that guides them.

![image](https://user-images.githubusercontent.com/78059716/205123795-43f35a3a-4770-431b-8f26-d8ce6b4b6c6c.png)

## Componenets
  1-ESP32 WROOM
  
  1- DC Piezo Buzzer

  8-Toggle Buttons
  
  8-470 Ohm Resistor

  8-0.1uF Capacitors
  
  8-Blue LEDs
  
  4-Yellow LEDs
  
  4-Red LEDs
  
  4-Green LEDs
  
  ## Breadboard Circuit
  ![image](https://user-images.githubusercontent.com/78059716/205127471-86b454b0-fddf-43aa-8069-faace82c3de6.png)
  
  ## Author
  
   By Christopher A. Mendoza

