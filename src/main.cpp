#include "beep.h"
#include "config.h"
#include "gen_voice_data.h"
#include "state.h"
#include "util.h"
#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <XT_DAC_Audio.h>

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

volatile spaceState_t state = spaceUndefined;
volatile spaceState_t lastState = spaceUndefined;
volatile bool localChange = false;

// Audio
XT_DAC_Audio_Class audioPlayer(26, 0);
XT_Wav_Class voiceOpen(voice_open);
XT_Wav_Class voiceMembers(voice_members);
XT_Wav_Class voiceClosed(voice_closed);

/* ----------------------------- util functions ---------------------------
 */
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

void setup() {
  Serial.begin(115200);
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
void playSound(spaceState_t s) {
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
    audioPlayer.Play(&voiceOpen);
    break;
  case spaceClosed:
    audioPlayer.Play(&voiceClosed);
    break;
  case spaceMembersOnly:
    audioPlayer.Play(&voiceMembers);
    break;
  }
}

void loop() {
  audioPlayer.FillBuffer();
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
    // playSound(state);
    outputVoice(state);
    // store state
    lastState = state;
  }
  mqttClient.poll();
}
