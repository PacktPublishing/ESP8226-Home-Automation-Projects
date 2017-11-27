Demoapp


This is an application that can be used as plug to start/stop a relay or can be the base to develop other kind of IoT products ( thermostats, blinds, boilers ...).

1. Get the code into the Arduino IDE.
2. Get the missing libraries. You will get an error when you compile the code.
3. Flash the code the ESP8266 board.
4. Go to the ESPap Access Point and then to the 192.168.4.1 web address in a browser.
5. Enter your WiFi credentials, the name of the device that will be discovered by the Amazon Echo ( "Alexa turn on .... and Alexa turn off ....) and the user and password you have on iotcentral.eu
6. Add your time zone. ( no usage for now) 
7. Press submit button.

How to send data:

Send {"device_name":"ESP_D232B4","type":"plug","state":1} to turn the relay on and
{"device_name":"ESP_D232B4","type":"plug","state":0} to turn it off. Replace the ESP_D232B4 with your device name.

The message above need to be sent on the topic YOUR_BASE_TOPIC/plug/command from Hivemq.com ( after you entered iotcentral.eu, port 9004, your user and password for iotcentral.eu) or to /YOUR_BASE_TOPIC/plug/command from inside your WiFi network
