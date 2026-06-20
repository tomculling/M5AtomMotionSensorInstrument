// By Tom Culling - 2026
// This code was partly adapted from example code which creates a bridge between Serial and Classical Bluetooth (SPP).
// This example code was written by Evandro Copercini in 2018, and is in the Public Domain. 
// Note: Pairing is authenticated automatically by the M5 Atom.

#include "BluetoothSerial.h"
#include "M5Atom.h"


String device_name = "LightyUppyMan";

CRGB led(0, 0, 0);
double roll, pitch;  // Stores pitch and roll variables
float accX, accY, accZ; // Stores accelarometer variables
double rad2deg = 180.0 / PI; 
bool buttonPressed = false; // true while button is pressed

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("LightyUppyMan");  //Bluetooth device name
  //SerialBT.deleteAllBondedDevices(); // Uncomment this to delete paired devices; Must be called after begin
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());

  M5.begin(true, true,
             true);  // Init Atom-Matrix(Initialize serial port, LED matrix).                    
    M5.IMU.Init();   // Init IMU sensor.  
}

CRGB HSVtoRGB(
    uint16_t h, uint16_t s,
    uint16_t
        v) {  // Adjust the color of Atom-Matrix LED Matrix according to posture
    CRGB ReRGB(0, 0, 0);
    int i;
    float RGB_min, RGB_max;
    RGB_max = v * 2.55f;
    RGB_min = RGB_max * (100 - s) / 100.0f;

    i             = h / 60;
    int difs      = h % 60;
    float RGB_Adj = (RGB_max - RGB_min) * difs / 60.0f;

    switch (i) {
        case 0:
            ReRGB.r = RGB_max;
            ReRGB.g = RGB_min + RGB_Adj;
            ReRGB.b = RGB_min;
            break;
        case 1:
            ReRGB.r = RGB_max - RGB_Adj;
            ReRGB.g = RGB_max;
            ReRGB.b = RGB_min;
            break;
        case 2:
            ReRGB.r = RGB_min;
            ReRGB.g = RGB_max;
            ReRGB.b = RGB_min + RGB_Adj;
            break;
        case 3:
            ReRGB.r = RGB_min;
            ReRGB.g = RGB_max - RGB_Adj;
            ReRGB.b = RGB_max;
            break;
        case 4:
            ReRGB.r = RGB_min + RGB_Adj;
            ReRGB.g = RGB_min;
            ReRGB.b = RGB_max;
            break;
        default:  // case 5:
            ReRGB.r = RGB_max;
            ReRGB.g = RGB_min;
            ReRGB.b = RGB_max - RGB_Adj;
            break;
    }
    return ReRGB;
}

void loop() {
    M5.update();
    M5.IMU.getAttitude(&roll,
                       &pitch);  // Read the attitude (pitch, roll) of the IMU
                                 // and store it in relevant variables.
    double TiltAngle = atan2(pitch, roll) * rad2deg + 180; // calculate angle between pitch/roll vectors (the direction of tilt 2D space)
    double TiltMag = sqrt(pitch * pitch + roll * roll); // calculate magnitude of tilt

    TiltMag = (TiltMag * 6) > 100 ? 100 : TiltMag * 6; // scale TiltMag 0-100 and cap at 100
    led = HSVtoRGB(TiltAngle, TiltMag, 100); 
    M5.dis.fillpix(
        led); 

    M5.IMU.getAccelData(&accX, &accY, &accZ); // Read accelarometer data
    buttonPressed = M5.Btn.isPressed(); // 1 while button is pressed, else 0

      
    Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%d\n",
           roll, pitch, accX, accY, accZ, buttonPressed ? 1 : 0); // serial port output the formatted string over USB.

    SerialBT.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%d\n",
           roll, pitch, accX, accY, accZ, buttonPressed ? 1 : 0); // serial port output the formatted string over BT.
    delay(20);
}