define({ "api": [
  {
    "type": "post",
    "url": "/code",
    "title": "",
    "name": "Change_log",
    "group": "Code",
    "description": "<p>Send the new logic code that comes from Google Blockly. All parameters are put in a JSON Document or application/x-www-form-urlencoded.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "token",
            "description": "<p>User's security token.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "code",
            "description": "<p>The new Javascript code.</p>"
          }
        ]
      }
    },
    "version": "0.0.0",
    "filename": "doc_src/doc.js",
    "groupTitle": "Code"
  },
  {
    "type": "post",
    "url": "/register",
    "title": "",
    "group": "Register",
    "name": "Register_Sensor",
    "description": "<p>Register a new sensor. All parameters are put in a JSON Document or application/x-www-form-urlencoded. Multiple calls with same id override.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The name of the sensor.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "token",
            "description": "<p>User's security token.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "value",
            "description": "<p>Sensor's value.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "type",
            "description": "<p>The sensor's type: &quot;DIGITAL_INPUT&quot;, &quot;DIGITAL_OUTPUT&quot;, &quot;PWM_OUTPUT&quot;, &quot;ANALOG_INPUT&quot;, &quot;GENERIC_INPUT&quot;, &quot;GENERIC_OUTPUT&quot;.</p>"
          }
        ]
      }
    },
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "int",
            "optional": false,
            "field": "error",
            "description": "<p>The value 0.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>Empty string.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The sensor's id.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "type",
            "description": "<p>The sensor's type.</p>"
          }
        ]
      }
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "type": "int",
            "optional": false,
            "field": "error",
            "description": "<p>A value different than 1.</p>"
          },
          {
            "group": "Error 4xx",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>The reason behind the error.</p>"
          }
        ]
      }
    },
    "version": "0.0.0",
    "filename": "doc_src/doc.js",
    "groupTitle": "Register"
  },
  {
    "type": "post",
    "url": "/get",
    "title": "",
    "group": "Retrieve",
    "name": "Retrieve_sensor_value",
    "description": "<p>Retrieve a sensor value. All parameters are put in a JSON Document or application/x-www-form-urlencoded.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The name of the sensor.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "token",
            "description": "<p>User's security token.</p>"
          },
          {
            "group": "Parameter",
            "type": "int",
            "optional": false,
            "field": "plain",
            "description": "<p>Send a full JSON document or just the value.</p>"
          }
        ]
      }
    },
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The name of the sensor. Only if plain!=1.</p>"
          },
          {
            "group": "Success 200",
            "type": "float",
            "optional": false,
            "field": "value",
            "description": "<p>The value of the sensor.</p>"
          },
          {
            "group": "Success 200",
            "type": "int",
            "optional": false,
            "field": "error",
            "defaultValue": "0",
            "description": "<p>The error field will be 0. Only if plain!=1.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>Empty string. Only if plain!=1.</p>"
          }
        ]
      }
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "type": "int",
            "optional": false,
            "field": "error",
            "description": "<p>A code different from 0. Only if plain!=1.</p>"
          },
          {
            "group": "Error 4xx",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>The reason for the error. Only if plain != 1.</p>"
          }
        ]
      }
    },
    "version": "0.0.0",
    "filename": "doc_src/doc.js",
    "groupTitle": "Retrieve"
  },
  {
    "type": "post",
    "url": "/send",
    "title": "",
    "name": "Send_sensor_value",
    "group": "Send",
    "description": "<p>Send a sensor value. All parameters are put in a JSON Document or application/x-www-form-urlencoded.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The name of the sensor.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "token",
            "description": "<p>User's security token.</p>"
          },
          {
            "group": "Parameter",
            "type": "float",
            "optional": false,
            "field": "value",
            "description": "<p>Sensor's value.</p>"
          }
        ]
      }
    },
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "id",
            "description": "<p>The name of the sensor. Only if plain!=1.</p>"
          },
          {
            "group": "Success 200",
            "type": "float",
            "optional": false,
            "field": "value",
            "description": "<p>The value of the sensor.</p>"
          },
          {
            "group": "Success 200",
            "type": "int",
            "optional": false,
            "field": "error",
            "defaultValue": "0",
            "description": "<p>The error field will be 0. Only if plain!=1.</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>Empty string. Only if plain!=1.</p>"
          }
        ]
      }
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "type": "int",
            "optional": false,
            "field": "error",
            "description": "<p>A code different from 0. Only if plain!=1.</p>"
          },
          {
            "group": "Error 4xx",
            "type": "String",
            "optional": false,
            "field": "reason",
            "description": "<p>The reason for the error. Only if plain != 1.</p>"
          }
        ]
      }
    },
    "version": "0.0.0",
    "filename": "doc_src/doc.js",
    "groupTitle": "Send"
  }
] });
