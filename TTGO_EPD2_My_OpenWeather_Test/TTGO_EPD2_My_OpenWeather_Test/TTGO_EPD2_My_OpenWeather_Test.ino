// Sketch for ESP32 to fetch the Weather Forecast from OpenWeather
// an example from the library here:
// https://github.com/Bodmer/OpenWeather

// Sign up for a key and read API configuration info here:
// https://openweathermap.org/

// You can change the number of hours and days for the forecast in the
// "User_Setup.h" file inside the OpenWeather library folder.
// By default this is 6 hours (can be up to 48) and 5 days
// (can be up to 8 days = today plus 7 days)

// Modified to display the weather on a TTGO T5 EPD Dev Board
// Library: https://github.com/ZinggJM/GxEPD2
// Author: Jean-Marc Zingg

// Board:  ESP32 Dev Module
// Updated to display on the TTGO T5 2.13 EPD dev board.  Also added deep sleep and wake every 15 min to refresh display and wake on using the built in button.

// Choose library to load
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#else // ESP32
#include <WiFi.h>
#endif

//needed for wifi autoconnect
#include <DNSServer.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

//#include <WiFiManager.h>

#include <JSON_Decoder.h>

#include <OpenWeather.h>

// Deep Sleep Settings
#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  900        //Time ESP32 will sleep (in seconds)
//RTC_DATA_ATTR int bootCount = 0;  // Use to trigger updating weather - not needed, keep for future reference

// Epaper Display Setup
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h> // including both doesn't hurt
#include <GxEPD2_3C.h> // including both doesn't hurt
#include <U8g2_for_Adafruit_GFX.h>
uint16_t bg = GxEPD_WHITE;
uint16_t fg = GxEPD_BLACK;

// copy constructor for your e-paper from GxEPD2_Example.ino, and for AVR needed #defines
#define MAX_DISPLAY_BUFFER_SIZE 800 // 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
//GxEPD2_BW<GxEPD2_154, MAX_HEIGHT(GxEPD2_154)> display(GxEPD2_154(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7)); // GDEP015OC1 no longer available <- works with Heltec 1.54 on nano
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEH0213B72 250 x 122 on the TTGO T5 board
U8G2_FOR_ADAFRUIT_GFX u8display;

// Just using this library for unix time conversion
#include <Time.h>

// =====================================================
// ========= User configured stuff starts here =========
// Further configuration settings can be found in the
// OpenWeather library "User_Setup.h" file

#define TIME_OFFSET 0UL * 3600UL // UTC + 0 hour:  GMT - note that this is only used for reporting weather update.  
//#define TIME_OFFSET -8UL * 3600UL // UTC -8 hour:  Pacific Time
//#define TIME_OFFSET -6UL * 3600UL // UTC -6 hour:  Central Time
//#define TIME_OFFSET 8UL * 3600UL // UTC -6 hour:  China Time

// Change to suit your WiFi router
#define WIFI_SSID     "gwifi"
#define WIFI_PASSWORD "c4d496c2"

// OpenWeather API Details, replace x's with your API key
//String APIKEY = "d268dda12c36bdc8809a9525f37ab560";  //407keith@gmail
String api_key = "122c8b5cd4731038ff78486f1faa70c5"; // Obtain this from your OpenWeather account

// Set both your longitude and latitude to at least 4 decimal places
//  Hayward  37.67211° N, -122.08396° W
//  Norman 35.2226° N, -97.4395° W
//  London 51.5074° N, 0.1278° W
//  Taipei 25.0330° N, 121.5654° E

//String location = Mt Everest;
//String latitude =  "27.9881"; // 90.0000 to -90.0000 negative for Southern hemisphere
//String longitude = "86.9250"; // 180.000 to -180.000 negative for West

//String location = "Hayward";
//String latitude =  "37.67211"; // 90.0000 to -90.0000 negative for Southern hemisphere
//String longitude = "-122.08396"; // 180.000 to -180.000 negative for West

//String location = "407 Keith St";
//String latitude =  "35.2226"; // 90.0000 to -90.0000 negative for Southern hemisphere
//String longitude = "-97.4395"; // 180.000 to -180.000 negative for West

