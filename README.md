# Identificador de Chips 74xx - Microcontroladores

Projeto acadêmico para identificação automática de circuitos integrados da família **74xx**, utilizando **Arduino Mega 2560**, **interface web em Flask**, comunicação **Serial USB** e armazenamento persistente em **EEPROM**.

## Integrantes

- Bruno Tardin
- Matheus Marques
- Thiago Bello

---

## Visão Geral

O sistema permite cadastrar chips lógicos da série 74xx, definir seus pinos, criar baterias de testes e enviar essas informações para o Arduino.

Depois que os chips são cadastrados no Arduino, o sistema consegue identificar automaticamente um chip inserido no soquete. Para isso, o Arduino executa os testes cadastrados, compara os valores lidos com os valores esperados e exibe o resultado em um display TFT.

A arquitetura do projeto é dividida em duas partes principais:

1. **Aplicação Web com Flask**
   - Responsável pelo cadastro, edição, exclusão e organização dos chips e testes.
   - Salva os dados localmente em um arquivo JSON.
   - Envia os chips cadastrados para o Arduino pela comunicação serial.

2. **Sistema Embarcado no Arduino**
   - Recebe os dados enviados pelo Flask.
   - Interpreta o protocolo serial.
   - Salva os chips na EEPROM.
   - Executa os testes físicos nos pinos do chip.
   - Exibe o resultado da identificação no display TFT.

---

## Arquitetura Geral

```mermaid
flowchart LR
    A[Usuário] --> B[Interface Web]
    B --> C[Servidor Flask]
    C --> D[Arquivo JSON]
    C --> E[Serial USB]
    E --> F[Arduino Mega]
    F --> G[EEPROM]
    F --> H[Soquete do Chip]
    F --> I[Display TFT]
    F --> J[Botão Físico]
