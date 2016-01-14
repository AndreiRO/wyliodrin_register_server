/**
 * @api {post} /get
 * @apiGroup Retrieve
 * @apiName Retrieve sensor value
 * @apiDescription Retrieve a sensor value. All parameters are put in a JSON Document or application/x-www-form-urlencoded.
 *
 * 
 * @apiParam {String} id The name of the sensor.
 * @apiParam {String} token User's security token.
 * @apiParam {int} plain Send a full JSON document or just the value.
 *
 * @apiSuccess {String} id The name of the sensor. Only if plain!=1.
 * @apiSuccess {float} value The value of the sensor.
 * @apiSuccess {int} error=0 The error field will be 0. Only if plain!=1.
 * @apiSuccess {String} reason Empty string. Only if plain!=1.
 *
 *
 * @apiError {int} error A code different from 0. Only if plain!=1.
 * @apiError {String} reason The reason for the error. Only if plain != 1.
 * 
 */
function get() {}

/**
 * @api {post} /send
 * @apiName Send sensor value
 * @apiGroup Send
 * @apiDescription Send a sensor value. All parameters are put in a JSON Document or application/x-www-form-urlencoded.
 *
 * 
 * @apiParam {String} id The name of the sensor.
 * @apiParam {String} token User's security token.
 * @apiParam {float} value Sensor's value.
 *
 * @apiSuccess {String} id The name of the sensor. Only if plain!=1.
 * @apiSuccess {float} value The value of the sensor.
 * @apiSuccess {int} error=0 The error field will be 0. Only if plain!=1.
 * @apiSuccess {String} reason Empty string. Only if plain!=1.
 *
 *
 * @apiError {int} error A code different from 0. Only if plain!=1.
 * @apiError {String} reason The reason for the error. Only if plain != 1.
 * 
 */
function send() {}

 /**
  * @api {post} /register
  * @apiGroup Register
  * @apiName Register Sensor
  * @apiDescription Register a new sensor. All parameters are put in a JSON Document or application/x-www-form-urlencoded. Multiple calls with same id override.
  *
  *
  * @apiParam {String} id The name of the sensor.
  * @apiParam {String} token User's security token.
  * @apiParam {String} value Sensor's value.
  * @apiParam {String} type The sensor's type: "DIGITAL_INPUT", "DIGITAL_OUTPUT", "PWM_OUTPUT", "ANALOG_INPUT", "GENERIC_INPUT", "GENERIC_OUTPUT".
  *
  * @apiSuccess {int} error The value 0.
  * @apiSuccess {String} reason Empty string.
  * @apiSuccess {String} id The sensor's id.
  * @apiSuccess {String} type The sensor's type.
  *
  * @apiError {int} error A value different than 1.
  * @apiError {String} reason The reason behind the error.
  *
  */
function register() {}
/**
 * @api {post} /code
 * @apiName Change log
 * @apiGroup Code
 * @apiDescription Send the new logic code that comes from Google Blockly. All parameters are put in a JSON Document or application/x-www-form-urlencoded.
 *
 * @apiParam {String} token User's security token.
 * @apiParam {String} code The new Javascript code.
 */
function code() {}



