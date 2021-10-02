#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define d5_led D5
#define a0_pin A0

const char* ssid = "";
const char* password = "";
const char* ip = "192.168.15.50";
const char* mqtt_server = "broker.mqtt-dashboard.com";
int broker_port = 1883;

int piezo = 0;
//deixar o led do NodeMCU aceso apenas enquanto estiver conectado no wi-fi, caso a conexão caia, desliga o led

void setup() {
  pinMode(d5_led, OUTPUT);
  digitalWrite(d5_led, LOW);
  Serial.begin(115200);
}

void loop() {
  piezo = analogRead(a0_pin);

  if(piezo > 300 && piezo < 700) {
    digitalWrite(d5_led, HIGH);
    Serial.println("Tom ao bater na porta: " + String(piezo));
    delay(2000);
  } else {
    digitalWrite(d5_led, LOW);
  }
}
