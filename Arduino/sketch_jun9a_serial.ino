#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <EEPROM.h>

MCUFRIEND_kbv tft;

#define BRANCO 0xFFFF
#define PRETO 0x0000

struct Teste {
  byte valorEsperado[16];
};

struct Chip {
  char nome[21];
  char codigo[8];
  int qtdPinos;
  char nomPinos[16][3];
  byte direita[8];
  byte esquerda[8];
  int qtdTestes;
  Teste testes[5];
};

int totalChips=0;

const int MAX_CHIPS=(4096-1)/sizeof(Chip);

struct No {
  Chip chip;
  No* proximo;
};

No* listaChips = nullptr;

void adicionarLista(Chip c) {
  No* novo = new No();
  novo->chip = c;
  novo->proximo = nullptr;

  if (listaChips == nullptr) {
    listaChips = novo;
    return;
  }

  No* atual = listaChips;
  while (atual->proximo != nullptr) {
    atual = atual->proximo;
  }
  atual->proximo = novo;
}

void carregaChip() {
  EEPROM.get(0, totalChips);
  if (totalChips < 0 || totalChips > MAX_CHIPS) totalChips = 0;

  for (int i = 0; i < totalChips; i++) {
    Chip c;
    int endereco = 1 + i * sizeof(Chip);
    EEPROM.get(endereco, c);
    adicionarLista(c);
  }
}

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
  adicionarLista(c);
}

// Lê um chip salvo na EEPROM pelo índice (n=1 é o primeiro chip salvo)
// e imprime os dados dele na Serial, sem alterar o display.
void selecionarChip(int n) {
  if (n < 1 || n > totalChips) {
    Serial.println("ERRO: indice de chip invalido.");
    return;
  }

  Chip c;
  int endereco = 1 + (n - 1) * sizeof(Chip);
  EEPROM.get(endereco, c);

  Serial.println("Chip selecionado ");
  Serial.print("Indice: ");
  Serial.println(n);
  Serial.print("Codigo: ");
  Serial.println(c.codigo);
  Serial.print("Nome: ");
  Serial.println(c.nome);
  Serial.print("Qtd Pinos: ");
  Serial.println(c.qtdPinos);
  Serial.print("Qtd Testes: ");
  Serial.println(c.qtdTestes);
}

void desenharChip(Chip c) {
  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);

  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print(c.nome);

  tft.setTextSize(1);
  tft.setCursor(5, 25);
  tft.print(c.codigo);

  int metade = c.qtdPinos / 2;
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
    String nomePino = String(c.nomPinos[i]);
    tft.setCursor(xRet - 14 - (nomePino.length() * 6) - 2, yPino + 3);
    tft.print(nomePino.substring(0, 5));
  }

  for (int i = 0; i < metade; i++) {
    int pinoIdx = c.qtdPinos - 1 - i;
    int yPino = yRet + i * 20 + 4;
    tft.drawRect(xRet + larguraRet, yPino, 14, 14, BRANCO);
    tft.setTextSize(1);
    tft.setCursor(xRet + larguraRet + 2, yPino + 3);
    tft.print(pinoIdx);
    String nomePino = String(c.nomPinos[pinoIdx]);
    tft.setCursor(xRet + larguraRet + 16, yPino + 3);
    tft.print(nomePino.substring(0, 5));
  }
}

const int pinoMega[16] = {
  22, 24, 26, 28, 30, 31, 32, 33,   // esquerda: pinos 0-7
  34, 35, 36, 37, 38, 39, 40, 41    // direita:  pinos 8-15
};

int mapeamento(int indicePino) {
  if (indicePino < 0 || indicePino >= 16) {
    return -1;
  }
  return pinoMega[indicePino];
}


// Execução de um teste único, usando direita[]/esquerda[] como tipo
// tipo de pino: 0 = saida do chip, 1 = entrada do chip, 2 = nao usado
// valor no teste: 0 = LOW, 1 = HIGH, 2 = don't care (nao aplicar / nao validar)

bool executaTeste(Chip c, int pos) {

  // 1. Configura saídas do chip (leitura) como INPUT primeiro,
  //    para evitar conflito elétrico com o Arduino
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 0) {
      int megaPin = mapeamento(i);
      if (megaPin == -1) return false;
      pinMode(megaPin, INPUT);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 0) {
      int megaPin = mapeamento(8 + i);
      if (megaPin == -1) return false;
      pinMode(megaPin, INPUT);
    }
  }

  // 2. Aplica as entradas — pula pinos marcados como don't care (valor 2)
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 1 && c.testes[pos].valorEsperado[i] != 2) {
      int megaPin = mapeamento(i);
      if (megaPin == -1) return false;
      pinMode(megaPin, OUTPUT);
      digitalWrite(megaPin, c.testes[pos].valorEsperado[i]);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 1 && c.testes[pos].valorEsperado[8 + i] != 2) {
      int megaPin = mapeamento(8 + i);
      if (megaPin == -1) return false;
      pinMode(megaPin, OUTPUT);
      digitalWrite(megaPin, c.testes[pos].valorEsperado[8 + i]);
    }
  }

  delay(100);

  // 3. Lê e valida as saídas — pula pinos marcados como don't care (valor 2)
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 0 && c.testes[pos].valorEsperado[i] != 2) {
      int megaPin = mapeamento(i);
      int valorLido = digitalRead(megaPin);
      if (valorLido != c.testes[pos].valorEsperado[i]) {
        return false;
      }
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 0 && c.testes[pos].valorEsperado[8 + i] != 2) {
      int megaPin = mapeamento(8 + i);
      int valorLido = digitalRead(megaPin);
      if (valorLido != c.testes[pos].valorEsperado[8 + i]) {
        return false;
      }
    }
  }

  return true;
}

