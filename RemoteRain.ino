#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <ESPAsyncTCP.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

WiFiClient wifiClient;
//USE NODEMCU OR OTHER ESP8266
//Needed Vars 
const float mmPerPulse = 0.173;
float mmTotali = 0;
int sensore = 0;
int statoPrecedente = 0;

#define SensorPin A0

const char* WiFiSSID = "WiFiSSID";
const char* WiFiPassword = "WiFiPassword";

//Default WiFi Creds 
const char* ssidd = "RemoteRain";
const char* passwordd = "Grow4life";
bool wifisetup = false;
int localonly = 0;

const char* PARAM_ABADDESS = "abaddress";
const char* PARAM_DEVID = "devid";

String mac = "";
String disPH = "";
String offsetdis = "";

AsyncWebServer server(80);

void localWifi ( void ){
    Serial.println("Local WiFi");
    WiFi.softAP(ssidd, passwordd) ;
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
    localonly = 1;
    wifisetup = true;
}

//HTML Four UI Embeded here
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head>
  <title>Remote Rain</title>

<style type="text/css">
            h2 {
                font-family: courier;
                font-size: 20pt;
                color: black;
                border-bottom: 2px solid blue;
            }
            p {
                font-family: arial, verdana, sans-serif;
                font-size: 12pt;
                color: grey;
            }
            .red_txt {
                color: red;
            }
        </style>
  
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <h2>ABdc_PH</h2>
  <p><i>ABdc_PH V1.0</i></p>
  <script>
    function submitMessage() {
      alert("Value Saved");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
    <p><b>Control</b></p>
  </form><br>
  <form action="/get" target="hidden-form">
    API Address: (Current: %abaddress%): <input type="number " name="abaddress">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
    Device MAC: %mac%
    </form><br>
    <form action="/get" target="hidden-form">
   Device ID: (Current: %devid%): <input type="text" name="devid">
   <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
    
   </b></p>Wireless Configuration</b></p>
   <form action="/get" target="hidden-form">
    WiFiSSID (%WiFiSSID%): <input type="text" name="WiFiSSID">
    <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
   <form action="/get" target="hidden-form">
    WiFiPassword (%WiFiPassword%): <input type="text" name="WiFiPassword">
    <input type="submit" value="Submit" onclick="submitMessage()">
   </form><br>
   
    
  </div>

</body>
<script>
  function UpdateNow() {
    var phr = new XMLHttpRequest();
    phr.open('GET', "/UpdateNow", true);
    phr.send();
  }  
 </script> 
 </html>)rawliteral";


 //SET UP SPIFFS
 String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

String processor(const String& var){
  //Vars to save to the local file system 
   if(var == "ph_act"){
    return String(disPH);
  }
 else if(var == "mac"){
    return String(mac);
  }
 else if(var == "WiFiSSID"){
    return readFile(SPIFFS, "/ssid.txt");
  }
  else if(var == "WiFiPassword"){
    return readFile(SPIFFS, "/wifipasswd.txt");
  }
  else if(var == "abaddress"){
    return readFile(SPIFFS, "/abaddress.txt");
  }
  else if(var == "devid"){
    return readFile(SPIFFS, "/id.txt");   
  }

  return String();
}


void setup() {
  Serial.begin(115200);

  pinMode(9, INPUT);


  //Dont boot unles you have a FS

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");  
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }


//wifi
Serial.print(wifisetup);
//// pull saved value from SPIFFs
String savedssid = readFile(SPIFFS, "/ssid.txt");
String savedpass = readFile(SPIFFS, "/wifipasswd.txt");
int ssidlength = savedssid.length();
Serial.print("SSID Length:");
Serial.print(ssidlength);

//if the SSID is blank, then revert to local mode
if (ssidlength == 0) {
  wifisetup == true;
  localWifi();
}

 //If Wifi config was pressed, skip this, if not, run:
 if (wifisetup == false) {
  // String savedssid = readFile(SPIFFS, "/ssid.txt");
 //  String savedpass = readFile(SPIFFS, "/wifipasswd.txt");
   WiFi.mode(WIFI_STA);
   Serial.print("Using SSID ");
   Serial.println(savedssid);
   Serial.print("Using Password ");
   Serial.println(savedpass);
   //char hname[19];
   //snprintf(hname, 12, "ESP%d-LIGHT", 32);
   char hname[] = "RemoteRain";
   WiFi.begin(savedssid.c_str(), savedpass.c_str());
   //WiFi.setHostname(hname);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    wifisetup == true;
    localWifi();
    //return;
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 }

//Load HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

//NEW Server Request handeling
  server.on("/UpdateNow", HTTP_GET, [](AsyncWebServerRequest * request) {
   Serial.print("Pulling readings");
   TakeData = 1;

   
    request->send_P(200, "text/plain", "Post");
  });

AsyncElegantOTA.begin(&server); 
//server.onNotFound(notFound);
// Respond to web requests 

server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(WiFiSSID)) {
      inputMessage = request->getParam(WiFiSSID)->value();
      writeFile(SPIFFS, "/ssid.txt", inputMessage.c_str());
    }
    else if (request->hasParam(WiFiPassword)) {
      inputMessage = request->getParam(WiFiPassword)->value();
      writeFile(SPIFFS, "/wifipasswd.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_ABADDESS)) {
      inputMessage = request->getParam(PARAM_ABADDESS)->value();
      writeFile(SPIFFS, "/abaddress.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_DEVID)) {
      inputMessage = request->getParam(PARAM_DEVID)->value();
      writeFile(SPIFFS, "/id.txt", inputMessage.c_str());
    }
    
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
server.begin(); 
mac = (WiFi.macAddress());
}

void loop() {

   sensore = digitalRead(9);
  
  if (sensore != statoPrecedente) {
    mmTotali = mmTotali + mmPerPulse;
  }
  
  delay(500);
  
  statoPrecedente = sensore;
  Serial.println(statoPrecedente);

}

// new global vars
void POSTDATA( void )
{
  HTTPClient http;
  String ABAddress = readFile(SPIFFS, "/abaddress.txt");
  String IDx = readFile(SPIFFS, "/id.txt");
       
      // Your Domain name with URL path or IP address with path
      http.begin(wifiClient, ABAddress);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "Id=" + IDx + "&msg=" + disPH + "&Temp=" + 0 + "&Hum=" + 0;          
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      Serial.println(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
      TakeData = 0;
}
