// https://www.instructables.com/ESP32-Bluetooth-Low-Energy/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "kxtj3-1057.h"
#include <Wire.h>

KXTJ3 myIMU(0x0E); // Address can be 0x0E(ADDR = LOW) or 0x0F(ADDR = HIGHT)
MAX30105 heartbeatSensor; 

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  5        // Time ESP32 will go to sleep (in seconds)

#define LOW_POWER              // Accelerometer provides different Power modes by changing output bit resolution
//#define HIGH_RESOLUTION

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // See the following for generating UUIDs:
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // https://www.uuidgenerator.net/

//BLE variables
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
char txString[8];

//Heart beat sensor variables
const byte RATE_SIZE = 5; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
float beatAvg;

//Accelerometer variables
float   sampleRate = 6.25;  // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
uint8_t accelRange = 2;     // Accelerometer range = 2, 4, 8, 16g

// Timer variables
unsigned long previousMillis = 0;    // Stores last time redings was published
const long interval = 1000;         // interval at which to publish sensor readings


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void sendData(float float_val, int min_width, int digits_after_decimal, char char_buffer, char char_title);


void setup() {
  
  // Initialize sensor max30102
  heartbeatSensor.begin(Wire, I2C_SPEED_FAST);
  heartbeatSensor.setup(); //Configure sensor with default settings
  heartbeatSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  heartbeatSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  
  // Initialize sensor kxtj3-1057
   myIMU.begin(sampleRate, accelRange);
   myIMU.intConf(123, 1, 10, HIGH); // Detection threshold, movement duration and polarity
   uint8_t readData = 0;
   

  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
//  Serial.println("Waiting a client connection to notify...");
}

void loop() {

    if (deviceConnected) {
      
      int16_t dataHighres = 0;

      long irValue = heartbeatSensor.getIR();

      if (checkForBeat(irValue) == true){
  
        long delta = millis() - lastBeat;
        lastBeat = millis();
        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20){
    
          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
          rateSpot %= RATE_SIZE; //Wrap variable

            //Take average of readings
          beatAvg = 0;
          for (byte x = 0 ; x < RATE_SIZE ; x++)
            beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }
      }

      unsigned long currentMillis = millis();
      
        // Send readings
      if (currentMillis - previousMillis >= interval){
        previousMillis = currentMillis;

        sendData(myIMU.axisAccel( X ), 2, 4, txString, "Acc X: "); // unidade: m/s^2
        sendData(myIMU.axisAccel( Y ), 2, 4, txString, "Acc Y: ");
        sendData(myIMU.axisAccel( Z ), 2, 4, txString, "Acc Z: ");
        
        if (irValue < 50000){
        
        pTxCharacteristic->setValue("No finger");
        pTxCharacteristic->notify();
        pTxCharacteristic->setValue(";");
        pTxCharacteristic->notify();
        pTxCharacteristic->setValue("\n");
        pTxCharacteristic->notify(); 
        
      }
        else
        sendData(beatAvg, 2, 0, txString, "BPM: ");
      }
   }
}



void sendData(float float_val, int min_width, int digits_after_decimal, char *char_buffer, char *char_title){
  
  dtostrf(float_val, min_width, digits_after_decimal, char_buffer);
  pTxCharacteristic->setValue(char_title); //seta o valor que a caracteristica notificará (enviar)
  pTxCharacteristic->notify(); // Envia o valor para o smartphone
  pTxCharacteristic->setValue(char_buffer);
  pTxCharacteristic->notify();
  pTxCharacteristic->setValue(";");
  pTxCharacteristic->notify();
  pTxCharacteristic->setValue("\n");
  pTxCharacteristic->notify(); 
  
}
