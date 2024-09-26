#include "I2Cdev.h"
#include "MPU6050.h"
#include "DFRobotDFPlayerMini.h"
#include <SoftwareSerial.h>

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#define TURN_ACCEL 6000   // Something?
#define TURN_DURATION 12  // Deciseconds.

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
SoftwareSerial softSerial(12, 14);

// https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299
DFRobotDFPlayerMini myDFPlayer;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

int counter = 0;

void setup() {
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif
    Serial.begin(115200);
    softSerial.begin(9600);

    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    if (!myDFPlayer.begin(softSerial, /*isACK = */true, /*doReset = */true)) {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        ESP.restart();
    }
}

void loop() {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

#ifdef OUTPUT_READABLE_ACCELGYRO
    Serial.print("a/g:\t");
    Serial.print(ax);
    Serial.print("\t");
    Serial.print(ay);
    Serial.print("\t");
    Serial.print(az);
    Serial.print("\t");
    Serial.print(gx);
    Serial.print("\t");
    Serial.print(gy);
    Serial.print("\t");
    Serial.println(gz);
#endif

#ifdef OUTPUT_BINARY_ACCELGYRO
    Serial.write((uint8_t)(ax >> 8));
    Serial.write((uint8_t)(ax & 0xFF));
    Serial.write((uint8_t)(ay >> 8));
    Serial.write((uint8_t)(ay & 0xFF));
    Serial.write((uint8_t)(az >> 8));
    Serial.write((uint8_t)(az & 0xFF));
    Serial.write((uint8_t)(gx >> 8));
    Serial.write((uint8_t)(gx & 0xFF));
    Serial.write((uint8_t)(gy >> 8));
    Serial.write((uint8_t)(gy & 0xFF));
    Serial.write((uint8_t)(gz >> 8));
    Serial.write((uint8_t)(gz & 0xFF));
#endif

    delay(100);
    if (abs(ax) > TURN_ACCEL) {
        if (counter == TURN_DURATION) {
            Serial.println("Playing audio...");
            myDFPlayer.volume(30);
            myDFPlayer.play(1);
        }
        // Keep incrementing the counter, so we don't keep triggering on a long turn.
        counter++;
    } else {
        counter = 0;
    }
}
