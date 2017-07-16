/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string.h>
#include <stdlib.h>

// Update these with values suitable for your network.
const char* ssid     = "SicomoroBonito";
const char* password = "ChV3u6G33t";
IPAddress mqttServer(192, 168, 1, 136);
// Configure Buttons
const byte btnPin01 = 12;
const byte btnPin02 = 13;
const byte btnPin03 = 14;
// Debug Flag
bool DEBUG = false;

// WifiClient
WiFiClient espClient;
PubSubClient client(espClient);

/************** Prototype functions**************/
void setupWifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int _length);
void configDigitalPins();
void lampTopics(char* _topic, char* _payload);
void i2cTopics(char* _topic, char* _payload);
void sendMqttTopic(char* _topic, char* _payload);
void sendMqttPayload(char* _payload);
void debugTopic(char* _topic, byte* _payload);
/**************End Prototype functions**************/

void setup() {
  // Serial Port baudrate
  Serial.begin(57600);
  // Init i2c protocol
  Wire.begin();
  setupWifi();
  // Connection to MQTT server(broker)
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  // Configure input pins
  configDigitalPins();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Event to handle subscriptions and publications
void callback(char* topic, byte* payload, unsigned int _length) {
  if (DEBUG == true){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  }
 
  char* strPayload;
  /* Initial memory allocation */
  strPayload = (char *) malloc(_length);

  for (int i = 0; i < _length; i++) {    
    strPayload[i] = (char)payload[i];
    if (DEBUG == true){
      Serial.print(strPayload[i]);
    }
  }
  // Add a NULL caracter to complete the string
  strPayload[_length] = '\0';
  
  // Send mqtt msg to arduino by i2c
  //sendMqttTopic(topic, strPayload);
  i2cTopics(topic, strPayload);
  
  // Enable lamp topics
  lampTopics(topic, strPayload);
  // Enable DEBUG mode
  debugTopic(topic, strPayload);
  
  if (DEBUG == true){
    Serial.println();  
  }  
  // Free reserved memory
  free(strPayload);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("room03")) {
      Serial.println("connected");
      client.subscribe("myHome/lamp01/room03");
      client.subscribe("myHome/lamp02/room03");
      client.subscribe("myHome/lamp03/room03");
      client.subscribe("myHome/fan01/room03");
      client.subscribe("myHome/lampRgb01/room03");
      client.subscribe("myHome/board/room03/debug");      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// We start by connecting to a WiFi network
void setupWifi() {
  delay(100);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Configure input pins
void configDigitalPins() {
  pinMode(btnPin01, OUTPUT);
  pinMode(btnPin02, OUTPUT);  
  pinMode(btnPin03, OUTPUT);
}

// Execute Topics
void lampTopics(char* _topic, char* _payload){
  // Lamp 01
  if (0 == strcmp(_topic, "myHome/lamp01/room03")){
    if (0 == strcmp(_payload, "1")){
      digitalWrite(btnPin01, HIGH);
    }
    else{
      digitalWrite(btnPin01, LOW);
    }
  }
  // Lamp 02
  if (0 == strcmp(_topic, "myHome/lamp02/room03")){
    if (0 == strcmp(_payload, "1")){
      digitalWrite(btnPin02, HIGH);
    }
    else{
      digitalWrite(btnPin02, LOW);
    }
  }
  // Lamp 03
  if (0 == strcmp(_topic, "myHome/lamp03/room03")){
    if (0 == strcmp(_payload, "1")){
      digitalWrite(btnPin03, HIGH);
    }
    else{
      digitalWrite(btnPin03, LOW);
    }
  }
}

// Execute topics for arduino by i2c
void i2cTopics(char* _topic, char* _payload){
  // Fan 01
  if (0 == strcmp(_topic, "myHome/fan01/room03")){
    //sendMqttPayload(_payload);
  }
  if (0 == strcmp(_topic, "myHome/lampRgb01/room03")){
    sendMqttPayload(_payload);
  }
}

// Send Topic and payload to arduino device
void sendMqttTopic(char* _topic, char* _payload){
  Wire.beginTransmission(240);  // transmit to device #240  
  Wire.write(_topic);           // send topic
  Wire.write(" ");
  Wire.write(_payload);               
  Wire.endTransmission();       // stop transmitting
}

// Send Topic and payload to arduino device
void sendMqttPayload(char* _payload){
  if(DEBUG == true){
    Serial.println();
    Serial.print(_payload);  
  }
  Wire.beginTransmission(240);  // transmit to device #240
  Wire.write(_payload);
  Wire.endTransmission();       // stop transmitting
}

// Enable message in serial port
void debugTopic(char* _topic, char* _payload){
    // Lamp 01
  if (0 == strcmp(_topic, "myHome/board/room03/debug")){
    if (0 == strcmp(_payload, "1")){
      DEBUG = true;
    }
    else{
      DEBUG = false;
    }
  }
}
