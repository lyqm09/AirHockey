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

#define INPUT_PIN1 27
#define INPUT_PIN2 12
#define OUTPUT_LEDR 32
#define OUTPUT_LEDG 25
#define OUTPUT_LEDB 33


#define MAX_POINTS 0x05;
uint8_t score[2];
bool bonus = false;
uint8_t red = LOW;
uint8_t green = LOW;
uint8_t blue = LOW;

static uint8_t outputData[2];
BLECharacteristic *pOutputChar;

void sendScoreboard() {
  outputData[0] = score[0];
  outputData[1] = score[1];

  pOutputChar->setValue((uint8_t *)outputData, 2);
  pOutputChar->notify();
  Serial.printf("Scoreboard : %02x - %02x\r\n", outputData[0], outputData[1]);
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
    score[(player+1)%2] += 2;
    return;
  }
  score[player] += points;
  updateGame();
}

void bonusAsyncTask(void *pvParameters) {
  (void) pvParameters;

  while(true) {
    vTaskDelay(pdMS_TO_TICKS((10+random(0,10))*60000));

    red = LOW;
    blue = LOW;
    green = HIGH;

    vTaskDelay(pdMS_TO_TICKS(10000));

    bonus = true;

    red = HIGH;
    blue = HIGH;
    green = LOW;

    vTaskDelay(pdMS_TO_TICKS(60000));
    bonus = false;

    red = LOW;
    blue = LOW;
    green = LOW;
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

  //delay(1000);

  outputData[0] = 0xFF;
  outputData[1] = 0xFF;
  pOutputChar->setValue((uint8_t *)outputData, 2);

  score[0] = 0xFF;
  score[1] = 0xFF;

  //delay(1000);

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  //delay(1000);

  Serial.println("BLE Service is advertising");

  //GOALS
  pinMode(INPUT_PIN1, INPUT);
  pinMode(INPUT_PIN2, INPUT);

  //LEDS
  pinMode(OUTPUT_LEDR, OUTPUT);
  pinMode(OUTPUT_LEDG, OUTPUT);
  pinMode(OUTPUT_LEDB, OUTPUT);

  xTaskCreatePinnedToCore(bonusAsyncTask, "BonusAsyncTask", 1024, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

int player1_threshold = 0;
int player2_threshold = 0;
int player2_off = 0;
int LIMIT = 10000;

void loop() {
  // put your main code here, to run repeatedly:
  int player1_entry = digitalRead(INPUT_PIN1);
  int player2_entry = digitalRead(INPUT_PIN2);

  if(player1_entry == HIGH) {
    if (player1_threshold < LIMIT + 15) {
      player1_threshold += 1;
    }
  } else {
    player1_threshold = 0;
  }

  if(player2_entry == HIGH) {
    if (player2_threshold < LIMIT + 15) {
      player2_threshold += 1;
    }
  } else {
    player2_off += 1;
  }
  if (player2_off > 200) {
    player2_threshold = 0;
    player2_off = 0;
  }


   if(Serial.available()>0) {
     int incomingData = Serial.parseInt();
     if (incomingData == 1) {
       player1_threshold = LIMIT;
     } else if (incomingData == 2) {
       player2_threshold = LIMIT;
     }
   }

  digitalWrite(OUTPUT_LEDR, red);
  digitalWrite(OUTPUT_LEDG, green);
  digitalWrite(OUTPUT_LEDB, blue);


  if(player1_threshold == LIMIT) {
    addScore(bonus? 1 : 0, bonus? -1 : 1);
  }
  if(player2_threshold == LIMIT) {
    addScore(bonus? 0 : 1, bonus? -1 : 1);
  }

}