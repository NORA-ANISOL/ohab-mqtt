# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time

light = 0
status = 0
mqttc = mqtt.Client("lightSensorRoomA")
#mqttc.username_pw_set(openhabian, password=rest1sum3)
mqttc.connect("192.168.1.136", 1883)
while True:
	mqttc.publish("myHome/lightSensor/roomA", status)
	#mqttc.loop(2) #timeout = 2s
	light = light + 1
	if light >= 300:
		light = 0
	if light >= 150:
		status = 1
	else
		status = 0
	time.sleep(3)