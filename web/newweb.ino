#include <MicroGear.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// constants won't change. They're used here to 
// set pin numbers:
#define D0 16  // USER LED Wake
#define ledPin D0        // the number of the LED pin
#define RXpin D7
#define TXpin D8

const char* ssid     = "WiMCA";
const char* password = "Muknairun2";
String text;
int count;
int state,goal;
char old;
char cmode;

#define APPID   "TestConnectArduino"
#define KEY     "0OdoDXe2A4BDKUT"
#define SECRET  "lGkPVKo9Fsgu4jcaRvkBWI70g"

#define ALIAS   "NodeMCU1"
#define TargetWeb "DigitalOUTPUT_HTML_web"

WiFiClient client;
MicroGear microgear(client);
SoftwareSerial uart(RXpin, TXpin);

void clearBuffer(){
  while(uart.read() == 'P' || uart.read() == 'D' || uart.read() == '2'){
    
  }
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) 
{
  Serial.print("Incoming message --> ");
  Serial.print(topic);
  Serial.print(" : ");
  char strState[msglen];
  char strGoal[msglen];
  int stateLen = 0,goalLen = 0;
  for (int i = 0; i < msglen; i++) 
  {
    if(msg[i] >= 65 && msg[i] <= 90){
      strState[stateLen] = (char)msg[i];
      stateLen++;
    }
    else{
      strGoal[goalLen] = (char)msg[i];
      goalLen++;
    }
    Serial.print((char)msg[i]);
  }
  Serial.println();

  String stateStr = String(strState).substring(0, stateLen);
  String goalStr = String(strGoal).substring(0, goalLen);

  Serial.println(stateStr);
  Serial.println(goalStr);

  if(stateStr == "ON") 
  {
    digitalWrite(ledPin, LOW);
    microgear.chat(TargetWeb, "ON");
    uart.write("TTTTT");
  } 
  else if (stateStr == "OFF") 
  {
    digitalWrite(ledPin, HIGH);
    microgear.chat(TargetWeb, "OFF");
    uart.write("FFFFF");
  }
  else if (stateStr == "D"){
    state = 1;
    cmode = 'D';
    goal = goalStr.toInt();
  }
  else if (stateStr == "P"){
    state = 1;
    cmode = 'P';
    goal = goalStr.toInt();
    Serial.println("Push Up Ready");
  }
  else if (stateStr == "STOP"){
    state = 0;
  }
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) 
{
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

void setup() 
{
    /* Event listener */
    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

    Serial.begin(115200);
    uart.begin(115200);
    Serial.println("Starting...");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
       delay(250);
       Serial.print(".");
    }

    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    microgear.init(KEY,SECRET,ALIAS);
    microgear.connect(APPID);

    pinMode(ledPin,OUTPUT);
    digitalWrite(ledPin,HIGH); // Turn off LED
    count = 0 ;
    old = '0' ;
    state = 0;
    goal = 0;
}

void loop() 
{
  text = "" ;
  //if(microgear.connected()) 
  //{
  //  Serial.println("connect...");
  //} 
  //else 
  //{
    //Serial.println("connection lost, reconnect...");
  //  microgear.connect(APPID);
  //}
  if(state == 0){
    count = 0;
    old = '0' ;
    goal = 0;
    clearBuffer();
    Serial.println("Do nothing");
    if (microgear.connected()){
          microgear.loop();
        } else {
        microgear.connect(APPID); }
        delay(250) ;
  }
  else{
    Serial.print("Working..");
  }
  while (uart.available() > 0 && state == 1){
    char character = uart.read() ;
    Serial.print(character);
    if (character == '2' || character == '0' || character == 'D' || character == 'P' ) 
    {
      Serial.println(old);
      Serial.println(character);
      if ((character == 'P' || character == '2') && (old =='0' || old == 'D') && cmode == 'P'){
        count++ ;
        if (microgear.connected()){
          microgear.loop();
        } else {
          microgear.connect(APPID); 
        }
        delay(250) ;
        microgear.chat(TargetWeb,String(count)) ;
        if(count == goal){
          state = 0;
          goal = 0;
          count = 0;
          old = '0';
          clearBuffer();
          break;
        }
      } 
      else if ((character == 'P' || character == '0') && (old =='2' || old == 'D') && cmode == 'D'){
        count++ ;
        if (microgear.connected()){
          microgear.loop();
        } else {
          microgear.connect(APPID); 
        }
        delay(250) ;
        microgear.chat(TargetWeb,String(count)) ;
        if(count == goal){
          state = 0;
          goal = 0;
          count = 0;
          old = '0';
          clearBuffer();
          break;
        }
      } 
      Serial.print("Counting :");
        Serial.println(count);
      old = character ;
      break ;
    }
  }
  delay(10);
  
}
