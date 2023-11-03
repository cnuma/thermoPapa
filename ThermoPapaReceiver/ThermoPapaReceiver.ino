/*********************************************************************************************************************
*         - 2023/11/02  - mise en place du repo sur Github
* - V0.3  - 2023/10/31  - connexion au site XML des données de météo France et parse du contenu du site.
* - V0.2  - 2023/10/31  - correction de l'affichage des graph; erreur des points 0 de référence - ajout des données RSSI et SNR on top de l'afficheur  
* - V0.1  - 2023/10/06  - Ajout gestion de l'écran et création de la classe mesure
* - V0    - 2023/10/05  - test module LORA 
*
*********************************
*  E.MALOSSE 2023 - #cNuma prod *
*********************************************************************************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <ArduinoOTA.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>	
#include <Fonts/Picopixel.h>	

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <time.h>
#include <Timezone.h>

#include <EEPROM.h>

#include <WiFiClientSecure.h>


#include "mesure.h"



// Var pour client NTP 
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;

int GTMOffset = 0; // SET TO UTC TIME
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", GTMOffset*60*60, 60*60*1000);
 
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);




// Weather API : https://api.open-meteo.com/v1/meteofrance?latitude=45.7485&longitude=4.8467&hourly=temperature_2m,weathercode&timezone=Europe%2FBerlin 
// Weather code: https://gist.github.com/stellasphere/9490c195ed2b53c707087c8c2db4ec0c
// Source  code: https://www.hackster.io/mheavers/parsing-xml-api-data-on-an-esp8266-with-arduino-ide-f35b6a

const char* host = "api.open-meteo.com";
String url = "/v1/meteofrance?latitude=45.7485&longitude=4.8467&hourly=temperature_2m,weathercode&timezone=Europe%2FBerlin";
const int httpsPort = 443;

bool awaitingArrivals = true;
bool arrivalsRequested = false;

int pollDelay = 30000; //time between each retrieval
int retryWifiDelay = 1000;

WiFiClientSecure client;


#define shiftVertical   80
#define positionGauche  10

#define xTemperature    positionGauche
#define yTemperature    50

#define xPression       positionGauche
#define yPression       yTemperature + shiftVertical

#define xHumidite       positionGauche
#define yHumidite       yPression + shiftVertical



// **** Définition des pin pour afficheur ILLI9341
#define TFT_DC D4
#define TFT_CS D2
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// **** Définition pour module LORA 
#define LoraNss 15
#define LoraReset 16
#define dio0 D1

int counter = 0;
String entrant;
int compteurTrame=0;

int16_t gbx1, gby1;
uint16_t gbw, gbh;

mesure pression     = mesure();
mesure temperature  = mesure();
mesure humidite     = mesure();


// *** Défintion du serveur associé à GP3 *****
#include "pageContact.h"
#include "pageWifiParam.h"

IPAddress local_IP(192,168,10,254);
IPAddress subnet(255,255,255,0);

#define EEPROM_SIZE 80 // *** 2 blocs de 40 caractères pour le SSI et le PWD associé ***

bool gp3=0;

ESP8266WebServer server(80);

//const char * wifi_ssid = "Freebox-manu"; 12
//const char * wifi_password = "nugor5-subduxi-factitando37-indemne";35
String wifi_ssid;
String wifi_password;




/**
 * Input time in epoch format and return tm time format
 * by Renzo Mischianti <www.mischianti.org> 
 */
static tm getDateTimeByParams(long time){
    struct tm *newtime;
    const time_t tim = time;
    newtime = localtime(&tim);
    return *newtime;
}
/**
 * Input tm time format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org>
 */
static String getDateTimeStringByParams(tm *newtime, char* pattern = (char *)"%d/%m/%Y %H:%M:%S"){
    char buffer[30];
    strftime(buffer, 30, pattern, newtime);
    return buffer;
}
 
/**
 * Input time in epoch format format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org> 
 */
static String getEpochStringByParams(long time, char* pattern = (char *)"%d/%m/%Y %H:%M:%S"){
//    struct tm *newtime;
    tm newtime;
    newtime = getDateTimeByParams(time);
    return getDateTimeStringByParams(&newtime, pattern);
}

/*
*/
void tftPrintMsg(int xPos, int yPos, String chaine, uint8_t texteTaille, uint16_t texteCouleur, const GFXfont *f) {
    tft.setTextColor(texteCouleur);  
    tft.setFont(f);
    tft.setTextSize(texteTaille);
    tft.getTextBounds(chaine, 0, 0, &gbx1, &gby1, &gbw, &gbh);
    tft.setCursor(xPos - (gbw/2), yPos);
    tft.println(chaine);
}