String location = "London UK";
String latitude =  "51.5074"; // 90.0000 to -90.0000 negative for Southern hemisphere
String longitude = "0.1278"; // 180.000 to -180.000 negative for West

//String location = "Taipei";
//String latitude =  "25.0330"; // 90.0000 to -90.0000 negative for Southern hemisphere
//String longitude = "121.5654"; // 180.000 to -180.000 negative for West

String units = "metric";  // or "imperial"
//String units = "imperial";
String language = "en";   // See notes tab

// =========  User configured stuff ends here  =========
// =====================================================

OW_Weather ow; // Weather forecast library instance

// Create Global structures that hold the retrieved weather
OW_current *current = new OW_current;
OW_hourly *hourly = new OW_hourly;
OW_daily  *daily = new OW_daily;

// weather variables
#define SUN  0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define MIST 5
#define SNOW 6
int8_t Symbol;
char buf[80];    // for sprintf

// variables to reset if wifi connect takes too long
void(* resetFunc) (void) = 0;  //declare reset funciton @ address 0
int connectWait = 0;

void setup() {
  Serial.begin(115200); // Fast to stop it holding up the stream

  //Set timer
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to wake after " + String(TIME_TO_SLEEP) +  " Seconds");

  //Configure GPIO39 as ext0 wake up source for LOW logic level
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  // start display
  display.init();
  display.setRotation(3);
  u8display.begin(display); // connect u8g2 procedures to Adafruit GFX

  u8display.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  u8display.setFontDirection(0);            // left to right (this is default)
  u8display.setForegroundColor(fg);         // apply Adafruit GFX color
  u8display.setBackgroundColor(bg);         // apply Adafruit GFX color

  // connect wifi using wifiManager
  //  WiFiManager wifiManager;
  //  wifiManager.autoConnect("WeatherAP");

  //Serial.printf("\n\nConnecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    connectWait++;
    if (connectWait >= 120) {  //if no connect after 1 min, show message
      display.firstPage();
      do {
        // Draw Current Condition
        u8display.setFont( u8g2_font_helvR12_tf  );
        uint16_t y_main = 25;
        u8display.setCursor(10, y_main);
        u8display.print("Connect to WeatherAP");
        Serial.println("Connect to WeatherAP");
      }
      while (display.nextPage());
    }
    if ( connectWait >= 960 ) ESP.restart();  //resetFunc(); //reset if no connect after 480 seconds (8min)
  }

  //  Get and Display Weather to Screen
  printCurrentWeather();  // get weather data

  //  Display Weather
  display.firstPage();
  do {
    display.fillScreen(bg);  // not needed if there is a reboot between every screen refresh.

    //********************************************************
    // Draw Forecast
    //********************************************************

    // Draw Forecast DOW
    //u8display.setFont( u8g2_font_cupcakemetoyourleader_tr  );
    u8display.setFont( u8g2_font_helvB12_tf  );
    uint16_t y_top = 15;
    // issue:  if these for loops are combined, the days info only shows the first day...
    for (int i = 0; i <= (MAX_DAYS - 1); i++) {
      u8display.setCursor(10 + i * 51, y_top); //set to row 10 for DOW
      u8display.print(strTime(daily->dt[i]).substring(0, 3));
    }

    // Draw Forecast Glyphs
    // font is defined in drawWeatherSymbol()
    uint16_t y_symb = y_top + 20;
    for (int i = 0; i <= (MAX_DAYS - 1); i++) {
      drawWeatherSymbol(19 + i * 50, y_symb, getSymbol(daily->id[i])); //drawGlyph(x,y, Glyph number), x, y is the lower left corner of the glyph.
    }

    // Draw Forecast Hi / Lo temps
    u8display.setFont( u8g2_font_helvR08_tf  );
    uint16_t y_hilo = y_symb + 12;
    for (int i = 0; i <= (MAX_DAYS - 1); i++) {
      String highTemp = String(daily->temp_max[i], 0);
      String lowTemp = String(daily->temp_min[i], 0);
      u8display.setCursor(15 + i * 50, y_hilo); //set to row 80 for Max / Min temp
      u8display.print(highTemp + " " + lowTemp);
    }

    //********************************************************
    // Draw Current Weather
    //********************************************************

    // Draw Current Weather Glyph
    uint16_t y_glyph = y_hilo + 58;
    drawWeatherSymbolBig(15, y_glyph, getSymbol(current->id));

    // Draw Current Condition
    u8display.setFont( u8g2_font_helvB12_tf  );
    uint16_t y_main = y_hilo + 25;
    u8display.setCursor(70, y_main);
    u8display.print(current->main);

    // Draw Current Temp
    uint16_t y_temp = y_main + 16;
    u8display.setCursor(70, y_temp);
    if (units == "imperial") {
      u8display.print(String(current->temp, 1) + "°F");   //Alt + 0176 for degree symbol
    } else
    {
      u8display.print(String(current->temp, 1) + "°C");
    }

    // Draw Current Humidity
    uint16_t y_humid = y_temp + 15;
    u8display.setCursor(70, y_humid);
    //u8display.print(String(current->humidity) + " %");
    u8display.printf("%02d%%", current->humidity);  //printf works too

    // Draw Location w: 120 - 255 / h: 73 - 110

    //u8display.setFont( u8g2_font_cupcakemetoyourleader_tr  );
    u8display.setFont( u8g2_font_helvB12_tf  );
    uint16_t y_loc = y_hilo + 20;
    u8display.setCursor(140, y_loc);
    u8display.print(location);

    // Draw Feels Like Temp
    u8display.setFont( u8g2_font_helvR08_tf  );
    uint16_t y_feel = y_loc + 12;
    u8display.setCursor(140, y_feel);
    if (units == "imperial") {
      u8display.print("Feels like: "  + String(current->feels_like, 1) + "°F");
    } else {
      u8display.print("Feels like: "  + String(current->feels_like, 1) + "°C");
    }

    // Draw Dew Point Temp
    uint16_t y_dew = y_feel + 12;
    u8display.setCursor(140, y_dew);
    if (units == "imperial") {
      u8display.print("Dew point: "  + String(current->dew_point, 1) + "°F");
    } else {
      u8display.print("Dew point: "  + String(current->dew_point, 1) + "°C");
    }

    // Draw Wind Speed
    uint16_t y_wind = y_dew + 12;
    u8display.setCursor(140, y_wind);
    if (units == "imperial") {
      u8display.print("Wind: "  + String(current->wind_speed, 1) + "mph");
    } else
    {
      u8display.print("Wind: "  + String(current->wind_speed, 1) + "km/h");
    }

    // Draw last update time at bottom of screen
    u8display.setFont( u8g2_font_finderskeepers_tr  );
    u8display.setCursor(10, 120);
    u8display.print("Last update: " + strTime(current->dt));
    u8display.setCursor(200, 120);
    u8display.print(location);

  }
  while (display.nextPage());

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete hourly;
  delete daily;

  //Go to sleep now - restart per TIME_TO_SLEEP constant
  esp_deep_sleep_start();

}

