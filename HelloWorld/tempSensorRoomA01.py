# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time

Celsius = 0
mqttc = mqtt.Client("tempSensorRoomA01")
#mqttc.username_pw_set(openhabian, password=rest1sum3)
mqttc.connect("192.168.1.136", 1883)
while True:
	mqttc.publish("myHome/tempSensor/RoomA/01", Celsius)
	#mqttc.loop(2) #timeout = 2s
	Celsius = Celsius + 1
	if Celsius >= 100:
		Celsius = 0
	time.sleep(2)

#Number homeTemp "Temperature [%.1f °C]" {mqtt="<[MulCam:myHome/temperature:state:default]"}