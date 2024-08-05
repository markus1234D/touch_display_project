#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE Stuff
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


void setup() {
    Serial.begin(115200);
    setupBLE();
}


void loop() {
    // if (Serial.available())
    // {
    //     value = Serial.readStringUntil('\n');
    //     printBLEValue(value);
    // }

    // printBLEValue("Hello World");
    // delay(5000);
    
    runBLE();
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