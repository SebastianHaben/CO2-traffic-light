## CO2 Traffic Light
The CO2 traffic light can help you to know when it is time to let some fresh air into your room and is also capable of logging your data online. The integrated CO2 & VOC (Volatile Organic Compounds) sensor tracks the air quality and indicates via an LED status light if the room air quality is good (green), ok(yellow) or bad (red) as well as display the values, as well as the room temperature on an e-Paper display.

  ![GIF of working project](/assets/demo.gif)
  
## Contents
- [Features](#Features)
- [Required parts](#Required Parts)
- [Circuit & breadboard diagram](#Circuit & Breadboard Diagram)
- [Code and how to create a Firebase account](#Code and how to create a Firebase account)
- [Setting the correct timezone] (#Setting the correct timezone)
- [First start-up](#First start-up)
- [Laser cut housing](#Laser cut housing)
---
## Features
-	Measuring of temperature, eCO2 and VOC concentration in room air
-	Display of data as well as time and date (incl. correct time zone and daylight-saving time) on an e-Paper display.
-	Upload Data to a Google Firebase Realtime Database for analysis of historic trends
---
## Required Parts
-	Wemos LOLIN D1 mini
-	Adafruit CSS811 air quality sensor
-	Adafruit MPL3115A2 Barometric Pressure Sensor
-	Waveshare 2.9 in E-Ink (E-Paper) display (black and white). Important: While other displays  sizes are also possible a lot of code rework (especially for graphics) will be necessary, for which no guide will be provided!
-	Adafruit Neopixel (or other WS2812B based  LED) or APA106 LED
-	Perf-board (I used a 40 x 60 mm one)

For Prototyping:
-	Large Breadboard
-	Dupont Jumper cables

For Housing:
-	6mm MDF plate
-	Screws (M2x8mm for Display and 2x10 mm for rest)
-	Hot glue or screws
---
## Circuit & Breadboard Diagram

The connections are done according to the breadboard view. In addition, a small table for each component is provided for each component with the corresponding pin on the D1 mini. 

  ![Breadboard based hook-up diagram of project](/schematics/CO2_Ampel_bb.png)

**CSS811 & MPL3115A2**
|Pin on CSS811/MPL3115A2|Pin on D1 mini|
|:---:|:---:|
|VIN|3.3 V|
|GND|GND|
|SDA|D1|
|SCL|D2|

**Waveshare E-Ink Display**
|Pin on Waveshare|E-Ink Display	Pin on D1 mini|
|:---:|:---:|
|VCC|3.3 V|
|GND|GND|
|DIN|D7|
|CLK|D5|
|CS|D8|
|DC|D3|
|RST|D0|
|BUSY|D6|

**Neopixel**
|Pin on Neopixel|Pin on D1 mini|
|:---:|:---:|
|VIN|5V|
|GND|GND|
|DIN|D4|
---
## Code set-up and how to create a Firebase account

To run the project several things, need to be changed before flashing the code to the D1 mini. To save our data in the Google Firebase Realtime Database we first need to create a project in Google Firebase, create a new Realtime Database, grab the API-Key of the project and the database-URL as well as create the authentication via an e-mail address/ password combination. This handy tutorial will help you through each step of setting up everything on Firebase:   (<https://randomnerdtutorials.com/esp8266-data-logging-firebase-realtime-database/>)
After you have gathered all the required information you need to update the firebase-credentials.h file.
---
## Setting the correct time zone

To have the correct date and time displayed we need to set the correct time zone. This is done in line 84. To choose your correct time zone find your region and city in this table: (<https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv>) and replace the String with the corresponding string for your location.
---
## First start-up
After you have adjusted the code and the credentials you can flash it on the D1 mini. The final step is to connect the D1 mini to your WiFi. For this connect with your smartphone or tablet to the network “AutoConnectAP” and on the resulting page (if not forwarded automatically type 192.168.4.1 into your browser) find your WiFi’s SSID from the list and enter your password. The D1 mini should restart automatically and will connect to the WiFi. The credentials are stored on the D1 mini and are still available after a loss of power.
---
## Laser cut Housing
I built this enclosure from 6mm birch MDF which might be also the new home for your own CO2 traffic light. I personally don’t own a laser cutter but online you can find shops that will cut the pieces for you at competitive rates. To place the project into the housing I soldered the sensors and the D1 mini onto a small perfboard.
  ![Front view of assembled PCB](/assests/PCB_assembly_front.JPG)
  ![Back view of assembled PCB](/assests/PCB_assembly_back.JPG)
  ![PCB & Display attached to housing](/assests/project_in_housing.jpg)

