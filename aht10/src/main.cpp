#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDRESS    0x38
#define STATE_BUSY     0x80
#define STATE_MODE_CYC 0x20
// note that can be 0x40 or 0x41 but checking with & should be enough
#define STATE_MODE_CMD 0x40
#define STATE_CALIBRATED 0x08

void setup()
{
    Wire.begin();
    Serial.begin(9600);
}

// table 9 from datasheet provides the commands definition
void sensorInit()
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xBE);
    Wire.write(0x08);
    Wire.write(0x00);
    Wire.endTransmission();
}

void sensorReset()
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xBA);
    Wire.endTransmission();
}

// details in 5.3 of the datasheet
void sensorTriggerMeasurement()
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xAC);
    Wire.write(0x33);
    Wire.write(0x00);
    Wire.endTransmission();
}

uint8_t sensorReadStatus()
{
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x71);
    Wire.requestFrom(I2C_ADDRESS, 1);
    while (!Wire.available());
    uint8_t status = (uint8_t)Wire.read();
    Wire.endTransmission();
    return status;
}

void readSensorData(uint8_t *state, float *humidity, float *temperature)
{
    uint32_t uint_humidity;
    uint32_t uint_temperature;
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.requestFrom(I2C_ADDRESS, 6);

    // there are two values of 20 bits and a status byte
    // read per byte
    if (Wire.available() >= 6) {
        uint8_t tmp;
        *state = (uint8_t)Wire.read();

        // read humidity (20 bits)
        uint_humidity = ((uint32_t)Wire.read() << 12) | ((uint32_t)Wire.read() << 4);

        // read temporary byte that have low 4 bits of humidity and high 4 bits of temperature
        tmp = (uint8_t)Wire.read();
        uint_humidity |= (uint32_t)(tmp >> 4);

        // read temperature (20 bits)
        uint_temperature = ((uint32_t)(tmp & 0xF) << 16) | ((uint32_t)Wire.read() << 8) | ((uint32_t)Wire.read());

        // convert into float values, section 6 of the datasheet
        *humidity = ((float)uint_humidity * 100.0f) / 1048576.0f;
        *temperature = ((float)uint_temperature * 200.0f) / 1048576.0f - 50.0f;
    } else {
        Serial.println("Error reading sensor data");
        *state = STATE_BUSY;
        *humidity = 0.0f;
        *temperature = 0.0f;
    }
    Wire.endTransmission();
}

void loop()
{
    Serial.println("Waiting for sensor to warmup");
    delay(40); // wait before checking status

    // checking if calibration is done
    uint8_t status = sensorReadStatus();
    Serial.print("Calibration state: 0x");
    Serial.println(status, HEX);
    if ((status & STATE_CALIBRATED) == 0) {
        Serial.println("Sensor not calibrated, resetting");

        while(1) {
            sensorInit();
            delay(40); // wait for sensor to initialize

            // check again
            status = sensorReadStatus();
            Serial.print("Calibration state: 0x");
            Serial.println(status, HEX);
            if (status & STATE_CALIBRATED) {
                Serial.println("Sensor calibrated");
                break;
            } else {
                Serial.println("Sensor still not calibrated, retrying");
            }
        }
    }

    while(true) {
        float humidity;
        float temperature;

        // ask for a measurement
        Serial.println("Triggering Measurement");
        sensorTriggerMeasurement();
        delay(80); // wait for measurement to complete

        readSensorData(&status, &humidity, &temperature);

        // check if still busy
        while(status & STATE_BUSY) {
            Serial.println("Sensor is busy, waiting for finish");
            delay(10);
            readSensorData(&status, &humidity, &temperature);
        }

        Serial.print("State: 0x");
        Serial.print(status, HEX);
        Serial.print(" Humidity: ");
        Serial.print(humidity);
        Serial.print("% Temperature: ");
        Serial.print(temperature);
        Serial.println(" C");

        delay(1000); // wait before next reading
    }
}