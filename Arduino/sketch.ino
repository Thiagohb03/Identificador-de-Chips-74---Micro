#include<Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;
bool identificado = false;

void setup()
{
  Serial.begin(9600);

  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setRotation(0);


  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(3);
  tft.setCursor(20, 150);
  tft.print("Iniciando...");
  delay(3000);


  tft.fillScreen(0x0000);
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print("Insira o chip");
  tft.setCursor(20, 110);
  tft.print("e pressione");
  tft.setCursor(20, 140);
  tft.print("o botao");

}

void loop()
{
  if (!identificado && Serial.available() > 0)
    {
      char cmd = Serial.read();
      if (cmd == '1') 
          {
            identificado = true;

            tft.fillScreen(0x0000);
            tft.setTextColor(0xFFFF);
            tft.setTextSize(3);
            tft.setCursor(20, 150);
            tft.print("Buscando...");
            delay(3000);

            tft.fillScreen(0x0000);
            tft.setTextSize(2);
            tft.setCursor(20, 80);
            tft.print("Chip");
            tft.setCursor(20, 110);
            tft.print("identificado!");
            tft.setTextSize(4);
            tft.setCursor(60, 160);
            tft.print("7408");
          }
    }

}
