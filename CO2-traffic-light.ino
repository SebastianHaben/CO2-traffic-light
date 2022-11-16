#include "time.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <GxEPD2_BW.h>
#include "GxEPD2_display_selection_new_style.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "ccs811.h"
#include <Adafruit_MPL3115A2.h>
#include "firebase_credentials.h"

// Display information
#define SCREEN_WIDTH   296
#define SCREEN_HEIGHT  128
#define ENABLE_GxEPD2_GFX 0
// Static text on display
const char CO2[] = "CO2:";
const char VOCs[] = "VOCs:";

// Neopixel Information
#define NEOPIN  D4
#define NUMPIXELS 1

// DateTime information
struct tm lt;         
struct tm utc;
const char* ntpServer = "europe.pool.ntp.org";
struct tm timeinfo;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;

// Necessary for Firebase authentication
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
String parentPath;
String uid;
String databasePath;
String tempPath = "/temperature";
String co2Path = "/eCO2";
String vocPath = "/VOC";
String timePath = "/timestamp";
String timeStamp;

// Initialize Neopixel & Sensors
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);
CCS811 ccs811(D3);
Adafruit_MPL3115A2 baro;

void setup() {
  Serial.begin(19200);
  // Use WifiManager to setup WiFi without providing password
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("AutoConnectAP"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
  // Start 
  Wire.begin(); 
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED");
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("setup: CCS811 start FAILED");
  if (!baro.begin()) {
    Serial.println("Could not find sensor. Check wiring.");
    while(1);
  }
  // Get config time based on NTP server
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer);
  // Setup Display
  display.init();
  display_setup();
  display.hibernate();
  // Start-up Neopixel
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(32, 32, 32));
  // Connect to Firebase
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  // Feedback for token
  config.token_status_callback = FirestoreTokenStatusCallback;
  config.max_token_generation_retry = 5;
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);
  // Update database path
  databasePath = "/UsersData/" + uid + "/CO2Ampel/readings";
}

void loop() {
  // Initiallize variables for readings
  uint16_t eco2, etvoc, errstat, raw;
  float temperature;
  
  // Get temperature
  temperature = baro.getTemperature();
  // Parse temperature & humidity to ccs811. Standrad value of 40% relative humidity is used due to lack of humidity sensor. 
  ccs811.set_envdata_Celsius_percRH(temperature, 40.0);
  ccs811.read(&eco2,&etvoc,&errstat,&raw);
  // Set color of Neopixel based on current CO2 value.
  if (eco2 <= 1000){
        pixels.setPixelColor(0, pixels.Color(0, 20, 0));
        }
      else if (eco2 > 1000 && eco2 <= 2000){
        pixels.setPixelColor(0, pixels.Color(10, 10, 0));
        }
      else{
        pixels.setPixelColor(0, pixels.Color(20, 0, 0));
        }
  pixels.show();
  // Update display
  if(!getLocalTime(&timeinfo,500)){
    Serial.println("Failed to obtain time");
    return;
  }
  time_t now = time(&now);
  localtime_r(&now, &lt);
  gmtime_r(&now, &utc);
  updateDate(timeinfo);
  updateAirQuality(eco2, etvoc);
  updateEnvironment(temperature);
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    static char buf[30];
    strftime (buf, sizeof(buf), "%d.%m.%Y %T", &utc); 
    timeStamp= String(buf);

    parentPath= databasePath + "/" + timeStamp;

    json.set(tempPath.c_str(), String(temperature));
    json.set(co2Path.c_str(), String(eco2));
    json.set(vocPath.c_str(), String(etvoc));
    json.set(timePath, timeStamp);
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  if (!(time(&now) % 86400)) {                          // einmal am Tag die Zeit vom NTP Server holen o. jede Stunde "% 3600" aller zwei "% 7200"
    configTime("CET-1CEST,M3.5.0,M10.5.0/3", ntpServer);  
    }
  delay(1000);
}

//Initialize all static elements of the display
void display_setup()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.drawRect(3,3,292,124,GxEPD_BLACK);
    display.drawRect(4,4,290,122,GxEPD_BLACK);
    display.drawRect(8,33,106,85,GxEPD_BLACK);
    display.drawRect(9,34,104,83,GxEPD_BLACK);
    display.drawRect(123,33,164,85,GxEPD_BLACK);
    display.drawRect(124,34,162,83,GxEPD_BLACK);
  display.setCursor(127, 60);
  display.print(CO2);
  display.setCursor(127, 90);
  display.print(VOCs);
  }
  while (display.nextPage());
}

// Update date and time information on display
void updateDate(struct tm timeinfo)
{ 
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  String date;
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMinute[3];
  strftime(timeMinute,3, "%M", &timeinfo);
  char dateWeekDay[4];
  strftime(dateWeekDay,4, "%a", &timeinfo);
  char dateDay[3];
  strftime(dateDay,3, "%d", &timeinfo);
  char dateMonth[3];
  strftime(dateMonth,3, "%m", &timeinfo);
  char dateYear[5];
  strftime(dateYear,5, "%Y", &timeinfo);
  date= String(dateWeekDay)+", "+String(dateDay)+"."+String(dateMonth)+"."+String(dateYear);
  String t;
  t = String(timeHour)+":"+String(timeMinute);
  int16_t dbx, dby; uint16_t dbw, dbh;
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(date, 0, 0, &dbx, &dby, &dbw, &dbh);
  display.getTextBounds(t, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.setPartialWindow(8,8, display.width()- 2*8, FreeMonoBold9pt7b.yAdvance);
  display.firstPage();
  do
  {
  display.setCursor(8,8+FreeMonoBold9pt7b.yAdvance/2-dby);
  display.print(date);
  display.setCursor(display.width()- 8 - tbw - tbx,8+FreeMonoBold9pt7b.yAdvance/2-tby);
  display.print(t);
  }
  while(display.nextPage()); 
}

// Update air quality variables on display
void updateAirQuality(uint16_t eco2, uint16_t etvoc)
{
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  String eco2String = String(eco2) + " ppm";
  String etvocString = String(etvoc) + " ppb";
  int16_t co2bx, co2by; uint16_t co2bw, co2bh;
  int16_t vocbx, vocby; uint16_t vocbw, vocbh;
  display.getTextBounds(eco2String, 0, 0, &co2bx, &co2by, &co2bw, &co2bh);
  display.getTextBounds(etvocString, 0, 0, &vocbx, &vocby, &vocbw, &vocbh);
  int boxw = 156;
  display.setPartialWindow(127+boxw/3,40, (boxw/3)*2, 70);
  display.firstPage();
  do
  {
    display.setCursor(display.width()- 12 - co2bw - co2bx,60);
    display.print(eco2String);
    display.setCursor(display.width()- 12 - vocbw - vocbx,90);
    display.print(etvocString);
  }
  while(display.nextPage()); 
}

// Update temperature on display
void updateEnvironment(float temperature)
{
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  String tString = String(temperature,1) + " C";
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(tString, 0, 0, &tbx, &tby, &tbw, &tbh);
  int boxw = 98;
  display.setPartialWindow(12,40, boxw , 70);
  display.firstPage();
  do
  {
    display.setCursor(15,80);
    display.print(tString);
  }
  while(display.nextPage());
}


void FirestoreTokenStatusCallback(TokenInfo info){
  Serial.printf("Token Info: type = %s, status = %s\n", getTokenType(info), getTokenStatus(info));
}

bool getLocalTime(struct tm * info, uint32_t ms)
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}
  
