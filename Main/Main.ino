/* HTTP REQUEST LIBRARIES */
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* RFID READER LIBRARIES */
#include <SPI.h>
#include <MFRC522.h>

/* OLED DISPLAY LIBRARIES */
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* HTTP REQUEST CONFIGURATION */
const char *ssid = "Soul Blade";
const char *password = "12345678";

String serverIPAddress = "192.168.101.77";
String serverPort = "8080";
String apiPath = "http://" + serverIPAddress + ":" + serverPort + "/api";

String studentPath = apiPath + "/student";
String instructorPath = apiPath + "/instructor";
String buildingPath = apiPath + "/building";
String keyPath = apiPath + "/key";
String roomPath = apiPath + "/room";
String recordPath = apiPath + "/record";

HTTPClient http;
int responseCode;

/* RFID CONFIGURATION */
#define SS_PIN 5
#define RST_PIN 2
const int ipaddress[4] = {103, 97, 67, 25};
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

/* OLED DISPLAY CONFIGURATION */
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* UTITLITIES */
const int readingKeyDuration = 10000; // in milliseconds
unsigned long previousReadingKeyTime = 0;
StaticJsonDocument<200> doc;

void setup()
{
    Serial.begin(115200);

    /* WIFI INITIALIZATION */
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    /* RFID INITIALIZATION */
    SPI.begin();
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();

    /* OLED DISPLAY INITIALIZATION */
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
}

void loop()
{
    String rfid = "";
    String studentName = "";
    String studentSchoolId = "";

    while (true)
    {
        // display rfid symbol animation
        printToDisplay("RFID SYMBOL ANIMATION NI SIYA...");

        rfid = readRFID();
        if (rfid == "")
            continue;

        Serial.print("RFID hex: ");
        Serial.println(rfid);

        String responseBody = getStudent(rfid);

        bool skipKeyHandling = false;

        if (responseCode == 200)
        {
            String first_name = valueFromJSON(responseBody, "first_name");
            String last_name = valueFromJSON(responseBody, "last_name");
            studentName = last_name + ", " + first_name;
            String studentSchoolId = valueFromJSON(responseBody, "school_id");

            String textToDisplay = String(studentName) + "\n" + String(studentSchoolId);
            printToDisplay(textToDisplay);

            previousReadingKeyTime = millis();
            Serial.println("waiting for rfid tag");

            while (millis() - previousReadingKeyTime <= readingKeyDuration)
            {
                rfid = readRFID();
                if (rfid == "")
                    continue;

                Serial.print("RFID hex: ");
                Serial.println(rfid);

                String keyResponseBody = getKey(rfid);
                Serial.println(keyResponseBody);

                if (responseCode != 200)
                {
                    printToDisplay("Invalid Key Tag.");
                    delay(2000);
                    skipKeyHandling = true;
                    break;
                }

                Serial.println("Key found.");

                if (valueFromJSON(keyResponseBody, "status") == "available")
                {
                    Serial.println("Borrowing key");
                    borrowKey(rfid, studentSchoolId);
                    printToDisplay("Key Borrowed");
                    delay(2000);
                    Serial.println("Key borrowed");
                    skipKeyHandling = true;
                    break;
                }

                if (valueFromJSON(keyResponseBody, "status") == "borrowed")
                {
                    Serial.println("Returning key");
                    returnKey(rfid, studentSchoolId);
                    printToDisplay("Key Returned");
                    delay(2000);
                    Serial.println("Key returned");
                    skipKeyHandling = true;
                    break;
                }
            }
        }

        if (skipKeyHandling)
            continue;

        Serial.println("Student RFID can't be found.");

        printToDisplay("RFID SYMBOL ANIMATION NI SIYA...");

        String keyResponseBody = getKey(rfid);

        if (responseCode != 200)
        {
            Serial.println("Key RFID can't be found.");
            printToDisplay("Invalid Tag");
            delay(2000);
            continue;
        }

        if (valueFromJSON(keyResponseBody, "status") == "available")
        {
            printToDisplay("Cannot borrow, School ID required");
            delay(2000);
            continue;
        }

        if (valueFromJSON(keyResponseBody, "status") == "borrowed")
        {
            returnKey(rfid, "");
            printToDisplay("Key Returned");
            delay(2000);
            continue;
        }
    }
}

String sendHTTPGETRequest(String requestPath)
{
    String responseBody = "";

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot send get request, WiFi Disconnected.");
        return "";
    }

    http.begin(requestPath.c_str());

    responseCode = http.GET();

    if (responseCode <= 0)
    {
        Serial.print("HTTP Error code: ");
        Serial.println(responseCode);
        return "";
    }

    Serial.print("HTTP Response code: ");
    Serial.println(responseCode);
    responseBody = http.getString();

    http.end();

    return responseBody;
}

String sendHTTPPOSTRequest(String requestPath, String postData)
{
    String responseBody = "";

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot send post request, WiFi Disconnected.");
        return "";
    }

    http.begin(requestPath.c_str());
    http.addHeader("Content-Type", "application/json");
    responseCode = http.POST(postData);

    if (responseCode <= 0)
    {
        Serial.print("HTTP Error code: ");
        Serial.println(responseCode);
        return "";
    }

    Serial.print("HTTP Response code: ");
    Serial.println(responseCode);
    responseBody = http.getString();
    Serial.println(responseBody);

    http.end();

    return responseBody;
}

String readRFID()
{
    String rfidValue = "";
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }
    // Look for new 1 cards
    if (!rfid.PICC_IsNewCardPresent())
        return "";
    // Verify if the NUID has been readed
    if (!rfid.PICC_ReadCardSerial())
        return "";
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
        nuidPICC[i] = rfid.uid.uidByte[i];
    }
    rfidValue = byteArrToHexString(rfid.uid.uidByte, rfid.uid.size);

    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return rfidValue;
}

String byteArrToHexString(byte *buffer, byte bufferSize)
{
    String hexString = "";

    for (byte i = 0; i < bufferSize; i++)
    {
        if (buffer[i] < 0x10)
        {
            hexString += '0';
            continue;
        }

        hexString += String(buffer[i], HEX);
    }

    return hexString;
}

String getStudent(String rfid)
{
    String requestPath = studentPath + "/" + rfid;
    return sendHTTPGETRequest(requestPath);
}

String getKey(String rfid)
{
    String requestPath = keyPath + "/rfid/" + rfid;
    return sendHTTPGETRequest(requestPath);
}

String borrowKey(String keyRfid, String studentSchoolId)
{
    String postData = "{\"type\": \"borrow\", \"school_id\": \"" + studentSchoolId + "\", \"rfid\": \"" + keyRfid + "\"}";

    return sendHTTPPOSTRequest(recordPath, postData);
}

String returnKey(String keyRfid, String studentSchoolId)
{
    String postData = "{\"type\": \"return\", \"school_id\": \"" + studentSchoolId + "\", \"rfid\": \"" + keyRfid + "\"}";
    return sendHTTPPOSTRequest(recordPath, postData);
}

void deserializeResponseBody(StaticJsonDocument<200> *doc, String responseBody)
{
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(*doc, responseBody);

    // Test if parsing succeeds.
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
}

void printToDisplay(String text)
{
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(text);
    display.display();
}

String valueFromJSON(String json, String key)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);

    // Test if parsing succeeds.
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return "";
    }

    const char *value = doc["data"][key];
    return String(value);
}