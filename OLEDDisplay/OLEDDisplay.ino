#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'profile.png'
 */

const unsigned char profile[] PROGMEM = {
  0x00,0x78,0x00,
  0x03,0x87,0x00,
  0x0C,0x00,0xC0,
  0x10,0x00,0x20,
  0x20,0x30,0x10,
  0x20,0xCC,0x10,
  0x41,0x02,0x08,
  0x41,0x02,0x08,
  0x42,0x01,0x08,
  0x82,0x01,0x04,
  0x81,0x02,0x04,
  0x81,0x02,0x04,
  0x80,0xCC,0x04,
  0x40,0x78,0x08,
  0x41,0x86,0x08,
  0x42,0x01,0x08,
  0x24,0x00,0x90,
  0x2C,0x00,0xD0,
  0x18,0x00,0x60,
  0x0C,0x00,0xC0,
  0x03,0x87,0x00,
  0x00,0x78,0x00
};




void setup()
{
    Serial.begin(9600);

    // initialize the OLED object
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Clear the buffer.
    display.clearDisplay();
    display.drawBitmap(8, 21, profile, 22, 22, WHITE);
    display.setTextColor(WHITE);
    display.setCursor(36, 10);
    display.setTextSize(1);
    display.print(limitString("sadfasdfdfdsafsdfa", 14));
    display.setCursor(36, 34);
    display.setTextSize(2);
    display.print(limitString("3200203", 7));
    display.display();    
}

void loop()
{

}

String limitString(String input, int maxLength)
{
    String result = input;
    if (result.length() > maxLength)
    {
        result = result.substring(0, maxLength); // Truncate the string to the maximum length
        if (maxLength >= 2)
        {
            result.remove(maxLength - 2); // Remove the last two characters
            result += ".."; // Append ".." to the string
        }
    }
    return result;
}