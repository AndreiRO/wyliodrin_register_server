/**
 * @author Andrei Stefanescu
 * Wyliodrin SRL
 * 
 */
#include "RegisterServer.h"


String sendRequest(const char* host, const char* path, const char* data) {
  EthernetClient client;

  /* connect and send request */
  String value;
  if (client.connect(host, 80)) {

    /* send headers */
    String line = "POST ";
    line += path;
    line += " HTTP/1.1";
    String h = "Host: ";
    h += host;
    Serial.println("Connected");
    String length = "Content-Length: ";
    length += strlen(data);

    client.println(line);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println(length);
    client.println("Connection: close");
    client.println(h);
    client.println();
    client.println(data);
    client.println();

    char old = '\0', current = '\0';
    bool mode = false;

    /* read result */
    while (client.connected()) {
      if (client.available()) {
        if (current != '\r') old = current;
        current = client.read();
        if (mode) value += current;
        
        if (current == old && current == '\n') mode = true;
      }
    }

    /* also deallocates data */
    client.stop();
  } else {
    Serial.println("Failed connecting");
  }

  /* for debugging */
  Serial.println("Got: " + value);

  return value;
}

Sensor::Sensor() {
    this->id = String("invalid");
    this->pin = -1;
    this->value = 0;
    this->type  = UNDEFINED;
    this->range = 0;
}
  
Sensor::Sensor(const String& id, sensor_type type, int pin, int range) {
    this->id = id;
    this->pin = pin;
    this->value = 0;
    this->type = type;
    this->range = range;
}

Sensor::Sensor(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;
}


Sensor& Sensor::operator=(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;

    return *(this);
}


Register::RegisterServer() {
    nrSensors = 0;
#ifdef NETWORKING_STATUS
    pinMode(13, OUTPUT);
#endif
}


int RegisterServer::begin(String serverAddress, byte mac[4], String token) {

    #if defined(ARDUINO_YUN_)
      Bridge.begin();  /* for internet */
      Console.begin(); /* for debugging/logging */
    #elif defined(ARDUINO_UNO_)
      /* begin ethernet */
      delay(1000);
      Serial.begin(9600);

      if (!Ethernet.begin(mac)) {
        Serial.println("Error configuring ethernet");
      }
      Serial.println("Configured");
    #endif


    this->token = token;
    this->url = serverAddress;
    this->nrSensors = 0;

    return 0;
}

bool RegisterServer::registerGenericInput(String id,
                                          int pin,
                                          int range,
                                          float (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
}
    
    /* create sensor and add it */
    Sensor s(id, GENERIC_INPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=GENERIC_INPUT";

    /* send request to server */
    #if defined(ARDUINO_YUN_)    
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
 
    return true;
}


bool RegisterServer::registerGenericOutput(String id,
                                           int pin,
                                           int range,
                                           float (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    /* create sensor and add it */
    Sensor s(id, GENERIC_OUTPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=GENERIC_OUTPUT";
    
    /* send request to server */
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
    func(0);

    return true;
}
  

bool RegisterServer::registerDigitalInput(String id, int pin) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }

    /* set the board for it */
    pinMode(pin, INPUT);
   
    /* create it */ 
    Sensor s(id, DIGITAL_INPUT, pin, 0);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=DIGITAL_INPUT";
    
    /* send request to server */
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif

    return true;
}

bool registerAnalogInput(String id, int pin, int range) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    /* create it and add it */
    Sensor s(id, ANALOG_INPUT, pin, range);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=ANALOG_INPUT";

    /* send request to server */
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
      Console.println("Registering aI");
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif

    return true;
}


bool RegisterServer::registerDigitalOutput(String id, int pin) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    pinMode(pin, OUTPUT);
    
    /* create and add it */
    Sensor s(id, DIGITAL_OUTPUT, pin, 0);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=DIGITAL_OUTPUT";
    
    /* send request to server */
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
      Console.println("Regist do");
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
    digitalWrite(pin, 0);

    return true;
}


