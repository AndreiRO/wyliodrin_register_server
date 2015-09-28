#include <ctype.h>
#define ARDUINO_UNO_

#if defined(ARDUINO_YUN_)
  #include <Bridge.h>
  #include <HttpClient.h>
#elif defined(ARDUINO_UNO_)
  #include <SPI.h>
  #include <string.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
#endif

String sendRequest(const char* host, const char* path, const char* data) {
  EthernetClient client;

  String value;
  if (client.connect(host, 80)) {
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

    while (client.connected()) {
      if (client.available()) {
        if (current != '\r') old = current;
        current = client.read();
        if (mode) value += current;
        
        if (current == old && current == '\n') mode = true;
      }
    }

    client.stop();
    
  } else {
    Serial.println("Failed connecting");
  }
  Serial.println("Got: " + value);

  return value;
}


enum sensor_type {
  DIGITAL_INPUT,
  DIGITAL_OUTPUT,
  PWM_OUTPUT,
  ANALOG_INPUT,
  GENERIC_INPUT,
  GENERIC_OUTPUT,
  UNDEFINED
};


class Sensor {

public:

  Sensor() {
    this->id = String("invalid");
    this->pin = -1;
    this->value = 0;
    this->type  = UNDEFINED;
    this->range = 0;
  }
  

  Sensor(const String& id, sensor_type type, int pin, int range) {
    this->id = id;
    this->pin = pin;
    this->value = 0;
    this->type = type;
    this->range = range;
  }

  Sensor(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;
  }

  Sensor& operator=(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;

    return *(this);
  }

  float value;
  int pin;
  int range;
  String id;
  sensor_type type;
};


class RegisterServer {

public:

  RegisterServer() {
    nrSensors = 0;
    pinMode(13, OUTPUT);
  }

  int begin(String serverAddress, byte mac[4], String token) {

    #if defined(ARDUINO_YUN_)
      Bridge.begin();
      Console.begin();
    #elif defined(ARDUINO_UNO_)
      //begin ethernet
      //TODO: edit MAC!!
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

  bool registerGenericInput(String id, int pin, int range, float (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    Sensor s(id, GENERIC_INPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=GENERIC_INPUT";

    #if defined(ARDUINO_YUN_)    
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      //arduino http client
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
 
    return true;
  }

  bool registerGenericOutput(String id, int pin, int range, float (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    Sensor s(id, GENERIC_OUTPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=GENERIC_OUTPUT";
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
    func(0);

    return true;
  }
  
  bool registerDigitalInput(String id, int pin, int range) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    pinMode(pin, INPUT);
    
    Sensor s(id, DIGITAL_INPUT, pin, range);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=DIGITAL_INPUT";
    
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
    
    Sensor s(id, ANALOG_INPUT, pin, range);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=ANALOG_INPUT";

    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
      Console.println("Registering aI");
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif

    return true;
  }

  bool registerDigitalOutput(String id, int pin) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    pinMode(pin, OUTPUT);
    
    Sensor s(id, DIGITAL_OUTPUT, pin, 0);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=DIGITAL_OUTPUT";
    
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

  bool registerPWMOutput(String id, int pin) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }

    Sensor s(id, PWM_OUTPUT, pin, 0);
    sensors[nrSensors] = s;
    nrSensors += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=PWM_OUTPUT";
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      sendRequest(url.c_str(), "/register", data.c_str());
    #endif
    analogWrite(pin, 0);
    

    return true;
  }

  void loop() {
    digitalWrite(13, HIGH);

    for (int i = 0; i < nrSensors; ++ i) {
      if (sensors[i].type == DIGITAL_INPUT ||
          sensors[i].type == ANALOG_INPUT  || 
          sensors[i].type == GENERIC_INPUT) {
        
        int val;
        
        if(sensors[i].type == DIGITAL_INPUT) {
          val = digitalRead(sensors[i].pin);
        } else if (sensors[i].type == ANALOG_INPUT) {
          val = analogRead(sensors[i].pin);
        } else {
          val = functions[i](0);
        }
        
        if (abs(sensors[i].value - val)  <= sensors[i].range) {
          /* should we keep new or old value */ 
          continue;
        }
        
        sensors[i].value = val;
        String data = "id=" + sensors[i].id + "&token=" + token + "&value=" + sensors[i].value;

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

        String data = "id=" + sensors[i].id + "&token=" + token + "&plain=1";
        String value = "";

        #if defined(ARDUINO_YUN_)
          HttpClient c;
          
          String newUrl = url + "/get";

          
          c.post(newUrl, data);

          while (c.available()) {
            char w = c.read();          
            if (!isdigit(w) && w != '.') break;
            value += w;
          }
          sensors[i].value = value.toFloat();
        #elif defined(ARDUINO_UNO_)
          value = sendRequest(url.c_str(), "/get", data.c_str());
          sensors[i].value = value.toFloat();
        #endif

        if (sensors[i].type == DIGITAL_OUTPUT) {
            digitalWrite(sensors[i].pin, value.toInt());
        } else if (sensors[i].type == PWM_OUTPUT) {
            analogWrite(sensors[i].pin, value.toInt());
        } else {
            functions[i](value.toFloat());
        }
        
      }
    }
    digitalWrite(13, LOW);
  }

  void printStatus() {
#if defined(ARDUINO_YUN_)
    Console.print("-------------------------\n");
    Console.flush();
    for (int i = 0; i < nrSensors; ++ i) {
      Console.print(sensors[i].id + " has value: " + String(sensors[i].value) + "\n");
    }

    Console.print("-------------------------\n");
#elif defined(ARDUINO_UNO_)
    Serial.print("-------------------------\n");
    Serial.flush();
    for (int i = 0; i < nrSensors; ++ i) {
      Serial.print(sensors[i].id + " has value: " + String(sensors[i].value) + "\n");
    }

    Serial.print("-------------------------\n");
#endif
  }

private:

  String token;
  Sensor sensors[MAX_SIZE];
  float (*functions[MAX_SIZE])(float);
  String url;
  int nrSensors;
  byte hostIp[4];

};
