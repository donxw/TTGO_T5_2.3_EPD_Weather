//********************** for SD Card  **********************
//#include <SPI.h>  // Already in main code
//#define FS_NO_GLOBALS //allow spiffs to coexist with SD card, define BEFORE including FS.h - I couldn't make SPIFFS and SD work nice...
//#include <FS.h>
//#include <SD.h>  // won't work with the build in SD card reader on the TTGO T5 because I don't know how to reasign the SPI pins to non standard for ESP32
File myPasswordFile;
File mySSIDFile;
File myLongFile;
File myLatFile;
File myTZFile;
File myLocationFile;
File myUnitsFile;
File root;
String strPassword = " ";
String strSSID = " ";
char myPassword[20];
char mySSID[20];
char myLong[10];
char myLat[10];
char myTZValue[4] = "1";
char myLocation[40];
char myUnits[10];
//***********************************************************

//******************** for EEPROM ***********************
#include <EEPROM.h>
const char useEEPROMAddress = 0;  // Bool value, only one byte needed
const char SSIDAddress = 1;  // Allow room for 20 char so 1 through 20
const char PasswordAddress = 22;  //start at 22 to leave a couple buffer spaces.  22 through 41
const char LongAddress = 43;  // 43 through 52
const char LatAddress = 55;  //  55 through 64
const char TZValueAddress = 66;  //  66 through 69
const char LocationAddress = 71;  // 71 through 110
const char UnitsAddress = 115;  // 115 

bool useEEPROM = false;

//Declare EEPROM Functions
void writeString(char add, String data);
String read_String(char add);
//***********************************************************

// ******************* Functions ****************************
// EEPROM functions from circuits4you.com
void writeString(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String read_String(char add)
{
  //int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

void printDirectory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
