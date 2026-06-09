#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#define BRANCO 0xFFFF
#define PRETO 0x0000

bool identificado = false;
String nome = "";
String codigo = "";
int qtdPinos = 0;
String pinos[32];

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10000);
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setRotation(0);

  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("Iniciando...");
  delay(3000);

  tft.fillScreen(PRETO);
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print("Insira o chip");
  tft.setCursor(20, 110);
  tft.print("e pressione");
  tft.setCursor(20, 140);
  tft.print("o botao");
}

void lerSerial() {
  String linhaQtd = Serial.readStringUntil('\n');
  linhaQtd.trim();
  qtdPinos = linhaQtd.toInt();

  nome = Serial.readStringUntil('\n');
  nome.trim();

  codigo = Serial.readStringUntil('\n');
  codigo.trim();

  Serial.println("Lido: " + String(qtdPinos) + " | " + nome + " | " + codigo);

  for (int i = 0; i < qtdPinos; i++) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    int sep = linha.indexOf(':');
    if (sep >= 0) {
      pinos[i] = linha.substring(sep + 1);
    }
    Serial.println("Pino " + String(i) + ": " + pinos[i]);
  }
}

void desenharChip() {
  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);


  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print(nome);


  tft.setTextSize(1);
  tft.setCursor(5, 25);
  tft.print(codigo);

  int metade = qtdPinos / 2;


  int espaco = 20;


  int retX = 90;
  int retY = 40;
  int retW = 60;
  int retH = metade * espaco;

  tft.drawRect(retX, retY, retW, retH, BRANCO);


  for (int i = 0; i < metade; i++) {
    int py = retY + i * espaco + 4;


    tft.drawRect(retX - 14, py, 14, 14, BRANCO);


    tft.setTextSize(1);
    tft.setCursor(retX - 11, py + 3);
    tft.print(i);


    tft.setCursor(retX - 14 - (pinos[i].length() * 6) - 2, py + 3);
    tft.print(pinos[i].substring(0, 5));
  }


  for (int i = 0; i < metade; i++) {
    int pinoIdx = qtdPinos - 1 - i;
    int py = retY + i * espaco + 4;


    tft.drawRect(retX + retW, py, 14, 14, BRANCO);


    tft.setTextSize(1);
    tft.setCursor(retX + retW + 2, py + 3);
    tft.print(pinoIdx);


    tft.setCursor(retX + retW + 16, py + 3);
    tft.print(pinos[pinoIdx].substring(0, 5));
  }
}

void loop() {
  if (!identificado && Serial.available() > 0) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    if (linha == "DESENHO CHIP") {
      identificado = true;
      lerSerial();
      desenharChip();
    }
  }
}
