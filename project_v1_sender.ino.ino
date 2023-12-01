#include "M5StickCPlus.h"
#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid       = "utexas-iot";
const char* password   = "11369131671533354503";
const char* mqtt_server = "test.mosquitto.org";


// const char* ssid = "The Standard";
// const char* password = "B491C6023AEDF578";
// const char* mqtt_server = "10.2.76.253";

WiFiClient espClient;
PubSubClient *client;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
char msg2[20];
int balance = 1000;
int reqGlobal = 0;
int amountSent = 0;


void setup_wifi() {
  delay(10);
  M5.Lcd.println();
  // M5.Lcd.print("Connecting to ");
  // M5.Lcd.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // M5.Lcd.print(".");
  }

  // randomSeed(micros());
  // M5.Lcd.println("");
  // M5.Lcd.println("WiFi connected");
  // M5.Lcd.println("IP address: ");
  // M5.Lcd.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
  // Handle MQTT message
  payload[length] = '\0'; // Null-terminate the payload

  String message = String((char *)payload);

  // Check the topic and convert string to integer accordingly
  if (strcmp(topic, "SendMoney") == 0) {
    // Extract the payload into a string
    String payloadStr = "";
    for (int i = 0; i < length; i++) {
      payloadStr += (char)payload[i];
      }
    // Convert the payload string to an integer
    int reqAmount = payloadStr.toInt();
    reqGlobal = reqAmount;
    amountSent = reqAmount;

    M5.Lcd.print("Amount Requested: ");
    M5.Lcd.println(reqAmount);
  }
}
  
void reconnect() {
    while (!client->connected()) {
      // M5.Lcd.print("Attempting MQTT connection...");
      String clientId = "Ahlad's project";
    if (client->connect(clientId.c_str())) {
      // M5.Lcd.println("connected");
      // client->publish("AhladTopic");
      client->subscribe("FloraTopic");
      client->subscribe("SendMoney");
    } 
    else {
      M5.Lcd.print("failed, rc = ");
      M5.Lcd.print(client->state());
      M5.Lcd.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  delay(500);
  M5.begin();
  M5.Lcd.setCursor(8, 30, 2);
  delay(500);
  setup_wifi();

  client = new PubSubClient(espClient);
  client->setServer(mqtt_server, 1883);
  client->setCallback(callback);
}

void loop() {
  M5.update();

  // Handle button press to send amount
  if (M5.BtnB.wasPressed()) {
    if (balance >= amountSent){
      M5.Lcd.print("Current Balance: ");
      M5.Lcd.println(balance);
      // Send the amount as a string to the second device
      //sendAmount(amountToSend);
    }
    else{
    M5.Lcd.print("Insufficient Balance");
    }
  }
    

  // Handle button press to initiate payment request
  if (M5.BtnA.wasPressed()) {
    if (balance < amountSent){
      M5.Lcd.println("Please have required balance");
      lastMsg = millis();
      snprintf(msg2, MSG_BUFFER_SIZE, "User does not have sufficient balance");
      client->publish("AhladTopicNew", msg2);

    }
    else {
      lastMsg = millis();
      snprintf(msg, MSG_BUFFER_SIZE, "Amount Sent");
      balance -= amountSent;
      M5.Lcd.print("Latest Balance: ");
      M5.Lcd.println(balance);
      String topic = "ReceiveMoney";
      char payload[10];
      //String payload = String(amountToSend);
      snprintf(payload, sizeof(payload), "%d", amountSent);
      client->publish("AhladTopic", msg);
      client->publish(topic.c_str(), payload);
      M5.Lcd.println("Amount Sent");
    }
  }

  // MQTT client loop and reconnection
  if (!client->connected()) {
    reconnect();
  }
  client->loop();
}


