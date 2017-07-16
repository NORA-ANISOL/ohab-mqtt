#include <Wire.h>
#define pinR 3
#define pinG 5
#define pinB 6

// Functions prototypes
void setColor(byte color[3]);

void setup() {
  Wire.begin(240);                // join i2c bus with address #240
  Wire.onReceive(receiveEvent);   // register event
  Serial.begin(57600);            
}

void loop() {
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  char *mqttBuffer = NULL;
  int count = 0;
  while (1 < Wire.available()) { // loop through all but the last
    char *_buffer = NULL;
    _buffer = (char*)realloc(mqttBuffer, 1);
    mqttBuffer = _buffer;
    mqttBuffer[count] = Wire.read(); // receive byte as a character
    count ++;
  }
  char *_buffer = NULL;
  _buffer = (char*)realloc(mqttBuffer, 2);
  mqttBuffer = _buffer;
  mqttBuffer[count] = Wire.read(); // receive byte as a character
  mqttBuffer[count + 1] = '\n';
  // Copy the msg to a string variable for next conversion
  int _lenghtBuffer = strlen(mqttBuffer);
  char val[_lenghtBuffer];
  strcpy(val, mqttBuffer);
  // free memory from variable
  free(mqttBuffer);
  // Convert a string number in hexadecimal format to a number in decimal format
  char *ptr;
  long color = strtol(val,&ptr, 16);
  // Convert the number in BGR channels
  byte colorBGR[3];
  colorBGR[0] = (color >> 16) & 0xff;
  colorBGR[1] = (color >> 8) & 0xff;
  colorBGR[2] = color & 0xff;
  // Set color in led RGB
  setColor(colorBGR);
}

void setColor(byte color[3]){
  analogWrite(pinB, color[0]);
  analogWrite(pinG, color[1]);
  analogWrite(pinR, color[2]);
}
