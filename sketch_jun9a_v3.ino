// --- Mapeamento de pinos ---

struct PinoMap {
    int chipPin;
    int megaPin;
};

const PinoMap mapeamento[] = {
    {1,  22},
    {2,  24},
    {3,  26},
    {5,  28},
    {6,  30},
    {7,  31},
    {8,  32},
    {9,  33},
    {10, 34},
    {11, 35}
};

const int QTD_PINOS = sizeof(mapeamento) / sizeof(mapeamento[0]);

int chipParaMega(int chipPin) {
    for (int i = 0; i < QTD_PINOS; i++) {
        if (mapeamento[i].chipPin == chipPin) {
            return mapeamento[i].megaPin;
        }
    }
    return -1;
}

// --- Estruturas de teste ---

struct Entrada {
    int pino;
    int valor;
};

struct Saida {
    int pino;
    int esperado;
};

struct Teste {
    Entrada entradas[10];
    int qtdEntradas;

    Saida saidas[10];
    int qtdSaidas;
};

struct Chip {
    const char* nome;
    Teste testes[16];
    int qtdTestes;
};

// --- Execução de teste ---

bool executarTeste(Teste teste) {

    // 1. Configura os pinos de leitura como INPUT antes de qualquer coisa
    for (int i = 0; i < teste.qtdSaidas; i++) {
        int megaPin = chipParaMega(teste.saidas[i].pino);
        if (megaPin == -1) {
            Serial.print("Pino de saida invalido: ");
            Serial.println(teste.saidas[i].pino);
            return false;
        }
        pinMode(megaPin, INPUT);
    }

    // 2. Aplica as entradas
    for (int i = 0; i < teste.qtdEntradas; i++) {
        int megaPin = chipParaMega(teste.entradas[i].pino);
        if (megaPin == -1) {
            Serial.print("Pino de entrada invalido: ");
            Serial.println(teste.entradas[i].pino);
            return false;
        }
        pinMode(megaPin, OUTPUT);
        digitalWrite(megaPin, teste.entradas[i].valor);
    }

    delay(10);

    // 3. Lê e valida as saídas
    for (int i = 0; i < teste.qtdSaidas; i++) {
        int megaPin = chipParaMega(teste.saidas[i].pino);
        int valorLido = digitalRead(megaPin);

        if (valorLido != teste.saidas[i].esperado) {
            return false;
        }
    }

    return true;
}

// --- Definição dos chips ---

Chip chips[] = {

    {
        "AND",
        {
            // 00 -> 0
            {{{1,LOW},{2,LOW}},   2, {{3,LOW}},  1},
            // 01 -> 0
            {{{1,LOW},{2,HIGH}},  2, {{3,LOW}},  1},
            // 10 -> 0
            {{{1,HIGH},{2,LOW}},  2, {{3,LOW}},  1},
            // 11 -> 1
            {{{1,HIGH},{2,HIGH}}, 2, {{3,HIGH}}, 1}
        },
        4
    },

    {
        "OR",
        {
            // 00 -> 0
            {{{1,LOW},{2,LOW}},   2, {{3,LOW}},  1},
            // 01 -> 1
            {{{1,LOW},{2,HIGH}},  2, {{3,HIGH}}, 1},
            // 10 -> 1
            {{{1,HIGH},{2,LOW}},  2, {{3,HIGH}}, 1},
            // 11 -> 1
            {{{1,HIGH},{2,HIGH}}, 2, {{3,HIGH}}, 1}
        },
        4
    }
};

// --- Setup ---

void setup() {

    Serial.begin(9600);
    int qtdChips = sizeof(chips) / sizeof(chips[0]);

    for (int c = 0; c < qtdChips; c++) {
        Chip chip = chips[c];

        Serial.println("====================");
        Serial.print("Testando chip: ");
        Serial.println(chip.nome);
        Serial.println("====================");

        bool chipOk = true;

        for (int t = 0; t < chip.qtdTestes; t++) {
            bool resultado = executarTeste(chip.testes[t]);

            Serial.print("  Teste ");
            Serial.print(t + 1);
            Serial.print(": ");
            Serial.println(resultado ? "Passou" : "Falhou");

            if (!resultado) {
                chipOk = false;
                break;
            }
        }

        Serial.print("Resultado: ");
        Serial.println(chipOk ? "CHIP OK" : "CHIP COM DEFEITO");
        Serial.println();
    }

    Serial.println("Testes encerrados.");
}

void loop() {
}
