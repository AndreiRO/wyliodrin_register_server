
#include <Bridge.h>
#include <HttpClient.h>

#define MAX_SIZE 7


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

  int value;
  int pin;
  int range;
  String id;
  sensor_type type;
};


class GenericSensor : public Sensor {

public:
  GenericSensor() : Sensor() {
    this->func = NULL;
  }

  GenericSensor(
          const String& id,
          sensor_type type,
          int pin,
          int range,
          int (*func)(void)) : Sensor(id, type, pin, range) {
    this->func = func;
  }

  GenericSensor(const GenericSensor& s) {
      this->id = s.id;
      this->pin = s.pin;
      this->value = s.value;
      this->type = s.type;
      this->range = s.range;
      this->func = s.func;
    }

    GenericSensor& operator=(const GenericSensor& s) {
    this->id = s.id;
      this->pin = s.pin;
      this->value = s.value;
      this->type = s.type;
      this->range = s.range;
      this->func = s.func;

      return *(this);
  }

  int (*func)(void);

};

class RegisterServer {

public:

  RegisterServer() {
    nrSensors = 0;
    pinMode(13, OUTPUT);
  }

  void begin(String serverAddress, String token) {
    Bridge.begin();
    Console.begin();
    this->token = token;
    this->url = serverAddress;
    this->nrSensors = 0;
    this->nrGenerics = 0;
  }

  bool registerGenericSensor(String id, int pin, int range, int (*func)(void)) {
    if (nrGenerics == MAX_SIZE) {
      return false;
    }
    
    GenericSensor s(id, GENERIC_INPUT, pin, range, func);
    generics[nrGenerics] = s;
    nrGenerics += 1;
   
    String newUrl = url + "/register";
    String data = "id=" + id + "&token=" + token + "&type=GENERIC_SENSOR";
    
    HttpClient client;
    client.post(newUrl, data);

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
    
    HttpClient client;
    client.post(newUrl, data);

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
    
    HttpClient client;
    client.post(newUrl, data);

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
    
    HttpClient client;
    client.post(newUrl, data);

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
    
    HttpClient client;
    client.post(newUrl, data);

    return true;
  }

  void loop() {
    digitalWrite(13, HIGH);

    for (int i = 0; i < nrSensors; ++ i) {
      if (sensors[i].type == DIGITAL_INPUT || sensors[i].type == ANALOG_INPUT) {
        
        int val;
        
        if(sensors[i].type == DIGITAL_INPUT) {
          val = digitalRead(sensors[i].pin);
        } else {
          val = analogRead(sensors[i].pin);
        }
        if (abs(sensors[i].value - val)  <= sensors[i].range) {
          /* should we keep new or old value */
          sensors[i].value = val;
          continue;
        }

        HttpClient c;
        
        String newUrl = url + "/send";
        String data = "id=" + sensors[i].id + "&token=" + token + "&value=" + sensors[i].value;

        
        c.post(newUrl, data);
        
      } else if (sensors[i].type == DIGITAL_OUTPUT || sensors[i].type == PWM_OUTPUT) {
        HttpClient c;
        
        String newUrl = url + "/get";
        String data = "id=" + sensors[i].id + "&token=" + token;

        
        c.post(newUrl, data);
        
        String value = "";
        int flag = 0;
        
        while (c.available() && flag != 9) {
          char w = c.read();          

         /* 34 is " */
         if (w == ' ') {
          ;
         } if (w == 'v') {
            flag = 1;
          } else if (w == 'a' && flag == 1) {
            flag = 2;  
          } else if (w == 'l' && flag == 2) {
            flag = 3;  
          } else if (w == 'u' && flag == 3) {
            flag = 4;  
          } else if (w == 'e' && flag == 4) {
            flag = 5; 
          }  else if (w == 34 && flag == 5) {
            flag = 6;  
          } else if (w == ':' && flag == 6) {
            flag = 7;
          } else if ((w >= '0' && w <= '9')  && flag == 7) {
            if (value == "") value = String(w);
            else value += String(w);
          } else if(w == ',' && flag == 7) {
            break;
          }
        }

        if (sensors[i].type == DIGITAL_OUTPUT) {
          digitalWrite(sensors[i].pin, value.toInt());
        } else {
          analogWrite(sensors[i].pin, value.toInt());
        }
        
      }
    }
    digitalWrite(13, LOW);
  }

  void printStatus() {
    Console.print("-------------------------\n");
    Console.flush();
    for (int i = 0; i < nrSensors; ++ i) {
      Console.print(sensors[i].id + " has value: " + String(sensors[i].value) + "\n");
    }

    for (int i = 0; i < nrGenerics; ++ i) {
      Console.print(generics[i].id + " has value: " + String(generics[i].value) + "\n");
    }
    Console.print("-------------------------\n");

  }

private:
  HttpClient client;
  String token;
  Sensor sensors[MAX_SIZE];
  GenericSensor generics[MAX_SIZE];
  String url;
  int nrSensors;
  int nrGenerics;

};

RegisterServer server;

void setup() {
  server.begin("http://andreiro-server.iot.wyliodrin.com", "salut123");
  server.registerDigitalInput("test_", 3, 0);
  server.registerAnalogInput("analog", 0, 10);
  server.registerDigitalOutput("out", 4);
  server.registerPWMOutput("var", 6);
}

void loop() {
  Console.print("New loop\n");
  server.loop();
  server.printStatus();
  delay(1000);
}


 