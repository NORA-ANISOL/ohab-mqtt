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
// Light Sensor TSL2561
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <string.h>
#include <stdlib.h>

// Update these with values suitable for your network.
const char* ssid     = "SicomoroBonito";
const char* password = "ChV3u6G33t";
IPAddress mqttServer(192, 168, 1, 136);
// Configure time to get data from sensor
unsigned long tAnt = 0, t1Ant = 0;
const int tSecs = 3000, t1Secs = 1000;
// Threshold for lightSensor
int luxThreshold = 0;
// Configure Buttons
const byte btnPin01 = 12;
const byte btnPin02 = 13;
bool btn01 = false;
bool btn02 = false;
// Configure leds
const byte ledPin03 = 14;
// Variables to allocate temperature and humidity from sensor
int temp = 0;
int hum = 0;
// Debug Flag
bool DEBUG = false;

// WifiClient
WiFiClient espClient;
PubSubClient client(espClient);
// Object for sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/************** Prototype functions**************/
void setupWifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int _length);
void displaySensorDetails(void);
void configureSensor(void);
void readSensor(void);
void setLuxThreshold(char* _topic, char* _payload);
void configDigitalPins();
void readBtns();
void lampTopics(char* _topic, char* _payload);
void debugTopic(char* _topic, char* _payload);
/**************End Prototype functions**************/

void setup() {
  // Serial Port baudrate
  Serial.begin(57600);
  setupWifi();
  // Connection to MQTT server(broker)
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  // Sensor configuration
  if (!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }
  /* Display some basic information on this sensor */
  displaySensorDetails();
  /* Setup the sensor gain and integration time */
  configureSensor();
  // Configure input pins
  configDigitalPins();
}

void loop()
{
  unsigned long tAct, t1Act;
  tAct = t1Act = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Light sensor publishing
  if ((tAct - tAnt) >= tSecs) {
    readSensor();
    tAnt = tAct;
  }
  if ((t1Act - t1Ant >= t1Secs)) {
    readBtns();
    t1Ant = t1Act;
  }
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
  // Enable lamp topics
  lampTopics(topic, strPayload);
  if (DEBUG == true){
    Serial.println();  
  }   
  // Set the threshold parameter from light sensor
  setLuxThreshold(topic, strPayload);
  // Enable DEBUG mode
  debugTopic(topic, strPayload);
   
  // Free reserved memory
  free(strPayload);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("room01")) {
      Serial.println("connected");
      client.subscribe("myHome/lamp01/room01");
      client.subscribe("myHome/lightSensor/room01/threshold");
      client.subscribe("myHome/board/room01/debug");
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

// Displays some basic information on this sensor from the unified
// sensor API sensor_t type (see Adafruit_Sensor for more information)
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

// Configures the gain and integration time for the TSL2561
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
}

// Read light sensor
void readSensor() {
  /* Get a new sensor event */
  sensors_event_t event;
  tsl.getEvent(&event);

  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    int lux = event.light;
    if (DEBUG == true){
      Serial.print(lux);
      Serial.println(" lux");  
    }
    
    if (lux <= luxThreshold) {
      client.publish("myHome/lightSensor/room01", "OFF");
    }
    else {
      client.publish("myHome/lightSensor/room01", "ON");
    }
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }

  if (temp >= 100) {
    temp = hum = 0;
  }

  char strTemp [4], strHum [4];
  itoa(temp, strTemp, 10);
  itoa(hum, strHum, 10);
  client.publish("myHome/temperatureSensor/room01", strTemp);
  client.publish("myHome/humiditySensor/room01", strHum);
  temp++;
  hum++;
}

// Set the threshold value from light sensor
void setLuxThreshold(char* _topic, char* _payload){
  if (0 == strcmp(_topic, "myHome/lightSensor/room01/threshold")){
    int _luxThreshold = atoi(_payload);
    if (DEBUG == true){
      Serial.print("Threshold: ");
      Serial.println(_luxThreshold);
    }
    luxThreshold = _luxThreshold;
  }
}

// Configure input pins
void configDigitalPins() {
  pinMode(btnPin01, INPUT_PULLUP);
  pinMode(btnPin02, INPUT_PULLUP);
  pinMode(ledPin03, OUTPUT);
}

// Read light sensor
void readBtns() {
  btn01 = digitalRead(btnPin01);
  btn02 = digitalRead(btnPin02);
  if (btn01 == true) {
    client.publish("myHome/windowSensor/room01", "OFF");
  }
  else {
    client.publish("myHome/windowSensor/room01", "ON");
  }
  if (btn02 == true) {
    client.publish("myHome/doorSensor/room01", "OFF");
  }
  else {
    client.publish("myHome/doorSensor/room01", "ON");
  }
}

// Execute Lamp Topics
void lampTopics(char* _topic, char* _payload){
  // Lamp 01
  if (0 == strcmp(_topic, "myHome/lamp01/room01")){
    if (0 == strcmp(_payload, "1")){
      digitalWrite(ledPin03, HIGH);
    }
    else{
      digitalWrite(ledPin03, LOW);
    }
  }
}

// Enable message in serial port
void debugTopic(char* _topic, char* _payload){
    // Lamp 01
  if (0 == strcmp(_topic, "myHome/board/room01/debug")){
    if (0 == strcmp(_payload, "1")){
      DEBUG = true;
    }
    else{
      DEBUG = false;
    }
  }
}
