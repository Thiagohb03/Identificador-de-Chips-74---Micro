#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <EEPROM.h>

MCUFRIEND_kbv tft;

#define BRANCO 0xFFFF
#define PRETO 0x0000

String nome = "";
String codigo = "";
int qtdPinos = 0;
String pinos[32];

typedef struct{
  byte valorEsperado[16];
} Teste;

typedef struct {
  char nome[21];
  char codigo[8];
  int qtdPinos;
  char nomPinos[16][3];
  byte direita[8];
  byte esquerda[8];
  int qtdTestes;
  Teste testes[5];
}Chip;

const int MAX_CHIPS=(4096-1)/sizeof(Chip);

int totalChips=0;

void salvarChip(Chip c){
  if (totalChips>=MAX_CHIPS){
    Serial.println("ERRO: Banco cheio");
    return;
  }
  int endereco =1+(totalChips*sizeof(Chip));
  EEPROM.put(endereco,c);
  totalChips++;
  EEPROM.put(0,totalChips);
  Serial.println(totalChips);
}

void setup() {
  Serial.begin(9600);
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

//CHIP:74283:Somador:14:A1:A2:A3:A4:A5:A6:A7:D1:D2:D3:D4:D5:D6:D7:DIR:0:1:2:1:0:2:1:0:ESQ:1:0:1:2:0:1:2:1:TESTES:2:0101010101010101:1010101010101010

void lerSerial(String linha) {
  Chip c;

  int atual = linha.indexOf(':') + 1;

  //codigo
  int prox = linha.indexOf(':', atual);
  codigo = linha.substring(atual, prox);
  codigo.toCharArray(c.codigo, 8);
  atual = prox + 1;

  //nome
  prox = linha.indexOf(':', atual);
  nome = linha.substring(atual, prox);
  nome.toCharArray(c.nome, 21);
  atual = prox + 1;


  //quantidade de pinos
  prox = linha.indexOf(':', atual);
  qtdPinos = linha.substring(atual, prox).toInt();
  c.qtdPinos=qtdPinos;
  atual = prox + 1;


  //nome de casda pino
  for (int i = 0; i < qtdPinos; i++) {
    prox = linha.indexOf(':', atual);
    String p=linha.substring(atual, prox);
    pinos[i] = p;
    atual = prox + 1;
  }

  //pula "dir"
  atual=linha.indexOf(':', linha.indexOf('DIR')+1);

  for (int i = 0; i < 8; i++) {
    prox = linha.indexOf(':', atual);
    c.direita[i] = linha.substring(atual, prox).toInt();
    atual = prox + 1;
  }

  //pula "esq"
  atual=linha.indexOf(':', linha.indexOf('ESQ')+1);

  for (int i = 0; i < 8; i++) {
    prox = linha.indexOf(':', atual);
    c.esquerda[i] = linha.substring(atual, prox).toInt();
    atual = prox + 1;
  }

  //pula "testes"
  atual=linha.indexOf(':', linha.indexOf('TESTES')+1);

  //quantidade de testes
  prox = linha.indexOf(':', atual);
  c.qtdPinos=linha.substring(atual, prox).toInt();
  atual = prox + 1;

  for (int t = 0; t < c.qtdTestes; t++) {
    prox = linha.indexOf(':', atual);
    if (prox == -1) prox = linha.length();
    String temp = linha.substring(atual, prox);
    for (int i = 0; i < 16; i++) {
      c.testes[t].valorEsperado[i] = temp[i] - '0';
    }
    atual = prox + 1;
  }

  Serial.println("Lido: " + codigo + ", " + nome + ", " + String(qtdPinos) + " pinos");

  salvarChip(c);
  desenharChip();
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
  int xRet = 90;
  int yRet = 40;
  int larguraRet = 60;
  int alturaRet = metade * 20;

  tft.drawRect(xRet, yRet, larguraRet, alturaRet, BRANCO);

  for (int i = 0; i < metade; i++) {
    int yPino = yRet + i * 20 + 4;
    tft.drawRect(xRet - 14, yPino, 14, 14, BRANCO);
    tft.setTextSize(1);
    tft.setCursor(xRet - 11, yPino + 3);
    tft.print(i);
    tft.setCursor(xRet - 14 - (pinos[i].length() * 6) - 2, yPino + 3);
    tft.print(pinos[i].substring(0, 5));
  }

  for (int i = 0; i < metade; i++) {
    int pinoIdx = qtdPinos - 1 - i;
    int yPino = yRet + i * 20 + 4;
    tft.drawRect(xRet + larguraRet, yPino, 14, 14, BRANCO);
    tft.setTextSize(1);
    tft.setCursor(xRet + larguraRet + 2, yPino + 3);
    tft.print(pinoIdx);
    tft.setCursor(xRet + larguraRet + 16, yPino + 3);
    tft.print(pinos[pinoIdx].substring(0, 5));
  }
}

void loop() {
  if (Serial.available() > 0) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    if (linha.startsWith("CHIP:")) {
      lerSerial(linha);
    }
  }
}
