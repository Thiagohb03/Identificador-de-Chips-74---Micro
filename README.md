# Identificador de Chips 74 - Micro

Projeto acadêmico para identificação automática de chips da série 74xx utilizando Arduino Mega.

## Integrantes
- Bruno Tardin
- Matheus Marques
- Thiago Bello

## Sobre o Projeto
O sistema identifica chips lógicos da série 74xx, exibindo o resultado em um display TFT 2.4" conectado ao Arduino Mega 2560. A interface web permite a adição e o gerenciamento dos chips cadastrados e as baterias de testes para sua identificação.

## Tecnologias Utilizadas
- Arduino Mega 2560
- Display TFT 2.4" (ILI9341)
- Bibliotecas: MCUFRIEND_kbv, Adafruit GFX, EEPROM, GFButton

## Como Usar

### Identificar um Chip
1. Insira o chip no soquete
2. Pressione o botão físico
3. O sistema percorre todos os chips cadastrados na EEPROM e executa os testes
4. Se identificado, o display exibe o nome, código e diagrama de pinos do chip
5. Se não identificado, o display exibe uma mensagem de erro

### O cadastro de um Novo Chip é realizado via interface web, junto com todas as suas especificações e as baterias de testes correspondentes à esse chip através de uma string enviada via serial

Onde:
- `codigo` — código do chip (ex: 74283)
- `nome` — nome do chip (ex: Somador 4 bits)
- `qtdPinos` — 14 ou 16
- `pino0..N` — nome de cada pino
- `DIR` — marcador seguido dos tipos dos pinos do lado direito (0=saída, 1=entrada)
- `ESQ` — marcador seguido dos tipos dos pinos do lado esquerdo
- `TESTES` — marcador seguido da quantidade de testes
- `teste0..N` — string com os valores esperados de cada pino (0, 1 ou 2 para ignorado)

### Comandos de debug via Serial
| Comando | Descrição |
|---|---|
| `CHIP:...` | Cadastra um novo chip na EEPROM |
| `Identificar` | Inicia a identificação manual via serial |
| `Selecionar:N` | Exibe detalhes do chip de índice N |
| `Chips` | Lista todos os chips salvos na EEPROM |
| `Limpar` | Apaga todos os chips da EEPROM |

## Armazenamento (EEPROM)
O Arduino Mega possui 4KB de EEPROM. Cada chip ocupa aproximadamente 107 bytes, permitindo o armazenamento de até 38 chips simultaneamente. Os dados persistem mesmo após desligar o aparelho.

## Estrutura do Código (Arduino)

### `salvarChip(Chip c)`
Salva um chip na EEPROM no próximo endereço disponível e incrementa o contador total de chips.

### `carregaChip()`
Lê todos os chips salvos na EEPROM ao iniciar o sistema e os carrega em uma lista encadeada na memória RAM para acesso rápido durante a identificação.

### `identificarChip()`
Percorre a lista de chips cadastrados e executa todos os testes de cada um. Se todos os testes de um chip passarem, ele é considerado identificado e seu diagrama é exibido no display.

### `executaTeste(Chip c, int pos)`
Executa um teste específico: aplica os valores de entrada nos pinos do chip via Arduino, aguarda a resposta e compara os valores lidos nas saídas com os esperados.

### `desenharChip(Chip c)`
Desenha no display TFT o diagrama do chip com o nome, código e todos os pinos identificados com seus respectivos nomes e numeração.

### `lerSerial(String linha)`

Interpreta a linha recebida pela serial no protocolo definido e popula a struct do chip para salvamento na EEPROM.

### `limparBanco()`
Zera o contador de chips na EEPROM e limpa a lista da RAM, efetivamente apagando todos os chips cadastrados.

### 'Esquemática circuito'
<img width="838" height="439" alt="Captura de tela 2026-07-08 001026" src="https://github.com/user-attachments/assets/34d21554-b903-4026-a893-de38e7497f11" />

