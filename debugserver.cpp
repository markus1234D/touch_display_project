#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
// #elif defined(ESP8266)
// #include <ESP8266WiFi.h>
// #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "SPI.h"
#include "TFT_eSPI.h"

const char* webtest_html = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
    <title>Web Servo</title>
    <style>
        body {
            text-align: center;
            background-color: rgb(50, 0, 56);
            color: white;
        }
    </style>
</head>
<body>
    <input type="range" min="0" max="100" value="0" size="500" class="slider" id="slider1">
    <p id="slider1_output">0</p>
    <button type="button" id="btn1">fetch</button>    
    <p id="real_slider1_output">X</p>
<script>
    var last_slider1_value = 0;
    var slider1 = document.getElementById("slider1");
    const slider1_output = document.getElementById("slider1_output");
    const btn1 = document.getElementById("btn1");
    const real_slider1_output = document.getElementById("real_slider1_output");

    slider1.addEventListener("input", function() {
        setTimeout(() => {
            slider1_output.textContent = slider1.value;
            if (slider1.value != last_slider1_value) 
            {
                last_slider1_value = slider1.value;
                fetch_command_and_output("/get?num=" + slider1.value, real_slider1_output);
            }
        }, 100
    );
    });

    btn1.addEventListener("click", function() {
        fetch_command_and_output("/get?num=" + slider1.value, real_slider1_output);
    });

    async function fetch_command_and_output(command, p_output) {
        console.log("fetch_command_and_output: " + command);
        fetch(command)
        .then((response) => {
            console.log(`Received response: ${response.status}`);
            return response.text();
        })
        .then((data) => {
            console.log(`Received data: ${data}`)
            const json = JSON.parse(data);
            p_output.textContent = json.num;
        });
    }

</script>
</body>
</html>

)rawliteral";

AsyncWebServer server(80);
TFT_eSPI tft = TFT_eSPI();
 

const char *ssid = "SM-Fritz";
const char *password = "47434951325606561069";
// const char* ssid = "ZenFone7 Pro_6535";
// const char* password = "e24500606";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void handle_get_requests (AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("num")) {
        message = request->getParam("num")->value();
    } else {
        message = "No message sent";
    }
    Serial.println("Param: num, " + message);
    tft.drawString("Param: num, " + message, 10 , 60);
    message = "{\"num\": \"" + message + "\"}\n";
    Serial.println("Sending message: " + message);
    tft.drawString(message, 10 , 100);
    request->send(200, "text/plain", message);
}

void setup() {

    Serial.begin(115200);
    pinMode(15, OUTPUT);
    analogWrite(15, 40);
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
   }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.drawString(WiFi.localIP().toString(), 10 , 20);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        tft.drawString("/", 10 , 60);
        Serial.println("Request to /");
        request->send(200, "text/html", webtest_html);
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, handle_get_requests);
    server.onNotFound(notFound);
    server.begin();
}

void loop() {

}