#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""
// #define BLYNK_PRINT Serial
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include "UUID.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>

const char* ssid = "";
const char* password = "";
const char* serverUrl = "";

const int SENSOR_BIG = 0;
const int SENSOR_MEDIUM = 12;
const int SENSOR_SMALL = 14;
const int SENSOR_METAL = 13;
const int SENSOR_POL = 10;
const int SENSOR_START = 15;

LiquidCrystal_I2C lcd(0x3F, 16, 2);
String* read_piece();

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
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
  pinMode(SENSOR_POL, INPUT_PULLUP);
  pinMode(SENSOR_METAL, INPUT_PULLUP);
  pinMode(SENSOR_START, OUTPUT);
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
    String* piece_info = read_piece();
    UUID uuid = create_uuid();
    send_message(uuid, piece_info);
  }
  Blynk.run();
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
  doc["material"] = piece_info[1];
  doc["size"] = piece_info[0];
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
  delay(10000);
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

String* read_piece() {
  String size = "Desconhecido";
  String material = "Polimero";

  int size_b = 0;
  int size_m = 0;
  int size_s = 0;

  // aguardar até ultimo sensor ser ativado para sair do loop
  while (digitalRead(SENSOR_POL) == 1) {

    // definindo o valor da peça baseado na leitura do sensor
    if (digitalRead(SENSOR_BIG) == 0) {
      size_b = 1;
    } else if (digitalRead(SENSOR_MEDIUM) == 0) {
      size_m = 1;
    } else if (digitalRead(SENSOR_SMALL) == 0) {
      size_s = 1;
    }

    // definir se a peça é de metal de acordo com o sensor
    if (digitalRead(SENSOR_METAL) == 0) {
      material = "Metal";
    }

    // OBS: sem chamar o yield() aqui o WTD da placa faz ela resetar caso o loop demore mais de 2 segundos
    // sem retorno
    yield();
  }

  // lógical para definir o tamanho da peça baseado na leitura do sensores
  // de acordo com o padrão da esteira
  if (size_b == 1) {
    size = "Grande";
  } else if (size_s == 1 && size_m == 1) {
    size = "Medio";
  } else if (size_s == 1 && size_m == 0) {
    size = "Pequeno";
  } else if (size_m == 1) {
    size = "Medio";
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
}

BLYNK_WRITE(V0)
{
  int value = param.asInt();
  
  if(value == 1) {
    digitalWrite(SENSOR_START, HIGH);
  } else {
    digitalWrite(SENSOR_START, LOW);
  }
}

BLYNK_CONNECTED()
{
  Serial.println("conectado blynk");
}
