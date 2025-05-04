//THIS VERSION OF REMOTE RAIN IS FOR USE WITH WIFI. IT IS BUILT TO SAVE POWER AND RESTABLISH CONNECTIONS EACH TIME
//THE DEVICE CHECKS IN.
// Sike, all thats commented. But it works well enough as a v1 
//VERSIN BETA2

#include <WiFi.h>
//#include <WiFiClient.h>
//#include <AsyncTCP.h>
#include <HTTPClient.h>

WiFiClient wifiClient;

const int SensorPin = 34;

const char* WiFiSSID = "YOURSSID";
const char* WiFiPassword = "YOURPASSWORD";

const char* PARAM_ABADDESS = "http://[GatewayIP]:1880/rain";
const char* PARAM_DEVID = "111";

String mac = "";

unsigned long lastTime = 0;

//Fires every hour
//unsigned long timerDelay = 3600000; // Post once Per Hour
unsigned long timerDelay = 900000; // Post every 15
//unsigned long timerDelay = 60000; // Post every 1 min
double rainfall =  0;
int bucketState = 1;
double reboot24 = 0;

void setup() {
  Serial.begin(115200);
// Print Version Number
Serial.println("Version: Beta V0.3");
Serial.println("ID: ");
Serial.println(PARAM_DEVID);
  //pinMode(14, INPUT);


//wifi
   WiFi.mode(WIFI_STA);
   Serial.print("Using SSID ");
   Serial.println(WiFiSSID);
   Serial.print("Using Password ");
   Serial.println(WiFiPassword);
   char hname[] = "RemoteRain";
   Serial.print("Attempting To Conenct...");
   //WiFi.begin(WiFiSSID.c_str(), WiFiPassword.c_str());
   WiFi.begin(WiFiSSID, WiFiPassword);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!, rebooting and trying again");
    ESP.restart();
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Default Gateway: ");
  Serial.println(WiFi.gatewayIP().toString());
 // SHutdown the Wifi now that you have connected and bring back up only for post data
// WiFi.forceSleepBegin();
 //Serial.print("WiFi in Sleep Mode");

//disableWiFi();
//disableWiFi();
//WiFi.setSleep(true);

//Serial.print("CPU Freq: ");
//Serial.println(getCpuFrequencyMhz());
 
//setCpuFrequencyMhz(10);
 
//Serial.print("CPU Freq: ");
//Serial.println(getCpuFrequencyMhz());




 }


void loop() {
//int bucketValue = digitalRead (14);
int bucketValue = analogRead(SensorPin);
//Serial.println(bucketValue);


if (bucketValue < 100){
  rainfall = rainfall + 0.007;
  //bucketState = bucketValue;
  Serial.println("Rainfall Captured!");
  Serial.println("Total Rainfall in this session: ");
  Serial.println(rainfall);
  delay(1000);
}

// Every X Minutes post to the API
    if ((millis() - lastTime) > timerDelay) {
    Serial.println("Taking Measurements and Posting Data");
    POSTDATA();
    lastTime = millis();
    reboot24 = reboot24 + 1;
  }
  //Reboot every 24 hours as a safty net 
  if (reboot24 > 95 & rainfall == 0) {
    ESP.restart();
  }
}
// new global vars
void POSTDATA( void )
{
  //Brings WiFi online and connects
  //Posts the value of rainfall to the API
  //Shuts down WiFi to save Battery
  
  //Power up CPU to 80 mhz to use WiFi
  //setCpuFrequencyMhz(80);
  //Serial.print("CPU Freq: ");
  //Serial.println(getCpuFrequencyMhz());

  //Start WiFi
  //enableWiFi();
  //Serial.println("Starting WiFi");
  //WiFi.setSleep(false);
  //delay(1);
  //Serial.println();
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.localIP());
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(WiFiSSID, WiFiPassword);

  //Post
  HTTPClient http;
     String APIID = "666";
      // Your Domain name with URL path or IP address with path
      //http.begin(wifiClient, PARAM_ABADDESS);
      http.begin(PARAM_ABADDESS);
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      Serial.println("Total Rainfall to send = ");
      Serial.println(rainfall);
      String httpRequestData = "Id=" + APIID + "&msg=" + rainfall;          
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      Serial.println(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);


      // Free resources
      http.end();

  // Clear counter only if Sensor was able to post to WiFi
  if (httpResponseCode == 200) {
    Serial.print("Post sucessful, clearing data!");
     rainfall =  0;
  }
  // Dump Restart WiFi and go back into the main loop
  else if (httpResponseCode != 200) {
    Serial.print("Post failed, restting WiFi!");
    WiFi.disconnect();
    disableWiFi();
    delay(3000);
    enableWiFi();
    delay(3000);
  }
  
  //Shutdown WiFi
  //Serial.println("Shutting Down WiFi");
  //WiFi.disconnect();
  //WiFi.setSleep(true);
  //disableWiFi();
  //delay(1); 
 //Powering Down CPU
 //setCpuFrequencyMhz(10);
 //Serial.print("CPU Freq: ");
 //Serial.println(getCpuFrequencyMhz());
}

void disableWiFi(){
        //adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void enableWiFi(){
    //adc_power_on();
    WiFi.disconnect(false);  // Reconnect the network
    WiFi.mode(WIFI_STA);    // Switch WiFi off
 
    Serial2.println("START WIFI");
    WiFi.begin(WiFiSSID, WiFiPassword);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial2.print(".");
    }
 
    Serial2.println("");
    Serial2.println("WiFi connected");
    Serial2.println("IP address: ");
    Serial2.println(WiFi.localIP());
}
