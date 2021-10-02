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
    digitalWrite(LED_BUILTIN, 1);
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  digitalWrite(LED_BUILTIN, 0);

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
    digitalWrite(d5_led, HIGH);
    Serial.println("Tom ao bater na porta: " + String(piezo));
    pubsubClient.publish(topic, "KNOCKING_DOOR");
    delay(2000);
  } else {
    digitalWrite(d5_led, LOW);
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
