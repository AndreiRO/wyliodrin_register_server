from register_server import RegisterServer


url = "http://server.iot.wyliodrin.com"
token = "password"

server = RegisterServer(url, token)

server.registerDigitalInput("Button", 4, 10)
server.registerAnalogInput("Temperature", 3, 10, 15)
server.registerDigitalOutput("LED", 5, 10)
serevr.registerPWMOutput("VarLED", 2, 10)
