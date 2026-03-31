# ArduinoNessoN1-Matter-Illuminance-Sensor

## Hardware Specifications

The Arduino Nesso N1 (Figure 4) is built with an ESP32-C6 MCU, as well as a SX1262 LoRa Module [1]. Wifi, Thread, bluetooth, and LoRa are all integrated on the board, but to use LoRa the antenna must be attached to the board. Additionally, It has a touchscreen that can display output as well as take user touch inputs, which I used to display the state of the board. Additionally, I implemented some functionality that uses the yellow user button on the board. I also attached a BH1750 light/lux sensor (Figure 4) using the QWIIC connector on the board, this means that the board was communicating with the sensor using I2C [2]. There are also sensors on the device that I did not use such as the 6-axis Inertial Measurement unit that measures accelerometer data as well as gyroscope data.

## Software Specifications

When developing for the Arduino I was using the Arduino IDE and creating Arduino sketches (.ino file type). The version of the Arduino IDE that I was using was 2.3.8. I downloaded all the libraries that I was using for the Arduino inside of the IDE.

For my Arduino project I used several libraries that enabled me to develop a Matter application, as well as use the BH1750 light sensor. The first thing I downloaded in my Arduino IDE was the board support for the Arduino Nesso N1 which is in the Board Manager Menu [1]. The board support is called esp32 by Espressif Systems and I installed version 3.3.7. This includes the Matter.h Library that enables you to develop Matter applications for esp32 boards and it currently uses Matter version 1.2.0. It also includes the Wire.h library which is required to use I2C communication. Once installed I ensured that the IDE could see the board. I did this by selecting the Arduino Nesso N1 in Tools>Board>esp32 (this is a long list of boards and the Arduino Nesso N1 is near the bottom). Next I had to select the COM port that the board was on. To see which COM port your device is on go to Tools>Port and there should be a port that says ESP32 Family Devices if your device is connected to your computer.

The other additional library that I downloaded is BH1750 by Christopher Law which enables me to read the values from the lux sensor. I am using version 1.3.0 of this library.

## Implimentation

For the Arduino Nesso N1 I started with the Matter On/Off Light Example that is provided with the Matter library. The example can be found in the Arduino Nesso N1 user manual [1]. I tried connecting this project to the Google Nest Hub to see that it worked and I was easily able to do so. The example is implemented so that you can easily use Thread or Wifi with the Matter protocol. In the example the program first sets up the on off light endpoint using the Matter.h library and sets the callback function to be the one that is defined to switch the state of the light upon a command from the controller. Then the Matter service starts and after we enter the loop of the program. In the loop we first check to see if the device is already commissioned if it has not been commissioned then we print the info to the serial output for the user to commission the device.

What I had to add to the example was the light sensor endpoint for use in the Matter protocol, reading of the light sensor, implementation of the user button, and the implementation of the display.

The first thing that I added was the addition of the use of the display. I did this so that it is easier to see what state the Nesso N1 is in, as it is difficult to see the LED that is inside the board. The display reflects the same state as the LED, if the LED is on then the display reads “on”, if the LED is off then the display reads “off”. Also, the display can say “pair” on it if the device has not been commissioned yet.

Unfortunately, for the light sensor there is no implementation using the Arduino Matter.h wrapper, so I had to use the lower level esp_matter.h library to implement the light endpoint. In reality, it is quite similar as Matter.h is using esp_matter.h under the hood. Additionally, I have to set up the communication with the sensor, this includes adding the Wire.h library that has the I2C implementation in it. In my program I have included a sensorSetup function that does all the work to set up the sensor, such as starting the I2C communication and then calling the setup function from the BH1750 library. Now that the light sensor has everything needed for reading it and updating the Matter cluster, in the loop function there is an if block that checks if the sensor is ready to be read. If the sensor is ready to read then it is read and printed to the serial output. Then we do a conversion of the value to a logarithmic scale. This is specified in the Matter protocol as the raw lux value has a range of values that is quite large. Lastly, the Illuminance Matter cluster is updated. This reading of the sensor and update of the Matter cluster is done every 5 seconds.

For the Nesso N1, I also added some functionality to the user button on the front of the device. In the loop for the program every five seconds the program polls the button to see if it has been pressed. If the button has been pressed then it decommissions the Matter device so that other devices can no longer see its endpoints and it forgets the Thread network. The Nesso N1 will then go back to the pairing state where it prints its commissioning credentials and it is ready to be commissioned. I did this so that the board will be in its original state and I can again test the commissioning of the board without having to do anything on my computer.

### References

[1]    Arduino, “Nesso N1 User Manual.” Accessed: Mar. 18, 2026. [Online]. Available: https://docs.arduino.cc/tutorials/nesso-n1/user-manual/#matter

[2]    B. Siepert, “Adafruit BH1750 Ambient Light Sensor,” Adafruit Learning System. Accessed: Mar. 20, 2026. [Online]. Available: https://learn.adafruit.com/adafruit-bh1750-ambient-light-sensor/overview
  
