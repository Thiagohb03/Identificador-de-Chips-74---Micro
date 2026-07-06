#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <EEPROM.h>
#include <string.h>

MCUFRIEND_kbv tft;

#define BRANCO 0xFFFF
#define PRETO  0x0000

#define MAX_TESTES 10

const int END_TOTAL_CHIPS  = 0;
const int END_INICIO_CHIPS = sizeof(int);

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
  Teste testes[MAX_TESTES];
};

int totalChips = 0;

struct No {
  Chip chip;
  No* proximo;
};

No* listaChips = nullptr;

int maxChips() {
  return (EEPROM.length() - END_INICIO_CHIPS) / sizeof(Chip);
}

int enderecoChip(int indice) {
  return END_INICIO_CHIPS + indice * sizeof(Chip);
}

void limparLista() {
  No* atual = listaChips;
  while (atual != nullptr) {
    No* prox = atual->proximo;
    delete atual;
    atual = prox;
  }
  listaChips = nullptr;
}

void inicializarChip(Chip &c) {
  memset(&c, 0, sizeof(Chip));

  c.qtdPinos = 0;
  c.qtdTestes = 0;

  for (int i = 0; i < 8; i++) {
    c.direita[i] = 2;
    c.esquerda[i] = 2;
  }

  for (int t = 0; t < MAX_TESTES; t++) {
    for (int i = 0; i < 16; i++) {
      c.testes[t].valorEsperado[i] = 2;
    }
  }
}

void adicionarLista(Chip c) {
  No* novo = new No();
  if (novo == nullptr) {
    Serial.println("ERRO: sem memoria RAM para lista.");
    return;
  }

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
  limparLista();

  EEPROM.get(END_TOTAL_CHIPS, totalChips);

  if (totalChips < 0 || totalChips > maxChips()) {
    totalChips = 0;
    EEPROM.put(END_TOTAL_CHIPS, totalChips);
  }

  for (int i = 0; i < totalChips; i++) {
    Chip c;
    EEPROM.get(enderecoChip(i), c);

    if (c.qtdPinos == 14 || c.qtdPinos == 16) {
      if (c.qtdTestes >= 0 && c.qtdTestes <= MAX_TESTES) {
        adicionarLista(c);
      }
    }
  }
}

void salvarChip(Chip c) {
  if (totalChips >= maxChips()) {
    Serial.println("ERRO: Banco cheio");
    return;
  }

  int endereco = enderecoChip(totalChips);

  EEPROM.put(endereco, c);
  totalChips++;
  EEPROM.put(END_TOTAL_CHIPS, totalChips);

  Serial.print("SALVO. Total chips: ");
  Serial.println(totalChips);

  adicionarLista(c);
}

