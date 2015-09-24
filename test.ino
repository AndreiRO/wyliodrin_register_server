#include <ctype.h>
#define ARDUINO_UNO_

#define MAX_SIZE 5

#if defined(ARDUINO_YUN_)
  #include <Bridge.h>
  #include <HttpClient.h>
#elif defined(ARDUINO_UNO_)
  #include <SPI.h>
  #include <Ethernet.h>
  #include <UnoHTTPClient.h>
  #include <RegisterServer.h>
#endif


RegisterServer server;

void setup() {
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xD2, 0x89};

  server.begin(PSTR("andreiro-server.iot.wyliodrin.com"), mac, PSTR("salut124"));

  server.registerAnalogInput(PSTR("light"), 0, 10);
  server.registerDigitalOutput(PSTR("led"), 5);

}

void loop() {
  server.loop();
  server.printStatus();
  delay(1000);
}


