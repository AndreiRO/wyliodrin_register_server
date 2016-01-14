Registration Server
===================

Author Andrei Ștefănescu
Wyliodrin SRL


Description
-----------

Project is to be used by Wyliodrin SRL within their architecture to enable users to more easily create IoT projects.
This project enables the user to quickly create IoT projects just
by selecting the type of wanted sensors and uploading the generated
code to the respective board. The values can be seen and modified for
each sensor. Also, the user can define rules using Google Blockly, rules
applied at each new sensor value changed.

The project is compatible with all Wyliodrin supported boards + Arduino Uno
and Arduino Yun.




Structure
=========

1. Arduino

There are two files: RegisterServer.h, RegisterServer.cpp which represent the implementation for both Arduino Uno and Arduino Yun. For
using please see the examples folder.

2. Python

The implementation is in register_server.py. It runs on all boards that are currently have libmraa ported on them and the Wyliodrin library. See examples folder.

3. IoTServer

The folder contains the fowllowing:

* web_content
The actual content that has to be put on the IoTServer project.

* nodes.xml
The exported form of the Streams nodes.

* apidoc.json
Generate documentation for RegistrationServer API. Use command:
`apidoc -i doc_src -o doc`

* doc_src
The source for the API documentation

* doc
The output of apidoc. Use `google-chrome doc/index.html` to view the documentation.


Possible "upgrades"
--------------------

1. Remove threads in Python implementation.

License
========

The license for this project is LGPL. See [this link](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html) for full license text.