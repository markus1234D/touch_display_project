#include <HardwareSerial.h>

   #define mySerial Serial2

void setup() 
{
  Serial.begin(115200);
  delay(500);
  mySerial.begin(115200, SERIAL_8N1, 18, 17);

  delay(500);
  mySerial.println("Hallo");
  delay(500);
    
}
void loop() 
{     
    if(mySerial.available())
    {
        String str = mySerial.readStringUntil('\n');
        delay(500);
        mySerial.println(str);
    }
}


