#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>	
#include <Fonts/FreeSans12pt7b.h>	
#include <Fonts/Picopixel.h>	

#include "mesure.h"

// *** Constructeur ***

mesure::mesure() : m_Xpos(0), m_Ypos(0), m_tft(m_TFT_CS, m_TFT_DC) {
  m_tft.begin();
  //m_tft.setRotation(1);
}

void mesure::SetPosition (int x, int y) {
  m_Xpos = x;
  m_Ypos = y;
}


void mesure::SetBGColor(uint16_t BGColor ){
  m_BGColor = BGColor;
}

void mesure::SetUnit( String unit) {
  m_unit = unit;
} 

void mesure::SetMinMax(int minVal, int maxVal) {
  /*if (minVal >=0 ) {
    minVal = 0;
  }*/
  m_MesureMinValue=minVal;
  m_MesureMaxValue=maxVal;

  for (int i=0; i<=NbrPointsHistoriqueMesure-1;  i++){
    m_fullMesure[i] = minVal;
  }
}

void mesure::NewValue(float value){
  Serial.println("Ajout nouvelle valeur");
  m_lastMesure = value;

  Serial.println("Appel addHistoryValue");
  AddHistoryValue(value);

}

void mesure::AddHistoryValue(float value){
  int i;

  for (i=NbrPointsHistoriqueMesure - 1; i>=1; i--) {
    m_fullMesure[i] = m_fullMesure[i-1];
    if (m_fullMesure[i] >= m_MesureMax) {
      m_MesureMax = m_fullMesure[i];
    } else if (m_fullMesure[i] < m_MesureMin) {
      m_MesureMin = m_fullMesure[i];
    }
  }

  m_fullMesure[0] = value;
}



void mesure::Tendance(int val, int previousVal, int xPos, int yPos) {

  #define tailleTendance 20 // *** nombre de pixels pour le carré d'affichage de la tendance ***

  int refxPos = xPos + NbrPointsHistoriqueMesure + 2;
  int refyPos = yPos - (m_GraphHeight/2) + 10 ;
  

  m_tft.fillRect(refxPos, refyPos, tailleTendance, tailleTendance, ILI9341_DARKGREY);

  if (val == previousVal) {
    m_tft.drawLine(refxPos, refyPos - (tailleTendance / 2 ), refxPos + (tailleTendance/2), refyPos , ILI9341_YELLOW);
    m_tft.drawLine(refxPos, refyPos - 1 - (tailleTendance / 2 ), refxPos + (tailleTendance/2), refyPos , ILI9341_YELLOW);
    m_tft.drawLine(refxPos, refyPos + 1 - (tailleTendance / 2 ), refxPos + (tailleTendance/2), refyPos , ILI9341_YELLOW);
  } else if (val < previousVal) {
    m_tft.drawLine(refxPos, refyPos - (tailleTendance / 2 ), refxPos + (tailleTendance/2), refyPos + tailleTendance, ILI9341_BLUE);
  } else {
    m_tft.drawLine(refxPos, refyPos - (tailleTendance / 2 ), refxPos + (tailleTendance/2), refyPos - tailleTendance , ILI9341_RED);
  }


}


