#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#define BRANCO 0xFFFF
#define PRETO 0x0000

bool identificado = false;
String nome = "Somador 4 bits";
String codigo = "74283";
int qtdPinos = 14;
String pinos[32];


void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(5000);
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setRotation(0);

  pinos[0]="aa";
  pinos[1]="bb";
  pinos[2]="cc";
  pinos[3]="dd";
  pinos[4]="ee";
  pinos[5]="ff";
  pinos[6]="gg";
  pinos[7]="hh";
  pinos[8]="ii";
  pinos[9]="jj";
  pinos[10]="kk";
  pinos[11]="ll";
  pinos[12]="mm";
  pinos[13]="nn";

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

  desenharChip(nome,codigo,qtdPinos, pinos);
  identificado = true;
}

void lerSerial()
{
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

void desenharChip(String nome, String codigo, int qtdPinos, String pinos[])
{
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


  int xRet = 90;
  int yRet = 40;
  int larguraRet = 60;
  int alturaRet = metade * espaco;

  tft.drawRect(xRet, yRet, larguraRet, alturaRet, BRANCO);


  for (int i = 0; i < metade; i++) {
    int py = yRet + i * espaco + 4;


    tft.drawRect(xRet - 14, py, 14, 14, BRANCO);


    tft.setTextSize(1);
    tft.setCursor(xRet - 11, py + 3);
    tft.print(i);


    tft.setCursor(xRet - 14 - (pinos[i].length() * 6) - 2, py + 3);
    tft.print(pinos[i].substring(0, 5));
  }


  for (int i = 0; i < metade; i++) {
    int pinoIdx = qtdPinos - 1 - i;
    int py = yRet + i * espaco + 4;


    tft.drawRect(xRet + larguraRet, py, 14, 14, BRANCO);


    tft.setTextSize(1);
    tft.setCursor(xRet + larguraRet + 2, py + 3);
    tft.print(pinoIdx);


    tft.setCursor(xRet + larguraRet + 16, py + 3);
    tft.print(pinos[pinoIdx].substring(0, 5));
  }
}

void loop()
{
  if (!identificado && Serial.available() > 0) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    if (linha == "DESENHO CHIP") {
      identificado = true;
      lerSerial();
      desenharChip(nome,codigo,qtdPinos, pinos);
    }
  }
}
