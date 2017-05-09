# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import time

state = False
mqttc = mqtt.Client("lights")
#mqttc.username_pw_set(openhabian, password=rest1sum3)
mqttc.connect("192.168.1.136", 1883)
while True:
	mqttc.publish("myHome/light/yard/01", str(state))
	#mqttc.loop(2) #timeout = 2s
	if state == False:
		state = True
	else :
		state = False
	time.sleep(.5)
	mqttc.publish("myHome/light/kitchen/01", str(not state))
	time.sleep(.2)

#Number homeTemp "Temperature [%.1f °C]" {mqtt="<[MulCam:myHome/temperature:state:default]"}