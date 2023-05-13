#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define PERIPHERAL_NAME "Amp Air Hockey"
#define SERVICE_UUID "A71120B3-6254-44BD-A27F-42A66F1732BA"
#define CHARACTERISTIC_INPUT_UUID "E3B3CADD-710C-4A3D-9030-CB99823CB738"
#define CHARACTERISTIC_OUTPUT_UUID "32FE11C3-1807-4D05-81B3-4A301BDC3928"

static uint8_t outputData[2];
BLECharacteristic *pOutputChar;

static uint8_t points = 0x0A;
uint8_t score[2];

void sendScoreboard() {
  outputData[0] = score[0];
  outputData[1] = score[1];
  Serial.printf("Scoreboard : %02x - %02x\r\n", outputData[0], outputData[1]);

  pOutputChar->setValue((uint8_t *)outputData, 2);
  pOutputChar->notify();
}

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("BLE Client Connected");

    if(score[0] == 0xFF) {
      score[0] = 0x00;
      score[1] = 0x00;
    }

    sendScoreboard();
  }
  
  void onDisconnect(BLEServer* pServer) {
    BLEDevice::startAdvertising();
    Serial.println("BLE Client Disconnected");
  }
};

// class InputReceivedCallbacks: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharWriteState) {
//     uint8_t *inputValues = pCharWriteState->getData();
//     //data
//     if(score[0] == 0xFF) {
//       score[0] = 0x00;
//       score[1] = 0x00;
//     }

//     outputData[0] = score[0];
//     outputData[1] = score[1];
//     Serial.printf("Sending response : %02x %02x\r\n", outputData[0], outputData[1]);

//     pOutputChar->setValue((uint8_t *)outputData, 2);
//     pOutputChar->notify();
//   }
// };

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Setup BLE Service and Characteristics");

  BLEDevice::init(PERIPHERAL_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  //BLECharacteristic *pInputChar = pService->createCharacteristic(CHARACTERISTIC_INPUT_UUID, BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE);
  pOutputChar = pService->createCharacteristic(CHARACTERISTIC_OUTPUT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pServer->setCallbacks(new ServerCallbacks());
  //pInputChar->setCallbacks(new InputReceivedCallbacks());

  outputData[0] = 0xFF;
  outputData[1] = 0xFF;
  pOutputChar->setValue((uint8_t *)outputData, 2);

  score[0] = 0xFF;
  score[1] = 0xFF;

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Service is advertising");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
