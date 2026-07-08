// Listas globais: guardam os pinos marcados pelo usuário (criar/editar chip)
let pinosEntrada = [];
let pinosSaida   = [];

// Globais usados na tela de editar, identificam o chip e os testes originais
let codigoChipOriginal = null;
let testesOriginais    = [];

// Nomes dos pinos: chave = número do pino (string), valor = nome
let nomesDosPinos = {};

function lerNomesPinos() {
    nomesDosPinos = {};
    document.querySelectorAll('[id^="nome-pino-"]').forEach(input => {
        if (input.value.trim()) {
            const pino = input.id.replace("nome-pino-", "");
            nomesDosPinos[pino] = input.value.trim();
        }
    });
}

// ALTERNAR PINO — toggle simples entre E (entrada) e S (saída)
function alternarPino(botao, numero) {
    if (botao.dataset.tipo === "S") {
        botao.dataset.tipo = "E";
        botao.textContent  = "E";
        botao.style.color  = "blue";
        pinosEntrada.push(numero);
        pinosEntrada.sort((a, b) => a - b);
        pinosSaida = pinosSaida.filter(p => p !== numero);
    } else {
        botao.dataset.tipo = "S";
        botao.textContent  = "S";
        botao.style.color  = "red";
        pinosSaida.push(numero);
        pinosSaida.sort((a, b) => a - b);
        pinosEntrada = pinosEntrada.filter(p => p !== numero);
    }

    atualizarResumo();
}

function atualizarResumo() {
    document.getElementById("lista-entradas").textContent = JSON.stringify(pinosEntrada);
    document.getElementById("lista-saidas").textContent   = JSON.stringify(pinosSaida);
}


// SALVAR CHIP
async function salvarChip() {
    const nome     = document.getElementById("nome").value.trim();
    const codigo   = Number(document.getElementById("codigo").value);
    const qtdPinos = Number(document.getElementById("qtd-pinos").value);

    if (!nome)   { alert("Informe o nome do chip.");   return; }
    if (!codigo) { alert("Informe o código do chip."); return; }

    lerNomesPinos();

    const chip = {
        name:        nome,
        code:        codigo,
        pin_count:   qtdPinos,
        input_pins:  pinosEntrada,
        output_pins: pinosSaida,
        pin_names:   nomesDosPinos,
        tests:       []
    };

    try {
        const resposta = await fetch("/chips", {
            method:  "POST",
            headers: { "Content-Type": "application/json" },
            body:    JSON.stringify(chip)
        });

        if (!resposta.ok) { alert("Erro ao salvar o chip."); return; }

        alert("Chip salvo com sucesso!");
        window.location = "/";

    } catch (erro) {
        alert("Erro de conexão com o servidor.");
        console.error(erro);
    }
}


// SALVAR EDIÇÃO
async function salvarEdicao() {
    const nome     = document.getElementById("nome").value.trim(); 
    const codigo   = Number(document.getElementById("codigo").value);
    const qtdPinos = Number(document.getElementById("qtd-pinos").value);

    if (!nome)   { alert("Informe o nome do chip.");   return; }
    if (!codigo) { alert("Informe o código do chip."); return; }

    lerNomesPinos();

    const chip = {
        name:        nome,
        code:        codigo,
        pin_count:   qtdPinos,
        input_pins:  pinosEntrada,
        output_pins: pinosSaida,
        pin_names:   nomesDosPinos,
        tests:       testesOriginais
    };

    try {
        const resposta = await fetch(`/chips/${codigoChipOriginal}`, {
            method:  "PUT",
            headers: { "Content-Type": "application/json" },
            body:    JSON.stringify(chip)
        });

        if (!resposta.ok) { alert("Erro ao salvar o chip."); return; }

        alert("Chip atualizado com sucesso!");
        window.location = "/";
    } catch (erro) {
        alert("Erro de conexão com o servidor.");
        console.error(erro);
    }
}


// EXCLUIR CHIP
async function excluirChip(code) {
    if (!confirm("Tem certeza que deseja excluir este chip?")) return;

    try {
        const resposta = await fetch(`/chips/${code}`, { method: "DELETE" });

        if (!resposta.ok) { alert("Erro ao excluir o chip."); return; }

        carregarChips();
    } catch (erro) {
        alert("Erro de conexão com o servidor.");
        console.error(erro);
    }
}


// ENVIAR CHIP PARA O ARDUINO
async function enviarChip(code) {
    try {
        const resposta = await fetch(`/chips/${code}/enviar-serial`, { method: "POST" });
        const dados = await resposta.json();
        alert(dados.message);
    } catch (erro) {
        alert("Erro de conexão com o servidor.");
        console.error(erro);
    }
}


// DIAGRAMA DO CHIP PARA LEITURA
function renderizarPino(numero, pinVcc, pinGnd, chip) {
    if (numero === pinVcc) return `<button disabled>VCC</button>`;
    if (numero === pinGnd) return `<button disabled>GND</button>`;

    if (chip.input_pins.includes(numero))  return `<button disabled style="color:blue">E</button>`;
    if (chip.output_pins.includes(numero)) return `<button disabled style="color:red">S</button>`;

    return `<button disabled>${numero}</button>`;
}

function renderizarDiagramaChip(chip) {
    const pinVcc = chip.pin_count - 1;
    const pinGnd = chip.pin_count === 14 ? 6 : 7;
    let rows = "";

    for (let i = 0; i < chip.pin_count / 2; i++) {
        const pinoEsq = i;
        const pinoDir = chip.pin_count - 1 - i;
        rows += `
            <tr>
                <td>${renderizarPino(pinoEsq, pinVcc, pinGnd, chip)}</td>
                <td class="num-pino esq">${pinoEsq}</td>
                <td class="corpo-chip">${i === 0 ? "74xx" : ""}</td>
                <td class="num-pino dir">${pinoDir}</td>
                <td>${renderizarPino(pinoDir, pinVcc, pinGnd, chip)}</td>
            </tr>`;
    }

    return `<table class="tabela-chip">${rows}</table>`;
}


// CARREGAR LISTA DE CHIPS
async function carregarChips() {
    const div = document.getElementById("lista-chips");

    try {
        const resposta = await fetch("/chips");
        const chips    = await resposta.json();

        if (chips.length === 0) {
            div.innerHTML = "<p>Nenhum chip salvo ainda.</p>";
            return;
        }

        div.innerHTML = chips.map(chip => {
            const nomes = chip.pin_names && Object.keys(chip.pin_names).length > 0
                ? `<br>Nomes: ${Object.entries(chip.pin_names).map(([p, n]) => `${p}: ${n}`).join(", ")}`
                : "";

            return `
            <div class="cartao">
                <b>${chip.name}</b> - Código ${chip.code} - ${chip.pin_count} pinos<br>
                Entradas: ${JSON.stringify(chip.input_pins)}<br>
                Saídas: ${JSON.stringify(chip.output_pins)}${nomes}
                ${renderizarDiagramaChip(chip)}
                <br>
                <button onclick="window.location='/editar/${chip.code}'">Editar chip</button>
                <button onclick="window.location='/testes/${chip.code}'">Testes</button>
                <button onclick="enviarChip(${chip.code})">Enviar chip</button>
                <button class="btn-excluir" onclick="excluirChip(${chip.code})">Excluir</button>
            </div>`;
        }).join("");
    } catch (erro) {
        alert("Erro ao carregar chips.");
        console.error(erro);
    }
}