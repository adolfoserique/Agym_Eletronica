/*Developer: Frederik Hauke
Important Notices:
    This Arduino-Code is written for Visualizating measurement data from a microcontroller via Bluetooth.
    Before starting this application, the Bluetooth-Modul (HC-05) has to be coupled to the Smartphone.
    In the special case of the HC-05 the default PinCode for initiating the Coupling-Process is "1234".
Wiring: 
  GND of HC-05 to GND Arduino, VCC of HC-05 to VCC Arduino,
  TX HC-05 to Arduino Pin 10 (RX)
  RX HC-05 to TX Arduino (TX)
 */

#include <SoftwareSerial.h>

#define TX_VX PB1   // Pino PB1(6) como TX
#define RX_VX PB4   // Pino PB4(3) como RX

SoftwareSerial BTserial(RX_VX ,TX_VX); // RX | TX

int sensorPin = A0; 
int sensorValue = 0;

void setup() {
  
 BTserial.begin(9600);    
}

void loop() {

sensorValue = analogRead(sensorPin);

  //IMPORTANT: The complete String has to be of the Form: 1234,1234,1234,1234;
  //(every Value has to be seperated through a comma (',') and the message has to
  //end with a semikolon (';'))
  
                BTserial.print("1234");  
                BTserial.print(",");  
                BTserial.print("1234.0");  
                BTserial.print(",");  
                BTserial.print("1234 hPa");  
                BTserial.print(",");  
                BTserial.print("500 ml/s");  
                BTserial.print(",");  
                BTserial.print(sensorValue);  
                BTserial.print(";");
                
        //message to the receiving device
           
           delay(20);  
}      