void selecionarChip(int n) {
  if (n < 1 || n > totalChips) {
    Serial.println("ERRO: indice de chip invalido.");
    return;
  }

  Chip c;
  EEPROM.get(enderecoChip(n - 1), c);

  Serial.println("Chip selecionado");
  Serial.print("Indice: ");
  Serial.println(n);
  Serial.print("Endereco EEPROM: ");
  Serial.println(enderecoChip(n - 1));
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

// Mapeamento entre pino logico do chip e pino fisico do Arduino Mega.
// Cada indice do array e o pino logico do chip usado pelo sistema.
// Cada valor e o pino digital do Arduino Mega ligado naquele pino do chip.
//
// Para chip DIP-14 no seu sistema:
// pino logico 0  = pino fisico 1  do CI
// pino logico 1  = pino fisico 2  do CI
// pino logico 2  = pino fisico 3  do CI
// pino logico 3  = pino fisico 4  do CI
// pino logico 4  = pino fisico 5  do CI
// pino logico 5  = pino fisico 6  do CI
// pino logico 6  = pino fisico 7  do CI = GND
// pino logico 7  = pino fisico 8  do CI
// pino logico 8  = pino fisico 9  do CI
// pino logico 9  = pino fisico 10 do CI
// pino logico 10 = pino fisico 11 do CI
// pino logico 11 = pino fisico 12 do CI
// pino logico 12 = pino fisico 13 do CI
// pino logico 13 = pino fisico 14 do CI = VCC
//
// Mantive seu mapeamento atual. Para 14 pinos, apenas indices 0..13 sao usados.
// Os indices 14 e 15 ficam -1 para nao usar D0 por acidente.
const int pinoMega[16] = {
  53, // pino logico 0  -> Arduino D53
  51, // pino logico 1  -> Arduino D51
  49, // pino logico 2  -> Arduino D49
  47, // pino logico 3  -> Arduino D47
  52, // pino logico 4  -> Arduino D52
  50, // pino logico 5  -> Arduino D50
  48, // pino logico 6  -> Arduino D48 = GND do CI DIP-14

  43, // pino logico 7  -> Arduino D43
  41, // pino logico 8  -> Arduino D41
  39, // pino logico 9  -> Arduino D39
  37, // pino logico 10 -> Arduino D37
  35, // pino logico 11 -> Arduino D35
  33, // pino logico 12 -> Arduino D33
  31, // pino logico 13 -> Arduino D31 = VCC do CI DIP-14

  -1, // pino logico 14 -> sem mapeamento por enquanto
  -1  // pino logico 15 -> sem mapeamento por enquanto
};

int mapeamento(int indicePino) {
  if (indicePino < 0 || indicePino >= 16) {
    return -1;
  }
  return pinoMega[indicePino];
}

byte tipoPino(const Chip &c, int pinoLogico) {
  int metade = c.qtdPinos / 2;

  if (pinoLogico < 0 || pinoLogico >= c.qtdPinos) {
    return 2;
  }

  if (pinoLogico < metade) {
    return c.esquerda[pinoLogico];
  }

  return c.direita[pinoLogico - metade];
}

void imprimirValor(byte valor) {
  if (valor == 0) {
    Serial.print("0");
  } else if (valor == 1) {
    Serial.print("1");
  } else {
    Serial.print("NC");
  }
}

void imprimirTipo(byte tipo) {
  if (tipo == 1) {
    Serial.print("ENTRADA_CHIP / Arduino ESCREVE");
  } else if (tipo == 0) {
    Serial.print("SAIDA_CHIP / Arduino LE");
  } else {
    Serial.print("NAO_CLASSIFICADO");
  }
}

void imprimirNomePino(const Chip &c, int pinoLogico) {
  if (strlen(c.nomPinos[pinoLogico]) > 0) {
    Serial.print(c.nomPinos[pinoLogico]);
  } else {
    Serial.print("sem_nome");
  }
}

void imprimirPinoTeste(const Chip &c, int testeIndice, int pinoLogico) {
  byte valor = c.testes[testeIndice].valorEsperado[pinoLogico];

  if (valor == 2) {
    return;
  }

  byte tipo = tipoPino(c, pinoLogico);
  int megaPin = mapeamento(pinoLogico);
  int metade = c.qtdPinos / 2;

  Serial.print("    Pino logico ");
  Serial.print(pinoLogico);

  Serial.print(" (");
  imprimirNomePino(c, pinoLogico);
  Serial.print(")");

  Serial.print(" | Lado: ");
  if (pinoLogico < metade) {
    Serial.print("ESQ");
  } else {
    Serial.print("DIR");
  }

  Serial.print(" | Arduino: ");
  if (megaPin == -1) {
    Serial.print("SEM_MAPEAMENTO");
  } else {
    Serial.print("D");
    Serial.print(megaPin);
  }

  Serial.print(" | Tipo: ");
  imprimirTipo(tipo);

  Serial.print(" | Acao: ");
  if (tipo == 1) {
    Serial.print("aplicar ");
    imprimirValor(valor);
  } else if (tipo == 0) {
    Serial.print("esperar leitura ");
    imprimirValor(valor);
  } else {
    Serial.print("ignorado pelo teste fisico");
  }

  Serial.println();
}

void imprimirDirecoesSalvas(const Chip &c) {
  int metade = c.qtdPinos / 2;

  Serial.println("  Direcoes salvas considerando pino logico:");

  Serial.print("    ESQ: ");
  for (int i = 0; i < metade; i++) {
    Serial.print("p");
    Serial.print(i);
    Serial.print("=");
    Serial.print(c.esquerda[i]);
    if (i < metade - 1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("    DIR: ");
  for (int i = 0; i < metade; i++) {
    int pinoLogico = metade + i;
    Serial.print("p");
    Serial.print(pinoLogico);
    Serial.print("=");
    Serial.print(c.direita[i]);
    if (i < metade - 1) Serial.print(", ");
  }
  Serial.println();
}

void listarChipsETestes() {
  int totalEeprom = 0;
  EEPROM.get(END_TOTAL_CHIPS, totalEeprom);

  Serial.println("================ CHIPS NA EEPROM ================");
  Serial.print("Endereco totalChips: ");
  Serial.println(END_TOTAL_CHIPS);
  Serial.print("Endereco inicio chips: ");
  Serial.println(END_INICIO_CHIPS);
  Serial.print("sizeof(Chip): ");
  Serial.println(sizeof(Chip));
  Serial.print("MAX_CHIPS: ");
  Serial.println(maxChips());

  if (totalEeprom < 0 || totalEeprom > maxChips()) {
    Serial.print("ERRO: totalChips invalido na EEPROM: ");
    Serial.println(totalEeprom);
    Serial.println("=================================================");
    return;
  }

  Serial.print("Total de chips salvos: ");
  Serial.println(totalEeprom);

  if (totalEeprom == 0) {
    Serial.println("Nenhum chip salvo.");
    Serial.println("=================================================");
    return;
  }

  for (int i = 0; i < totalEeprom; i++) {
    Chip c;
    EEPROM.get(enderecoChip(i), c);

    Serial.println();
    Serial.print("Chip #");
    Serial.println(i + 1);
    Serial.print("  Endereco EEPROM: ");
    Serial.println(enderecoChip(i));
    Serial.print("  Codigo: ");
    Serial.println(c.codigo);
    Serial.print("  Nome: ");
    Serial.println(c.nome);
    Serial.print("  Qtd pinos: ");
    Serial.println(c.qtdPinos);
    Serial.print("  Qtd testes: ");
    Serial.println(c.qtdTestes);

    if (!(c.qtdPinos == 14 || c.qtdPinos == 16)) {
      Serial.println("  ERRO: chip com qtdPinos invalida. Pulando detalhes.");
      continue;
    }

    if (c.qtdTestes < 0 || c.qtdTestes > MAX_TESTES) {
      Serial.println("  ERRO: qtdTestes invalida. Pulando detalhes.");
      continue;
    }

    imprimirDirecoesSalvas(c);

    if (c.qtdTestes == 0) {
      Serial.println("  Nenhum teste cadastrado para este chip.");
      continue;
    }

    for (int t = 0; t < c.qtdTestes; t++) {
      Serial.println();
      Serial.print("  Teste #");
      Serial.println(t + 1);
      Serial.println("  Pinos considerados no teste:");

      bool algumPino = false;
      for (int pino = 0; pino < c.qtdPinos; pino++) {
        if (c.testes[t].valorEsperado[pino] != 2) {
          algumPino = true;
          imprimirPinoTeste(c, t, pino);
        }
      }

      if (!algumPino) {
        Serial.println("    Nenhum pino incluido neste teste.");
      }
    }
  }

  Serial.println("=================================================");
}

int pinoGND(const Chip &c) {
  if (c.qtdPinos == 14) return 6;
  if (c.qtdPinos == 16) return 7;
  return -1;
}

int pinoVCC(const Chip &c) {
  if (c.qtdPinos == 14 || c.qtdPinos == 16) return c.qtdPinos - 1;
  return -1;
}

bool ehPinoAlimentacao(const Chip &c, int pinoLogico) {
  return pinoLogico == pinoGND(c) || pinoLogico == pinoVCC(c);
}

void desligarTodosPinosDoChip(const Chip &c) {
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    int megaPin = mapeamento(pino);
    if (megaPin != -1) {
      digitalWrite(megaPin, LOW);
      pinMode(megaPin, INPUT);
    }
  }
}

void liberarTodosPinosMapeados() {
  for (int i = 0; i < 16; i++) {
    int megaPin = mapeamento(i);
    if (megaPin != -1) {
      digitalWrite(megaPin, LOW);
      pinMode(megaPin, INPUT);
    }
  }
}

void liberarPinosDeSinal(const Chip &c) {
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    if (ehPinoAlimentacao(c, pino)) {
      continue;
    }

    int megaPin = mapeamento(pino);
    if (megaPin != -1) {
      digitalWrite(megaPin, LOW);
      pinMode(megaPin, INPUT);
    }
  }
}

bool aplicarAlimentacao(const Chip &c) {
  int gnd = pinoGND(c);
  int vcc = pinoVCC(c);

  int megaGND = mapeamento(gnd);
  int megaVCC = mapeamento(vcc);

  if (megaGND == -1 || megaVCC == -1) {
    Serial.println("ERRO: GND ou VCC sem mapeamento.");
    return false;
  }

  // Alimentacao sempre vem antes dos sinais.
  pinMode(megaGND, OUTPUT);
  digitalWrite(megaGND, LOW);

  pinMode(megaVCC, OUTPUT);
  digitalWrite(megaVCC, HIGH);

  delay(50);
  return true;
}

void desligarAlimentacao(const Chip &c) {
  int gnd = pinoGND(c);
  int vcc = pinoVCC(c);

  int megaGND = mapeamento(gnd);
  int megaVCC = mapeamento(vcc);

  if (megaGND != -1) {
    digitalWrite(megaGND, LOW);
    pinMode(megaGND, INPUT);
  }

  if (megaVCC != -1) {
    digitalWrite(megaVCC, LOW);
    pinMode(megaVCC, INPUT);
  }
}

bool executaTeste(const Chip &c, int pos) {
  if (pos < 0 || pos >= c.qtdTestes || pos >= MAX_TESTES) {
    return false;
  }

  bool testeOk = true;

  // Nesta versao, GND/VCC ja devem estar ligados antes do teste.
  // Aqui liberamos apenas sinais, sem desligar alimentacao.
  liberarPinosDeSinal(c);
  delay(20);

  // Garante alimentacao antes de qualquer entrada/sinal.
  if (!aplicarAlimentacao(c)) {
    return false;
  }

  // Primeiro configura as saidas do chip como entradas no Arduino.
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    if (ehPinoAlimentacao(c, pino)) {
      continue;
    }

    if (tipoPino(c, pino) == 0) {
      int megaPin = mapeamento(pino);
      if (megaPin == -1) {
        testeOk = false;
        break;
      }
      pinMode(megaPin, INPUT);
    }
  }

  // Depois aplica os valores nas entradas de sinal do chip.
  // GND e VCC nao passam por aqui; foram tratados antes.
  if (testeOk) {
    for (int pino = 0; pino < c.qtdPinos; pino++) {
      if (ehPinoAlimentacao(c, pino)) {
        continue;
      }

      if (tipoPino(c, pino) == 1 && c.testes[pos].valorEsperado[pino] != 2) {
        int megaPin = mapeamento(pino);
        if (megaPin == -1) {
          testeOk = false;
          break;
        }
        pinMode(megaPin, OUTPUT);
        digitalWrite(megaPin, c.testes[pos].valorEsperado[pino] ? HIGH : LOW);
      }
    }
  }

  delay(100);

  // Por fim, confere as saidas esperadas.
  if (testeOk) {
    for (int pino = 0; pino < c.qtdPinos; pino++) {
      if (ehPinoAlimentacao(c, pino)) {
        continue;
      }

      if (tipoPino(c, pino) == 0 && c.testes[pos].valorEsperado[pino] != 2) {
        int megaPin = mapeamento(pino);
        if (megaPin == -1) {
          testeOk = false;
          break;
        }

        int valorLido = digitalRead(megaPin);
        if (valorLido != c.testes[pos].valorEsperado[pino]) {
          testeOk = false;
          break;
        }
      }
    }
  }

  // Libera apenas sinais. Alimentacao continua ligada para o proximo teste.
  liberarPinosDeSinal(c);
  delay(20);

  return testeOk;
}

