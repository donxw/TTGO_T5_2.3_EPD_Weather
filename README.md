# TTGO_T5_2.3_EPD_Weather
Adapts Bodmer OpenWeather example for use on the TTGO T5 EPD V2.3 ESP32 Dev Board
![Openweather_TTGO_T5 (Small)](https://user-images.githubusercontent.com/31633408/99468428-6a45f400-28f5-11eb-8221-ee64c49564c1.jpeg)

# Libraries used:
* https://github.com/ZinggJM/GxEPD2
* https://github.com/Bodmer/OpenWeather
* https://github.com/Bodmer/JSON_Decoder
* https://github.com/tzapu/WiFiManager
* https://github.com/olikraus/U8g2_for_Adafruit_GFX
* https://github.com/nhatuan84/esp32-micro-sdcard

# Hardware used:
* LILYGO® TTGO T5 V2.0 WiFi Wireless Module bluetooth Base ESP-32 ESP32 2.13 e-Paper Display Development Board
* https://www.banggood.com/LILYGO-TTGO-T5-V2_0-WiFi-Wireless-Module-bluetooth-Base-ESP-32-ESP32-2_13-e-Paper-Display-Development-Board-p-1332909.html?rmmds=search&cur_warehouse=CN
* https://github.com/lewisxhe/TTGO-EPaper-Series

# Software
Arduino software.  Need standard install for ESP32 and the libraries noted above.  There are two versions:
## 1)  TTGO_EPD2_My_OpenWeather_Test
This takes the example code provided in Bodmer's OpenWeather library and replaces the serial port output with a graphical output on the TTGO T5 2.13" EPD screen.  Everything is done in the setup.  The code goes into deep sleep at the end of setup and is set to auto-wake very 15 min to refresh the data and to update the EPD screen.  The built in button is also mapped to wake from sleep if a refresh before 15 min is needed.

Wifimanager proved to be unreliable.  It is normally great, but in an environment of waking from deep sleep, it fails to connect to the wifi roughly once every 20 resumes.  There is a setting in wifimanager to handle a timeout; however, for this project, using an SD card to store wifi credentials was preferred, so this was not debugged.

## 2)  TTGO_EPD2_My_OpenWeather_GPIO39Wake_SD
This is the same code as #1, except the wifi credentials and the location parameters are read from an SD card. This makes it easy to change those parameters without reflashing the code.  The build in SD card on the TTGO T5 dev board does not use default SPI pin mapping so using SD.h was problematic.  Using mySD.h was preferred since it allows choosing the SPI pins.  All other code for the SD card is identical to using SD.h.  

Everthing from the SD card (wifi credentials and weather location info) is stored to EEPROM so once it is read in, the SD card can be removed.  If a change is needed to update wifi or weather location, re-inserting the card with new info will auto update the EEPROM.

Note that instead of using a complex JSON formatted string, simple text files for each variable are used.  This is for ease of use by anyone wanting to update the data.  The biggest caution is to not end text entries in the text files with a carraige return.  Text read in with a carraige return will not be recognized by wifi.begin.  Windows notepad and notepad++ both work fine.  I have not tried any other editors.

## Issues
Code will run for days without hanging if using wifi.begin and credential values either hard coded, or text files read in from an SD card.  However, in deep sleep, the blue power LED stays on and draws 8ma.  The similar ESP32 TTGO T-display devboard turns off the power LED when in deep sleep and can last weeks in deep sleep using a small 350mah lipo.  The TTGO T5 EPD will only last 2 days on the same lipo in deep sleep.  Turning off the Power LED in sleep needs investigation.  The battery LED turns off in sleep mode.

Case for TTGO T5 is modified from a nice case found on Thingiverse.  The original has a very nice snap fit, but is lacking an SD card slot and buttons to activate the build in buttons on the TTGO T5 devboard.  The modified works, but is not a smooth snapping.  Investigation needed.

TZCode.txt can be ignored.  It is used in another version with an NTP clock displayed with the weather.

# Case for TTGO T5 2.13EPD
![OpenweatherTTGO_in_case (Small)](https://user-images.githubusercontent.com/31633408/99886177-67961800-2bef-11eb-90a1-991a03e8c340.jpeg)

Adapted this case:  https://www.thingiverse.com/thing:4055993 
Added button push levers, SD Card slot, thickness and bosses to hold TTGO up against lid.

   Copyright [2022] [don williams]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
