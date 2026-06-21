struct Teste {
  byte valores_esperados[16];
};
struct chip {
  char codigo[8];
  //entradas e saidas
  byte tipos_pino[16];
  Teste testes[5];
  byte qtdTestes;
};
struct pinos_chip_ard {
  int pino_chip;
  int pino_ard;
};
const pinos_chip_ard pinagem[] = { { 1, 22 },
                                   { 2, 24 },
                                   { 3, 26 },
                                   { 4, 28 },
                                   { 5, 30 },
                                   { 6, 31 },
                                   { 7, 32 },
                                   { 8, 33 },
                                   { 9, 34 },
                                   { 10, 35 } };
chip chips[] = {

  { "7408",
    { 1, 1, 0, 1, 1, 0, 2, 0, 1, 1, 0, 1,2,2,2},
    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0},
    1 },

    {
    "7432",
    {1, 1, 0, 1, 1, 0, 2, 0, 1, 1, 0, 1, 1,2,2,2 },
    {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,0,0,0 },

            
    1
    }
};

int mapeamento(int pino_chip) {
  pino_chip++;
  int tam = sizeof(pinagem) / sizeof(pinagem[0]);
  for (int i = 0; i < tam; i++) {
    if (pinagem[i].pino_chip == pino_chip) {
      return pinagem[i].pino_ard;
    }
  }
}

bool executa_teste(chip Chip, int pos) {
    for (int i = 0; i < 10; i++) {
        //Serial.println(Chip.tipos_pino[i]);
        if (Chip.tipos_pino[i] == 1)
        {
          int entrada_mega = mapeamento(i);
          pinMode(entrada_mega, OUTPUT);
        //   if(chips.testes[pos].valores_esperados[i]==1)
            digitalWrite(entrada_mega,Chip.testes[pos].valores_esperados[i]);
        //   else
        //     digitalWrite(entrada_mega,LOW);
        }
    }
    delay(200);
    for (int i=0;i<10;i++)
    {
        if (Chip.tipos_pino[i] == 0) {
        int saida_mega = mapeamento(i);
        pinMode(saida_mega, INPUT);
        int valor_lido = digitalRead(saida_mega);
        Serial.println(i);
        Serial.println(saida_mega);
        Serial.print("Valor lido : ");
        Serial.print(valor_lido);
        Serial.print(" Valor esperado : ");
        Serial.println(Chip.testes[pos].valores_esperados[i]);
        if (valor_lido != Chip.testes[pos].valores_esperados[i])
        {
            
            return false;
        }
        }
    }
    return true;
}

void setup() {
  Serial.begin(9600);
  delay(1000);
    int qtd_chips = sizeof(chips) / sizeof(chips[0]);
    bool identificado = false;

    for (byte i = 0; i < qtd_chips; i++) {
      bool todos_passaram = true;

      for (int k = 0; k < chips[i].qtdTestes; k++) {
        if (!executa_teste(chips[i], k)) {
          todos_passaram = false;
          Serial.print("Falhou ");
          Serial.println(chips[i].codigo);
          break;
        }
      }

      if (todos_passaram) {
        Serial.print("chips identificado: ");
        Serial.println(chips[i].codigo);
        identificado = true;
        break;
      }
    }

    if (!identificado) {
      Serial.println("chips nao identificado.");
    }
}
void loop() {
}