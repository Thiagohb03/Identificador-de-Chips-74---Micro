#include <Adafruit_GFX.h> 
#include <MCUFRIEND_kbv.h> 
#include <EEPROM.h>
#include <string.h>
#include <GFButton.h>

GFButton botao(24);

MCUFRIEND_kbv tft; //inicialização da tela
                                                 
#define BRANCO 0xFFFF
#define PRETO 0x0000

#define MAX_TESTES 10  // maximo de baterias de teste por chip

const int END_TOTAL_CHIPS = 0;  // onde fica salvo o total de chips
const int END_INICIO_CHIPS = sizeof(int);  // onde comecam os dados dos chips

// Guarda os valores esperados em uma bateria de teste
struct Teste {
  byte valorEsperado[16];
};

// Guarda todos os dados de um chip cadastrado
struct Chip {
  char nome[21];
  char codigo[8];
  int qtdPinos;
  char nomPinos[16][4];  // até 3 caracteres + fim de texto
  byte direita[8];
  byte esquerda[8];
  int qtdTestes;
  Teste testes[MAX_TESTES];
};

// total de chips salvos na eeprom
int totalChips = 0;

// No da lista encadeada
struct No {
  Chip chip;
  No *proximo;
};

//inicializaçao da lista encadeada vazia
No *listaChips = nullptr;

// Calcula quantos chips cabem na EEPROM. (tam da eeprom - 1° endereço)/ tamanho de um chip
int maxChips() {
  return (EEPROM.length() - END_INICIO_CHIPS) / sizeof(Chip);
}

// Calcula o endereco de um chip dentro da EEPROM. endereço do inicio dos chips + indice * tamanho de um chip
int enderecoChip(int indice) {
  return END_INICIO_CHIPS + indice * sizeof(Chip);
}

// Limpa a lista em RAM antes de recarregar os chips
//o no atual aponta pro inicio da lista, o no proximo aponta para o proximo no de acordo com o atual. o atual libera a memoria e depois aponta para o mesmo do proximo, continuando o loop ate o atual apontar para null
void limparLista() {
  No *atual = listaChips;
  while (atual != nullptr) {
    No *prox = atual->proximo;
    delete atual;
    atual = prox;
  }
  listaChips = nullptr;
}

// Zera o chip e deixa todos os testes como ignorados
//
//memset recebe um endereço, um valor e um tamanho e seta todos os valores do inicio do endereço ate o finbal do tamanho como o valor passado
void inicializarChip(Chip &c) {
  memset(&c, 0, sizeof(Chip));
//para nao receber lixo no chip, ele seta todos valores de testes para 2 pois é o valor ignorado pelo sistema
  for (int t = 0; t < MAX_TESTES; t++) {
    for (int i = 0; i < 16; i++) {
      c.testes[t].valorEsperado[i] = 2;
    }
  }
}

// Adiciona um chip na lista encadeada.
//faz um check para garantir que he espeço em memoria para novos chips
//um novo no é criado e recebe o chip recebido pela função e nulo no ponteiro.
void adicionarLista(Chip c) {
  No *novo = new No();
  if (novo == nullptr) {
    Serial.println("ERRO: sem memoria RAM para lista.");
    return;
  }

  novo->chip = c;
  novo->proximo = nullptr;
  
//se a lista for vazia, adiciona o no novo no 1° endereço
  if (listaChips == nullptr) {
    listaChips = novo;
    return;
  }
//se a lista nao for vazia ele percorre a lista todo ate achar um ponteiro para nulo e entao aponta para o novo no
  No *atual = listaChips;
  while (atual->proximo != nullptr) {
    atual = atual->proximo;
  }
  atual->proximo = novo;
}

