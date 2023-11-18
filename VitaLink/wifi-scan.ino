#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Configurações de WiFi
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";  // Substitua pelo sua senha

// Configurações de MQTT
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "esp32_mqtt";
const char *TOPIC_PUBLISH_PRESSURE = "fiap/iot/pressure";
const char *TOPIC_PUBLISH_HEART_RATE = "fiap/iot/heartrate";

// Configurações de Hardware
#define PIN_LED_VERDE 15
#define PIN_LED_AMARELO 2
#define PIN_LED_VERMELHO 4
#define PUBLISH_DELAY 500 // Intervalo de leitura e publicação em milissegundos (0,5 segundos)

// Variáveis globais
WiFiClient espClient;
PubSubClient MQTT(espClient);
unsigned long publishUpdate = 0;
const int TAMANHO = 10; // Reduzido para acomodar apenas o valor

// Variáveis simuladas de pressão arterial e frequência cardíaca
float simulatedPressure = 120.0; // Valor inicial simulado para pressão arterial
float simulatedHeartRate = 70.0; // Valor inicial simulado para frequência cardíaca
float pressureIncrement = 2.0;    // Incremento da pressão arterial
float heartRateIncrement = 1.0;   // Incremento da frequência cardíaca

// Protótipos de funções
void updateSimulatedValues();
void initWiFi();
void initMQTT();
void reconnectMQTT();
void reconnectWiFi();
void checkWiFIAndMQTT();

void updateSimulatedValues() {
  // Simula variação nos valores de pressão arterial e frequência cardíaca
  simulatedPressure += random(-5, 5) + pressureIncrement;
  simulatedHeartRate += random(-2, 2) + heartRateIncrement;

  // Garante que os valores simulados estejam dentro de limites razoáveis
  simulatedPressure = constrain(simulatedPressure, 80.0, 160.0);
  simulatedHeartRate = constrain(simulatedHeartRate, 60.0, 100.0);
}

void initWiFi() {
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar com o Broker MQTT: ");
    Serial.println(BROKER_MQTT);

    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
    } else {
      Serial.println("Falha na conexão com MQTT. Tentando novamente em 2 segundos.");
      delay(2000);
    }
  }
}

void checkWiFIAndMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void reconnectWiFi(void) {
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Wifi conectado com sucesso");
  Serial.print(SSID);
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  initWiFi();
  initMQTT();
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
}

void loop() {
  checkWiFIAndMQTT();
  MQTT.loop();

  if ((millis() - publishUpdate) >= PUBLISH_DELAY) {
    publishUpdate = millis();
    updateSimulatedValues();

    // Publica os valores simulados nos tópicos MQTT correspondentes
    char buffer[TAMANHO];

    // Publica pressão arterial
    snprintf(buffer, TAMANHO, "%f", simulatedPressure);
    MQTT.publish(TOPIC_PUBLISH_PRESSURE, buffer);
    Serial.printf("Pressão Arterial: %s\n", buffer);

    // Publica frequência cardíaca
    snprintf(buffer, TAMANHO, "%f", simulatedHeartRate);
    MQTT.publish(TOPIC_PUBLISH_HEART_RATE, buffer);
    Serial.printf("Frequência Cardíaca: %s\n", buffer);

    // Atualiza LEDs com base nos valores simulados
    if (simulatedPressure >= 120.0 && simulatedPressure <= 140.0 &&
        simulatedHeartRate >= 60.0 && simulatedHeartRate <= 100.0) {
      digitalWrite(PIN_LED_VERDE, HIGH);
      digitalWrite(PIN_LED_AMARELO, LOW);
      digitalWrite(PIN_LED_VERMELHO, LOW);
    } else if (simulatedPressure >= 140.0 || simulatedHeartRate > 100.0) {
      digitalWrite(PIN_LED_VERDE, LOW);
      digitalWrite(PIN_LED_AMARELO, LOW);
      digitalWrite(PIN_LED_VERMELHO, HIGH);
    } else {
      digitalWrite(PIN_LED_VERDE, LOW);
      digitalWrite(PIN_LED_AMARELO, HIGH);
      digitalWrite(PIN_LED_VERMELHO, LOW);
    }
  }
}