// *****************************************************************************************************************************************************
// ********************************************************************************* SETUP *************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println("ThermoPAPA - Module affichage"); 


  pinMode(3, INPUT);
  delay(200);

  EEPROM.begin(EEPROM_SIZE);
  
 
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(0);

  /*tft.setTextColor(ILI9341_WHITE);  
  tft.setFont(&FreeMonoBold18pt7b);
  tft.setTextSize(1);
  tft.getTextBounds("Thermo Papa", 0, 0, &gbx1, &gby1, &gbw, &gbh);
  tft.setCursor(120 - (gbw/2), 120);
  tft.println("Thermo Papa");
*/
  tftPrintMsg(120, 120, F("Thermo Papa"), 1, ILI9341_WHITE, &FreeMonoBold18pt7b);

  /*tft.setTextColor(ILI9341_WHITE);  
  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1);
  tft.getTextBounds("V0_2 - 2023-10", 0, 0, &gbx1, &gby1, &gbw, &gbh);
  tft.setCursor(120 - (gbw/2), 180);
  tft.println("V0_3 - 2023-11");
  */
  tftPrintMsg(120, 160, F("V0_3 - 2023-11"), 1, ILI9341_WHITE, &FreeSans9pt7b);


  // ************************************************************************************************************
  // ********************* Si BP GPIO3 est positionné à 1 au boot, lancement du serveur Web pour config wifi ****
  if (digitalRead(3) == 0) {

    gp3 = 1;

    /*
    tft.setTextColor(ILI9341_WHITE);  
    tft.setFont(&FreeSans9pt7b);
    tft.setTextSize(1);
    tft.getTextBounds("* Wifi setup *", 0, 0, &gbx1, &gby1, &gbw, &gbh);
    tft.setCursor(120 - (gbw/2), 180);
    tft.println(F("* Wifi setup *"));
    */

    WiFi.mode(WIFI_AP); //Our ESP8266-12E is an AccessPoint
    WiFi.softAP("Hello_cNuma", "12345678"); // Provide the (SSID, password);

    tftPrintMsg(120, 200, F("*** Wifi setup ***"), 1, ILI9341_YELLOW, &FreeSans9pt7b);
    tftPrintMsg(120, 240, F("Attente config !"), 1, ILI9341_WHITE, &FreeSans9pt7b);
    tftPrintMsg(120, 260, F("SSID: Hello_cNuma"), 1, ILI9341_ORANGE, &FreeSans9pt7b);
    tftPrintMsg(120, 280, F("pwd: 12345678"), 1, ILI9341_ORANGE, &FreeSans9pt7b);
    
    server.on("/", [] {server.send(200, "text/html", pageWifiParam);} );
    server.on("/contact", [] {server.send(200, "text/html", pageContact);} );

    server.begin(); // Start the HTTP Server
    


  } else {
    // *************************************************************************************************
    // *************************************************************************************************
    // ************************************************** Démarrage NORMAL *****************************
    // *************************************************************************************************


    // *******************************************************************************************************************
    // ********************************************************************************************** Affichage checks 
    int yPosSetup = 200;
    tftPrintMsg(120, yPosSetup, F("Init Objets"), 1, ILI9341_CYAN, &FreeSans9pt7b);
    yPosSetup += 20;
    tftPrintMsg(120, yPosSetup, F("Init LORA:"), 1, ILI9341_ORANGE, &FreeSans9pt7b);
    yPosSetup += 20;
    tftPrintMsg(120, yPosSetup, F("Init WIFI"), 1, ILI9341_PINK, &FreeSans9pt7b);
    yPosSetup += 20;
    tftPrintMsg(120, yPosSetup, F("Init NTP"), 1, ILI9341_YELLOW, &FreeSans9pt7b);

    delay(800);
    
    


    // *******************************************************************************************************************
    // ***************************************************************************************************** Init Objets
    yPosSetup = 200;

    // ****************************************** TEMPERATURE **********************
    temperature.SetPosition(xTemperature, yTemperature);
    temperature.SetBGColor(ILI9341_DARKCYAN);
    temperature.SetUnit (" C");
    temperature.SetMinMax(-15, 45);

    // ****************************************** PRESSION **********************
    pression.SetPosition(xPression, yPression);
    pression.SetBGColor(ILI9341_BLUE);
    //pression.SetUnit (" Mb");
    pression.SetMinMax(600, 1050);

    // ****************************************** HUMIDITE **********************
    humidite.SetPosition(xHumidite, yHumidite);
    humidite.SetBGColor(ILI9341_ORANGE);
    humidite.SetUnit(" %");
    humidite.SetMinMax(0, 100);

    tftPrintMsg(200, yPosSetup, F("Ok !"), 1, ILI9341_GREEN, &FreeSans9pt7b);
    delay(500);


    // *******************************************************************************************************************
    // ***************************************************************************************************** Init LORA
    yPosSetup += 20;
    while (!Serial);
    Serial.println("LoRa Receiver");
    LoRa.setPins(LoraNss, LoraReset, dio0);

      if (!LoRa.begin(433E6)) {
      Serial.println("Starting LoRa failed!");
      tftPrintMsg(200, yPosSetup, F("Failed !"), 1, ILI9341_RED, &FreeSans9pt7b);
      delay(100);
      while (1);
    } else {
      Serial.println("Starting LoRa module OK!");
      tftPrintMsg(200, yPosSetup, F("Ok !"), 1, ILI9341_GREEN, &FreeSans9pt7b);
      Serial.println("LoRa Receiver mode");
      LoRa.receive();
    }
    delay(500);

    // register the receive callback
    //LoRa.onReceive(onReceive);
  
    // put the radio into receive mode
    
    // *******************************************************************************************************************
    // ***************************************************************************************************** Init WIFI
    yPosSetup += 20;
    Serial.printf("Connecting to '%s'\n", wifi_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      tftPrintMsg(200, yPosSetup, F("Ok !"), 1, ILI9341_GREEN, &FreeSans9pt7b);
      //Serial.print("Connected. IP: ");
      //Serial.println(WiFi.localIP());
      ArduinoOTA.setHostname("ThemoPapa-ModuleAffichage");
      ArduinoOTA.setPassword("123456");
      ArduinoOTA.begin();
    } else {
      Serial.println("Connection Failed!");
      tftPrintMsg(200, yPosSetup, F("Failed !"), 1, ILI9341_RED, &FreeSans9pt7b);
    }
    delay(500);


    // *******************************************************************************************************************
    // ***************************************************************************************************** Init NTP 
    yPosSetup += 20; 
    timeClient.begin();
    delay ( 1000 );
    if (timeClient.update()){
      Serial.print ( "Adjust local clock" );
      tftPrintMsg(200, yPosSetup, F("Ok !"), 1, ILI9341_GREEN, &FreeSans9pt7b);
      unsigned long epoch = timeClient.getEpochTime();
      setTime(epoch);
    }else{
      Serial.print ( "NTP Update not WORK!!" );
      tftPrintMsg(200, yPosSetup, F("Failed !"), 1, ILI9341_RED, &FreeSans9pt7b);
    }
    delay(500);


    tftPrintMsg(120, 310, F("Fin setup"), 1, ILI9341_WHITE, &FreeSans9pt7b);
    delay(5000);
    tft.fillScreen(ILI9341_BLACK);

  } // *** Else du IF si le BP3 est appuyé ***



  Serial.println("end setup - start loop");

} // *** Setup ***


