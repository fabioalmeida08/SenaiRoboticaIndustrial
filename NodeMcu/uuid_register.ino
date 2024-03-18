#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include "UUID.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Arduino.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
const char* ssid = "";
const char* password = "";
const char* serverUrl = "";
const int BUTTON_PIN = 0;
int buttonState = 0;

byte wifiChar[8] = {
  B00000,
  B00000,
  B00000,
  B11110,
  B00001,
  B11001,
  B00101,
  B10101
};

void setup()
{
  Serial.begin(115200);
  lcd.createChar(0, wifiChar);
  lcd.init();
  lcd.backlight();

  WiFi.begin(ssid, password);
  for (int i = 0; i <= 10; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      wifi_connecting_display_lcd();
      Serial.println("Conectando ao WiFi...");
    } else if (WiFi.status() == WL_CONNECTED) {
      wifi_connected_success_lcd();
      Serial.println("Conectado ao WiFi");
      delay(2000);
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    wifi_connecting_fail_lcd();
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}
 
void loop()
{
  buttonState = digitalRead(BUTTON_PIN);
  Serial.println(buttonState);
  if (buttonState == LOW) {
    UUID uuid = create_uuid();
    send_message(uuid);
    displayUUID(uuid);
  }
  
}

void displayUUID (UUID uuid) {
  lcd.clear();
  lcd.setCursor(6, 0); 
  lcd.print("UUID:");
  lcd.setCursor(0, 1);
  lcd.print(uuid);
  for (int positionCounter = 0; positionCounter < 24; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(250);
  }
  for (int positionCounter = 0; positionCounter < 24; positionCounter++) {
    lcd.scrollDisplayRight();
    delay(250);
  }
}

String send_message(UUID uuid) {
  sending_msg_display_lcd();

  WiFiClientSecure client;
  client.setInsecure();

  StaticJsonDocument<48> doc;

  doc["uuid"] = uuid;
  String json_payload;
  serializeJson(doc, json_payload);

  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(json_payload);
  Serial.print("codigo de resposta == ");
  Serial.print(httpResponseCode);
  display_message_lcd(httpResponseCode);
  return http.getString();
}


void display_message_lcd(int code) {
  char code_buffer[10];
  sprintf(code_buffer, "%d", code);
  if (code == 201 || code == 200) {
    success_msg_lcd();
  } else {
    fail_msg_lcd(code_buffer);
  }
}

void success_msg_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mensagem Enviada");
  lcd.setCursor(0, 1);
  lcd.print("com sucesso !!!");
  delay(2000);
}

void fail_msg_lcd(char* code) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Erro ao enviar");
  lcd.setCursor(0, 1);
  lcd.print("Msg - (err:");
  lcd.setCursor(11, 1);
  lcd.print(code);
  lcd.setCursor(14, 1);
  lcd.print(")");
  delay(4000);
}

void wifi_connecting_display_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando ao wifi");
  lcd.setCursor(0, 1);
  lcd.print("Por favor aguarde...");
  for (int positionCounter = 0; positionCounter < 5; positionCounter++) {
    lcd.scrollDisplayLeft();
    delay(500);
  }
  for (int positionCounter = 0; positionCounter < 5; positionCounter++) {
    lcd.scrollDisplayRight();
    delay(500);
  }
}

void wifi_connected_success_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado");
  lcd.setCursor(0, 1);
  lcd.print("com Sucesso !!!");
  lcd.setCursor(15, 1);
  lcd.write((uint8_t)0);
  delay(2000);
}

void wifi_connecting_fail_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Erro ao ");
  lcd.setCursor(0, 1);
  lcd.print("Conectar Wifi :(");
}
void sending_msg_display_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enviando");
  lcd.setCursor(0, 1);
  lcd.print("mensagem...");
}

UUID create_uuid() {
  UUID uuid;
  uint32_t seed1 = millis();
  uint32_t seed2 = 0;
  uuid.seed(seed1, seed2);
  
  uuid.generate();
  return uuid;
}
