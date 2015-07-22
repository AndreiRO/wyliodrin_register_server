#!/bin/env python

import requests
import json
import time

from threading import Thread, Lock

from wyliodrin import *


DIGITAL_INPUT   = 0
DIGITAL_OUTPUT  = 1
PWM_OUTPUT      = 2
ANALOG_INPUT    = 3



class Sensor(object):

    def __init__(self, _id, _type, pin, sleep_time):
        self._id = _id
        self._type = _type
        self.lock = Lock()
        self.value = 0
        self.pin = pin
        self.sleep_time = 0 if sleep_time < 0 else sleep_time

        if self._type == DIGITAL_INPUT:
            pinMode(pin, INPUT)
        elif self._type == DIGITAL_OUTPUT:
            pinMode(pin, OUTPUT)


    def get(self, url, token):
        url = url + ('/' if url[-1] != '/' else '') + 'get'
        while True:     
            try:
                self.lock.acquire()
                
                data = {"id" : self._id, "token" : token}
                res = requests.post(url, data=data)

                if res.status_code == 200:
                    obj = json.loads(res.content)

                    if obj["error"] != 0:
                        print obj["reason"]
                    else:
                        self.value = obj["value"]
                        print "Got value: ", self.value, " for sensor on pin: ", self.pin

                        if self._type == DIGITAL_OUTPUT:
                            digitalWrite(self.pin, self.value)
                        elif self._type == PWM_OUTPUT:
                            analogWrite(self.pin, self.value)
                else:
                    print "Error: " + res.reason

            finally:
                self.lock.release()
            time.sleep(self.sleep_time)



    def send(self, url, token):
        url = url + ('/' if url[-1] != '/' else '') + 'get'
        while True:
            try:
                self.lock.acquire()
                
                if self._type == DIGITAL_INPUT:
                    self.value = digitalRead(self.pin)
                elif self._type == ANALOG_INPUT:
                    self.value = analogRead(self.pin)

                data = {"id" : self._id, "token" : token, "value" : self.value}
                res = requests.post(url, data=data)

                if res.status_code != 200:
                    print "Error: " + res.reason
                else:
                    print "Sent value: ", self.value, " for sensor on pin: ", self.pin

            finally:
                self.lock.release()
            time.sleep(self.sleep_time)


def validate_pin(pin):
    if pin > 0:
        return True
    return False


class RegisterServer(object):

    def __init__(self, url, token):
        self.sensors = {}
        self.url = url
        self.token = token

    def registerDigitalInput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            print "Id: ", _id, " already registerd"
            return False
        else:
            s = Sensor(_id, DIGITAL_INPUT, pin, sleep_time)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}
    
            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : DIGITAL_INPUT}
            res = requests.post(url, data=data)

            if res.status_code != 200:
                print "Error regitstering DIGITAL_INPUT on pin: ", pin, " Reason: ", res.reason
                return False
            
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering DIGITAL_INPUT on pin: ", pin, " Reason: ", obj["reason"]
                return False

            s.thread = Thread(target=s.send, args=(self.url, self.token))
            s.thread.start()

    def registerAnalogInput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            print "Id: ", _id, " already registerd"
            return False
        else:
            s = Sensor(_id, ANALOG_INPUT, pin, sleep_time)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : ANALOG_INPUT}
            res = requests.post(url, data=data)

            if res.status_code != 200:
                print "Error regitstering ANALOG_INPUT on pin: ", pin, " Reason: ", res.reason
                return False
            
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering ANALOG_INPUT on pin: ", pin, " Reason: ", obj["reason"]
                return False

            s.thread = Thread(target=s.send, args=(self.url, self.token))
            s.thread.start()

    def registerDigitalOutput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            print "Id: ", _id, " already registerd"
            return False
        else:
            s = Sensor(_id, DIGITAL_OUTPUT, pin, sleep_time)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : DIGITAL_OUTPUT}
            res = requests.post(url, data=data)

            if res.status_code != 200:
                print "Error regitstering DIGITAL_OUTPUT on pin: ", pin, " Reason: ", res.reason
                return False
            
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering DIGITAL_OUTPUT on pin: ", pin, " Reason: ", obj["reason"]
                return False

            s.thread = Thread(target=s.get, args=(self.url, self.token))
            s.thread.start()

    def registerPWMOutput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            print "Id: ", _id, " already registerd"
            return False
        else:
            s = Sensor(_id, PWM_OUTPUT, pin, sleep_time)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : PWM_OUTPUT}
            res = requests.post(url, data=data)

            if res.status_code != 200:
                print "Error regitstering PWM_OUTPUT on pin: ", pin, " Reason: ", res.reason
                return False
            
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering PWM_OUTPUT on pin: ", pin, " Reason: ", obj["reason"]
                return False

            s.thread = Thread(target=s.get, args=(self.url, self.token))
            s.thread.start()