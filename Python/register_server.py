#!/bin/env python

""""
Author: Andrei Ștefănescu
Wyliodrin SRL
"""

import requests
import json
import time

from threading import Thread, Lock

from wyliodrin import *

# Sensor types used by server
DIGITAL_INPUT   = "DIGITAL_INPUT"
DIGITAL_OUTPUT  = "DIGITAL_OUTPUT"
PWM_OUTPUT      = "PWM_OUTPUT"
ANALOG_INPUT    = "ANALOG_INPUT"
GENERIC_OUTPUT  = "GENERIC_OUTPUT"
GENERIC_INPUT   = "GENERIC_INPUT"



class Sensor(object):
    """
        The base class for sensors. It is instanced by RegisterServer with
        one instance per sensor. Each sensor will have a separate thread.
    """

    def __init__(self, _id, _type, pin, sleep_time, value_range, function = None):
        """
            Constructor for a sensor.

            Arguments:
                _id         -> the sensor's name
                type        -> the sensor's type (DIGITAL_INPUT, PWM_OUTPUT etc.)
                pin         -> physical pin
                sleep_time  -> sleep for sensor's thread
                value_range -> the range for limiting server commmunication
                function    -> pointer to a function, used only for generic sensors
        """
        # the sensor name, eg. light_sensor, button1
        self._id = _id
       
        # Digital/Analog/Generic Input/Output
        self._type = _type
       
        # lock use for protecting variables' value
        # during communication with server
        self.lock = Lock()

        # the current value for the sensor
        self.value = 0

        # its pin on the board used by libwyliodrin
        self.pin = pin

        # value range to reduce communication with server
        # if the value read is in range with the new value
        self.value_range = value_range

        # sleep time for each sensor between reads/writes
        self.sleep_time = 0 if sleep_time < 0 else sleep_time

        # used only for generic sensors
        self.function = function

        # setup
        if self._type == DIGITAL_INPUT:
            pinMode(pin, INPUT)
        elif self._type == DIGITAL_OUTPUT:
            pinMode(pin, OUTPUT)



    def get(self, url, token):
        """
            Function called to get the value for a sensor.
            Communicates with the server and waits for response.

            Arguments:
                url   -> the host part of the url
                       eg http://iot.wyliodrin.register-server.org/
                token -> user's security token
        """
        # create url
        url = url + ('/' if url[-1] != '/' else '') + 'get'

        while True:     
            try:
                self.lock.acquire()
                
                # request body
                data = {"id" : self._id, "token" : token}
                res = requests.post(url, data=data)

                if res.status_code == 200:
                    obj = json.loads(res.content)

                    if obj["error"] != 0:
                        # troubleshooting
                        print obj["reason"]
                    else:
                        self.value = obj["value"]

                        # log
                        print "Got value: ",
                              self.value,
                              " for sensor on pin: ",
                              self.pin

                        if self._type == DIGITAL_OUTPUT:
                            digitalWrite(self.pin, self.value)
                        elif self._type == PWM_OUTPUT:
                            analogWrite(self.pin, self.value)
                        elif self._type == GENERIC_OUTPUT:
                            self.function(self.value)
                else:
                    print "Error: " + res.reason

            finally:
                self.lock.release()
            time.sleep(self.sleep_time)



    def send(self, url, token):
        """
            Function called to set the value for a sensor.
            Communicates with the server and waits for the response.

            Arguments:
                url   -> the host part of the url
                       eg http://iot.wyliodrin.register-server.org/
                token -> user's security token
        """
        # create url
        url = url + ('/' if url[-1] != '/' else '') + 'send'

        while True:
            try:
                self.lock.acquire()
                
                if self._type == DIGITAL_INPUT:
                    val = digitalRead(self.pin)

                    # check that the value is greater than the range
                    if abs(val - self.value) <= self.value_range:
                        continue
                    self.value = val
                elif self._type == ANALOG_INPUT:
                    val = analogRead(self.pin)

                    # check that the value is greater than the range
                    if abs(val - self.value) <= self.value_range:
                        continue
                    self.value = val
                elif self._type == GENERIC_INPUT:
                    val = self.function(0)

                    # check that the value is greater than the range
                    if abs(val - self.value) <= self.value_range:
                        continue
                    self.value = val

                # form request body
                data = {"id" : self._id, "token" : token, "value" : self.value}
                res = requests.post(url, data=data)

                if res.status_code != 200:
                    print "Error: " + res.reason
                else:
                    print "Sent value: ",
                          self.value,
                          " for sensor on pin: ",
                          self.pin

            finally:
                self.lock.release()
            time.sleep(self.sleep_time)


# simply check if the pin number is a valid one 
def validate_pin(pin):
    if pin >= 0:
        return True
    return False