void mesure::GraphHistoryValue(int xPos, int yPos){

  int i;
  int16_t gbx1, gby1;
  uint16_t gbw, gbh;
  
  //m_tft.getTextBounds(String(m_lastMesure) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
  
  // *** Clear du précédent graph ***
  Serial.println("Clear Graph précédent");
  m_tft.fillRect(xPos, yPos - m_GraphHeight, NbrPointsHistoriqueMesure, m_GraphHeight, ILI9341_BLACK);
  

  // *** Génération du "T" du graphique ***
  // *** Trace ligne horizontale ***
  Serial.println("Génération T - Horizontal");
 
  if (m_MesureMinValue < 0 ) {
    m_tft.drawFastHLine(xPos, yPos - (abs(m_MesureMinValue)) , NbrPointsHistoriqueMesure, ILI9341_LIGHTGREY);
    
    m_tft.setTextColor(ILI9341_WHITE);  
    m_tft.setTextSize(1);
    m_tft.setFont(&Picopixel);
    m_tft.setCursor(xPos - 9, yPos + 2 - (abs(m_MesureMinValue)));
    m_tft.print("0");

    m_tft.getTextBounds(String(m_MesureMinValue) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
    m_tft.setCursor(xPos - gbw - 2 , yPos + 2);
    m_tft.print(m_MesureMinValue);


  } else {
    m_tft.drawFastHLine(xPos, yPos , NbrPointsHistoriqueMesure, ILI9341_LIGHTGREY);

    m_tft.setTextColor(ILI9341_WHITE);  
    m_tft.setTextSize(1);
    m_tft.setFont(&Picopixel);
    m_tft.getTextBounds(String(m_MesureMinValue) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
    m_tft.setCursor(xPos - gbw - 2 , yPos + 2 );
    m_tft.print(m_MesureMinValue);
  }

  m_tft.setTextColor(ILI9341_WHITE);  
  m_tft.setTextSize(1);
  m_tft.setFont(&Picopixel);
  m_tft.getTextBounds(String(m_MesureMaxValue) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
  m_tft.setCursor(xPos - gbw - 2 , yPos - m_GraphHeight );
  m_tft.print(m_MesureMaxValue);



/*

  if (m_MesureMin > 0 ) {
    m_tft.drawFastHLine(xPos, yPos, NbrPointsHistoriqueMesure, ILI9341_LIGHTGREY);

    m_tft.setTextColor(ILI9341_WHITE);  
    m_tft.setTextSize(1);
    m_tft.setFont(&Picopixel);
    m_tft.getTextBounds("0" + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
    m_tft.setCursor(xPos - gbw - 2 , yPos + 2 );
    m_tft.print("0");
  } else {

  }


  // *** Affichage de la valeur MAX en vertical ***
  m_tft.setTextColor(ILI9341_WHITE);  
  m_tft.setTextSize(1);
  m_tft.setFont(&Picopixel);
  m_tft.getTextBounds(String(m_MesureMax) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
  m_tft.setCursor(xPos - gbw - 2 , yPos - m_GraphHeight );
  m_tft.print(m_MesureMax);
*/

  
  
  // *** Trace lignes verticale du T
  Serial.println("Génération du T - Barres verticales");
  for (i=0; i<=3; i++) {

    m_tft.drawFastVLine(xPos+(NbrPointsHistoriqueMesure/4*i), yPos-m_GraphHeight, m_GraphHeight, ILI9341_MAROON);

    m_tft.setTextColor(ILI9341_WHITE);  
    m_tft.setTextSize(1);
    m_tft.setFont(&Picopixel);
    m_tft.setCursor(xPos+(NbrPointsHistoriqueMesure/4*i)-4, yPos+9);
    if (i==0) {
      m_tft.print("-48");  
    } else {
      m_tft.print("-" + String(48-12*i));
    }
    

  }

  // *** dessine le graphe ***
  Serial.println("Dessine le Graph !");
  int val;
  int previousVal;
  for (i=0; i<=NbrPointsHistoriqueMesure-1;  i++){

    // *** MAP les valeurs stockées dans le tableau pour l'adapter au graphique ***
    //val = map(int(m_fullMesure[i]), m_MesureMinValue, m_MesureMaxValue, 0, m_GraphHeight);
    Serial.println("MAP pour i=" + String (i) + " - m_fullMesure[i]=" + String(m_fullMesure[i]));
    val = map(int(m_fullMesure[i]), m_MesureMinValue, m_MesureMax, 0, m_GraphHeight);
  
    Serial.println("Mesure: " + m_unit + " ! xpos+i: " + String(xPos+i) + " ! Val"+String(i)+ "-"+String(m_fullMesure[i])+"->"+String(val) + " ! yPos origine: " + String(yPos) + " ! PosY corrigé: " + String(yPos-val));
    if (i==0){
      m_tft.drawPixel(xPos + NbrPointsHistoriqueMesure - i, yPos-val, m_BGColor);
    } else {
      //m_tft.drawLine(xPos + NbrPointsHistoriqueMesure - i - 1, previousVal, xPos + NbrPointsHistoriqueMesure - i, val, m_BGColor);
      m_tft.drawPixel(xPos + NbrPointsHistoriqueMesure - i, yPos-val, m_BGColor);
    }

    Tendance(val, previousVal, xPos, yPos);

    previousVal = val;
  } // *** FOR *** 


}

// **************************************************
String mesure::GetLastValue(){
  return String(m_lastMesure);
}


// **************************************************
void mesure::PrintLastValue(){
  int16_t gbx1, gby1;
  uint16_t gbw, gbh;

  m_tft.setTextColor(ILI9341_WHITE);  
  m_tft.setTextSize(1);
  m_tft.setFont(&FreeSans12pt7b);
  
  m_tft.getTextBounds(String(m_lastMesure) + m_unit, m_Xpos, m_Ypos, &gbx1, &gby1, &gbw, &gbh);
  //m_tft.setCursor(m_Xpos-gbw/2, m_Ypos);

  m_tft.fillRoundRect(gbx1-5, gby1-5, gbw+10, gbh+10, 5, m_BGColor);
  
  m_tft.setCursor(m_Xpos, m_Ypos);
  m_tft.print(String(m_lastMesure) + m_unit);

  //m_tft.setCursor(m_Xpos+gbw+10, m_Ypos);
  //m_tft.print("C");



}

