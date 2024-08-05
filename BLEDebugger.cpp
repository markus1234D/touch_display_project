#include "BLEDebugger.h"



BLEDebugger::BLEDebugger() {
    class MyServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
            setDeviceConnected(true);
        };

        void onDisconnect(BLEServer* pServer) {
            BLEDebugger::setDeviceConnected(false);
        }
    };
}
void BLEDebugger::makeClass() {
    
void BLEDebugger::setDeviceConnected(bool connected) {
    deviceConnected = connected;
}
void BLEDebugger::setOldDeviceConnected(bool connected) {
    oldDeviceConnected = connected;
}