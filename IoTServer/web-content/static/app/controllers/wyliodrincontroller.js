/*
  Author: Andrei Ștefănescu
  Wyliodrin SRL
  File has some content taken from
  defaultly generated wyliodrincontroller.js
 */

angular.module('myApp',['ui.ace']).controller ('WyliodrinCtrl', function ($scope, $http)
{
    
    var socket = io('/');
    $scope.sensors = {};
    $scope.codeSensors = {};
    $scope.boardMAC = "";
    $scope.boardType = "ARDUINO_UNO";
    
    socket.on('value', function (data) {
      if (data.variable === "sensors_str") {
        
        if ($scope.sensors) {
          for (var sensor in $scope.sensors) {
            data.value[sensor].val = $scope.sensors[sensor].val;

            /* forced means the value was set but hasn't received confirmation */
            /* so it will hold current value for forced_timeout time or until */
            /* the new value equals the current one */
            /* otherwise it will get the server's value */
            if ($scope.sensors[sensor].forced) {
              if ($scope.sensors[sensor].val === data.value[sensor].value) {
                /* recieved set value */
                data.value[sensor].forced = false;
                clearTimeout($scope.sensors[sensor].forced_timeout);
              } else {
                data.value[sensor].forced = true;
                data.value[sensor].forced_timeout = $scope.sensors[sensor].forced_timeout;
              }
            } else if (data.value[sensor].value !== $scope.sensors[sensor].val) {
              data.value[sensor].val = data.value[sensor].value;
            }
          }
        }

        $scope.sensors = data.value;

      } else {
        /* only for debugging */
        console.log("!!! Forgot me" + data.variable);
        $scope[data.variable] = data.value;
      }
      $scope.$apply();
    });

    /* just send the new code */
    $scope.updateRules = function() {
      $http.post('/rules', {token : $scope.token, code : Blockly.JavaScript.workspaceToCode(workspace)})
      .error(function(data, status, headers, config) {
        console.log("Error updating rules");
      });
    }

    /* get the sensor value */
    $scope.sensorValue = function(id) {
      if ($scope.sensors[id].forced) {
        return $scope.sensors[id].forced_value;
      }
      
      return $scope.sensors[id].value;
    };

    /* update sensor values */
    $scope.update = function(key) {

      $http.post('/send', {id : key, token : $scope.token, value : $scope.sensors[key].val})
      .error(function(data, status, headers, config) {
        console.log("Error sending http request");
      });

      console.log($scope.sensors[key].val)
      $scope.sensors[key].forced = true;
      $scope.sensors[key].forced_timeout = setTimeout(function() {
                                                        console.log($scope.sensors[key].val + " " + $scope.sensors[key].value)
                                                        if ($scope.sensors[key].val !== $scope.sensors[key].value) {
                                                          $scope.sensors[key].val = $scope.sensors[key].value;
                                                          console.log($scope.sensors[key].val + " " + $scope.sensors[key].value)
                                                        }
                                                        $scope.sensors[key].forced = false; 
                                                        console.log("timeouted from forced");
                                                      },
                                            4000);
                                            
      console.log("changed")
    }

    /* add a new sensor */
    $scope.addSensor = function(sensorName, sensorType, sensorPin) {
      console.log(sensorName, sensorType, sensorPin);
      $scope.codeSensors[sensorName]=  {sensorType : sensorType, sensorPin : sensorPin};
      $scope.sensorName = "";
      $scope.sensorType = "";
      $scope.sensorPin = "";
      $scope.generateCode();
    }
    
    /* remove a sensor */
    $scope.removeSensor = function(sensorName) {
      console.log(sensorName);
      delete $scope.codeSensors[sensorName];
      $scope.generateCode();
    }
    
    /* checks if there is a servo in current servo list */
    function hasServo() {
      for (var s in $scope.codeSensors) {
        if ($scope.codeSensors[s].sensorType === "SERVOMOTOR") {
          return true;
        }
      }
      
      return false;
    }
    
    /* generate code for current sensors list */
    $scope.generateCode = function() {
      var code = "#define MAX_SIZE " + Object.keys($scope.codeSensors).length + "\n";
      
      var currentServo = 0;

      if ($scope.boardType === "ARDUINO_UNO") {
        /* add headers */
        code += "#include <SPI.h>\n#include <Ethernet.h>\n" + 
                "#include <EthernetClient.h>\n#include <intttypes.h>\n" +
                "#define ARDUINO_UNO_\n#include <RegisterServer.h>\n";
        
        if (hasServo()) {
          /* add Servo header and generate separate Servo function */
          code += "#include <Servo.h>\n";
          for (var s in $scope.codeSensors) {
            if ($scope.codeSensors[s].sensorType !== "SERVOMOTOR") continue;
            
            code += "Servo servo" + currentServo + ";\n";
            code += "float setServo" + currentServo + "(float value) {\n";
            code += "servo" + currentServo + ".write(value);\n}\n";
            currentServo += 1;
          }
        }
        
        code += "RegisterServer server;\n\n";
      } else if ($scope.boardType === "ARDUINO_YUN") {
        /* add headers */
        code += "#define ARDUINO_YUN_\n#include <RegisterServer.h>\n";
        if (hasServo()) {
          /* add Servo header and generate separate Servo function */
          code += "#include <Servo.h>\n";
          for (var s in $scope.codeSensors) {
            if ($scope.codeSensors[s].sensorType !== "SERVOMOTOR") continue;
            
            code += "Servo servo" + currentServo + ";\n";
            code += "float setServo" + currentServo + "(float value) {\n";
            code += "servo" + currentServo + ".write(value);\n}\n";
            currentServo += 1;
          }
        }
        code += "RegisterServer server;\n\n";
      } else if ($scope.boardType === "OTHERS") {
        /* add imports */
        code = "from register_server import RegisterServer\n";
        
        if (hasServo()) {
          /* import servo and generate separate functions*/
          code += "from pyupm_servo import ES08A as Servo\n";
          for (var s in $scope.codeSensors) {
            if ($scope.codeSensors[s].sensorType !== "SERVOMOTOR") continue;
            
            code += "Servo servo" + currentServo + "\n";
            code += "def setServo" + currentServo + "(value):\n";
            code += "\tservo" + currentServo + ".setAngle(value);\n";
            currentServo += 1;
          }
        }
        code += "server = RegisterServer()\n";
      } else {
        /* for debugging */
        alert("invalid context");
      }

      var currentServo = 0;
      if ($scope.boardType === "ARDUINO_UNO" || $scope.boardType === "ARDUINO_YUN") {
        /* set editor styling */
        $scope.ace.getSession().setMode("ace/mode/c_cpp");

        /* add setup function */
        code += "void setup() {\n";
        code += "byte mac[] = {" + $scope.boardMAC.replace(/\./g, ',') + "};\n";
        code += "\tserver.begin(\""+ window.location.hostname + "\", mac, \"" +
                $scope.token + "\");\n";

        /* register all sensors */
        for (var sensor in $scope.codeSensors) {
          switch ($scope.codeSensors[sensor].sensorType) {
            case 'DIGITAL_OUTPUT':
              code += "\tserver.registerDigitalOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ");\n";
              break;
            case 'DIGITAL_INPUT':
              code += "\tserver.registerDigitalInput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 10);\n";
              break;
            case 'ANALOG_OUTPUT':
              code += "\tserver.registerAnalogOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ");\n";
              break;
            case 'ANALOG_INPUT':
              code += "\tserver.registerAnalogInput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 10);\n";
              break;
            case 'SERVOMOTOR':
              code += "\tservo" + currentServo + ".attach(" + $scope.codeSensors[s].sensorPin + ");\n";
              code += "\tserver.registerGenericOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 10, setServo" + currentServo + ");\n";
              currentServo += 1;
              break;
            default:
          }
        }

        /* simple main loop */
        code += "\n}\n\nvoid loop() {\n\tserver.loop();\n\tdelay(1000);\n}"
      } else if ($scope.boardType === "OTHERS") {
        /* set editor styling mode */
        $scope.ace.getSession().setMode("ace/mode/python");

        /* add sensors */
        var currentServo = 0;
        for (var sensor in $scope.codeSensors) {
          switch ($scope.codeSensors[sensor].sensorType) {
            case 'DIGITAL_OUTPUT':
              code += "server.registerDigitalOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 150, 10);\n";
              break;
            case 'DIGITAL_INPUT':
              code += "server.registerDigitalInput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 150, 10);\n";
            break;
            case 'ANALOG_OUTPUT':
              code += "server.registerAnalogOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 150, 10);\n";
            break;
            case 'ANALOG_INPUT':
              code += "server.registerAnalogInput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 150, 10);\n";
            break;
            case 'SERVOMOTOR':
              code += "servo" + currentServo + ".attach(" + $scope.codeSensors[s].sensorPin + ");\n"
              code += "server.registerGenericOutput(\"" + sensor + "\", " + $scope.codeSensors[sensor].sensorPin + ", 150, 10, setServo" + currentServo + ");\n";
              currentServo += 1;
              break;
            default:
          }
        }
      }
      
      /* update code in browser *(ace.text = code)* */
      $scope.boardCode = code;
    }
    
    
    $scope.aceOption = {
      onLoad: function (_ace) {
        $scope.ace = _ace; 
        _ace.setReadOnly(true);
    }
  };
 
});

var wyliodrin = {
  onUpdate: null,
};
