/*********************************************************************************************************************
*
*  - V2.1   - 2023/10/31 - Ajout de la mesure de la tension du module et envoie dans la trame LORA  
*  - V2     - 2023/10/31 - Nouvelle librairie pour gestion du BME280 - Ajout du DeepSleep
*  - V1     - 2023/10/15 - PB avec le mode deepSleep
*  - V0     - 2023/10/10 - ajout module BME280
*  - V0     - 2023/10/05 - test module LORA 
*
*********************************
*  E.MALOSSE 2023 - #cNuma prod *
*********************************************************************************************************************/

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <LoRa.h> 

#include <Wire.h>
#include <BMx280I2C.h>

#include <RTCVars.h>
RTCVars stateVar;



ADC_MODE(ADC_VCC);    // *** Lecture de la tension d'alimentation de l'ESP - A0 doit rester volant ! ***

#define I2C_ADDRESS 0x76
BMx280I2C bmx280(I2C_ADDRESS);


bool state;
int cpt;

#define LoraNss 15
#define LoraReset A0
#define dio0 0

#define adresseI2CduBME280                0x76


// Wi-Fi connection parameters



void setup() { 
  Serial.begin(115200); 

  Serial.println(F("******************* SETUP ****************"));

  // ********************************************************************************************************************
  Serial.println(F("*** Init var - converse valeur de la variable cpt suite au reboot du deepSleep ***"));
  stateVar.registerVar( &cpt );
  stateVar.loadFromRTC();


  // ********************************************************************************************************************
  Serial.println(F("*** Init BME280 ***"));
  Wire.begin(D2, D1);
  Serial.print("Init BME280: ");
  if (!bmx280.begin())
	{
		Serial.println(" --> ECHEC");
		while (1);
	} else  {
    Serial.print(" --> REUSSI ");
  }

	if (bmx280.isBME280())
		Serial.println("- sensor is a BME280");
	else
		Serial.println("- sensor is a BMP280");

  bmx280.resetToDefaults();
  bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
	bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);

	//if sensor is a BME280, set an oversampling setting for humidity measurements.
	if (bmx280.isBME280()) {
    bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
  }
		

  // ********************************************************************************************************************
  Serial.println(F("*** Init Diode de visualisation ***"));
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);


  // ********************************************************************************************************************
  Serial.println(F("*** Init LORA Module ***"));
  LoRa.setPins(LoraNss, LoraReset, dio0);


  // ********************************************************************************************************************
  Serial.print(F("*** LORA Begin "));
  if (!LoRa.begin(433E6)) { 
    Serial.println(F("--> ÉCHEC…")); 
    return; 
  } else {
    Serial.println(F("--> RÉUSSIE !"));
  }

/*
  // ********************************************************************************************************************
  Serial.println(F("*** Init WIFI ***"));
  Serial.printf("Connecting to '%s' \n", wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.print(" -> Connected. IP: ");
    Serial.println(WiFi.localIP());
    ArduinoOTA.setHostname("ThemoPapaModuleMesure");
    ArduinoOTA.setPassword("123456");
    ArduinoOTA.begin();
  } else {
    Serial.println(" -> Connection Failed!");
  }
*/


} 

void loop() { 

  //ArduinoOTA.handle();

  if (!bmx280.measure()) {
		Serial.println("could not start measurement, is a measurement already running?");
		return;
	}

  do {
		delay(100);
	} while (!bmx280.hasValue());


  delay(2000);
  sendFrame ("Temperature", String(bmx280.getTemperature()));

  delay(2000);
  sendFrame ("Humidite", String(bmx280.getHumidity()));
 
  delay(2000);
  sendFrame ("Pression", String(bmx280.getPressure()/100.0F));

  
  stateVar.saveToRTC();
  Serial.println("Départ en deepSleep pour 30 minutes !");
  ESP.deepSleep( 30 * 60 * 1000000 ); 
  //ESP.deepSleep( 10 * 1000000 );
}


// ************************************************ Envoi de la trame via LORA *********************
void sendFrame (String source, String valeur) {

  String message;

  message = "#cnuma###" + String(cpt++);
  message += "### - ";
  message += source;
  message += ": " + valeur;
  message += " batterie:" + String(ESP.getVcc()/1000);
  Serial.println(message);

  state = 1;
  for (int i=0; i<=2; i++) {
    LoRa.beginPacket(); 
    LoRa.print(message); 
    LoRa.endPacket();
    Serial.println("Envoi du message: " + message);

    Serial.println("Led High");
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    Serial.println("Led Low");
    digitalWrite(LED_BUILTIN, HIGH);

    delay(2000);
  }

}