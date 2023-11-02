#include <Wire.h>
#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>	
#include <Fonts/Picopixel.h>	


class mesure {

    private:

      #define m_TFT_DC D4
      #define m_TFT_CS D2

      Adafruit_ILI9341 m_tft;


      int   m_Xpos;
      int   m_Ypos;

      int   m_MesureMinValue;
      int   m_MesureMaxValue;

      int   m_GraphHeight = 50;

      //const GFXfont   &m_fontValue;
      uint16_t   m_BGColor;
      
      String m_unit;

      int   m_textColor;
      byte  m_textSize;

      float m_tabMesure[10];
      float m_lastMesure;

      #define NbrPointsHistoriqueMesure 96
      float m_fullMesure[NbrPointsHistoriqueMesure];

      void AddHistoryValue(float lastValue);

      void Tendance(int val, int previousVal, int xPos, int yPos);



    public: 
      // *** constructeur par défaut ***
      mesure();
      
      //void SetFont(uint16_t colorValue);
      void SetBGColor (uint16_t colorValue);
      void SetUnit (String unit);
      void SetMinMax(int min, int max);

      void SetPosition(int x, int y); 
      void NewValue(float value);
      String GetLastValue();

      void PrintLastValue();

      void GraphHistoryValue(int xPos, int Ypos);

};