// Identifica o chip testando todos os cadastrados na EEPROM 

void identificarChip() {
  if (totalChips <= 0 || totalChips > MAX_CHIPS) {
    Serial.println("EEPROM vazia ou corrompida.");
    tft.fillScreen(PRETO);
    tft.setTextColor(BRANCO);
    tft.setCursor(10, 100);
    tft.print("Nenhum Chip Carregado");
    return;
  }

  Serial.print(totalChips);
  Serial.println(" chips carregados da EEPROM.");
  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);
  tft.setCursor(10, 100);
  tft.print(totalChips);
  tft.println(" Chips carregados");
  delay(1000);
  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("identificando...");

  bool identificado = false;

  No* atual = listaChips;
  while (atual != nullptr) {
    Chip c = atual->chip;
    bool todosPassaram = true;

    for (int k = 0; k < c.qtdTestes; k++) {
      if (!executaTeste(c, k)) {
        todosPassaram = false;
        break;
      }
    }

    if (todosPassaram) {
      Serial.print("Chip identificado: ");
      Serial.print(c.codigo);
      Serial.print(" - ");
      Serial.println(c.nome);
      identificado = true;
      desenharChip(c);
      break;
    }

    atual = atual->proximo;
  }

  if (!identificado) {
    Serial.println("Chip nao identificado.");
    tft.fillScreen(PRETO);
    tft.setTextColor(BRANCO);
    tft.setCursor(10, 100);
    tft.print("Nenhum Chip");
    tft.println("Identificado");
  }
}


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10000);
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setRotation(0);
  carregaChip();
  

  tft.fillScreen(PRETO);
  tft.setTextColor(BRANCO);
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("Iniciando...");
  delay(1000);

  tft.fillScreen(PRETO);
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print("Insira o chip");
  tft.setCursor(20, 110);
  tft.print("e envie o comando");
  tft.setCursor(20, 140);
  tft.print("Identificar");
}

//CHIP:7408:AND_Quad:14:1A:1B:1Y:2A:2B:2Y:GND:VCC:4B:4A:4Y:3B:3A:3Y:DIR:2:1:1:0:1:1:0:ESQ:1:1:0:1:1:0:2:TESTES:2:11111100111111:10011100111000
void lerSerial(String linha) {
  Chip c;

  int atual = linha.indexOf(':') + 1;

  //codigo
  int prox = linha.indexOf(':', atual);
  String codigo = linha.substring(atual, prox);
  codigo.toCharArray(c.codigo, 8);
  atual = prox + 1;

  //nome
  prox = linha.indexOf(':', atual);
  String nome = linha.substring(atual, prox);
  nome.toCharArray(c.nome, 21);
  atual = prox + 1;


  //quantidade de pinos
  prox = linha.indexOf(':', atual);
  int qtdPinos = linha.substring(atual, prox).toInt();
  c.qtdPinos=qtdPinos;
  atual = prox + 1;


  //nome de casda pino
  for (int i = 0; i < qtdPinos; i++) {
    prox = linha.indexOf(':', atual);
    String p=linha.substring(atual, prox);
    p.toCharArray(c.nomPinos[i], 3);
    atual = prox + 1;
  }

  //pula "dir"
  atual=linha.indexOf(':', linha.indexOf("DIR")+1)+1;

  for (int i = 0; i < qtdPinos/2; i++) {
    prox = linha.indexOf(':', atual);
    c.direita[i] = linha.substring(atual, prox).toInt();
    atual = prox + 1;
  }

  //pula "esq"
  atual=linha.indexOf(':', linha.indexOf("ESQ")+1)+1;

  for (int i = 0; i < qtdPinos/2; i++) {
    prox = linha.indexOf(':', atual);
    c.esquerda[i] = linha.substring(atual, prox).toInt();
    atual = prox + 1;
  }

  //pula "testes"
  atual=linha.indexOf(':', linha.indexOf("TESTES")+1)+1;

  //quantidade de testes
  prox = linha.indexOf(':', atual);
  c.qtdTestes=linha.substring(atual, prox).toInt();
  atual = prox + 1;

  for (int t = 0; t < c.qtdTestes; t++) {
    prox = linha.indexOf(':', atual);
    if (prox == -1) prox = linha.length();
    String temp = linha.substring(atual, prox);
    for (int i = 0; i < qtdPinos; i++) {
      c.testes[t].valorEsperado[i] = temp[i] - '0';
    }
    atual = prox + 1;
  }

  Serial.println("Lido: " + codigo + ", " + nome + ", " + String(qtdPinos) + " pinos");

  salvarChip(c);
  desenharChip(c);
}

void loop() {
  if (Serial.available() > 0) {
    String linha = Serial.readStringUntil('\n');
    linha.trim();
    if (linha.startsWith("CHIP:")) {
      lerSerial(linha);
    } else if (linha.startsWith("Selecionar:")) {
      int n = linha.substring(linha.indexOf(':') + 1).toInt();
      selecionarChip(n);
    } else if (linha.startsWith("Identificar")) {
      identificarChip();
    } else if (linha.startsWith("Limpar")){
      for (int i=0;i<4096;i++){
        EEPROM.write(i,0);
      }
      totalChips=0;
      listaChips=nullptr;
      Serial.println("EEPROM limpa.");
    }

  }
}
