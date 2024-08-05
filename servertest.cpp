#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include "Free_Fonts.h" 

// const char *ssid = "SM-Fritz";
// const char *password = "47434951325606561069";
const char* ssid = "ZenFone7 Pro_6535";
const char* password = "e24500606";

WebServer server(80);
TFT_eSPI tft = TFT_eSPI();

void handleRoot() {
  const char* htmlCode = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Slider Example</title>
    <style>
        body {
            text-align: center;
            background-color: rgb(50, 0, 56);
            color: white;
        }
        .slider {
            width: 200px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <input type="range" min="0" max="100" value="0" class="slider" id="slider1">
    <button type="button" id="btn1" onclick="funk1()">btn</button>
    <p id="output">output if output</p>
    
<script>
    var js_slider1 = document.getElementById("slider1");
    const js_btn1 = document.getElementById("btn1");
    const js_output = document.getElementById("output");
    js_btn1.click();

    js_slider1.addEventListener("input", function() {
        js_output.textContent = this.value + " %";
        sendToESPServer("/" + this.value, "POST");
    });
    // js_output.textContent = js_output.innerText + " btn clicked";
    
    function funk1() {
        js_output.textContent = js_output.innerText + " btn clicked";
        sendToESPServer("/btn", "GET", callbackFunc);
    }
    function callbackFunc(sender, responseText){
        if (responseText != ""){
            console.log("callback: response for " + sender + ": " + responseText);
        }
    }

    function sendToESPServer(theUrl, command, callbackFunc=null)
    {
        var xmlHttp = new XMLHttpRequest();
        var responseText = "";
        xmlHttp.onreadystatechange = function() { 
            if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
                responseText = xmlHttp.responseText;
                console.log("Bifröst: request: " + theUrl + "; response: " + responseText);
                if (callbackFunc != null){
                    callbackFunc(theUrl, responseText);
                }
            }
        xmlHttp.open(command, theUrl, true); // true for asynchronous 
        xmlHttp.send(null);
        console.log(command + ": " + theUrl);

        //responseText = fetch(theUrl).then(response => response.text()).then(text => console.log(text));
        return responseText;
    }
</script>
</body>
</html>
    )rawliteral";


  server.send(200, "text/html", htmlCode );
  Serial.println("Sent HTML");
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        tft.fillScreen(TFT_BLACK);
        tft.drawString("...", 50 , 100, GFXFF);
        delay(1000);
    }
    Serial.println("");
    Serial.print("Connected to ");
    tft.drawString("Connected to ", 10 , 0, GFXFF);
    Serial.println(ssid);
    tft.drawString(ssid, 10 , 30, GFXFF);
    Serial.print("IP address: ");
    tft.drawString("IP address: ", 10 , 60, GFXFF);
    Serial.println(WiFi.localIP());
    tft.drawString(WiFi.localIP().toString(), 10 , 90, GFXFF);
}

void setup() {
  tft.begin();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FSB18);       

  Serial.begin(115200);
  WiFi.disconnect();
  delay(100);
  initWiFi();
  server.begin();

  server.on("/", handleRoot);
//   tft.drawString("Server started", 50 , 100, GFXFF);
  server.on("/btn", HTTP_GET, []() {
    Serial.println("Button pressed");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Button pressed", 50 , 100, GFXFF);
    server.send(200, "text/plain", "nice Button");
  });
  for (int i = 0; i <= 100; i++){
    server.on("/" + String(i), [i]() { // Capture 'i' in the lambda function
      Serial.println("client says: " + String(i));
      tft.fillScreen(TFT_BLACK);
      tft.fillCircle(i, i, 20, TFT_RED);
    });
  }
}

void loop() {
  server.handleClient();
}

