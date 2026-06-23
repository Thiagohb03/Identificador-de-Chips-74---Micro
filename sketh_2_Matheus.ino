#include <EEPROM.h>



typedef struct {
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
} Chip;

const int MAX_CHIPS = (4096 - 1) / sizeof(Chip);

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

// Leitura da EEPROM 

int lerTotalChips() {
  int total;
  EEPROM.get(0, total);
  return total;
}

Chip lerChip(int indice) {
  Chip c;
  int endereco = 1 + (indice * sizeof(Chip));
  EEPROM.get(endereco, c);
  return c;
}

// Execução de um teste único, usando direita[]/esquerda[] como tipo
// tipo: 0 = saida, 1 = entrada, 2 = nao usado

bool executaTeste(Chip c, int pos) {

  // 1. Configura saídas do chip (leitura) como INPUT primeiro,
  //    para evitar conflito elétrico com o Arduino
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 0) {  // saida
      int megaPin = mapeamento(i);
      if (megaPin == -1) return false;
      pinMode(megaPin, INPUT);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 0) {  // saida
      int megaPin = mapeamento(8 + i);
      if (megaPin == -1) return false;
      pinMode(megaPin, INPUT);
    }
  }

  // 2. Aplica as entradas
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 1) {  // entrada
      int megaPin = mapeamento(i);
      if (megaPin == -1) return false;
      pinMode(megaPin, OUTPUT);
      digitalWrite(megaPin, c.testes[pos].valorEsperado[i]);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 1) {  // entrada
      int megaPin = mapeamento(8 + i);
      if (megaPin == -1) return false;
      pinMode(megaPin, OUTPUT);
      digitalWrite(megaPin, c.testes[pos].valorEsperado[8 + i]);
    }
  }

  delay(100);

  // 3. Lê e valida as saídas
  for (int i = 0; i < 8; i++) {
    if (c.esquerda[i] == 0) {  // saida
      int megaPin = mapeamento(i);
      int valorLido = digitalRead(megaPin);
      if (valorLido != c.testes[pos].valorEsperado[i]) {
        return false;
      }
    }
  }
  for (int i = 0; i < 8; i++) {
    if (c.direita[i] == 0) {  // saida
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

  int totalChips = lerTotalChips();

  if (totalChips <= 0 || totalChips > MAX_CHIPS) {
    Serial.println("EEPROM vazia ou corrompida.");
    return;
  }

  Serial.print(totalChips);
  Serial.println(" chips carregados da EEPROM.");

  bool identificado = false;

  for (int i = 0; i < totalChips; i++) {
    Chip c = lerChip(i);
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
      break;
    }
  }

  if (!identificado) {
    Serial.println("Chip nao identificado.");
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  identificarChip();
}

void loop() {
}