"""
    The class for RegisterServer. It handles registration of sensors.
"""
class RegisterServer(object):

    def __init__(self, url, token):
        # list with all sensors
        self.sensors = {}

        # IoT server url
        self.url = url

        # security token
        self.token = token

    """
        Registers a digital input sensor and starts it.

        Arguments:
            _id         -> the name of the sensor
            pin         -> the physical pin
            sleep_time  -> the pause between reads
            value_range -> range to limit connection usage
    """
    def registerDigitalInput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create sensor
            s = Sensor(_id, DIGITAL_INPUT, pin, sleep_time, 0)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}
    
            # try to register it
            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : DIGITAL_INPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering DIGITAL_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      res.reason

                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering DIGITAL_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      obj["reason"]

                return False

            # start it
            s.thread = Thread(target=s.send, args=(self.url, self.token))
            s.thread.start()


    """
        Registers a analog input sensor and starts it.

        Arguments:
            _id         -> the name of the sensor
            pin         -> the physical pin
            sleep_time  -> the pause between reads
            value_range -> range to limit connection usage
    """
    def registerAnalogInput(self, _id, pin, sleep_time, value_range):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create sensor
            s = Sensor(_id, ANALOG_INPUT, pin, sleep_time, value_range)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : ANALOG_INPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering ANALOG_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      res.reason

                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering ANALOG_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      obj["reason"]
                return False


            # start it
            s.thread = Thread(target=s.send, args=(self.url, self.token))
            s.thread.start()


    """
        Registers a digital output sensor and starts it.

        Arguments:
            _id         -> the name of the sensor
            pin         -> the physical pin
            sleep_time  -> the pause between reads
            value_range -> range to limit connection usage
    """
    def registerDigitalOutput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create it
            s = Sensor(_id, DIGITAL_OUTPUT, pin, sleep_time, 0)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : DIGITAL_OUTPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering DIGITAL_OUTPUT on pin: ",
                      pin,
                      " Reason: ",
                      res.reason

                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering DIGITAL_OUTPUT on pin: ",
                      pin,
                      " Reason: ",
                      obj["reason"]

                return False

            # start it
            s.thread = Thread(target=s.get, args=(self.url, self.token))
            s.thread.start()

    """
        Registers a pwm output sensor and starts it.

        Arguments:
            _id         -> the name of the sensor
            pin         -> the physical pin
            sleep_time  -> the pause between reads
            value_range -> range to limit connection usage
    """
    def registerPWMOutput(self, _id, pin, sleep_time):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create it
            s = Sensor(_id, PWM_OUTPUT, pin, sleep_time, 0)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : PWM_OUTPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering PWM_OUTPUT on pin: ", pin, " Reason: ", res.reason
                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering PWM_OUTPUT on pin: ", pin, " Reason: ", obj["reason"]
                return False

            # start it
            s.thread = Thread(target=s.get, args=(self.url, self.token))
            s.thread.start()


    """
        Registers a generic output sensor and starts it.

        Arguments:
            _id         -> the name of the sensor
            pin         -> the physical pin
            sleep_time  -> the pause between reads
            value_range -> range to limit connection usage
            function    -> pointer to function
    """
    def registerGenericOutput(self, _id, pin, sleep_time, value_range, function):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create sensor
            s = Sensor(_id, GENERIC_OUTPUT, pin, sleep_time, value_range, function)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : GENERIC_OUTPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering GENERIC_OUTPUT on pin: ",
                      pin,
                      " Reason: ",
                      res.reason

                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering GENERIC_OUTPUT on pin: ",
                      pin,
                      " Reason: ",
                      obj["reason"]

                return False

            # start it
            s.thread = Thread(target=s.get, args=(self.url, self.token))
            s.thread.start()


    """
        Registers a generic input sensor and starts it.

        Arguments:
            _id -> the name of the sensor
            pin -> the physical pin
            sleep_time -> the pause between reads
            value_range -> range to limit connection usage
            function -> pointer to function
    """
    def registerGenericInput(self, _id, pin, sleep_time, value_range, function):

        if _id in self.sensors or not validate_pin(pin):
            # cannot add it
            print "Id: ", _id, " already registerd or invalid pin"
            return False
        else:
            # create it
            s = Sensor(_id, GENERIC_INPUT, pin, sleep_time, value_range, function)
            self.sensors[_id] = {"object" : s, "autosend" : sleep_time}

            url = self.url + ("/" if self.url[-1] != "/" else "") + "/register"
            data = {"id" : _id, "token" : self.token, "type" : GENERIC_INPUT}
            res = requests.post(url, data=data)

            # check for errors
            if res.status_code != 200:
                print "Error regitstering GENERIC_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      res.reason

                return False
            
            # check for errors
            obj = json.loads(res.content)
            if obj["error"] != 0 and obj["error"] != 2:
                print "Error regitstering GENERIC_INPUT on pin: ",
                      pin,
                      " Reason: ",
                      obj["reason"]
                      
                return False

            # start it
            s.thread = Thread(target=s.send, args=(self.url, self.token))
            s.thread.start()