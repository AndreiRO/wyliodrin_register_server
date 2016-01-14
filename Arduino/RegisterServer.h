/**
 * @author Andrei Stefanescu
 * Wyliodrin SRL
 * 
 */

#include <ctype.h>

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

/* use led for displaying that the board is communicating with the server */
#ifndef NETWORKING_STATUS
 #define NETWORKING_STATUS 0
#endif

/* the pin for the led used to show active communication with server */
#ifndef NETWORKING_PIN
 #define NETWORKING_PIN 13
#endif


/**
 * Function that sends a request and reads the result
 * @param  host the url for the IoT server
 * @param  path the endpoint eg /register, /send, /get
 * @param  data the body of the request
 * @return String containing the result
 */
String sendRequest(const char* host, const char* path, const char* data);

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
  Sensor();
  
  /**
   * Constructor
   * @param id sensor name
   * @param type the sensor's type
   * @pin physical pin
   * @param range the value range
   */
  Sensor(const String& id, sensor_type type, int pin, int range);

  /* copy constructor */
  Sensor(const Sensor& s);

  /* overriding = operator */
  Sensor& operator=(const Sensor& s);

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
  RegisterServer();

  /** Initial setup
   * @param serverAddres IoT server url
   * @param mac 4 bytes representing the MAC address
   * @param token security token
   */
  int begin(String serverAddress, byte mac[4], String token);

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
                            float (*func)(float));

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
                             float (*func)(float));
  /**
   * Registers a Digital input
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerDigitalInput(String id, int pin, int range);

  /**
   * Registers a Analog input
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerAnalogInput(String id, int pin, int range);

  /**
   * Registers a Digital output
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerDigitalOutput(String id, int pin);

   /**
   * Registers a PWM output
   * @param id the name of the sensor
   * @param pin the physical pin
   * @param range the values' range before sending a new request
   */
  bool registerPWMOutput(String id, int pin);

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
  void loop();
  
  /* used for debug/log only */
  void printStatus();

private:

  String token;                         /* security token */
  Sensor sensors[MAX_SIZE];             /* array of sensors */
  float (*functions[MAX_SIZE])(float);  /* array of generic functions */
  String url;                           /* url of IoT server */
  int nrSensors;                        /* current number of sensors */
};
