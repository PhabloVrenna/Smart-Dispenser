#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <ESP32Servo.h>

// ===== WiFi / MQTT =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;



WiFiClient espClient;
PubSubClient client(espClient);

// ===== Periféricos =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo servo;

// ===== Pinos =====
const int PIN_SERVO = 13;
const int PIN_BUZZER = 12;
const int PIN_LED_GREEN = 25;
const int PIN_LED_YELLOW = 26;
const int PIN_LED_RED = 27;

const int PIN_BTN_CONFIRM = 33;
const int PIN_BTN_MENU = 32;

const int PIN_PIR = 14;

// ===== Variáveis de estado =====
bool medicacaoPendente = false;
bool ajustandoHora = false;
int horaTemp = 0;
unsigned long tempoInicio = 0;
const unsigned long TEMPO_LIMITE_MS = 15UL * 60UL * 1000UL;

// ===== MQTT Topics =====
const char* topic_status = "medicamentos/status";
const char* topic_alert = "medicamentos/alerta";
const char* topic_log = "medicamentos/log";
const char* topic_config = "medicamentos/config";

// =============================
// WiFi
// =============================
void setup_wifi() {
  WiFi.begin(ssid, password);
  unsigned long tstart = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - tstart < 15000) {
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi não conectado.");
  }
}

// =============================
// MQTT
// =============================
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("MQTT -> ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "MedDispenser-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Conectado");
      client.subscribe(topic_config);
    } else {
      delay(3000);
    }
  }
}

void publishJson(const char* topic, const String &json) {
  if (client.connected()) client.publish(topic, json.c_str());
}

// =============================
// Funções principais
// =============================
void alertaMedicamento(const char* nome = "Remedio") {
  medicacaoPendente = true;
  tempoInicio = millis();

  Serial.println(">>> ALERTA: medicacao pendente!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hora do remédio!");
  lcd.setCursor(0, 1);
  lcd.print(nome);

  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, HIGH);

  digitalWrite(PIN_BUZZER, HIGH);
  delay(600);
  digitalWrite(PIN_BUZZER, LOW);

  servo.write(90); // abre compartimento

  String payload = String("{\"device_id\":\"MedDispenser\",\"status\":\"pendente\",\"medicamento\":\"") + nome + "\"}";
  publishJson(topic_status, payload);
}

void confirmaMedicacao() {
  medicacaoPendente = false;

  Serial.println(">>> CONFIRMADO: dose tomada");

  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Med tomado!");

  servo.write(0);

  publishJson(topic_log, "{\"device_id\":\"MedDispenser\",\"status\":\"confirmado\"}");
}

void alertaAtraso() {
  medicacaoPendente = false;

  Serial.println(">>> ATRASO: +15 minutos");

  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Atraso > 15 min");

  servo.write(0);

  publishJson(topic_alert, "{\"device_id\":\"MedDispenser\",\"status\":\"atrasado\"}");

  delay(5000);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);
}

// =============================
// Setup
// =============================
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  rtc.begin();

  servo.attach(PIN_SERVO);

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  // 🔥 CORREÇÃO IMPORTANTE
  pinMode(PIN_BTN_CONFIRM, INPUT_PULLUP);
  pinMode(PIN_BTN_MENU, INPUT_PULLUP);

  pinMode(PIN_PIR, INPUT);

  digitalWrite(PIN_LED_GREEN, HIGH);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Dispenser");
  lcd.setCursor(0, 1);
  lcd.print("Pronto.");

  delay(1000);
}

// =============================
// Loop principal
// =============================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  DateTime now = rtc.now();

  // DEBUG DOS BOTÕES (INPUT_PULLUP)
  if (digitalRead(PIN_BTN_MENU) == LOW)  
    Serial.println("DEBUG → Botão MENU pressionado");

  if (digitalRead(PIN_BTN_CONFIRM) == LOW)
    Serial.println("DEBUG → Botão CONFIRMAR pressionado");

  // DEBUG PIR
  if (digitalRead(PIN_PIR) == HIGH)
    Serial.println("DEBUG → PIR detectou movimento");

  // ==== AJUSTE DE HORA ====
  if (!ajustandoHora && digitalRead(PIN_BTN_MENU) == LOW) {
    delay(150);
    if (digitalRead(PIN_BTN_MENU) == LOW) {
      ajustandoHora = true;
      horaTemp = now.hour();

      Serial.println("Modo ajuste de hora");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ajuste Hora:");
      lcd.setCursor(0, 1);
      lcd.print(horaTemp);
      lcd.print(":00");
    }
  }

  while (ajustandoHora) {

    if (digitalRead(PIN_BTN_MENU) == LOW) {
      delay(200);
      horaTemp = (horaTemp + 1) % 24;

      Serial.print("Hora ajustada: ");
      Serial.println(horaTemp);

      lcd.setCursor(0, 1);
      lcd.print("    ");
      lcd.setCursor(0, 1);
      lcd.print(horaTemp);
      lcd.print(":00");
    }

    if (digitalRead(PIN_BTN_CONFIRM) == LOW) {
      delay(200);

      rtc.adjust(DateTime(now.year(), now.month(), now.day(), horaTemp, 0, 0));

      Serial.println("Hora salva no RTC!");

      ajustandoHora = false;

      lcd.clear();
      lcd.print("Hora salva!");
      delay(1000);

      lcd.clear();
      lcd.print("Smart Dispenser");
    }
  }

  // ==== ALERTA DE MEDICAMENTO ====
  if (!medicacaoPendente) {
    if ((now.hour() == 8 || now.hour() == 12 || now.hour() == 18 || now.hour() == 22) &&
        now.minute() == 0 && now.second() < 5) {

      alertaMedicamento("Dose");
      delay(2000);
    }
  }

  // ==== CONFIRMAÇÃO POR BOTÃO ====
  if (digitalRead(PIN_BTN_CONFIRM) == LOW && medicacaoPendente) {
    delay(100);
    if (digitalRead(PIN_BTN_CONFIRM) == LOW) {
      confirmaMedicacao();
      delay(300);
    }
  }

  // ==== CONFIRMAÇÃO VIA PIR ====
  if (medicacaoPendente && digitalRead(PIN_PIR) == HIGH) {
    Serial.println("CONFIRMADO via PIR");
    confirmaMedicacao();
    delay(1000);
  }

  // ==== ATRASO ====
  if (medicacaoPendente && (millis() - tempoInicio > TEMPO_LIMITE_MS)) {
    alertaAtraso();
  }

  delay(100);
}
