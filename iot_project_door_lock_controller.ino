#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

#define d5_led D5
#define a0_pin A0
#define MQTT_KEEPALIVE 600

const char* ssid = "";
const char* password = "";
const char* ip = "192.168.15.50";
const char* mqtt_host = "controller.cloud.shiftr.io";
const int broker_port = 1883;
const char* mqtt_user = "controller";
const char* secret = "xxxxxxx";
const char* topic = "demo_controller";

int piezo = 0;
int posit = 0;
int secret_knock[5] = {345, 350, 660, 440, 580};
int door_knock[5] = {};
unsigned long last_time = 0;

Servo servo;
WiFiClient wifiClient;
PubSubClient pubsubClient(wifiClient);

void wifi_setup();
void reconnect();
void publish_topic();
void move_servo();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  servo.attach(D2); // D2 do NodeMCU

  pinMode(LED_BUILTIN, OUTPUT); // led do NodeMCU
  pinMode(d5_led, OUTPUT);
  digitalWrite(d5_led, LOW);
  Serial.begin(115200);
  delay(5000);
  wifi_setup();
  pubsubClient.setServer(mqtt_host, broker_port);
  pubsubClient.setCallback(callback);
  pubsubClient.setKeepAlive(MQTT_KEEPALIVE);
}

void wifi_setup() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.print("Conectando a rede: ");
  Serial.println(ssid);

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
    Serial.print("Conectando ao MQTT... ");
    bool isConnected = pubsubClient.connect("NodeClient", mqtt_user, secret);

    if (isConnected) {
      Serial.println("connected");
      pubsubClient.publish(topic, "Conectado");
    } else {
      Serial.print("failed, rc= ");
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
      Serial.print("Idx: ");
      Serial.print(posit);
      Serial.print(", ");
      Serial.print(secret_knock[posit]);
      Serial.print(", ");
      Serial.println(door_knock[posit]);
      digitalWrite(d5_led, HIGH);
      delay(500);
      ++posit;
    }
    digitalWrite(d5_led, LOW);
  }

  //  metodo teste para zerar o toque secreto
  unsigned long now = millis();
  if (now - last_time > 5000 and posit != 0) {
    last_time = now;
    Serial.println("you failed... try again");
    posit = 0;
  }

  if (posit == 5) {
    Serial.print("Sending message to: ");
    Serial.println(topic);
    pubsubClient.publish(topic, "KNOCKING_DOOR");
    delay(2000);
    posit = 0;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.print("New message from: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    char aux = (char) payload[i];
    Serial.print(aux);
    msg += aux;
  }

  if (msg == String("ALLOW_ACCESS")) {
    move_servo();
  } else {
    Serial.println("\nOops!!!");
  }
}

void move_servo() {
  Serial.println("\nUnlocking the door...");
  for (int pos = 0; pos <= 90; pos++) {
    servo.write(pos);
    delay(150);
  }
}

void loop() {
  if (!pubsubClient.connected()) {
    reconnect();
    pubsubClient.subscribe(topic);
  }

  pubsubClient.loop();

  if (WiFi.status() != WL_CONNECTED) {
    wifi_setup();
  }

  publish_topic();
}
