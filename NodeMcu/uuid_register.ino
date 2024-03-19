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
const int SENSOR_BIG = 0;
const int SENSOR_MEDIUM = 14;
const int SENSOR_SMALL = 12;
const int SENSOR_METAL = 13;
const int SENSOR_PLASTIC = 10;

String* read_piece();

void setup() {
  Serial.begin(115200);
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

  pinMode(SENSOR_BIG, INPUT_PULLUP);
  pinMode(SENSOR_MEDIUM, INPUT_PULLUP);
  pinMode(SENSOR_SMALL, INPUT_PULLUP);
  pinMode(SENSOR_PLASTIC, INPUT_PULLUP);
  pinMode(SENSOR_METAL, INPUT_PULLUP);
}

void loop() {

  waiting_piece_message();

  int read_sensor_big = digitalRead(SENSOR_BIG);
  int read_sensor_medium = digitalRead(SENSOR_MEDIUM);
  int read_sensor_small = digitalRead(SENSOR_SMALL);


  // verificação se um dos três primeiros sensores foi ativado
  // caso um dos sensores for acionado a função read_piece registrara o tamanho da peça
  // baseada em qual sensor foi ativado
  // e aguardará o acionamento dos sensores de materiais para retornar um array contendo as informações
  // do sensores e continuar o fluxo do código
  if (read_sensor_big == 0 || read_sensor_medium == 0 || read_sensor_small == 0) {
    String* piece_info = read_piece(read_sensor_big, read_sensor_medium, read_sensor_small);
    UUID uuid = create_uuid();
    send_message(uuid, piece_info);
  }
}

void displayUUID(UUID uuid) {
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

String send_message(UUID uuid, String* piece_info) {
  lcd.clear();
  sending_msg_display_lcd();

  WiFiClientSecure client;
  client.setInsecure();

  StaticJsonDocument<48> doc;

  doc["uuid"] = uuid;
  doc["material"] = piece_info[0];
  doc["size"] = piece_info[1];
  String json_payload;
  serializeJson(doc, json_payload);

  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(json_payload);
  Serial.print("codigo de resposta == ");
  Serial.print(httpResponseCode);
  display_message_lcd(httpResponseCode);
  lcd.clear();
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
  delay(1000);
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
  delay(1000);
}

void wifi_connecting_display_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando");
  lcd.setCursor(0, 1);
  lcd.print("Wifi...");
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
  lcd.clear();
}

void wifi_connecting_fail_lcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Erro ao ");
  lcd.setCursor(0, 1);
  lcd.print("Conectar Wifi :(");
  lcd.clear();
}
void sending_msg_display_lcd() {
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

String* read_piece(int read_sensor_big, int read_sensor_medium, int read_sensor_small) {
  String size = "Desconhecido";
  String material = "Desconhecido";

  int read_sensor_plastic = digitalRead(SENSOR_PLASTIC);
  int read_sensor_metal = digitalRead(SENSOR_METAL);


  // verifição se um dos três primeiros sensores foi ativado
  if (read_sensor_big == 0 || read_sensor_medium == 0 || read_sensor_small == 0) {
    // determinar o tamnho da peça com base no sensor ativado
    if (read_sensor_big == 0) {
      size = "Grande";
    } else if (read_sensor_medium == 0) {
      size = "Medio";
    } else if (read_sensor_small == 0) {
      size = "Pequeno";
    }

    // aguardar até que um dos sensores de material ser ativado
    while (read_sensor_plastic == 1 && read_sensor_metal == 1) {
      // Ler os estados dos sensores novamente
      read_sensor_plastic = digitalRead(SENSOR_PLASTIC);
      read_sensor_metal = digitalRead(SENSOR_METAL);

      // OBS: sem chamar o yield a placa se auto-resetava em 2 segundos
      yield();
    }

    // determinar o tipo do material com base no sensor ativado
    if (read_sensor_plastic == 0) {
      material = "Polimero";
    } else if (read_sensor_metal == 0) {
      material = "Metal";
    }
  }

  // alocação de espaço na memória para o ponteiro e acionar o display lcd com informaçoes
  // da peça segundo a leitura do sensor
  String* piece = new String[2];
  piece[0] = size;
  piece[1] = material;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tamanho: " + piece[0]);
  lcd.setCursor(0, 1);
  lcd.print("Mat: " + piece[1]);
  delay(1000);
  lcd.clear();
  return piece;
}

void waiting_piece_message() {
  lcd.setCursor(0, 0);
  lcd.print("Aguardando ");
  lcd.setCursor(0, 1);
  lcd.print("Sensor...");
  delay(200);
}