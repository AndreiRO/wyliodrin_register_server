#define MAX_SIZE 5
#define NETWORKING_STATUS 1
#define NETWORKING_PIN 13
#define ARDUINO_UNO_


RegisterServer server;
byte[] mac = {0x1, 0x2, 0x3, 0x4};
String url = "http://register-server.iot.wyliodrin.com";
String token = "password";

void setup() 
    server.registerDigitalOutput("LED", 10);
    server.registerPWMOutput("VarLED", 11);
    server.registerDigitalInput("Button", 4, 0);
    server.registerAnalogInput("Temperature", 0, 10);

    server.begin(url, mac, token);
}

void loop() {
    server.loop();
}