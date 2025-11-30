# AHT10 / AHT20 Temperature and Humidity Sensor

Datasheet: https://cdn.sparkfun.com/assets/d/2/b/e/d/AHT20.pdf

This sensor is a 20 bit temperature and humidity I2C sensor.
It has hardcoded a `0x38` slave address.
It have 4 write commands:
* **Init** `0xBE` followed by `0x08` and `0x00`
* **Get Status** `0x71` followed by read a single byte
* **Trigger Measurement** `0xAC` followed by `0x08` and `0x00`
* **Reset** `0xBA` (no more data)

If sensor is read, it will return 6 bytes (48 bits) which consist of:
* 8 bits for state
* 20 bits for humidity
* 20 bits for temperature

The third byte contains a nibble for temperature and other for humidy.

The formulas are provided in the datasheet (humidity is a percent) and degrees
are in celsius.

The sensor have some sequence that needs to be followed (Section 5.4
of datasheet) but in general is very simple as long as is already calibrated.

The flow is just to trigger a measurement (`sensorTriggerMeasurement`)
wait for 80ms (75ms is the datasheet specification) and later read the
sensor data (`readSensorData`) and check if state is not busy, and convert
the humidity and temperature using the formulas.

The code will output:
```
Triggering Measurement
State: 0x18 Humidity: 52.54% Temperature: 26.14 C
Triggering Measurement
State: 0x18 Humidity: 52.52% Temperature: 26.15 C
```
