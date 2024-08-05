
#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include "TouchDrvCSTXXX.hpp"

#ifndef SENSOR_SDA
#define SENSOR_SDA  11
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL  10
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ  14
#endif

#ifndef SENSOR_RST
#define SENSOR_RST  13
#endif

TouchDrvCSTXXX touch;
int16_t x[5], y[5];

// BLE debug Stuff
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "c4848b93-b7ee-4f14-a5cd-924ac53c259d"
#define CHARACTERISTIC_UUID "da4b9871-05d9-41e8-bf0b-85b5f1fa3d82"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

String value = "";
bool changed = false;

void setupBLE();
void printBLEValue(String value);
void runBLE();
// BLE debug Stuff End

// BLE Mouse Stuff
#include <BleMouse.h>
BleMouse bleMouse;
// BLE Mouse Stuff End

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setupBLE() {
    BLEDevice::init("ESP32_BLE_DEBUGGER");
    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE  |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_INDICATE
                    );
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());
    // Start the service
    pService->start();
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void printBLEValue(String value) {
    // Serial.println("value: " + value);
    // pCharacteristic->setValue(value.c_str());
    // pCharacteristic->notify();
}

void runBLE() {
        if (!deviceConnected && oldDeviceConnected) {
            Serial.println("disconnecting");
            delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            Serial.println("start advertising");
            oldDeviceConnected = deviceConnected;
        }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        Serial.println("connecting");
        oldDeviceConnected = deviceConnected;
    }
}

void scanDevices(void)
{
    byte error, address;
    int nDevices = 0;
    Serial.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            printBLEValue("I2C device found at address 0x" + String(address));
            nDevices++;
        } else if (error != 2) {
            Serial.printf("Error %d at address 0x%02X\n", error, address);
        }
    }
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
        printBLEValue("No I2C devices found");
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    // setupBLE();
    bleMouse.begin();
    // delay(60000);
    // printBLEValue("Hello World!");


#if SENSOR_RST != -1
    pinMode(SENSOR_RST, OUTPUT);
    digitalWrite(SENSOR_RST, LOW);
    delay(30);
    digitalWrite(SENSOR_RST, HIGH);
    delay(50);
    // delay(1000);
#endif

    // Search for known CSTxxx device addresses
    uint8_t address = 0x15;

#ifdef ARDUINO_ARCH_RP2040
    Wire.setSCL(SENSOR_SCL);
    Wire.setSDA(SENSOR_SDA);
#else
    Wire.begin(SENSOR_SDA, SENSOR_SCL);
#endif

    // Scan I2C devices
    scanDevices();

    Wire.beginTransmission(CST816_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        address = CST816_SLAVE_ADDRESS;
    }
    Wire.beginTransmission(CST226SE_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        address = CST226SE_SLAVE_ADDRESS;
    }
    Wire.beginTransmission(CST328_SLAVE_ADDRESS);
    if (Wire.endTransmission() == 0) {
        address = CST328_SLAVE_ADDRESS;
    }
    while (address == 0xFF) {
        Serial.println("Could't find touch chip!"); 
        printBLEValue("Could't find touch chip!");
        delay(1000);
    }

    touch.setPins(SENSOR_RST, SENSOR_IRQ);
    touch.begin(Wire, address, SENSOR_SDA, SENSOR_SCL);

    Serial.print("Model :"); Serial.println(touch.getModelName());
    printBLEValue("Model :" + String(touch.getModelName()));

    // T-Display-S3 CST816 touch panel, touch button coordinates are is 85 , 160
    // touch.setCenterButtonCoordinate(85, 360);

    // T-Display-AMOLED 1.91 Inch CST816T touch panel, touch button coordinates is 600, 120.
    // touch.setCenterButtonCoordinate(600, 120);  // Only suitable for AMOLED 1.91 inch


    // Depending on the touch panel, not all touch panels have touch buttons.
    // touch.setHomeButtonCallback([](void *user_data) {
    //     Serial.println("Home key pressed!");
    // }, NULL);


    // Unable to obtain coordinates after turning on sleep
    // CST816T sleep current = 1.1 uA
    // CST226SE sleep current = 60 uA
    // touch.sleep();

    // Set touch max xy
    // touch.setMaxCoordinates(536, 240);

    // Set swap xy
    // touch.setSwapXY(true);

    // Set mirror xy
    // touch.setMirrorXY(true, true);

}
uint16_t xDistance = 0;
uint16_t yDistance = 0;
uint16_t xPrev = -1;
uint16_t yPrev = -1;
void loop()
{
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
        int i = 0;
        printBLEValue("X[" + String(i) + "]: " + String(x[i]) + " Y[" + String(i) + "]: " + String(y[i]));
            if(xPrev == -1 && yPrev == -1) {
                xPrev = x[i];
                yPrev = y[i];
            }else {
                xDistance = x[i] - xPrev;
                
                yDistance = y[i] - yPrev;
                xPrev = x[i];
                yPrev = y[i];
                bleMouse.move(xDistance, yDistance, 0);
            }
    }else {
        xPrev = -1;
        yPrev = -1;
    }
    delay(100);
    // runBLE();
}



