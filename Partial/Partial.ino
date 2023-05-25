#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

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

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
// unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

// Constants
#define SS_PIN 5
#define RST_PIN 2
// Parameters
const int ipaddress[4] = {103, 97, 67, 25};
// Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

void setup()
{
    // Init Serial USB
    Serial.begin(115200);
    Serial.println(F("Initialize System"));

    // initialize rfid D8,D5,D6,D7
    SPI.begin();
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();
    WiFi.begin(ssid, password);

    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop()
{
    String rfid = "";
    rfid = readRFID();
    if (rfid != "")
    {
        Serial.println(rfid);
        sendHTTPGet(rfid);
    }
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
    Serial.print(F("RFID In dec: "));
    rfidValue = arrByteToHexString(rfid.uid.uidByte, rfid.uid.size);

    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return rfidValue;
}

String arrByteToHexString(byte *buffer, byte bufferSize)
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

void sendHTTPGet(String rfid)
{
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String newPath = studentPath + "/" + rfid;
        http.begin(newPath.c_str());

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);
            Serial.println(valueFromJSON(payload, "first_name"));
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
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