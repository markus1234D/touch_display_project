#include <Arduino.h>
// BLE Stuff
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
// BLE Stuff End

//TouchPad Stuff
#include <Wire.h>
#include <SPI.h>
#include "TFT_eSPI.h"
#include "TouchDrvCSTXXX.hpp"


#ifndef SENSOR_SDA
#define SENSOR_SDA  11
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL  10
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ  5
#endif

#ifndef SENSOR_RST
#define SENSOR_RST  -1
#endif

TFT_eSPI tft = TFT_eSPI();
TouchDrvCSTXXX touch;
int16_t x[1], y[1];
void scanDevices(void);
void setupTFT();
void loopTFT();

//TouchPad Stuff End
void setup() {
    pinMode(15, OUTPUT);
    analogWrite(15, 10);
    Serial.begin(115200);
    setupBLE();
    delay(60000);

    setupTFT();
}


void loop() {
    if (Serial.available())
    {
        value = Serial.readStringUntil('\n');
        printBLEValue(value);
    }
    runBLE();
    loopTFT();
}



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
    Serial.println("value: " + value);
    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();
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
    // tft.println("Scanning for I2C devices ...");
    for (address = 0x01; address < 0x7f; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0) {
            tft.printf("I2C device found at address 0x%02X\n", address);
            printBLEValue("I2C device found");
            printBLEValue("Address: " + String(address));
            nDevices++;
        } else if (error != 2) {
            tft.printf("Error %d at address 0x%02X\n", error, address);
            printBLEValue("Error");
        }
    }
    if (nDevices == 0) {
        tft.println("No I2C devices found");
    }
}

void setupTFT() {
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.println("tft is ready!");
    printBLEValue("tft is ready!");

        // Search for known CSTxxx device addresses
    scanDevices();
    uint8_t address = 0x15;
    Wire.begin(SENSOR_SDA, SENSOR_SCL);
    touch.setPins(SENSOR_RST, SENSOR_IRQ);
    touch.begin(Wire, address, SENSOR_SDA, SENSOR_SCL);
    printBLEValue("touch.begin");
    tft.print("Model :"); tft.println(touch.getModelName());
}

void loopTFT() {
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
        printBLEValue("touched");
        tft.fillScreen(TFT_BLACK);
        for (int i = 0; i < touched; ++i) {
            tft.print("X[" + String(i) + "]: " + String(x[i]) + " Y[" + String(i) + "]: " + String(y[i]) + " ");
            tft.fillCircle(y[i], x[i], 30, TFT_RED);
            printBLEValue("X[" + String(i) + "]: " + String(x[i]) + " Y[" + String(i) + "]: " + String(y[i]) + " ");
        }
        tft.println();
    }

    delay(200);
}