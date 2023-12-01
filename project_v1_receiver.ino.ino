#include "M5StickCPlus.h"
#include <PubSubClient.h>
#include <WiFi.h>

//const char* ssid = "The Standard"; // TODO: Insert the right ssid and password!!
//const char* password = "B491C6023AEDF578";

// wifi network
const char* ssid   	= "utexas-iot";
//const char* ssid   	= "TheNine_Resident_WiFi";
// wifi password utexas
const char* password   = "93013329667666155260";
//const char* password   = "sofa007vast";
// broker that mediates the connection between publisher and subscriber
const char* mqtt_server = "test.mosquitto.org";
//const char* buttonTopic = "button_press";
//const char* mqtt_server = "10.2.76.253";

// declare objects for establishing wifi connectivity
WiFiClient espClient;
PubSubClient* client;

// keep track of the last time a message was sent or an action was performed
// control when a new message is sent in response to a button press
unsigned long lastMsg = 0;
// defines a constant for buffer size
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
char msg2[20];

//char currentBalance[MSG_BUFFER_SIZE];
int currentBalance = 100;
int amountToSend = 200;
//int amountRequested = 0;
int receivedGlobal = 0;


// connecting to wifi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  M5.Lcd.println();
  // print connecting to
  M5.Lcd.print("Connecting to ");
  // print ssid
  M5.Lcd.println(ssid);
  // connect to wifi
  WiFi.begin(ssid, password);

  // while in the process of connecting, print a .
  while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	M5.Lcd.print(".");
	}
  // seed random number generator with a random value based on the microsecond
  // time since the device was powered on/
  randomSeed(micros());

  // print wifi connected and IP when the wifi is connected
  M5.Lcd.println("");
  M5.Lcd.println("WiFi connected");
  M5.Lcd.println("IP address: ");
  M5.Lcd.println(WiFi.localIP());
}

// function handles incoming MQTT messages.
// ONLY NEEDED FOR SUBSCRIBER AND NOT PUBLISHER
// but I still included it for future records.
// Called when a message is received from MQTT broker, and prints the received message to the M5StickC Plus
void callback(char* topic, byte* payload, unsigned int length) {
 
  // prints message arrive and the topic
  // M5.Lcd.print("Message arrived [");
  // M5.Lcd.print(topic);
  // M5.Lcd.print("] ");
 
  // Handle MQTT message
  payload[length] = '\0'; // Null-terminate the payload

  String message = String((char *)payload);

  // Check the topic and convert string to integer accordingly
  if (strcmp(topic, "ReceiveMoney") == 0) {
	// Extract the payload into a string
	String payloadStr = "";
	for (int i = 0; i < length; i++) {
  	payloadStr += (char)payload[i];
  	}
	// Convert the payload string to an integer
	int amountReceived = payloadStr.toInt();
	receivedGlobal = amountReceived;

	M5.Lcd.print("Amount Received: ");
	M5.Lcd.println(amountReceived);
  }

  if (receivedGlobal > 0) {
	currentBalance += receivedGlobal;
	M5.Lcd.print("Updated balance: ");
	M5.Lcd.println(currentBalance);
  }


  // iterates through the payload byte array, prints each character in the payload
  for (int i = 0; i < length; i++) {
	// the payload is the actual data or message content being transmitted from the publisher to the subscriber
	M5.Lcd.print((char)payload[i]);
  }
  M5.Lcd.println();
}

void reconnect() {
  // Loop until we’re reconnected
  while (!client->connected()) {
	M5.Lcd.print("Attempting MQTT connection...");
	// Create a random client ID
	String clientId = "ESP8266Client - MyClient2"; // TODO: only one client with a unique name per broker!!
	// Attempt to connect
	if (client->connect(clientId.c_str())) {
  	// Insert your password
  	M5.Lcd.println("connected");
  	// Receiving "amount sent"
  	client->subscribe("AhladTopic");
  	// receiving the amount which whill be added to my currentBalance
  	client->subscribe("ReceiveMoney"); // TODO: the subscriber should do this, the publisher should not!!
  	// otherwise!
  	// For when the client does not have sufficient funds
  	client->subscribe("AhladTopicNew");
 	 
	} else {
  	// if it didn't connect, print that it failed
  	M5.Lcd.print("failed, rc = ");
  	// print the return code from the client's state. It returns an integer that represents the connection state of the MQTT client
  	M5.Lcd.print(client->state());
  	// Wait 5 seconds before retrying
  	M5.Lcd.println(" try again in 5 seconds");
  	delay(5000);
	}
  }
}

// this function initializes the M5StickC Plus hardware
void setup() {
  // initializing the hardware, setting up display and other components
  M5.begin();
  // sets up the Wi-Fi connection
  setup_wifi();
  // establishing a new instance of PubSubClient
  client = new PubSubClient(espClient);
  // MQTT broker's address and MQTT broker's port are configured. Tells MQTT client where to connect for message exchange
  client->setServer(mqtt_server, 1883);
  // sets the callback function, which handles incoming messages
  client->setCallback(callback);
}

void loop() {
  // check if MQTT client is connected to the broker
  if (!client->connected()) {
	// if not connected, reconnect
	reconnect();
  }
  // processing incoming MQTT messages and maintaining the MQTT connection
  client->loop();

   // Check if the button is pressed and then publish a message
  if (M5.BtnA.wasPressed()) {
	// records the current time in milliseconds when button was pressed
	lastMsg = millis();
	// prepares a message to be published. Stores "button was pressed" in msg variable
	snprintf(msg, MSG_BUFFER_SIZE, "Amount required: ");
	// The topic that sends the amount I request
	String topic = "SendMoney";
	// converting amountToSend to a string to SEND
	char payload[10];
	snprintf(payload, sizeof(payload), "%d", amountToSend);
	// Sending "Amount required: 40"
	client->publish("FloraTopic", msg); // Replace with the actual topic to publish
	client->publish(topic.c_str(), payload);
	delay(1000);
    
	// On my own device, print Amount Required: 40 and Current Balance: 100
	M5.Lcd.println("Amount Required: ");
	M5.Lcd.println(amountToSend);
	// print "current balance"
	M5.Lcd.println("Current Balance: ");
	M5.Lcd.println(currentBalance);
	// the message is published to NQTT topoic named FloraTopic with the message in msg
    
  }

  // update hardware components
  M5.update(); // Update the M5StickC to get the latest button press events
}

