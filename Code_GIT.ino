//code fenetre et porte capteur avec keypad

#include <WiFiS3.h>

#include <ArduinoMqttClient.h>

#include <ArduinoBLE.h>

 

// =======================================================

// CONFIG WIFI + MQTT

// =======================================================

const char* SSID        = "GG_TEP";

const char* PASS        = "lolaflorez&";

 

const char* AIO_SERVER  = "io.adafruit.com";

const uint16_t AIO_PORT = 1883;

 

const char* AIO_USERNAME = "sohaib817923541";

const char* AIO_KEY      = "aio_DEHO62AxsdEkkU5T7WvmmybLYkQ5";

 

const char* FEED_NAME   = "security";

 

// =======================================================

// CONFIG BLE

// =======================================================

const char* TARGET_NAME = "lampe";

 

const char* NUS_SERVICE = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";

const char* NUS_TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

 

BLEDevice peripheral;

BLECharacteristic txChar;

 

// =======================================================

// MQTT + GLOBALS

// =======================================================

WiFiClient wifiClient;

MqttClient mqttClient(wifiClient);

 

unsigned long lastMqttAttempt = 0;

unsigned long lastBleAttempt  = 0;

 

// =======================================================

// CLAVIER 12 TOUCHES

// =======================================================

const int buttonPins[12] = {2, 5, 9, 12, 8, 7, 13, 3, 6, 11, 4, 10};

 

const char keyMap[12] = {

  '1','2','#',

  '4','5','6',

  '7','8','9',

  '*','0','3'

};

 

bool buttonState[12] = {false};

String codeBuffer = "";

 

// =======================================================

// MQTT PUBLISH FUNCTION

// =======================================================

void publishStatus(String msg) {

  String topic = String(AIO_USERNAME) + "/feeds/" + FEED_NAME;

 

  mqttClient.beginMessage(topic);

  mqttClient.print(msg);

  mqttClient.endMessage();

 

  Serial.print("MQTT => ");

  Serial.print(topic);

  Serial.print(" : ");

  Serial.println(msg);

}

 

// =======================================================

// KEYBOARD HANDLER (VERSION QUI ENVOIE LE CODE BRUT À NODE-RED)

// =======================================================

void checkKeypad() {

  for (int i = 0; i < 12; i++) {

    bool pressed = (digitalRead(buttonPins[i]) == LOW);

 

    if (pressed && !buttonState[i]) {

      char key = keyMap[i];

 

      Serial.print("Touche : ");

      Serial.println(key);

 

      // Chiffres

      if (key >= '0' && key <= '9') {

        if (codeBuffer.length() < 12) {

          codeBuffer += key;

          Serial.print("Code actuel : ");

          Serial.println(codeBuffer);

        }

      }

 

      // RESET (#)

      if (key == '#') {

        codeBuffer = "";

        Serial.println("Code effacé (#)");

      }

 

      // ENVOYER (*) → envoie tout le code brut

      if (key == '*') {

        if (codeBuffer.length() > 0) {

 

          String msg = "code=" + codeBuffer;

          Serial.print("Envoi du code vers MQTT : ");

          Serial.println(msg);

 

          publishStatus(msg);   // ENVOIE LE CODE BRUT À NODE-RED

 

        } else {

          Serial.println("Aucun code à envoyer");

        }

 

        codeBuffer = ""; // reset après l’envoi

      }

 

      buttonState[i] = true;

    }

 

    if (!pressed && buttonState[i])

      buttonState[i] = false;

  }

}

 

// =======================================================

// WIFI CONNECT

// =======================================================

void connectWiFi() {

  Serial.print("WiFi connecting");

  WiFi.begin(SSID, PASS);

 

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");

    delay(500);

  }

 

  Serial.println("\nWiFi connected.");

  Serial.print("IP: ");

  Serial.println(WiFi.localIP());

}

 

// =======================================================

// MQTT CONNECT (PATCH ANTI CODE=6)

// =======================================================

bool connectMQTT() {

  Serial.print("MQTT connecting... ");

 

  mqttClient.setId("unoR4");  // STABLE

  mqttClient.setUsernamePassword(AIO_USERNAME, AIO_KEY);

 

  if (!mqttClient.connect(AIO_SERVER, AIO_PORT)) {

    Serial.print("FAILED, code=");

    Serial.println(mqttClient.connectError());

    delay(6000);  // très important

    return false;

  }

 

  Serial.println("OK");

  return true;

}

 

// =======================================================

// BLE CONNECT TO PHOTON

// =======================================================

bool ensureBleConnected() {

  if (peripheral && peripheral.connected() && txChar)

    return true;

 

  if (millis() - lastBleAttempt < 3000)

    return false;

 

  lastBleAttempt = millis();

 

  Serial.println("BLE scanning...");

  BLE.scan();

 

  unsigned long start = millis();

  while (millis() - start < 8000) {

    BLEDevice dev = BLE.available();

    if (!dev) continue;

 

    if (dev.localName() == TARGET_NAME) {

      BLE.stopScan();

      Serial.println("Photon trouvé, connexion...");

 

      if (!dev.connect()) return false;

      if (!dev.discoverAttributes()) return false;

 

      BLEService svc = dev.service(NUS_SERVICE);

      if (!svc) return false;

 

      txChar = svc.characteristic(NUS_TX_UUID);

      if (!txChar) return false;

 

      if (txChar.canSubscribe())

        txChar.subscribe();

 

      peripheral = dev;

      Serial.println("BLE connecté au Photon");

      return true;

    }

  }

 

  Serial.println("Photon introuvable");

  return false;

}

 

// =======================================================

// SETUP

// =======================================================

void setup() {

  Serial.begin(115200);

  delay(200);

 

  Serial.println("UNO R4 WiFi : BLE + MQTT + KEYBOARD READY");

 

  connectWiFi();

  connectMQTT();

 

  if (!BLE.begin()) {

    Serial.println("BLE FAILED");

    while (1);

  }

  Serial.println("BLE OK");

 

  // Init clavier

  for (int i = 0; i < 12; i++)

    pinMode(buttonPins[i], INPUT_PULLUP);

 

  Serial.println("Clavier prêt !");

}

 

// =======================================================

// LOOP

// =======================================================

void loop() {

 

  // --- MQTT reconnection ---

  if (!mqttClient.connected()) {

    unsigned long now = millis();

    if (now - lastMqttAttempt > 10000) {

      lastMqttAttempt = now;

      connectMQTT();

    }

  } else {

    mqttClient.poll();

  }

 

  // --- BLE ---

  ensureBleConnected();

  BLE.poll();

 

  // Lire les données du Photon

  if (peripheral && peripheral.connected() && txChar) {

    if (txChar.valueUpdated()) {

 

      uint8_t buf[64];

      int len = txChar.valueLength();

      if (len > 63) len = 63;

 

      txChar.readValue(buf, len);

      buf[len] = '\0';

 

      String msg = (char*)buf;

 

      Serial.print("BLE -> ");

      Serial.println(msg);

 

      publishStatus(msg);

    }

  }

 

  // --- CLAVIER ---

  checkKeypad();

 

  delay(5);

}

 