void loop() {
  /**
     Everyting is done in setup.
     CPU wakes up every 15 min to refresh EPD with new weather data
    **/
}

int getSymbol(int id) {
  //Maps weather id to symbol
  int symbol = 0;

  if ((id >= 200) && (id <= 232)) {
    symbol = THUNDER;
  }
  else if ((id >= 300) && (id <= 531)) {
    symbol = RAIN;
  }
  else if ((id >= 600) && (id <= 622)) {
    symbol = SNOW;
  }
  else if ((id >= 700) && (id <= 781)) {
    symbol = MIST;
  }
  else if (id == 800) {
    symbol = SUN;
  }
  else if ((id >= 801) && (id <= 804)) {
    symbol = CLOUD;
  }

  return symbol;
}

/********************************************************************************************************************************
   Draw Weather Symbols
 * ******************************************************************************************************************************/
void drawWeatherSymbol(uint8_t x, uint8_t y, uint8_t symbol)
{
  // fonts used:
  // u8g2_font_open_iconic_embedded_6x_t
  // u8g2_font_open_iconic_weather_6x_t
  // 1x = 8x8 px, 2x = 16x16 px ... 6x = 48x48 px,  8x = 64x64 px
  // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic

  switch (symbol)
  {
    case SUN:
      u8display.setFont(u8g2_font_open_iconic_weather_2x_t);  // 16 px
      u8display.drawGlyph(x, y, 69);
      break;
    case SUN_CLOUD:
      u8display.setFont(u8g2_font_open_iconic_weather_2x_t);
      u8display.drawGlyph(x, y, 65);
      break;
    case CLOUD:
      u8display.setFont(u8g2_font_open_iconic_weather_2x_t);
      u8display.drawGlyph(x, y, 64);
      break;
    case RAIN:
      u8display.setFont(u8g2_font_open_iconic_weather_2x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case THUNDER:
      u8display.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case MIST:
      u8display.setFont(u8g2_font_open_iconic_other_2x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case SNOW:
      u8display.setFont(u8g2_font_open_iconic_embedded_2x_t);
      u8display.drawGlyph(x, y, 66);
      break;
  }
}

/********************************************************************************************************************************
   Draw Weather Symbols
 * ******************************************************************************************************************************/
void drawWeatherSymbolBig(uint8_t x, uint8_t y, uint8_t symbol)
{
  // fonts used:
  // u8g2_font_open_iconic_embedded_6x_t
  // u8g2_font_open_iconic_weather_6x_t
  // 1x = 8x8 px, 2x = 16x16 px ... 6x = 48x48 px,  8x = 64x64 px
  // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic

  switch (symbol)
  {
    case SUN:
      u8display.setFont(u8g2_font_open_iconic_weather_6x_t);  // 16 px
      u8display.drawGlyph(x, y, 69);
      break;
    case SUN_CLOUD:
      u8display.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8display.drawGlyph(x, y, 65);
      break;
    case CLOUD:
      u8display.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8display.drawGlyph(x, y, 64);
      break;
    case RAIN:
      u8display.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case THUNDER:
      u8display.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case MIST:
      u8display.setFont(u8g2_font_open_iconic_other_6x_t);
      u8display.drawGlyph(x, y, 67);
      break;
    case SNOW:
      u8display.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8display.drawGlyph(x, y, 66);
      break;
  }
}

/***************************************************************************************
**                          Send weather info to serial port
***************************************************************************************/
void printCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  //OW_current *current = new OW_current;
  //OW_hourly *hourly = new OW_hourly;
  //OW_daily  *daily = new OW_daily;

  //time_t time;

  Serial.print("\nRequesting weather information from OpenWeather... ");

  ow.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);

  /**
    Serial.println("Weather from Open Weather\n");

    // We can use the timezone to set the offset eventually...
    Serial.print("Timezone            : "); Serial.println(current->timezone);

    Serial.println("############### Current weather ###############\n");
    Serial.print("dt (time)        : "); Serial.print(strTime(current->dt));
    Serial.print("sunrise          : "); Serial.print(strTime(current->sunrise));
    Serial.print("sunset           : "); Serial.print(strTime(current->sunset));
    Serial.print("temp             : "); Serial.println(current->temp);
    Serial.print("feels_like       : "); Serial.println(current->feels_like);
    Serial.print("pressure         : "); Serial.println(current->pressure);
    Serial.print("humidity         : "); Serial.println(current->humidity);
    Serial.print("dew_point        : "); Serial.println(current->dew_point);
    Serial.print("uvi              : "); Serial.println(current->uvi);
    Serial.print("clouds           : "); Serial.println(current->clouds);
    Serial.print("visibility       : "); Serial.println(current->visibility);
    Serial.print("wind_speed       : "); Serial.println(current->wind_speed);
    Serial.print("wind_gust        : "); Serial.println(current->wind_gust);
    Serial.print("wind_deg         : "); Serial.println(current->wind_deg);
    Serial.print("rain             : "); Serial.println(current->rain);
    Serial.print("snow             : "); Serial.println(current->snow);
    Serial.println();
    Serial.print("id               : "); Serial.println(current->id);
    Serial.print("main             : "); Serial.println(current->main);
    Serial.print("description      : "); Serial.println(current->description);
    Serial.print("icon             : "); Serial.println(current->icon);

    Serial.println();


      Serial.println("############### Hourly weather  ###############\n");
      for (int i = 0; i < MAX_HOURS; i++)
      {
        Serial.print("Hourly summary  "); if (i < 10) Serial.print(" "); Serial.print(i);
        Serial.println();
        Serial.print("dt (time)        : "); Serial.print(strTime(hourly->dt[i]));
        Serial.print("temp             : "); Serial.println(hourly->temp[i]);
        Serial.print("feels_like       : "); Serial.println(hourly->feels_like[i]);
        Serial.print("pressure         : "); Serial.println(hourly->pressure[i]);
        Serial.print("humidity         : "); Serial.println(hourly->humidity[i]);
        Serial.print("dew_point        : "); Serial.println(hourly->dew_point[i]);
        Serial.print("clouds           : "); Serial.println(hourly->clouds[i]);
        Serial.print("wind_speed       : "); Serial.println(hourly->wind_speed[i]);
        Serial.print("wind_gust        : "); Serial.println(hourly->wind_gust[i]);
        Serial.print("wind_deg         : "); Serial.println(hourly->wind_deg[i]);
        Serial.print("rain             : "); Serial.println(hourly->rain[i]);
        Serial.print("snow             : "); Serial.println(hourly->snow[i]);
        Serial.println();
        Serial.print("id               : "); Serial.println(hourly->id[i]);
        Serial.print("main             : "); Serial.println(hourly->main[i]);
        Serial.print("description      : "); Serial.println(hourly->description[i]);
        Serial.print("icon             : "); Serial.println(hourly->icon[i]);

        Serial.println();
      }


    Serial.println("###############  Daily weather  ###############\n");
    for (int i = 0; i < MAX_DAYS; i++)
    {
      Serial.print("Daily summary   "); if (i < 10) Serial.print(" "); Serial.print(i);
      Serial.println();
      Serial.print("dt (time)        : "); Serial.print(strTime(daily->dt[i]));
      Serial.print("sunrise          : "); Serial.print(strTime(daily->sunrise[i]));
      Serial.print("sunset           : "); Serial.print(strTime(daily->sunset[i]));

      Serial.print("temp.morn        : "); Serial.println(daily->temp_morn[i]);
      Serial.print("temp.day         : "); Serial.println(daily->temp_day[i]);
      Serial.print("temp.eve         : "); Serial.println(daily->temp_eve[i]);
      Serial.print("temp.night       : "); Serial.println(daily->temp_night[i]);
      Serial.print("temp.min         : "); Serial.println(daily->temp_min[i]);
      Serial.print("temp.max         : "); Serial.println(daily->temp_max[i]);

      Serial.print("feels_like.morn  : "); Serial.println(daily->feels_like_morn[i]);
      Serial.print("feels_like.day   : "); Serial.println(daily->feels_like_day[i]);
      Serial.print("feels_like.eve   : "); Serial.println(daily->feels_like_eve[i]);
      Serial.print("feels_like.night : "); Serial.println(daily->feels_like_night[i]);

      Serial.print("pressure         : "); Serial.println(daily->pressure[i]);
      Serial.print("humidity         : "); Serial.println(daily->humidity[i]);
      Serial.print("dew_point        : "); Serial.println(daily->dew_point[i]);
      Serial.print("uvi              : "); Serial.println(daily->uvi[i]);
      Serial.print("clouds           : "); Serial.println(daily->clouds[i]);
      Serial.print("visibility       : "); Serial.println(daily->visibility[i]);
      Serial.print("wind_speed       : "); Serial.println(daily->wind_speed[i]);
      Serial.print("wind_gust        : "); Serial.println(daily->wind_gust[i]);
      Serial.print("wind_deg         : "); Serial.println(daily->wind_deg[i]);
      Serial.print("rain             : "); Serial.println(daily->rain[i]);
      Serial.print("snow             : "); Serial.println(daily->snow[i]);
      Serial.println();
      Serial.print("id               : "); Serial.println(daily->id[i]);
      Serial.print("main             : "); Serial.println(daily->main[i]);
      Serial.print("description      : "); Serial.println(daily->description[i]);
      Serial.print("icon             : "); Serial.println(daily->icon[i]);

      Serial.println();

    }**/

  // Delete to free up space and prevent fragmentation as strings change in length
  //delete current;
  //delete hourly;
  //delete daily;
}

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}