// Carrega da EEPROM para a lista
void carregaChip() {
  //garante que a lista esta vazia antes de adicionar qualquer coisa
  limparLista();
//pega da eeprom o total de chips e salva na variavel global
  EEPROM.get(END_TOTAL_CHIPS, totalChips);
//garante que o total de chips é pelo menos 0 e nenor que o tamanho total de chips que a eeprom aguenta para evitar lixo e erros
  if (totalChips < 0 || totalChips > maxChips()) {
    totalChips = 0;
    EEPROM.put(END_TOTAL_CHIPS, totalChips);
  }
// faz um loop do tamanho total de chips na eeprom, cria um tipo chip novo e poe as informações do chip da eeprom no indice i no tipo chip
//verifica que o chip tem a quantidade de testes e de pinos esperado pelo sistema para evitar erros nas demais funções e depois adiciona à lista
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

// Salva um chip novo no fim da EEPROM.
//garante que a eeprom possiu espaço suficiente para adicionar um novo chip, caso contrario mostra uma mensagem de erro
void salvarChip(Chip c) {
  if (totalChips >= maxChips()) {
    Serial.println("ERRO: Banco cheio");
    return;
  }
//calcula exatamente qual endereço deve ser usado para o proximo chip na eeprom
  int endereco = enderecoChip(totalChips);
// coloca o tipo chip na eeprom
  EEPROM.put(endereco, c);
//incrementa o valor total de chips e o salva no endereço do total de chips
  totalChips++;
  EEPROM.put(END_TOTAL_CHIPS, totalChips);

  Serial.print("SALVO. Total chips: ");
  Serial.println(totalChips);
// adiciona o chip recebido no fim da lista encadeada com a função 
  adicionarLista(c);
}

// Mostra os dados basicos de um chip salvo pelo indice
//se o indice passado esta fora do range do total de chips ele exibe uma mensagem de erro
void selecionarChip(int n) {
  if (n < 1 || n > totalChips) {
    Serial.println("ERRO: indice de chip invalido.");
    return;
  }
//cria um tipo chip e carrega os dados do endereço no novo chip criado(endereçoChip recebe n-1 pois ele recebe um indice que começa em 0 e n é a posição passada para a função)
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

// Desenha o chip identificado no display.
void desenharChip(Chip c) {
  //limpa a tela pintando ela toda de preto
  tft.fillScreen(PRETO);
  //escreve o nome e o codigo do chip no canto da tela
  tft.setTextColor(BRANCO);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.print(c.nome);
  tft.setTextSize(1);
  tft.setCursor(5, 25);
  tft.print(c.codigo);
  //define a metade dos pinos para saber quantos pinos terao na esquerda e na direita
  int metade = c.qtdPinos / 2;
  //x e y iniciais do retangulo
  int xRet = 90;
  int yRet = 40;
  //define uma altura e largura para o retangulo
  int larguraRet = 60;
  int alturaRet = metade * 20;

  tft.drawRect(xRet, yRet, larguraRet, alturaRet, BRANCO);
  // para cada pino do lado esquerdo ele define o y do pino como o y do retangulo + (indice do pino * o espaço entre cada um)+ um pequeno valor para ele nao ficar alinhado com o retangulo maior
  for (int i = 0; i < metade; i++) {
    int yPino = yRet + i * 20 + 4;
    // anda 14 espaços para a esquerda para iniciar o desenho do pino quadrado em relaçãoi ao retangulo
    tft.drawRect(xRet - 14, yPino, 14, 14, BRANCO);
    tft.setTextSize(1);
    //esxcreve o numero do pino dentro do quadrado do pino
    tft.setCursor(xRet - 11, yPino + 3);
    tft.print(i);
    //escreve o nome do pino do lado de fora dele
    String nomePino = String(c.nomPinos[i]);
    tft.setCursor(xRet - 14 - (nomePino.length() * 6) - 2, yPino + 3);
    tft.print(nomePino.substring(0, 5));
  }

  for (int i = 0; i < metade; i++) {
    //como ele desenha de cima para baixo, precisa desenhar o ultimo pino 1°, como o numero vai de 0 a 14 ou 16 ele diminui 1 do numero de pinos e depois diminui o indice pois o primeiro vai ser o de cima
    int pinoIdx = c.qtdPinos - 1 - i;
    int yPino = yRet + i * 20 + 4;
    // anda a largura total do retangulo maior + 14 espaços para a direita para iniciar o desenho do pino quadrado em relação ao retangulo
    tft.drawRect(xRet + larguraRet, yPino, 14, 14, BRANCO);
    //escreve o numero do pino dentro do quadrado do pino
    tft.setTextSize(1);
    tft.setCursor(xRet + larguraRet + 2, yPino + 3);
    tft.print(pinoIdx);
    //escreve o nome do pino do lado de fora dele
    String nomePino = String(c.nomPinos[pinoIdx]);
    tft.setCursor(xRet + larguraRet + 16, yPino + 3);
    tft.print(nomePino.substring(0, 5));
  }
}

const int pinoMega14[14] = {
  53,  // pino 0
  51,  // pino 1
  47,  // pino 2
  45,  // pino 3
  52,  // pino 4
  50,  // pino 5
  48,  // pino 6 = GND no 14
  43,  // pino 7
  41,  // pino 8
  39,  // pino 9
  37,  // pino 10
  35,  // pino 11
  33,  // pino 12
  31   // pino 13 = VCC no 14
};

const int pinoMega16[16] = {
  53,  // pino 0
  51,  // pino 1
  47,  // pino 2
  45,  // pino 3
  52,  // pino 4
  50,  // pino 5
  48,  // pino 6
  46,  // pino 7 = GND no 16

  26,  // pino 8
  43,  // pino 9
  41,  // pino 10
  39,  // pino 11
  37,  // pino 12
  35,  // pino 13
  33,  // pino 14
  31   // pino 15 = VCC no 16
};

// Retorna o pino digital do Arduino correspondente ao pino do CI
// Usa um mapa para 14 pin e outro para 16 pin
int mapeamento(const Chip &c, int indicePino) {
  if (indicePino < 0 || indicePino >= c.qtdPinos) {
    return -1;
  }

  if (c.qtdPinos == 14) {
    return pinoMega14[indicePino];
  }

  if (c.qtdPinos == 16) {
    return pinoMega16[indicePino];
  }

  return -1;
}

// Retorna o tipo do pino: 1 entrada, 0 saida, 2 ignorado
byte tipoPino(const Chip &c, int pino) {
  int metade = c.qtdPinos / 2;

  if (pino < 0 || pino >= c.qtdPinos) {
    return 2;
  }

  if (pino < metade) {
    return c.esquerda[pino];
  }

  return c.direita[pino - metade];
}

// Retorna o pino de GND no padrao usado pelo projeto
// 14: GND no pino 6
// 16: GND no pino 7
int pinoGND(const Chip &c) {
  if (c.qtdPinos == 14) return 6;
  if (c.qtdPinos == 16) return 7;
  return -1;
}

// Retorna o pino de VCC no padrao usado pelo projeto
// 14: VCC no pino 13
// 16: VCC no pino 15
int pinoVCC(const Chip &c) {
  if (c.qtdPinos == 14) return 13;
  if (c.qtdPinos == 16) return 15;
  return -1;
}

// Verifica se o pino e GND ou VCC
bool ehPinoAlimentacao(const Chip &c, int pino) {
  return pino == pinoGND(c) || pino == pinoVCC(c);
}

// Desliga todos os pinos do chip e coloca como entrada
void desligarTodosPinosDoChip(const Chip &c) {
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    int megaPin = mapeamento(c, pino);
    if (megaPin != -1) {
      digitalWrite(megaPin, LOW);
      pinMode(megaPin, INPUT);
    }
  }
}

// Coloca um pino do Arduino em alta impedancia
void liberarPinoArduino(int megaPin) {
  if (megaPin != -1) {
    digitalWrite(megaPin, LOW);
    pinMode(megaPin, INPUT);
  }
}

// Libera todos os pinos usados nos mapas 14 e 16
void liberarTodosPinosMapeados() {
  for (int i = 0; i < 14; i++) {
    liberarPinoArduino(pinoMega14[i]);
  }

  for (int i = 0; i < 16; i++) {
    liberarPinoArduino(pinoMega16[i]);
  }
}

// Libera apenas os pinos de sinal, mantendo GND e VCC fora
void liberarPinosDeSinal(const Chip &c) {
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    if (ehPinoAlimentacao(c, pino)) {
      continue;
    }

    int megaPin = mapeamento(c, pino);
    if (megaPin != -1) {
      digitalWrite(megaPin, LOW);
      pinMode(megaPin, INPUT);
    }
  }
}

