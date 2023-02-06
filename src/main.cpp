#include "config.h"
#include "state.h"
#include "util.h"
#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <Audio.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

/* ---------------------------- global variables -------------------------- */
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

volatile spaceState_t state = spaceUndefined;
volatile spaceState_t lastState = spaceUndefined;
volatile bool localChange = false;

//****************************************************************************************
//                                   A U D I O _ T A S K *
//****************************************************************************************
Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);

struct audioMessage {
  uint8_t cmd;
  const char *txt;
  uint32_t value;
  uint32_t ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t {
  SET_VOLUME,
  GET_VOLUME,
  CONNECTTOHOST,
  CONNECTTOSD,
  CONNTECTTOTTS
};

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

void CreateQueues() {
  audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter) {
  CreateQueues();
  if (!audioSetQueue || !audioGetQueue) {
    log_e("queues are not initialized");
    while (true) {
      ;
    } // endless loop
  }

  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;

  audio.setVolume(21); // 0...21

  while (true) {
    if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
      if (audioRxTaskMessage.cmd == SET_VOLUME) {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      } else if (audioRxTaskMessage.cmd == CONNECTTOSD) {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      } else if (audioRxTaskMessage.cmd == CONNTECTTOTTS) {
        audioTxTaskMessage.cmd = CONNTECTTOTTS;
        audioTxTaskMessage.ret =
            audio.connecttospeech(audioRxTaskMessage.txt, "en");
      } else {
        log_i("error");
      }
    }
    audio.loop();
  }
}

void setupAudio() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SD.begin(SD_CS);
  xTaskCreatePinnedToCore(audioTask,   /* Function to implement the task */
                          "audioplay", /* Name of the task */
                          5000,        /* Stack size in words */
                          NULL,        /* Task input parameter */
                          2 | portPRIVILEGE_BIT, /* Priority of the task */
                          NULL,                  /* Task handle. */
                          0 /* Core where the task should run */
  );
}

audioMessage transmitReceive(audioMessage msg) {
  xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS) {
    if (msg.cmd != audioRxMessage.cmd) {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}

void audioSetVolume(uint8_t vol) {
  audioTxMessage.cmd = SET_VOLUME;
  audioTxMessage.value = vol;
  audioMessage RX = transmitReceive(audioTxMessage);
}

bool audioConnecttohost(const char *host) {
  audioTxMessage.cmd = CONNECTTOHOST;
  audioTxMessage.txt = host;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

bool audioConnecttoSD(const char *filename) {
  audioTxMessage.cmd = CONNECTTOSD;
  audioTxMessage.txt = filename;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}
bool audioConnecttoSpeech(const char *txt) {
  audioTxMessage.cmd = CONNTECTTOTTS;
  audioTxMessage.txt = txt;
  audioMessage RX = transmitReceive(audioTxMessage);
  return RX.ret;
}

/* ----------------------------- util functions --------------------------- */
void setSpaceMembersOnly() {
  state = spaceMembersOnly;
  localChange = true;
}
void setSpaceOpen() {
  state = spaceOpen;
  localChange = true;
}
void setSpaceClosed() {
  state = spaceClosed;
  localChange = true;
}

void setupPins() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(SW_OPEN, INPUT);
  pinMode(SW_MEMBER, INPUT);
  pinMode(SW_CLOSE, INPUT);
  pinMode(BUZZER, OUTPUT);
}

/* ----------------------------- mqtt functions --------------------------- */
void setupMQTT() {
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(mqttBroker);

  if (!mqttClient.connect(mqttBroker, mqttPort)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    displayErrorLoop(LED_BUILTIN);
  }

  Serial.println("Connected to MQTT broker");
  Serial.println();
  digitalWrite(LED_BUILTIN, HIGH); // signal connection
}

void publishState(spaceState_t s) {
  mqttClient.beginMessage(spaceStateTopic, true);
  mqttClient.print(stateToString(s));
  mqttClient.endMessage();
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("': ");
  char msg[50] = {};
  int p = 0;
  while (mqttClient.available()) {
    msg[p] = (char)mqttClient.read();
    p++;
  }
  Serial.println(msg);
  if (strcmp(msg, spaceOpenStr) == 0) {
    state = spaceOpen;
    Serial.println(spaceOpen);
  } else if (strcmp(msg, spaceMembersOnlyStr) == 0) {
    state = spaceMembersOnly;
    Serial.println(spaceMembersOnly);
  } else {
    state = spaceClosed;
    Serial.println(spaceClosed);
  }
}

/* ----------------------------- wifi functions --------------------------- */

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID, wifiPass);
  while (WiFi.status() != WL_CONNECTED) {
    blinkLED(LED_BUILTIN, 250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

//****************************************************************************************
//                                   S E T U P *
//****************************************************************************************

void setup() {
  Serial.begin(115200);
  Serial.print("Setup: Executing on core ");
  Serial.println(xPortGetCoreID());
  setupAudio();
  setupPins();
  setupWifi();
  setupMQTT();

  // monitor topic
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(spaceStateTopic);

  /* ------------------------------- interrupts ----------------------------- */
  attachInterrupt(SW_OPEN, setSpaceOpen, FALLING);
  attachInterrupt(SW_MEMBER, setSpaceMembersOnly, FALLING);
  attachInterrupt(SW_CLOSE, setSpaceClosed, FALLING);
}

void updateLEDs(spaceState_t s) {
  switch (s) {
  case spaceOpen:
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    break;
  case spaceClosed:
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    break;
  case spaceMembersOnly:
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    break;
  }
}

void outputVoice(spaceState_t s) {
  switch (s) {
  case spaceOpen:
    audioConnecttoSD("/open.wav");
    break;
  case spaceClosed:
    audioConnecttoSD("/closed.wav");
    break;
  case spaceMembersOnly:
    audioConnecttoSD("/members.wav");
    break;
  }
}

//****************************************************************************************
//                                   L O O P *
//****************************************************************************************

void loop() {
  static bool once = true;
  if (once) {
    Serial.print("Loop: Executing on core ");
    Serial.println(xPortGetCoreID());
    once = false;
  }
  if (lastState != state) {
    Serial.printf("Transition from %s to %s.\n", stateToString(lastState),
                  stateToString(state));
    // check if change needs to be published
    if (localChange) {
      publishState(state);
      localChange = false;
    }
    // update LEDs and play sound
    updateLEDs(state);
    outputVoice(state);
    // store state
    lastState = state;
  }
  mqttClient.poll();
}

//*****************************************************************************************
//                                  E V E N T S *
//*****************************************************************************************

void audio_info(const char *info) {
  Serial.print("audio: ");
  Serial.println(info);
}
