#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static const int FSR_PIN = 36;   

#define SERVICE_UUID   "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_UUID      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

BLECharacteristic* fsrChar;
bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override { deviceConnected = true; }
  void onDisconnect(BLEServer* pServer) override { deviceConnected = false; }
};

void setup() {
  Serial.begin(9600);

  analogReadResolution(12); 

  BLEDevice::init("ESP32-FSR");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  fsrChar = service->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  fsrChar->addDescriptor(new BLE2902()); 
  fsrChar->setValue((uint16_t)0);

  service->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  adv->start();

  Serial.println("BLE ready. Look for ESP32-FSR.");
}

void loop() {
  uint16_t raw = analogRead(FSR_PIN);

  
  Serial.println(raw);

  if (deviceConnected) {
    uint8_t payload[2] = { (uint8_t)(raw & 0xFF), (uint8_t)(raw >> 8) };
    fsrChar->setValue(payload, 2);
    fsrChar->notify();
  }

  delay(50);
}