// Liga GND em LOW e VCC em HIGH antes dos testes
bool aplicarAlimentacao(const Chip &c) {
  int gnd = pinoGND(c);
  int vcc = pinoVCC(c);

  int megaGND = mapeamento(c, gnd);
  int megaVCC = mapeamento(c, vcc);

  if (megaGND == -1 || megaVCC == -1) {
    Serial.println("ERRO: GND ou VCC sem mapeamento.");
    return false;
  }

  // Alimentacao sempre vem antes dos sinais
  pinMode(megaGND, OUTPUT);
  digitalWrite(megaGND, LOW);

  pinMode(megaVCC, OUTPUT);
  digitalWrite(megaVCC, HIGH);

  delay(50);
  return true;
}

// Desliga GND e VCC depois do teste
void desligarAlimentacao(const Chip &c) {
  int gnd = pinoGND(c);
  int vcc = pinoVCC(c);

  int megaGND = mapeamento(c, gnd);
  int megaVCC = mapeamento(c, vcc);

  if (megaGND != -1) {
    digitalWrite(megaGND, LOW);
    pinMode(megaGND, INPUT);
  }

  if (megaVCC != -1) {
    digitalWrite(megaVCC, LOW);
    pinMode(megaVCC, INPUT);
  }
}

// Executa uma bateria de teste de um chip
bool executaTeste(const Chip &c, int pos) {
  if (pos < 0 || pos >= c.qtdTestes || pos >= MAX_TESTES) {
    return false;
  }

  bool testeOk = true;

  // Nesta versao, GND/VCC ja devem estar ligados antes do teste
  // Liberamos apenas sinais, sem desligar alimentacao
  liberarPinosDeSinal(c);
  delay(20);

  // Garante alimentacao antes de qualquer entrada/sinal
  if (!aplicarAlimentacao(c)) {
    return false;
  }

  // Primeiro configura as saidas do chip como entradas no Arduino
  for (int pino = 0; pino < c.qtdPinos; pino++) {
    if (ehPinoAlimentacao(c, pino)) {
      continue;
    }

    if (tipoPino(c, pino) == 0) {
      int megaPin = mapeamento(c, pino);
      if (megaPin == -1) {
        testeOk = false;
        break;
      }
      pinMode(megaPin, INPUT);
    }
  }

  // Aplica os valores nas entradas de sinal do chip
  if (testeOk) {
    for (int pino = 0; pino < c.qtdPinos; pino++) {
      if (ehPinoAlimentacao(c, pino)) {
        continue;
      }

      if (tipoPino(c, pino) == 1 && c.testes[pos].valorEsperado[pino] != 2) {
        int megaPin = mapeamento(c, pino);
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

  // Confere as saidas esperadas
  if (testeOk) {
    for (int pino = 0; pino < c.qtdPinos; pino++) {
      if (ehPinoAlimentacao(c, pino)) {
        continue;
      }

      if (tipoPino(c, pino) == 0 && c.testes[pos].valorEsperado[pino] != 2) {
        int megaPin = mapeamento(c, pino);
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

  // Libera apenas sinais. Alimentacao continua ligada para o proximo teste
  liberarPinosDeSinal(c);
  delay(20);

  return testeOk;
}

// Tenta identificar o chip comparando com todos os cadastrados
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

  No *atual = listaChips;
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

// Le o proximo campo separado por ':' na string recebida, recebe apenas uma referencia para string e uma variavel gçlobal para continuar a contagem de : ate o final da string
String proximoCampo(const String &linha, int &atual) {
  //acha o proximo : a partir da posição atual
  int prox = linha.indexOf(':', atual);
  // se prox for -1 é pq indexof nao achou nada
  if (prox == -1) {
    String campo = linha.substring(atual);
    atual = linha.length() + 1;
    return campo;
  }

  String campo = linha.substring(atual, prox);
  //atualiza o atual para achar o proximo
  atual = prox + 1;
  //retorna a string entre os :
  return campo;
}

// Le a string CHIP enviada pela Serial e salva o cadastro
bool lerSerial(String linha) {
  //cria um chip e lipa todos os dados em sua memoria
  Chip c;
  inicializarChip(c);

  int atual = 0;
  //verifica se a string recebida começa com CHIP:
  String cabecalho = proximoCampo(linha, atual);
  if (cabecalho != "CHIP") {
    Serial.println("ERRO: protocolo invalido.");
    return false;
  }
  //proximoCampo le o proximo texto depois do ultimo : , no caso depois de CHIP:
  String codigo = proximoCampo(linha, atual);
  //transforma a string recebida em um array de caracteres para salvar na struct
  codigo.toCharArray(c.codigo, sizeof(c.codigo));

  //mesmo do anterior mas mpara o nome
  String nome = proximoCampo(linha, atual);
  nome.toCharArray(c.nome, sizeof(c.nome));

  //mesmo do anterior mas dessa vez transforam o texto para int para salvar
  c.qtdPinos = proximoCampo(linha, atual).toInt();

  //exibe uma mensagem de erro caso a string com a quantidade de pinos seja diferente de 14 ou 16
  if (c.qtdPinos != 14 && c.qtdPinos != 16) {
    Serial.println("ERRO: quantidade de pinos invalida.");
    return false;
  }
  //define quantos pinos tem de cada lado
  int metade = c.qtdPinos / 2;
  //salva cada nome de cada pino do array de nomes com max de 3 char
  for (int i = 0; i < c.qtdPinos; i++) {
    String nomePino = proximoCampo(linha, atual);
    nomePino.toCharArray(c.nomPinos[i], sizeof(c.nomPinos[i]));
  }
  // lê o marcador dos pinos da direita
  String marcadorDir = proximoCampo(linha, atual);
  if (marcadorDir != "DIR") {
    Serial.println("ERRO: marcador DIR nao encontrado.");
    return false;
  }
  //mesma conta do desenho dos pinos da direita de cima para baixo, ele remove um para o indice de pinos e depois remove o indice do for para ler de cima para baixo
  for (int i = 0; i < metade; i++) {
    int pino = c.qtdPinos - 1 - i;
    //depois diminui a metade para saber qual seria o indice de 0 ate a metade para adicionar ao array
    int indiceDireita = pino - metade;
    c.direita[indiceDireita] = proximoCampo(linha, atual).toInt();
  }
  // lê o marcador dos pinos da esquerda
  String marcadorEsq = proximoCampo(linha, atual);
  if (marcadorEsq != "ESQ") {
    Serial.println("ERRO: marcador ESQ nao encontrado.");
    return false;
  }
  //le os pinos da esquerda e os salva como inteiros no array
  for (int i = 0; i < metade; i++) {
    c.esquerda[i] = proximoCampo(linha, atual).toInt();
  }
  //le o marcador de testes
  String marcadorTestes = proximoCampo(linha, atual);
  if (marcadorTestes != "TESTES") {
    Serial.println("ERRO: marcador TESTES nao encontrado.");
    return false;
  }
  //le a quantidade de testes  como inteiro e salva
  c.qtdTestes = proximoCampo(linha, atual).toInt();
  //exibe erro caso tenha mais testes que o maximo permitido
  if (c.qtdTestes < 0 || c.qtdTestes > MAX_TESTES) {
    Serial.print("ERRO: quantidade de testes invalida. Maximo permitido: ");
    Serial.println(MAX_TESTES);
    return false;
  }
  // salva o valor do teste esperado como 0, 1 ou 2
  //le uma listas de 0 1 ou 2 e depois passa um for para verificar qual deles é e salvar
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
  return true;
}

// Apaga o banco de chips na eeprom
//seta o total de chips para 0 transformando todos os chips salvos na eeprom em lixo e depois chama a função para liberar a lista
void limparBanco() {
  totalChips = 0;
  EEPROM.put(END_TOTAL_CHIPS, totalChips);
  limparLista();
  Serial.println("EEPROM limpa.");
}

// Funcao para o botao chamar quando o botao é apertado
void aoApertar(GFButton &btn) {
  identificarChip();
}

// Configuracao inicial do Arduino, display, eeprom e botao
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1000);

  pinMode(24, INPUT);

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
  tft.print("e aperte o botao");
  //chama a função de identificar ao apertar o botao
  botao.setClicksHandler(aoApertar);
}

// Loop principal
void loop() {
  botao.process();
  if (Serial.available() > 0) {
    //le uma string recebida ate o \n e salva
    String linha = Serial.readStringUntil('\n');
    //remove espaços da string recebida
    linha.trim();
    //se a string tiver o identificador CHIP: ele salva o chip
    if (linha.startsWith("CHIP:")) {
      lerSerial(linha);
    } else if (linha.startsWith("Selecionar:")) {
      int n = linha.substring(linha.indexOf(':') + 1).toInt();
      selecionarChip(n);
    } else if (linha.startsWith("Identificar")) {
      identificarChip();
    } else if (linha.startsWith("Limpar")) {
      limparBanco();
    } else if (linha.length() > 0) {
      Serial.print("Comando desconhecido: ");
      Serial.println(linha);
    }
  }
}
