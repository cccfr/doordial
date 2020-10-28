//#include <WiFi.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <OneButton.h>
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <TM1637Display.h> // 7-Segment
#include "7-seg-anim.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
unsigned long lastUpdate = millis();

// dial
#define busyPin 5 // becomes low while dialing a number
#define countPin 4 // is normaly high, becomes low for each count event
#define pinLen 4// how long should a pin be
#define resetPin 15 // phonehook button
char buf[100];

//OneButton busyButton(busyPin, true);
OneButton countButton(countPin, false);
OneButton resetButton(resetPin, true);
//bool busy = false;
bool newDigit = false;
int count = 0;
int digitPos = 0;
char digits[pinLen+1]; // +1 because the char array terminates with an extra null byte
char challenge[pinLen+1];

// rfid
uint8_t successRead;
bool newData;
char rfidbuf[32];
// Create MFRC522 instance.
#define RFID_SS_PIN 10
#define RFID_RST_PIN 9
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

// 7-Segment
#define sevenCLK 0
#define sevenDIO 2
uint8_t sevenData[] = { 0x00, 0xf0, 0x01, 0xa1 };
#define TEST_DELAY   2000
uint8_t ANIM_pos = 0;
bool HOR_pos = false;
bool VER_pos = false;

// Relay
#define RelayPin 15

TM1637Display display(sevenCLK, sevenDIO);

// https
WiFiClientSecure httpsClient; 
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  //WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("waiting for wifi");
  //ss.begin(9600);

  display.setBrightness(0x0f);
  
  initDial();
  
  while (WiFi.status() != WL_CONNECTED)
  {
    sevenCircle();
    delay(500);
    Serial.print(".");
  }
  sevenDashed();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  setupOTA();
  mqttClient.setServer(MQTT_BROKER, 1883);
  Serial.print("MQTT_Broker: ");
  Serial.println(MQTT_BROKER);
  mqttClient.setCallback(callback);
  randomSeed(micros());

  // Relay
  pinMode(RelayPin , OUTPUT);
  digitalWrite(RelayPin, LOW);

  // rfid
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware
  //If you set Antenna Gain to Max it will increase reading distance
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.println(F("RFID Reader: "));   // For debugging purposes
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

  httpsClient.setFingerprint(FINGERPRINT);
  httpsClient.setTimeout(5000);

  sevenReady();


}

void setupOTA() {
  Serial.println("setup OTA");
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp-doordial");
  ArduinoOTA.setPasswordHash("482d2757995ddf74dcf0bac1f94be186");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
}

void initDial()
{
  resetDigits();
  //busyButton.attachClick(busyClick);
  pinMode(busyPin, INPUT_PULLUP);
  countButton.attachClick(countClick);
  //resetButton.attachClick(resetClick);
  //pinMode(resetPin, INPUT_PULLUP);
}

void busyStart()
{
  //Serial.println("busyStart");
  //busy = true;
}

void busyClick()
{
  //Serial.println("busyClick");
  //busy = false;
}

void countClick()
{
  count ++;
  //Serial.print("countClick: ");
  //Serial.println(count);
}

void resetClick()
{
  Serial.println("reset clicked");
  resetDigits();
}

void loop()                                                                                             
{
  while (WiFi.status() != WL_CONNECTED)
  {
    sevenCircle();
    delay(500);
    Serial.print(".");
  }
  ArduinoOTA.handle();
  //Serial.print("reset: ");
  //Serial.println(digitalRead(resetPin));
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  mqttClient.loop();
  
  //dialLoop();

  newData = false;
  rfidLoop();
  if ( newData ) {
      if (!getChallenge()) {
        sevenDashed();
        return;
      }
      displayChallenge();
      triggerRelay(); // open Hatch
      unsigned long startTime = millis();
      while ( digitPos < pinLen ) {
        dialLoop();
        if (millis() - startTime > 30000) {
          Serial.println("challenge timed out");
          return;
        }
        delay(1);
      }
      if (sendChallenge()) {
        for(int i = 0; i < 2; i++)
        {
          sevenBlink();
          delay(500);
        }
      } else  {
        for(int i = 0; i < 5; i++)
        {
          sevenFailed();
          delay(100);
        }
      }
      resetDigits();
  }
  

  //Serial.print("new Data: ");
  //Serial.println(newData);
  /*
  if (newData && millis() - lastUpdate >= 1000) {
    updateTopics();
    lastUpdate = millis();
  }
  */
  delay(1);

}

void rfidLoop()
{
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.println("new Card");
    } else {
      Serial.println("failed to read Card");
    }

    unsigned long hex_num;
    hex_num =  mfrc522.uid.uidByte[0] << 24;
    hex_num += mfrc522.uid.uidByte[1] << 16;
    hex_num += mfrc522.uid.uidByte[2] <<  8;
    hex_num += mfrc522.uid.uidByte[3];
    //mfrc522.PICC_HaltA(); // Stop reading
    sprintf(rfidbuf, "%ul", hex_num);
    Serial.println(rfidbuf);
    newData = true;
  }
}

void dialLoop()
{
  //busyButton.tick();
  countButton.tick();
  resetButton.tick();

  //newData = false;
  while(!digitalRead(busyPin)) {
    //busyButton.tick();
    countButton.tick();
    delay(1);
  }

  if(count > 0 && count < 11) {
    /*
    Serial.print("adding ");
    Serial.print(count);
    Serial.print(" to ");
    Serial.print(digits);
    Serial.print(" at pos ");
    Serial.println(digitPos);
    */
    addDigit();
    count = 0;
  }
  else if (count != 0) {
    Serial.print("invalid count ");
    Serial.println(count);
    count = 0;
  }
  
  // trigger if entered pin is long enough
  if( digitPos == pinLen) {
    Serial.print("\nentered Pin: ");
    Serial.println(digits);
    //resetDigits();
  }
}

