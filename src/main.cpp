#include "config.h"
#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>

const uint8_t SW_OPEN = GPIO_NUM_34;
const uint8_t SW_MEMBER = GPIO_NUM_33;
const uint8_t SW_CLOSE = GPIO_NUM_35;

const uint8_t LED_GREEN = GPIO_NUM_32;
const uint8_t LED_RED = GPIO_NUM_25;
const uint8_t LED_YELLOW = GPIO_NUM_27;

const char spaceOpenStr[] = "open";
const char spaceClosedStr[] = "closed";
const char spaceMembersOnlyStr[] = "membersOnly";

void blinkLED(uint8_t pin, uint16_t d) {
  digitalWrite(pin, HIGH);
  delay(d);
  digitalWrite(pin, LOW);
  delay(d);
}

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

enum spaceState_t { spaceUndefined, spaceOpen, spaceClosed, spaceMembersOnly };

volatile spaceState_t state = spaceUndefined;
volatile spaceState_t lastState = spaceUndefined;
volatile bool localChange = false;

void setSpaceMembersOnly() {
  state = spaceMembersOnly;
  localChange = true;
  Serial.println("member");
}
void setSpaceOpen() {
  state = spaceOpen;
  localChange = true;
  Serial.println("open");
}
void setSpaceClosed() {
  state = spaceClosed;
  localChange = true;
  Serial.println("close");
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.println("'");

  // use the Stream interface to print the contents
  char msg[50] = {};
  int p = 0;
  while (mqttClient.available()) {
    msg[p] = (char)mqttClient.read();
    p++;
  }
  // auto msg = mqttClient.readString().c_str();
  Serial.print("Value: '");
  Serial.print(msg);
  Serial.println("'");
  Serial.print("Setting state from MQTT to: ");
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

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(SW_OPEN, INPUT);
  pinMode(SW_MEMBER, INPUT);
  pinMode(SW_CLOSE, INPUT);

  /* ------------------------------- setup wifi ----------------------------- */
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID, wifiPass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* ---------------------------------- mqtt -------------------------------- */
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(mqttBroker);

  if (!mqttClient.connect(mqttBroker, mqttPort)) {

    Serial.print("MQTT connection failed! Error code = ");

    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("Connected to MQTT broker");
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println();
  Serial.println("Reading last state...");

  // monitor topic
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(spaceStateTopic);

  attachInterrupt(SW_OPEN, setSpaceOpen, FALLING);
  attachInterrupt(SW_MEMBER, setSpaceMembersOnly, FALLING);
  attachInterrupt(SW_CLOSE, setSpaceClosed, FALLING);
}

void publishState(spaceState_t s) {
  mqttClient.beginMessage(spaceStateTopic, true);
  switch (s) {
  case spaceOpen:
    mqttClient.print(spaceOpenStr);
    break;
  case spaceClosed:
    mqttClient.print(spaceClosedStr);
    break;
  case spaceMembersOnly:
    mqttClient.print(spaceMembersOnlyStr);
    break;
  }
  mqttClient.endMessage();
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

void loop() {
  if (lastState != state) {
    Serial.print("Last state: ");
    Serial.println(lastState);
    Serial.print("State: ");
    Serial.println(state);

    if (localChange) {
      Serial.println("publish...");
      publishState(state);
      localChange = false;
    }
    updateLEDs(state);
    lastState = state;
  }
  mqttClient.poll();
}
