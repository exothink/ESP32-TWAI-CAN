/*
C:\Users\EliteDesk\Documents\PlatformIO\Projects\TWAI\ESP32-TWAI-CAN\src\main.cpp  9/19/24

https://github.com/handmade0octopus/ESP32-TWAI-CAN/blob/master/examples/OBD2-querry/OBD2-querry.ino

Uses a very small CAN driver from Espirif. The one used in gdDash dosn't work with the 'S' variant.
This one should work with them all.

This example alternates sending low/high rpm and mph. It also listens for adr 0x0000F000.

PLATFORM: Espressif 32 (6.7.0) > Espressif ESP32-S3-DevKitC-1-N16R8V (16 MB QD, 8MB PSRAM)
HARDWARE: ESP32S3 240MHz, 320KB RAM, 16MB Flash

WORKS                                                                              9/24
RAM:   [=         ]   5.9% (used 19220 bytes from 327680 bytes)
Flash: [          ]   4.4% (used 288481 bytes from 6553600 bytes)

union MSG {
  uint8_t bytes[8] = {0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff};
  int16_t int16[4];
  int32_t int32[2];
  int64_t int64;
}; 
This doesn't work on the esp32. It does on the stm32. It is now considered bad practice. 
Termed 'type-punning'. I'm now using memcpy instead.

*/
#include <ESP32-TWAI-CAN.hpp>

// Showcasing simple use of ESP32-TWAI-CAN library driver.
#define CANBR TWAI_SPEED_250KBPS

#define CAN_TX 16 // org wire
#define CAN_RX 17 // brn wire

#define FNAME "ESP32-TWAI-CAN "
#define VERS "v092524"

#define Serial Serial0 // 'ESP32 S' version sends Serial to left (usb) USB connector. Serial0 to the right.

 CanFrame rxFrame;
 CanFrame txFrame = {0};

void initTxFrame()
{
    // CanFrame txFrame = {0};
    txFrame.identifier = 0x0b; // OBD2 address for gdDash rpm/mph;
    txFrame.extd = 0;
    txFrame.data_length_code = 3; // room for one int and a byte
    
    txFrame.data[0] = 0x01; // rpm0 lsb
    txFrame.data[1] = 0x02; // rpm1 msb
    txFrame.data[2] = 0x03; // mph
}

void setup()
{
    // Setup serial for debbuging.
    Serial.begin(115200);
    Serial.print("\n\n");
    Serial.print(FNAME);
    Serial.println(VERS);

    uint8_t speed = 250; // TWAI_SPEED_250KBPS

    // if (ESP32Can.begin(ESP32Can.convertSpeed(speed), CAN_TX, CAN_RX, 10, 10)) // TWAI_SPEED_250KBPS
    if (ESP32Can.begin(CANBR, CAN_TX, CAN_RX, 10, 10)) // TWAI_SPEED_250KBPS
        Serial.println("CAN bus started!");                                   // if (ESP32Can.begin(TWAI_SPEED_250KBPS, CAN_TX, CAN_RX, 10, 10))  // TWAI_SPEED_250KBPS
    else
        Serial.println("CAN bus failed!");

    initTxFrame();
}

void loop()
{
    u_int16_t rpm = 4000; // 0x0fa0
    u_int8_t mph = 55;
    memcpy(&txFrame.data[0], &rpm, sizeof(rpm) );  // copies a 2byte int to a byte array locations 0 and 1.
    txFrame.data[2] = mph;
    ESP32Can.writeFrame(txFrame, 100);
    Serial.printf("%3d mph  %4d rpm\n", mph, rpm);
    delay(1000);

    rpm = 900; // 0x384
    mph = 6;
    memcpy(&txFrame.data[0], &rpm, sizeof(rpm) );
    txFrame.data[2] = mph;
    ESP32Can.writeFrame(txFrame, 100);
    Serial.printf("%3d mph  %4d rpm\n", mph, rpm);

    if (ESP32Can.readFrame(rxFrame, 1000))
    {
        Serial.printf("Received frame: 0x%03X  \r\n", rxFrame.identifier);
        if (rxFrame.identifier == 0xF000l)  // note the 'l' suffix
            Serial.printf("0xf000 is a cmd for nodes to send current measurements\n");
    }
    delay(1000);
}