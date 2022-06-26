#include "GFX.h"
#include "ST7789.h"
#include <SPI.h>

Arduino_ST7789 tft = Arduino_ST7789(22, 21, -1, -1);

uint8_t  uartRxBuff[1024];
int  rxPos = 0;
int  cmdLength = 0;
uint8_t  cmdType = 0;
long lastRxReceive = 0;

int x = 120, y = 120, oldx, oldy, shift, newkey, oldkey, button, button1;

String deviceType[] = {"UNKNOWN", "POINTER", "MOUSE", "RESERVED", "JOYSTICK", "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM"};
String keyboardstring;
void setup(void) {
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, 18, 19);
  Serial.println("OK There");
  tft.init(240, 240);
  clearscreen();
}

void clearscreen() {
  tft.setCursor(0, 0);
  tft.fillScreen(GREEN);
  tft.setTextColor(BLACK, GREEN);
  tft.setTextWrap(true);
  tft.println("USB Host Example");
  tft.println("");
  tft.println("X:");
  tft.println("");
  tft.println("Y:");
}

void loop() {
  while (Serial2.available())
  {
    lastRxReceive = millis();
    //Serial.print("h0x");//Only for Debug
    //Serial.print(Serial2.peek(),HEX);//Only for Debug
    //Serial.print(" ");//Only for Debug
    uartRxBuff[rxPos] = Serial2.read();
    if (rxPos == 0 && uartRxBuff[rxPos] == 0xFE) {
      cmdType = 1;
    } else if (rxPos == 1 && cmdType == 1) {
      cmdLength = uartRxBuff[rxPos];
    } else if (rxPos == 2 && cmdType == 1) {
      cmdLength += (uartRxBuff[rxPos] << 8);
      //printf( "Length: %i\n", cmdLength);//Only for Debug
    } else if (cmdType == 0 && uartRxBuff[rxPos] == '\n') {
      printf("No COMMAND Received\n");
      for (uint8_t i = 0; i < rxPos; i ++ )
      {
        printf( "0x%02X ", uartRxBuff[i]);
      }
      printf("\n");
      rxPos = 0;
      cmdType = 0;
    }
    if (rxPos > 0 && rxPos == cmdLength + 11 && cmdType || rxPos > 1024) {
      filterCommand(cmdLength, uartRxBuff);
      for (int i = 0; i < rxPos; i ++ )
      {
        //printf( "0x%02X ", uartRxBuff[i]);//Only for Debug
      }
      //printf("\n");//Only for Debug
      rxPos = 0;
      cmdType = 0;
    } else {
      rxPos++;
    }

  }
  rxPos = 0;

  if (Serial.available())
  {
    Serial2.write(Serial.read());
  }
  if (button1) {
    clearscreen();
    button1 = 0;
  }

  if (x != oldx || y != oldy) {
    tft.fillTriangle(oldx, oldy, oldx + 6, oldy + 3, oldx + 3, oldy + 6, GREEN);
    tft.fillTriangle(x, y, x + 6, y + 3, x + 3, y + 6, BLACK);
    if (button)tft.drawLine(oldx - 1, oldy - 1, x - 1, y - 1, BLACK);
    tft.setCursor(16, 16);
    tft.print(x);
    tft.println("  ");
    tft.setCursor(16, 32);
    tft.print(y);
    tft.println("  ");
    oldx = x;
    oldy = y;
  }
  if (newkey != oldkey) {
    if (newkey != 0) {
      if (newkey == 0x2c) {
        keyboardstring += ' ';
      } else if (newkey == 0x36) {
        keyboardstring += ',';
      } else if (newkey == 0x37) {
        keyboardstring += '.';
      } else if (newkey == 0x38) {
        keyboardstring += '-';
      } else if (newkey == 0x30) {
        keyboardstring += '+';
      } else if (newkey == 0x28) {
        keyboardstring += '\n';
      } else if (newkey == 0x2A) {
        keyboardstring = keyboardstring.substring(0, keyboardstring.length() - 1);
      } else
        keyboardstring += char((unsigned char)newkey + (shift ? 61 : 93));
      tft.setCursor(0, 160);
      tft.print(keyboardstring);
      tft.print(" ");
    }
    oldkey = newkey;
  }
}

