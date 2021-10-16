#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define d5_led D5
#define a0_pin A0

const char* ssid = "";
const char* password = "";
const char* ip = "192.168.15.50";
const char* mqtt_host = "xxxxxxx.cloud.shiftr.io";
const int broker_port = 1883;
const char* mqtt_user = "xxxxxxx";
const char* secret = "xxxxxxx";
const char* topic = "demo_controller";

int piezo = 0;
int posit = 0;
int secret_knock[5] = {345, 350, 660, 440, 580};
int door_knock[5] = {};

WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

void wifi_setup();
void reconnect();
void publish_topic();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // led do NodeMCU
  pinMode(d5_led, OUTPUT);
  digitalWrite(d5_led, LOW);
  Serial.begin(115200);
  delay(5000);
  wifi_setup();
  pubsubClient.setServer(mqtt_host, broker_port);
}

void wifi_setup() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.println("Conectando a rede: " + String(ssid));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("IP inicial (DHCP): ");
  Serial.println(WiFi.localIP());

  IPAddress ipAddress;
  ipAddress.fromString(ip);
  WiFi.config(ipAddress, WiFi.gatewayIP(), WiFi.subnetMask());
  Serial.print("IP alterado para: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!pubsubClient.connected()) {
    Serial.print("Conectando ao MQTT...");
    bool isConnected = pubsubClient.connect("Client-ID", mqtt_user, secret);

    if (isConnected) {
      Serial.println("connected");
      pubsubClient.publish(topic, "Conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubsubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publish_topic() {
  piezo = analogRead(a0_pin);

  if (piezo > 300 && piezo < 700) {
    if (piezo >= secret_knock[posit] * 0.95 && piezo <= secret_knock[posit] * 1.05) {
      door_knock[posit] = piezo;
      Serial.println("Idx: " + String(posit) + "," + String(secret_knock[posit]) + ", " + String(door_knock[posit]));
      digitalWrite(d5_led, HIGH);
      delay(500);
      ++posit;
    }
    digitalWrite(d5_led, LOW);
  }

  if (posit == 5) {
    Serial.println("Sending message to: " + String(topic));
    pubsubClient.publish(topic, "KNOCKING_DOOR");
    delay(2000);
    posit = 0;
  }
}

void loop() {
  if (!pubsubClient.connected()) {
    reconnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    wifi_setup();
  }

  publish_topic();
}
