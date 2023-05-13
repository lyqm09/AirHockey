#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define PERIPHERAL_NAME "Amp Air Hockey"
#define SERVICE_UUID "A71120B3-6254-44BD-A27F-42A66F1732BA"
#define CHARACTERISTIC_INPUT_UUID "E3B3CADD-710C-4A3D-9030-CB99823CB738"
#define CHARACTERISTIC_OUTPUT_UUID "32FE11C3-1807-4D05-81B3-4A301BDC3928"

#define INPUT_PIN1 4
#define INPUT_PIN2 5

#define MAX_POINTS = 0x0A;
uint8_t score[2];

static uint8_t outputData[2];
BLECharacteristic *pOutputChar;

void sendScoreboard() {
  outputData[0] = score[0];
  outputData[1] = score[1];
  Serial.printf("Scoreboard : %02x - %02x\r\n", outputData[0], outputData[1]);

  pOutputChar->setValue((uint8_t *)outputData, 2);
  pOutputChar->notify();
}

void updateGame() {
  
  bool isWin1 = score[0] >= MAX_POINTS;
  bool isWin2 = score[1] >= MAX_POINTS;

  if(isWin1 || isWin2) {
    score[0] = 0xFE;
    score[1] = isWin1? 0x00: 0x01;
  }

  sendScoreboard();

  if(isWin1 || isWin2) {
    delay(5000);

    score[0] = 0x00;
    score[1] = 0x00;

    sendScoreboard();
  }
}

void addScore(int player, uint8_t points) {
  if(player < 0 || player > 1) {
    return;
  }
  score[player] += points;
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

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Setup BLE Service and Characteristics");

  BLEDevice::init(PERIPHERAL_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pOutputChar = pService->createCharacteristic(CHARACTERISTIC_OUTPUT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  pServer->setCallbacks(new ServerCallbacks());

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

  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  player1 = digitalRead(INPUT_PIN1);
  player2 = digitalRead(INPUT_PIN2);

  if(player1 == HIGH) {
    addScore(0, 0x01);
  }
  if(player2 == HIGH) {
    addScore(1, 0x01);
  }

}