bool RegisterServer::registerPWMOutput(String id, int pin) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }

    /* create sensor and add it */
    Sensor s(id, PWM_OUTPUT, pin, 0);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=PWM_OUTPUT";
    
    /* send request to server */
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
    analogWrite(pin, 0);
    

    return true;
}


void RegisterServer::loop() {

    /* communicating with the server notification */
#ifdef NETWORKING_STATUS
    digitalWrite(NETWORKING_PIN, HIGH);
#endif
    /* iterate over sensors and update them */
    for (int i = 0; i < nrSensors; ++ i) {
      if (sensors[i].type == DIGITAL_INPUT ||
          sensors[i].type == ANALOG_INPUT  || 
          sensors[i].type == GENERIC_INPUT) {
        
        int val;
        
        /* retrieve value */
        if(sensors[i].type == DIGITAL_INPUT) {
          val = digitalRead(sensors[i].pin);
        } else if (sensors[i].type == ANALOG_INPUT) {
          val = analogRead(sensors[i].pin);
        } else {
          val = functions[i](0);
        }
        
        if (abs(sensors[i].value - val)  <= sensors[i].range) {
          /* new value is within range */ 
          continue;
        }
        
        /* prepare request for server */
        sensors[i].value = val;
        String data = "id=" +
                      sensors[i].id +
                      "&token=" +
                      token +
                      "&value=" +
                      sensors[i].value;

        /* send request */
        #if defined(ARDUINO_YUN_)
          HttpClient c;
          String newUrl = url + "/send";

          c.post(newUrl, data);
        #elif defined(ARDUINO_UNO_)
          sendRequest(url.c_str(), "/send", data.c_str());
        #endif

      } else if (sensors[i].type == DIGITAL_OUTPUT || 
                 sensors[i].type == PWM_OUTPUT     ||
                 sensors[i].type == GENERIC_OUTPUT) {

        /* prepare retrieval request */
        String data = "id=" + sensors[i].id + "&token=" + token + "&plain=1";
        String value = "";

        /* send request */
        #if defined(ARDUINO_YUN_)
          HttpClient c;
          
          String newUrl = url + "/get";   
          c.post(newUrl, data);

          /* read response */
          while (c.available()) {
            char w = c.read();          
            if (!isdigit(w) && w != '.') break;
            value += w;
          }

          sensors[i].value = value.toFloat();

        #elif defined(ARDUINO_UNO_)
          /* send and retrieve response */
          value = sendRequest(url.c_str(), "/get", data.c_str());
          sensors[i].value = value.toFloat();
        #endif

        /* set the sensor's value */
        if (sensors[i].type == DIGITAL_OUTPUT) {
            digitalWrite(sensors[i].pin, value.toInt());
        } else if (sensors[i].type == PWM_OUTPUT) {
            analogWrite(sensors[i].pin, value.toInt());
        } else {
            functions[i](value.toFloat());
        }
        
      }
    }

    /* no more communicating with the server */
#ifdef NETWORKING_STATUS
    digitalWrite(NETWORKING_PIN, LOW);
#endif
}


void RegisterServer::printStatus() {
    #if defined(ARDUINO_YUN_)
      Console.print("-------------------------\n");
      Console.flush();
      for (int i = 0; i < nrSensors; ++ i) {
        Console.print(sensors[i].id +
                      " has value: " +
                      String(sensors[i].value) +
                      "\n");
      }

      Console.print("-------------------------\n");
    #elif defined(ARDUINO_UNO_)
      Serial.print("-------------------------\n");
      Serial.flush();
      for (int i = 0; i < nrSensors; ++ i) {
        Serial.print(sensors[i].id + " has value: " +
                     String(sensors[i].value) + "\n");
      }

      Serial.print("-------------------------\n");
    #endif
}
