#define ARDUINO_UNO_

#if defined(ARDUINO_YUN_)
  #include <Bridge.h>
  #include <HttpClient.h>
#elif defined(ARDUINO_UNO_)
  #include <SPI.h>
  #include <Ethernet.h>
#endif


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
        Serial.println(PSTR("Error configuring ethernet"));
      }
      Serial.println(PSTR("Configured"));
    #endif

    this->token = token;
    this->url = serverAddress;
    this->nrSensors = 0;

    return 0;
  }

  bool registerGenericInput(String id, int pin, int range, int (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    Sensor s(id, GENERIC_INPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=GENERIC_INPUT&cool=true");

    #if defined(ARDUINO_YUN_)    
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      //arduino http client
      http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"),
                      NULL,
                      data.c_str(), 
                      header);
      if (res) client.closeStream(res);
    #endif
 
    return true;
  }

  bool registerGenericOutput(String id, int pin, int range, int (*func)(float)) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }
    
    Sensor s(id, GENERIC_OUTPUT, pin, range);
    sensors[nrSensors] = s;
    functions[nrSensors] = func;
    nrSensors += 1;
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=GENERIC_OUTPUT&cool=true");
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      //arduino uno
      http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"), 
                      NULL, 
                      data.c_str(), 
                      header);
      if (res) client.closeStream(res);
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
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=DIGITAL_INPUT&cool=true");
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      //do sth
     http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"), 
                      NULL, 
                      data.c_str(), 
                      header);
      if (res) client.closeStream(res);
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
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=ANALOG_INPUT&cool=true");

    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
      Console.println(PSTR("Registering aI"));
    #elif defined(ARDUINO_UNO_)
      http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"), 
                      NULL, 
                      data.c_str(),
                      header);
                     

      if (res) client.closeStream(res);
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
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=DIGITAL_OUTPUT&cool=true");
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
      Console.println(PSTR("Regist do"));
    #elif defined(ARDUINO_UNO_)
      // do sth
      http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"), 
                      NULL,
                      data.c_str(),
                      header);
      if (res) client.closeStream(res);
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
   
    String newUrl = url + PSTR("/register");
    String data = PSTR("id=") + id + PSTR("&token=") + token + PSTR("&type=PWM_OUTPUT&cool=true");
    
    #if defined(ARDUINO_YUN_)
      HttpClient client;
      client.post(newUrl, data);
    #elif defined(ARDUINO_UNO_)
      //do sth
      http_client_parameter header[] = {
                        {PSTR("id"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
      };
      HTTPClient client(url.c_str());
      FILE* res = client.postURI(
                      PSTR("/register"), 
                      NULL,
                      data.c_str(),
                      header);
      if (res) client.closeStream(res);
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
        String data = PSTR("id=") + sensors[i].id + PSTR("&token=") + token + PSTR("&value=") + sensors[i].value + PSTR("&cool=true");

        #if defined(ARDUINO_YUN_)
          HttpClient c;
          String newUrl = url + PSTR("/send");

          c.post(newUrl, data);
        #elif defined(ARDUINO_UNO_)
          //do sth
          http_client_parameter header[] = {
                        {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                        {NULL, NULL}
          };
          HTTPClient client(url.c_str());
          FILE* res = client.postURI(
                      PSTR("/send"), 
                      NULL,
                      data.c_str(),
                      header);
          if (res) client.closeStream(res);
        #endif
      } else if (sensors[i].type == DIGITAL_OUTPUT || 
                 sensors[i].type == PWM_OUTPUT     ||
                 sensors[i].type == GENERIC_OUTPUT) {

        String data = PSTR("id=") + sensors[i].id + PSTR("&token=") + token + PSTR("&plain=1&cool=true");
        String value = PSTR("");

        #if defined(ARDUINO_YUN_)
          HttpClient c;
          
          String newUrl = url + PSTR("/get");

          
          c.post(newUrl, data);

          while (c.available()) {
            char w = c.read();          
            value += w;
          }
          sensors[i].value = value.toInt();
        #elif defined(ARDUINO_UNO_)
          HTTPClient client(url.c_str());
          
          http_client_parameter header[] = {
                                      {PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded")},
                                      {NULL, NULL}
          };
          FILE* in = client.postURI(
                                    PSTR("/get"),
                                    NULL,
                                    data.c_str(),
                                    header);
          char w;
          while ((w = fgetc(in)) != EOF) {
            value += w;  
          }

          sensors[i].value = value.toInt();
          
          client.closeStream(in);
        #endif

        if (sensors[i].type == DIGITAL_OUTPUT) {
          #if defined(ARDUINO_YUN_)
            digitalWrite(sensors[i].pin, value.toInt());
          #elif defined(ARDUINO_UNO_)
            digitalWrite(sensors[i].pin, value.toInt());
          #endif
        } else if (sensors[i].type == PWM_OUTPUT) {
          #if defined(ARDUINO_YUN_)
            analogWrite(sensors[i].pin, value.toInt());
          #elif defined(ARDUINO_UNO_)
            analogWrite(sensors[i].pin, value.toInt());
          #endif
        } else {
          #if defined(ARDUINO_YUN_)
            functions[i](value.toFloat());
          #elif defined(ARDUINO_UNO_)
            functions[i](value.toFloat());
          #endif
        }
        
      }
    }
    digitalWrite(13, LOW);
  }

  void printStatus() {
#if defined(ARDUINO_YUN_)
    Console.print(PSTR("-------------------------\n"));
    Console.flush();
    for (int i = 0; i < nrSensors; ++ i) {
      Console.print(sensors[i].id + PSTR(" has value: ") + String(sensors[i].value) + PSTR("\n"));
    }

    Console.print(PSTR("-------------------------\n"));
#elif defined(ARDUINO_UNO_)
    Serial.print(PSTR("-------------------------\n"));
    Serial.flush();
    for (int i = 0; i < nrSensors; ++ i) {
      Serial.print(sensors[i].id + PSTR(" has value: ") + String(sensors[i].value) + PSTR("\n"));
    }

    Serial.print(PSTR("-------------------------\n"));
#endif
  }

private:

  String token;
  Sensor sensors[MAX_SIZE];
  int (*functions[MAX_SIZE])(float);
  String url;
  int nrSensors;
  byte hostIp[4];

};


 