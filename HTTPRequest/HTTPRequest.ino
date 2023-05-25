#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Soul Blade";
const char *password = "12345678";

String serverIPAddress = "192.168.101.77";
String serverPort = "8080";
String apiPath = "http://" + serverIPAddress + ":" + serverPort + "/api";

String studentPath = apiPath + "/student/f3f5d1d9";
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

void setup()
{
    Serial.begin(115200);

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
    // Send an HTTP POST request every 10 minutes
    if ((millis() - lastTime) > timerDelay)
    {
        sendHTTPGet();
        lastTime = millis();
    }
}

void sendHTTPGet()
{
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(studentPath.c_str());

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);
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