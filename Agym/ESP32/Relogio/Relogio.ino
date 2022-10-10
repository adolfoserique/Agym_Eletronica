// https://www.instructables.com/ESP32-Bluetooth-Low-Energy/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Wire.h>

Adafruit_MPU6050 mpu;
MAX30105 particleSensor;

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
char txString[8];

const byte RATE_SIZE = 5; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
float beatAvg;

// Timer: auxiliar variables
unsigned long previousMillis = 0;    // Stores last time redings was published
const long interval = 1000;         // interval at which to publish sensor readings

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


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
  
  // Initialize sensor MPU6050
  mpu.begin();

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2G, 4G, 8G e 16G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); // 250deg, 500deg, 1000deg e 2000deg
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // 260Hz, 184Hz, 94Hz, 44Hz, 21Hz, 10Hz e 5Hz
 

  // Initialize sensor max30102
  particleSensor.begin(Wire, I2C_SPEED_FAST);

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED



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
      
        /* Get new sensor events with the readings */
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      long irValue = particleSensor.getIR();

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

        sendData(a.acceleration.x, 2, 2, txString, "Acc X: "); // unidade: m/s^2
        sendData(a.acceleration.y, 2, 2, txString, "Acc Y: ");
        sendData(a.acceleration.z, 2, 2, txString, "Acc Z: ");
      
        sendData(g.gyro.x, 2, 2, txString, "Gyro X: "); // unidade: rad/s
        sendData(g.gyro.y, 2, 2, txString, "Gyro Y: "); 
        sendData(g.gyro.z, 2, 2, txString, "Gyro Z: ");
        
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