#define MSG_TYPE_CONNECTED      0x01
#define MSG_TYPE_DISCONNECTED   0x02
#define MSG_TYPE_ERROR          0x03
#define MSG_TYPE_DEVICE_POLL    0x04
#define MSG_TYPE_DEVICE_STRING  0x05
#define MSG_TYPE_DEVICE_INFO    0x06
#define MSG_TYPE_HID_INFO       0x07
#define MSG_TYPE_STARTUP        0x08



void filterCommand(int buffLength, unsigned char *msgbuffer) {
  int cmdLength = buffLength;
  unsigned char msgType = msgbuffer[3];
  unsigned char devType = msgbuffer[4];
  unsigned char device = msgbuffer[5];
  unsigned char endpoint = msgbuffer[6];
  unsigned char idVendorL = msgbuffer[7];
  unsigned char idVendorH = msgbuffer[8];
  unsigned char idProductL = msgbuffer[9];
  unsigned char idProductH = msgbuffer[10];
  switch (msgType) {
    case MSG_TYPE_CONNECTED:
      Serial.print("Device Connected on port");
      Serial.println(device);
      break;
    case MSG_TYPE_DISCONNECTED:
      Serial.print("Device Disconnected on port");
      Serial.println(device);
      break;
    case MSG_TYPE_ERROR:
      Serial.print("Device Error ");
      Serial.print(device);
      Serial.print(" on port ");
      Serial.println(devType);
      break;
    case MSG_TYPE_DEVICE_POLL:
      /*Serial.print("Device HID Data from port: ");
        Serial.print(device);
        Serial.print(" , Length: ");
        Serial.print(cmdLength);
        Serial.print(" , Type: ");
        Serial.print (deviceType[devType]);
        Serial.print(" , ID: ");
        for (int j = 0; j < 4; j++) {
        Serial.print("0x");
        Serial.print(msgbuffer[j + 7], HEX);
        Serial.print(" ");
        }
        Serial.print(" ,  ");
        for (int j = 0; j < cmdLength; j++) {
        Serial.print("0x");
        Serial.print(msgbuffer[j + 11], HEX);
        Serial.print(" ");
        }*/
      if (devType == 2) {
        x += (int16_t)((uint8_t)msgbuffer[11 + 2] + ((uint8_t)msgbuffer[11 + 3] << 8));
        y += (int16_t)((uint8_t)msgbuffer[11 + 4] + ((uint8_t)msgbuffer[11 + 5] << 8));
        if (x > 240)x = 240;
        if (x < 0)x = 0;
        if (y > 240)y = 240;
        if (y < 0)y = 0;
        if (msgbuffer[11 + 1] & 1) {
          button = 1;
        } else button = 0;
        if (msgbuffer[11 + 1] & 2) {
          button1 = 1;
        } else button1 = 0;
      }
      if (devType == 6) {
        if (msgbuffer[11 + 0] == 2)shift = 1; else shift = 0;
        newkey = msgbuffer[11 + 2];
      }
      //Serial.println();
      //Serial.println("X= " + String(x) + " Y= " + String(y));
      break;
    case MSG_TYPE_DEVICE_STRING:
      Serial.print("Device String port ");
      Serial.print(devType);
      Serial.print(" Name: ");
      for (int j = 0; j < cmdLength; j++)
        Serial.write(msgbuffer[j + 11]);
      Serial.println();
      break;
    case MSG_TYPE_DEVICE_INFO:
      Serial.print("Device info from port");
      Serial.print(device);
      Serial.print(", Descriptor: ");
      for (int j = 0; j < cmdLength; j++) {
        Serial.print("0x");
        Serial.print(msgbuffer[j + 11], HEX);
        Serial.print(" ");
      }
      Serial.println();
      break;
    case MSG_TYPE_HID_INFO:
      Serial.print("HID info from port");
      Serial.print(device);
      Serial.print(", Descriptor: ");
      for (int j = 0; j < cmdLength; j++) {
        Serial.print("0x");
        Serial.print(msgbuffer[j + 11], HEX);
        Serial.print(" ");
      }
      Serial.println();
      break;
    case MSG_TYPE_STARTUP:
      Serial.println("USB host ready");
      break;

  }
}
