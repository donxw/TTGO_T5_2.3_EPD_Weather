/*

    [units] should be one of the following:

    metric
    imperial

    lang=[language] optional

    Return summary properties in the desired language.
    (Note that units in the summary will be set according to the units parameter,
    so be sure to set both parameters appropriately.) language may be:

    af: Africans
    ar: Arabic
    az: Azerbaijani
    bg: Bulgarian
    ca: Catalan
    cs: Czech
    da: Danish
    de: German
    el: Greek
    en: English (which is the default)
    eu: Basque
    fa: Persian (Farsi)
    fi: Finnish
    fr: French
    gl: Galician
    he: Hebrew
    hi: Hindi
    hr: Croatian
    hu: Hungarian
    id: Indonesian
    it: Italian
    ja: Japanese
    kr: Korean
    la: Latvian
    lt: Lithuanian
    mk: Macedonian
    no: Norwegian Bokmål
    nl: Dutch
    pl: Polish
    pt: Portuguese
    pt_br: Português Brasil
    ro: Romanian
    ru: Russian
    sk: Slovak
    sl: Slovenian
    sp, es: Spanish
    sr: Serbian
    sv, se: Swedish
    th: Thai
    tr: Turkish
    ua, uk: Ukrainian
    vi: Vietnamese
    zh_cn: Chinese Simplified
    zh-tw: Chinese Traditional
    zu: Zulu

    some reference code
https://www.codingunit.com/printf-format-specifiers-format-conversions-and-formatted-output
%d (print as a decimal integer)
%6d (print as a decimal integer with a width of at least 6 wide)
%f (print as a floating point)
%4f (print as a floating point with a width of at least 4 wide)
%.4f (print as a floating point with a precision of four characters after the decimal point)
%3.2f (print as a floating point at least 3 wide and a precision of 2)

//zoomkat 11-12-13 String capture and parsing 
//from serial port input (via serial monitor)
//and print result out serial port
//copy test strings and use ctrl/v to paste in
//serial monitor if desired
// * is used as the data string delimiter
// , is used to delimit individual data

String readString; //main captured String
String angle; //data String
String fuel;
String speed1;
String altidude;

int ind1; // , locations
int ind2;
int ind3;
int ind4;
 
void setup() {
  Serial.begin(9600);
  Serial.println("serial delimit test 11-12-13"); // so I can keep track of what is loaded
}

void loop() {

  //expect a string like 90,low,15.6,125*
  //or 130,hi,7.2,389*

  if (Serial.available())  {
    char c = Serial.read();  //gets one byte from serial buffer
    delay(3);  //small delay to allow input buffer to fill
    if (c == '*') {
      //do stuff
     
      Serial.println();
      Serial.print("captured String is : ");
      Serial.println(readString); //prints string to serial port out
     
      ind1 = readString.indexOf(',');  //finds location of first ,
      angle = readString.substring(0, ind1);   //captures first data String
      ind2 = readString.indexOf(',', ind1+1 );   //finds location of second ,
      fuel = readString.substring(ind1+1, ind2+1);   //captures second data String
      ind3 = readString.indexOf(',', ind2+1 );
      speed1 = readString.substring(ind2+1, ind3+1);
      ind4 = readString.indexOf(',', ind3+1 );
      altidude = readString.substring(ind3+1); //captures remain part of data after last ,

      Serial.print("angle = ");
      Serial.println(angle);
      Serial.print("fuel = ");
      Serial.println(fuel);
      Serial.print("speed = ");
      Serial.println(speed1);
      Serial.print("altidude = ");
      Serial.println(altidude);
      Serial.println();
      Serial.println();
     
      readString=""; //clears variable for new input
      angle="";
      fuel="";
      speed1="";
      altidude="";
    } 
    else {     
      readString += c; //makes the string readString
    }
  }
}



*/
