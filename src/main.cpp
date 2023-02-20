#include "beep.h"
#include "config.h"
#include "gen_voice_data.h"
#include "state.h"
#include "util.h"
#include <Arduino.h>
#include <ArduinoHA.h>
#include <ArduinoOTA.h>
#include <Bounce2.h>
#include <WiFi.h>
#include <XT_DAC_Audio.h>

WiFiClient wifiClient;
HADevice haDevice;
HAMqtt mqtt(wifiClient, haDevice);
HASelect xState("xState");

volatile spaceState_t state = spaceUndefined;
volatile spaceState_t lastState = spaceUndefined;
volatile bool localChange = false;

// Audio
XT_DAC_Audio_Class audioPlayer(26, 0);
XT_Wav_Class voiceOpen(voice_open);
XT_Wav_Class voiceMembers(voice_members);
XT_Wav_Class voiceClosed(voice_closed);

// Buttons
Bounce openButton = Bounce();
Bounce memberButton = Bounce();
Bounce closeButton = Bounce();
const uint8_t debounceInterval = 2;
Bounce *buttons[] = {&openButton, &memberButton, &closeButton};

/* ---------------------------- button functions -------------------------- */
void setupButtons() {
  pinMode(SW_OPEN, INPUT);
  openButton.attach(SW_OPEN);
  openButton.interval(debounceInterval);

  pinMode(SW_CLOSE, INPUT);
  closeButton.attach(SW_CLOSE);
  closeButton.interval(debounceInterval);

  pinMode(SW_MEMBER, INPUT);
  memberButton.attach(SW_MEMBER);
  memberButton.interval(debounceInterval);
}

void updateButtons() {
  for (auto btn : buttons) {
    btn->update();
  }
}

/* ----------------------------- util functions ------------------------- */
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

void setupLEDs() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
}

void setupBuzzer() { pinMode(BUZZER, OUTPUT); }

/* ----------------------------- wifi functions --------------------------- */

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(wifiSSID);

  WiFi.setHostname("xstate-buttons");
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

void onSelectCommand(int8_t s, HASelect *sender) {
  if (sender == &xState) {
    state = (spaceState_t)s;
  }
  sender->setState(s); // report state back to the Home Assistant
}

void setupHomeAssistant() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  haDevice.setUniqueId(mac, sizeof(mac));
  haDevice.setName("xState");
  haDevice.setSoftwareVersion("1.0.0");
  haDevice.setManufacturer("xHain hack+makespace (gueldi)");
  haDevice.setModel("xState-esp");
  haDevice.enableSharedAvailability();
  haDevice.enableLastWill();
  xState.setRetain(true);
  xState.setOptions(stateOptionsStr); // xHain is ...
  xState.setIcon("mdi:door");
  xState.onCommand(onSelectCommand);
  if (!mqtt.begin(mqttBroker, mqttUser, mqttPass)) {
    Serial.println("mqtt error");
    displayErrorLoop(LED_BUILTIN);
  }
}

void setupOTA() {
  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
        // using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  setupButtons();
  setupLEDs();
  setupBuzzer();
  setupWifi();
  setupOTA();
  setupHomeAssistant();
  state = (spaceState_t)xState.getCurrentState();
  digitalWrite(LED_BUILTIN, HIGH);
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
void playBuzzer(spaceState_t s) {
  switch (s) {
  case spaceOpen:
    playOpen(BUZZER);
    break;
  case spaceClosed:
    playClose(BUZZER);
    break;
  case spaceMembersOnly:
    playMember(BUZZER);
    break;
  }
}

void outputVoice(spaceState_t s) {
  switch (s) {
  case spaceOpen:
    audioPlayer.Play(&voiceOpen, false);
    break;
  case spaceClosed:
    audioPlayer.Play(&voiceClosed, false);
    break;
  case spaceMembersOnly:
    audioPlayer.Play(&voiceMembers, false);
    break;
  }
}

void loop() {
  mqtt.loop();
  updateButtons();
  if (openButton.fell()) {
    state = spaceOpen;
  } else if (memberButton.fell()) {
    state = spaceMembersOnly;
  } else if (closeButton.fell()) {
    state = spaceClosed;
  } else {
    state = (spaceState_t)xState.getCurrentState();
  }
  if (lastState != state) {
    Serial.printf("Transition from %s to %s.\n", stateToString(lastState),
                  stateToString(state));
    if (!xState.setState(state)) {
      Serial.println("error publishing state");
    }
    playBuzzer(state);
    lastState = state;
  }
  updateLEDs(state);
  ArduinoOTA.handle();
}
