/**
 * @author Andrei Stefanescu
 * Wyliodrin SRL
 * 
 */

#include <ctype.h>
#define ARDUINO_UNO_

/* select what to import */

#if defined(ARDUINO_YUN_)
  #include <Bridge.h>
  #include <HttpClient.h>
#elif defined(ARDUINO_UNO_)
  #include <SPI.h>
  #include <string.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
#endif

/**
 * Function that sends a request and reads the result
 * @param  host the url for the IoT server
 * @param  path the endpoint eg /register, /send, /get
 * @param  data the body of the request
 * @return String containing the result
 */
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

/* current sensor types */
enum sensor_type {
  DIGITAL_INPUT,
  DIGITAL_OUTPUT,
  PWM_OUTPUT,
  ANALOG_INPUT,
  GENERIC_INPUT,      /* used for sensors with a function as a parameter*/
  GENERIC_OUTPUT,     /* used for sensors with a function as a parameter*/
  UNDEFINED           /* NULL type like */
};

/**
 * Sensor implementation. Contains reference to the sensor:
 * : name           -> the so called id in http requests
 * : physical pin   -> the pin on the arduino board
 * : current value  -> its value
 * : sensor type    -> sensor_type enum value
 * : sensor range   -> used to limit sending data if value read is in range
 */
class Sensor {

public:

  /* default constructor, not to be used */
  Sensor() {
    this->id = String("invalid");
    this->pin = -1;
    this->value = 0;
    this->type  = UNDEFINED;
    this->range = 0;
  }
  
  /**
   * Constructor
   * @param id sensor name
   * @param type the sensor's type
   * @pin physical pin
   * @param range the value range
   */
  Sensor(const String& id, sensor_type type, int pin, int range) {
    this->id = id;
    this->pin = pin;
    this->value = 0;
    this->type = type;
    this->range = range;
  }

  /* copy constructor */
  Sensor(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;
  }

  /* overriding = operator */
  Sensor& operator=(const Sensor& s) {
    this->id = s.id;
    this->pin = s.pin;
    this->value = s.value;
    this->type = s.type;
    this->range = s.range;

    return *(this);
  }

  float value;      /* current value */
  int pin;          /* physical pin */
  int range;        /* value range */
  String id;        /* sensor name */
  sensor_type type; /* sensor type */
};

/**
 * RegisterServer class. Registers each sensor and handles them.
 *
 * It contains a setup function and a loop function to be used
 * Arduino-like style. Also led 13 is used to notify current state.
 * When it is ON, it means the board is communicating with the server.
 * 
 */
class RegisterServer {

public:

  /* constructor */
  RegisterServer() {
    nrSensors = 0;
    pinMode(13, OUTPUT);
  }

  /** Initial setup
   * @param serverAddres IoT server url
   * @param mac 4 bytes representing the MAC address
   * @param token security token
   */
  int begin(String serverAddress, byte mac[4], String token) {

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

  /**
   * Registers a Generic input
   * 
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   * @param func pointer to a function
   */
  bool registerGenericInput(String id,
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

  /**
   * Registers a Generic output
   * 
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   * @param func pointer to a function
   */
  bool registerGenericOutput(String id,
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
  
  /**
   * Registers a Digital input
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerDigitalInput(String id, int pin, int range) {
    if (nrSensors == MAX_SIZE) {
      return false;
    }

    /* set the board for it */
    pinMode(pin, INPUT);
   
    /* create it */ 
    Sensor s(id, DIGITAL_INPUT, pin, range);
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

  /**
   * Registers a Analog input
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
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

  /**
   * Registers a Digital output
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerDigitalOutput(String id, int pin) {
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

   /**
   * Registers a PWM output
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerPWMOutput(String id, int pin) {
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

  /* main loop */
  /**
   * Main loop
   *
   * Takes each sensor and initiates communication to server if needed.
   * Reads values for input sensors and outputs values for output sensors.
   *
   * WARNING: LED 13 is used to show that the board is communicating with the server.
   * 
   */
  void loop() {

    /* communicating with the server notification */
    digitalWrite(13, HIGH);

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
    digitalWrite(13, LOW);
  }

  /* used for debug/log only */
  void printStatus() {
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

private:

  String token;                         /* security token */
  Sensor sensors[MAX_SIZE];             /* array of sensors */
  float (*functions[MAX_SIZE])(float);  /* array of generic functions */
  String url;                           /* url of IoT server */
  int nrSensors;                        /* current number of sensors */
};
