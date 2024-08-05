#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLEDebugger {
    private:
        BLEServer* pServer = NULL;
        BLECharacteristic* pCharacteristic = NULL;
        bool deviceConnected = false;
        bool oldDeviceConnected = false;

        String value = "";
        bool changed = false;
        BLEServerCallbacks MyServerCallbacks;
        

    public:
        BLEDebugger();
        void setDeviceConnected(bool connected);
        void setOldDeviceConnected(bool connected);
        void init(){
            // class MyServerCallbacks: public BLEServerCallbacks {
            //     void onConnect(BLEServer* pServer) {
            //         setDeviceConnected = true;
            //     };

            //     void onDisconnect(BLEServer* pServer) {
            //         BLEDebugger::setDeviceConnected = false;
            //     }
            // };

        };
        void print();

        
};


