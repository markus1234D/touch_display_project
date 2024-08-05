#include <Arduino.h>
#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

const char* html = R"rawliteral(

<!DOCTYPE html>
<html>
   <head>
      <script type = "text/javascript">
        var ws = new WebSocket();
 
        ws.onopen = function() {
            window.alert("Connected");
         };
 
         ws.onmessage = function(evt) {
            document.getElementById("display").innerHTML  = "temperature: " + evt.data + " C";
        };
 
      </script>
   </head>
 
   <body>
      <div>
         <p id = "display">Not connected</p>
      </div>
   </body>
</html>

)rawliteral";
 
const char *ssid = "SM-Fritz";
const char *password = "47434951325606561069";
// const char* ssid = "ZenFone7 Pro_6535";
// const char* password = "e24500606";
 
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
 
AsyncWebSocketClient * globalClient = NULL;
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
    globalClient = client;
 
  } else if(type == WS_EVT_DISCONNECT){
 
    Serial.println("Websocket client connection finished");
    globalClient = NULL;
 
  }
}
 
void setup(){
  Serial.begin(115200);
 
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", html);
  });
 
  server.begin();
}
 
void loop(){
   if(globalClient != NULL && globalClient->status() == WS_CONNECTED){
      String randomNumber = String(random(0,20));
      globalClient->text(randomNumber);
   }
   delay(4000);
}