void loop(void) {

  if (gp3 == 1) {
    // *************************************** MODE GP3 - CONFIG SSID Et PWD  *********
    
    server.handleClient();



  } else {

    // *************************************** MODE NORMAL ****************************
    ArduinoOTA.handle();

    //timeClient.update();
    Serial.println("Time: " + getEpochStringByParams(CE.toLocal(now())));

    // ***** Lecture des données de openMeteo ******
    if (awaitingArrivals) {
      if (!arrivalsRequested) {
        arrivalsRequested = true;
        getArrivals();
      }
    }

    //Serial.println("Lora - ParsePaquet");
    int tailleDuPaquet = LoRa.parsePacket(); 
    entrant = "";

    if (tailleDuPaquet) { 

      Serial.println("taille du paquet: " + String(tailleDuPaquet));
    
      Serial.println("Lora - Lecture reception");
      while (LoRa.available()) { 
        entrant += (char)LoRa.read(); 
      } 

      Serial.print("=== LORA SIGNAL - RSSI: ");
      Serial.print(LoRa.packetRssi()); // *** valeurs comprises entre -120dBm(weak) et -30dBm(strong)
      Serial.print(" - SNR: ");
      Serial.println(LoRa.packetSnr()); // *** valuers comprises entre -20dB et +10dB
      

      Serial.println("Entrant: " + entrant);

      // **************************************** Detection d'une trame #cnuma### ***************************
      if (entrant.indexOf("cnuma###") > 0) {

        // ********************************************************************
        // *** Affichage du niveau de réception du signal 
        tft.fillRect(0, 0, 100, 20, ILI9341_BLACK);
        tft.setFont(&Picopixel);

        /*
        => https://lora.readthedocs.io/en/latest/#rssi


        RSSI:
        -------------------------------------------------------
        The RSSI is measured in dBm and is a negative value.
        The closer to 0 the better the signal is.
        Typical LoRa RSSI values are:

        => RSSI minimum = -120 dBm.

        If RSSI=-30dBm: signal is strong.
        If RSSI=-120dBm: signal is weak.
        
        
        SNR:
        -------------------------------------------------------
        Signal-to-Noise Ratio (SNR) is the ratio between the received power signal and the noise floor power level.

        The noise floor is an area of all unwanted interfering signal sources which can corrupt the transmitted signal and therefore re-transmissions will occur.

        If SNR is GREATER than 0, the received signal operates above the noise floor.
        If SNR is SMALLER than 0, the received signal operates below the noise floor.

        Normally the noise floor is the physical limit of sensitivity, however LoRa works below the noise level.

        => Typical LoRa SNR values are between: -20dB and +10dB

        A value closer to +10dB means the received signal is less corrupted.
        LoRa can demodulate signals which are -7.5 dB to -20 dB below the noise floor.

        */

        // **** RSSI ****
        tft.setCursor(5, 10);
        tft.setTextSize(1);
        int RSSI = map(LoRa.packetRssi(), -120, -30, 0, 100);
        if (RSSI >= 61) {
          tft.setTextColor(ILI9341_GREEN);
        } else if ((RSSI >= 31) && (RSSI<=60)) {
          tft.setTextColor(ILI9341_YELLOW);
        } else if (RSSI <= 30) {
          tft.setTextColor(ILI9341_RED);
        }
        tft.print("RSSI: " + String(RSSI) + " %");


        // **** SNR ****
        tft.setCursor(50, 10);
        tft.setTextSize(1);
        int SNR = map(LoRa.packetSnr(), -20, 10, 0, 100);
        if (SNR >= 61) {
          tft.setTextColor(ILI9341_GREEN);
        } else if ((RSSI >= 31) && (RSSI<=60)) {
          tft.setTextColor(ILI9341_YELLOW);
        } else if (RSSI <= 30) {
          tft.setTextColor(ILI9341_RED);
        }
        tft.print("SNR: " + String(SNR) + " %");

        String cpt = entrant.substring(9, entrant.indexOf("###",9));

        if (compteurTrame != cpt.toInt()) {
          Serial.println("## Nouvelle trame " + cpt + "- Traitement ...");
          compteurTrame = cpt.toInt();

          tft.fillRect(280, 230, 100, 20, ILI9341_BLACK);
          tft.setFont(&Picopixel);
          tft.setCursor(280, 235);
          tft.setTextColor(ILI9341_WHITE);  
          tft.setTextSize(1);
          tft.println(cpt);

            // ****************************************** TEMPERATURE **********************
          if (entrant.indexOf("Temperature") > 0) {
            Serial.println("Affichage Temperature");

            entrant = entrant.substring(entrant.indexOf("Temperature:")+12);
            temperature.NewValue(entrant.toFloat());

            temperature.PrintLastValue();
            temperature.GraphHistoryValue(xTemperature + 100, yTemperature + 20);

          }

          // ****************************************** PRESSION **********************
          if (entrant.indexOf("Pression") > 0) {
            Serial.println("Affichage Pression");
          
            entrant = entrant.substring(entrant.indexOf("Pression:")+10);
            pression.NewValue(entrant.toFloat());
          
            pression.PrintLastValue(); 
            pression.GraphHistoryValue(xPression + 100, yPression + 20); 
          }

          // ****************************************** HUMIDITE **********************   
          if (entrant.indexOf("Humidite") > 0) {
            entrant = entrant.substring(entrant.indexOf("Humidite:")+10);
            humidite.NewValue(entrant.toFloat());
          
            humidite.PrintLastValue();
            humidite.GraphHistoryValue(xHumidite + 100, yHumidite + 20); 
          }
        } else {
          Serial.println("MEME TRAME - compteurTrame: " + String(compteurTrame) + " - cpt: " + String(cpt));
        }

      } else {
        Serial.println ("*** Pas une trame CNUMA !!!");
      }

      
    } // *** Détection d'un paquet ***


    Serial.print(".");
    delay(50);
  } // *** Else du GP3 - Fonctionnement NORMAL ! ***

} // *** Loop ***




void getArrivals() {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    delay(retryWifiDelay);
    return;
  }

  //Query the API
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: BuildFailureDetectorESP8266\r\n" +
                "Connection: close\r\n\r\n");
  Serial.println("request sent");
  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String matchString = "";
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line.endsWith("</resultSet>")) { //this signals the end of the response from XML API
      matchString = line;
    }
  }
  Serial.println(matchString); //log result xml response from server
  awaitingArrivals = false;
 
  client.stop();
}