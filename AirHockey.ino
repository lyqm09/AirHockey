#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define PERIPHERAL_NAME "Amp Air Hockey"
#define SERVICE_UUID "A71120B3-6254-44BD-A27F-42A66F1732BA"
#define CHARACTERISTIC_INPUT_UUID "E3B3CADD-710C-4A3D-9030-CB99823CB738"
#define CHARACTERISTIC_OUTPUT_UUID "32FE11C3-1807-4D05-81B3-4A301BDC3928"

#define INPUT_PIN1 4
#define INPUT_PIN2 5

#define MAX_POINTS 0x0A;
uint8_t score[2];
bool bonus = false;

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

void addScore(int player, int points) {
  if(player < 0 || player > 1) {
    return;
  }
  if(score[player] < 1 && points < 0) {
    return;
  }
  score[player] += points;
}

void bonusAsyncTask(void *pvParameters) {
  (void) pvParameters;

  while(true) {
    vTaskDelay(pdMS_TO_TICKS((10+random(0,10))*60000));
    //TODO: led
    vTaskDelay(pdMS_TO_TICKS(10000));
    bonus = true;
    //TODO: led
    vTaskDelay(pdMS_TO_TICKS(60000));
    bonus = false;
  }
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

  delay(1000);

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  delay(1000);

  Serial.println("BLE Service is advertising");

  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);

  xTaskCreatePinnedToCore(bonusAsyncTask, "BonusAsyncTask", 1024, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  // put your main code here, to run repeatedly:
  int player1 = digitalRead(INPUT_PIN1);
  int player2 = digitalRead(INPUT_PIN2);

  if(player1 == HIGH) {
    addScore(bonus? 0 : 1, bonus? -1 : 1);
  }
  if(player2 == HIGH) {
    addScore(bonus? 1 : 0, bonus? -1 : 1);
  }

}