bool getChallenge() {
  sevenVer();
  Serial.println("getChallenge");
  if (!httpsClient.connect(AUTH_HOST, AUTH_PORT)) {
    Serial.println("getChallenge failed");
    return false;
  }
  httpsClient.print(String("POST /api/startchallenge") + " HTTP/1.1\r\n" +
               "Host: " + AUTH_HOST + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +
               "Content-Length: 32" + "\r\n\r\n" +
               rfidbuf + "\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");

  while (httpsClient.connected()) {
    sevenVer();
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  Serial.println("reply was:");
  Serial.println("==========");
  char statuscode[pinLen+1];
  for (int i = 0; i<= pinLen; i++) {
    sevenVer();     
    statuscode[i] = httpsClient.read();
  }
  statuscode[pinLen] = '\0';
  if (strcmp(statuscode, "OkKc") != 0) {
    Serial.printf("%s error getting the challenge\n", statuscode);
    httpsClient.stop();
    return false;
  }
  for(int i=0; i<=pinLen; i++) {
    sevenVer();
    challenge[i] = httpsClient.read();
  }
  challenge[pinLen] = '\0';
  Serial.print("challenge: ");
  Serial.println(challenge);

  Serial.println("==========");
  Serial.println("closing connection");
  httpsClient.stop();

  return true;
}

bool sendChallenge() {
  sevenHor();
  Serial.println("sendChallenge");
  if (!httpsClient.connect(AUTH_HOST, AUTH_PORT)) {
    Serial.println("sendChallenge failed");
    return false;
  }
  httpsClient.print(String("POST /api/solvechallenge") + " HTTP/1.1\r\n" +
               "Host: " + AUTH_HOST + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +
               "Content-Length: 32" + "\r\n\r\n" +
               rfidbuf + "#" + digits + "\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("challenge sent");

  while (httpsClient.connected()) {
    sevenHor();
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  char statuscode[pinLen+1];
  for (int i = 0; i<= pinLen; i++) {
    sevenHor();     
    statuscode[i] = httpsClient.read();
  }
  statuscode[pinLen] = '\0';
  if (strcmp(statuscode, "OkAg") == 0) {
    Serial.println("Access Granted");
    httpsClient.stop();
    return true;
  }
  httpsClient.stop();
  Serial.printf("%s error with sent challenge\n", statuscode);
  return false;
}

void addDigit()
{
  if (digitPos > pinLen) {
    Serial.println("digitPos out of bound");
    resetDigits();
    return;
  }

  // '0' is encoded as 10 interrupts
  if (count == 10){
    count = 0;
  }
  itoa(count, buf, 10);
  digits[digitPos] = buf[0];
  sevenData[digitPos] = 0;
  display.setSegments(sevenData);
  digitPos ++;
  Serial.print(count);
}

void resetDigits()
{
  Serial.println("resetting");
  digitPos = 0;
  for(int i=0; i<pinLen; i++) {
    digits[i] = '0';
  }
  digits[pinLen] = '\0';
  sevenDashed();
}

void displayChallenge() {
    for(int i = 0; i < 4; i++)
    {
      sevenData[i] = display.encodeDigit(int(challenge[i]));
      challenge[i] = SEG_E | SEG_B;
    }
    /*
    Serial.print("display challenge: ");
    for(int i = 0; i < 4; i++)
    {
      Serial.print(sevenData[i]);
    }
    Serial.println("");
    */
    display.setSegments(sevenData);
    
}

void sevenDashed() {
    for(int i = 0; i < 4; i++)
    {
      sevenData[i] = SEG_G;
    }
    display.setSegments(sevenData);
}

void sevenHor() {
    if (HOR_pos) {
      ANIM_HOR(sevenData, 0);
      HOR_pos = false;
    } else {
      ANIM_HOR(sevenData, 1);
      HOR_pos = true;
    }
    display.setSegments(sevenData);
}

void sevenVer() {
    if (VER_pos) {
      ANIM_VER(sevenData, 0);
      VER_pos = false;
    } else {
      ANIM_VER(sevenData, 1);
      VER_pos = true;
    }
    display.setSegments(sevenData);
}

void sevenCircle() {
    //Serial.println("sevenCircle");
    ANIM_CIRCLE(sevenData, ANIM_pos);
    //for(int i = 0; i < 4; i++)
    //{
    //  Serial.println(sevenData[i]);
    //}
    display.setSegments(sevenData);
    ANIM_pos++;
    if (ANIM_pos >= 12) {
      ANIM_pos = 0;
    }
}

void sevenBlink() {
  ANIM_BLINK(sevenData, 0);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 0);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
}

void sevenFailed() {
  ANIM_BLINK(sevenData, 2);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 2);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
}

void sevenReady() {
  ANIM_BLINK(sevenData, 3);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 3);
  display.setSegments(sevenData);
  delay(300);
  ANIM_BLINK(sevenData, 1);
  display.setSegments(sevenData);
  delay(300);
}

void updateTopics() {
  Serial.println("update topics");
  mqttClient.publish("door/lastOpening", "not implemented"); // 
}

void reconnectMQTT() {
  Serial.println("reconnect");
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    sevenCircle();
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Display-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      sevenDashed();
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received message [");
    Serial.print(topic);
    Serial.print("] ");

    char text[length+1];
    for (int i = 0; i < length; i++) {
      text[i] = (char)payload[i];
    }
    text[length] = '\0';
    Serial.println(text);
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    while (true); // do not go further
  }
}

void triggerRelay() {
  digitalWrite(RelayPin,HIGH);
  delay(3000);
  digitalWrite(RelayPin,LOW);
}
