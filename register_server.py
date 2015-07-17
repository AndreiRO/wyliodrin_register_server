#!/bin/env python

import requests
import json
import time

from threading import Thread, Lock, Timer

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
        while True:     
            try:
                self.lock.acquire()
                url = url + ('/' if url[-1] != '/' else '') + 'get'
                data = {"id" : self._id, "token" : token}
                res = requests.post(url, data=data)

                if res.status_code == 200:
                    obj = json.loads(res.content)
                    self.value = obj["value"]
                else:
                    print "Error: " + res.reason

            finally:
                self.lock.release()
            time.sleep(self.sleep_time)



    def send(self, url, token):
        while True:
            try:
                self.lock.acquire()
                url = url + ('/' if url[-1] != '/' else '') + 'get'
                
                if self._type == DIGITAL_INPUT:
                    self.value = digitalRead(self.pin)
                else:
                    self.value = analogRead(self.pin)

                data = {"id" : self._id, "token" : token, "value" : self.value}
                res = requests.post(url, data=data)

                if res.status_code != 200:
                    print "Error: " + res.reason

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
            s.thread = Thread(s.send, args=(self.url, self.token))
            s.thread.start()