void identificarChip() {
  Serial.println("INICIO_IDENTIFICAR");

  liberarTodosPinosMapeados();
  delay(50);

  if (totalChips <= 0 || totalChips > maxChips()) {
    Serial.println("EEPROM vazia ou corrompida.");
    tft.fillScreen(PRETO);
    tft.setTextColor(BRANCO);
    tft.setCursor(10, 100);
    tft.print("Nenhum Chip Carregado");
    liberarTodosPinosMapeados();
    Serial.println("FIM_IDENTIFICAR");
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
  Chip chipIdentificado;
  inicializarChip(chipIdentificado);

  No* atual = listaChips;
  while (atual != nullptr) {
    Chip &c = atual->chip;

    if (c.qtdTestes <= 0 || c.qtdTestes > MAX_TESTES) {
      desligarTodosPinosDoChip(c);
      atual = atual->proximo;
      continue;
    }

    bool todosPassaram = true;

    desligarTodosPinosDoChip(c);
    delay(30);

    if (!aplicarAlimentacao(c)) {
      todosPassaram = false;
    }

    if (todosPassaram) {
      for (int k = 0; k < c.qtdTestes; k++) {
        if (!executaTeste(c, k)) {
          todosPassaram = false;
          break;
        }
      }
    }

    // Depois de testar este cadastro, desliga tudo antes de tentar outro chip.
    desligarTodosPinosDoChip(c);
    delay(30);

    if (todosPassaram) {
      Serial.print("Chip identificado: ");
      Serial.print(c.codigo);
      Serial.print(" - ");
      Serial.println(c.nome);

      identificado = true;
      chipIdentificado = c;
      break;
    }

    atual = atual->proximo;
  }

  if (identificado) {
    desenharChip(chipIdentificado);
  } else {
    Serial.println("Chip nao identificado.");
    tft.fillScreen(PRETO);
    tft.setTextColor(BRANCO);
    tft.setCursor(10, 100);
    tft.print("Nenhum Chip");
    tft.println("Identificado");
  }

  liberarTodosPinosMapeados();
  Serial.println("FIM_IDENTIFICAR");
}

String proximoCampo(const String &linha, int &atual) {
  int prox = linha.indexOf(':', atual);

  if (prox == -1) {
    String campo = linha.substring(atual);
    atual = linha.length() + 1;
    return campo;
  }

  String campo = linha.substring(atual, prox);
  atual = prox + 1;
  return campo;
}

bool lerSerial(String linha) {
  Chip c;
  inicializarChip(c);

  int atual = 0;

  String cabecalho = proximoCampo(linha, atual);
  if (cabecalho != "CHIP") {
    Serial.println("ERRO: protocolo invalido.");
    return false;
  }

  String codigo = proximoCampo(linha, atual);
  codigo.toCharArray(c.codigo, sizeof(c.codigo));

  String nome = proximoCampo(linha, atual);
  nome.toCharArray(c.nome, sizeof(c.nome));

  c.qtdPinos = proximoCampo(linha, atual).toInt();

  if (c.qtdPinos != 14 && c.qtdPinos != 16) {
    Serial.println("ERRO: quantidade de pinos invalida.");
    return false;
  }

  int metade = c.qtdPinos / 2;

  for (int i = 0; i < c.qtdPinos; i++) {
    String nomePino = proximoCampo(linha, atual);
    nomePino.toCharArray(c.nomPinos[i], sizeof(c.nomPinos[i]));
  }

  String marcadorDir = proximoCampo(linha, atual);
  if (marcadorDir != "DIR") {
    Serial.println("ERRO: marcador DIR nao encontrado.");
    return false;
  }

  for (int i = 0; i < metade; i++) {
    int pinoLogico = c.qtdPinos - 1 - i;
    int indiceDireita = pinoLogico - metade;
    c.direita[indiceDireita] = proximoCampo(linha, atual).toInt();
  }

  String marcadorEsq = proximoCampo(linha, atual);
  if (marcadorEsq != "ESQ") {
    Serial.println("ERRO: marcador ESQ nao encontrado.");
    return false;
  }

  for (int i = 0; i < metade; i++) {
    c.esquerda[i] = proximoCampo(linha, atual).toInt();
  }

  String marcadorTestes = proximoCampo(linha, atual);
  if (marcadorTestes != "TESTES") {
    Serial.println("ERRO: marcador TESTES nao encontrado.");
    return false;
  }

  c.qtdTestes = proximoCampo(linha, atual).toInt();

  if (c.qtdTestes < 0 || c.qtdTestes > MAX_TESTES) {
    Serial.print("ERRO: quantidade de testes invalida. Maximo permitido: ");
    Serial.println(MAX_TESTES);
    return false;
  }

  for (int t = 0; t < c.qtdTestes; t++) {
    String temp = proximoCampo(linha, atual);

    for (int i = 0; i < c.qtdPinos; i++) {
      if (i < temp.length()) {
        char ch = temp.charAt(i);
        if (ch >= '0' && ch <= '2') {
          c.testes[t].valorEsperado[i] = ch - '0';
        } else {
          c.testes[t].valorEsperado[i] = 2;
        }
      } else {
        c.testes[t].valorEsperado[i] = 2;
      }
    }
  }

  Serial.print("Lido: ");
  Serial.print(c.codigo);
  Serial.print(", ");
  Serial.print(c.nome);
  Serial.print(", ");
  Serial.print(c.qtdPinos);
  Serial.print(" pinos, ");
  Serial.print(c.qtdTestes);
  Serial.println(" testes");

  salvarChip(c);
  desenharChip(c);
  return true;
}

void limparBanco() {
  totalChips = 0;
  EEPROM.put(END_TOTAL_CHIPS, totalChips);
  limparLista();
  Serial.println("EEPROM limpa.");
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1000);

  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setRotation(0);

  carregaChip();
  liberarTodosPinosMapeados();

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

  Serial.print("Sistema iniciado. MAX_TESTES=");
  Serial.print(MAX_TESTES);
  Serial.print(" | MAX_CHIPS=");
  Serial.println(maxChips());
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
    } else if (linha.startsWith("Chips")) {
      listarChipsETestes();
    } else if (linha.startsWith("Limpar")) {
      limparBanco();
    } else if (linha.length() > 0) {
      Serial.print("Comando desconhecido: ");
      Serial.println(linha);
    }
  }
}
