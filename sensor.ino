#include <SoftwareSerial.h>

#define RX 10
#define TX 11

String AP = "test";
String PW = "test123456";

SoftwareSerial esp8266(RX,TX); 
 
  
void setup() {
  
  Serial.begin(9600);
  esp8266.begin(115200);

  sendCommand("AT",5,(char *)"OK");
  sendCommand("AT+CWMODE=1",5,(char *)"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PW + "\"",20,(char *)"OK");
}
void loop() {
  char c = esp8266.read();

  if ((int)c != -1){
    Serial.print(c);
  }
}

bool sendCommand(String command, int timeOut, char replay[]){
  int times = 0;
  while(timeOut != 0){
    Serial.print(++times);
    Serial.println(" > " + command);
    esp8266.println(command);
    if(esp8266.find(replay)){
      Serial.println("OK");
      return true;
    }
    timeOut--;
  }
  Serial.println("FAIL");
  return false;
}




