# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time

UV = 0
mqttc = mqtt.Client("uvSensor")
#mqttc.username_pw_set(openhabian, password=rest1sum3)
mqttc.connect("192.168.1.136", 1883)
while True:
	mqttc.publish("myHome/uvSensor/yard/01", UV)
	#mqttc.loop(2) #timeout = 2s
	UV = UV + 5.5555
	if UV >= 100:
		UV = 0
	time.sleep(.5)
	mqttc.publish("myHome/uvSensor/atic/01", UV + 1.9999)
	time.sleep(.2)

#Number homeTemp "Temperature [%.1f °C]" {mqtt="<[MulCam:myHome/temperature:state:default]"}