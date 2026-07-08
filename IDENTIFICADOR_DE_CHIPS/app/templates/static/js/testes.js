// Globais usados na tela de testes, para montar/listar os testes do chip
let pinosTesteEntrada = [];
let pinosTesteSaida   = [];
let testesAtuais      = [];
let nomesPinos        = {};

// Globais usados na tela de testes, identificam o chip sendo testado
let codigoChip   = null;
let nomeChip     = "";
let qtdPinosChip = 0;

// Índice do teste sendo editado inline (-1 = nenhum)
let editandoIndice = -1;

// TESTES (casos de teste do chip)
function adicionarTeste() {
    const inputs = {};
    pinosTesteEntrada.forEach(pino => {
        inputs[pino] = Number(document.getElementById(`teste-e-${pino}`).value);
    });

    const outputs = {};
    pinosTesteSaida.forEach(pino => {
        outputs[pino] = Number(document.getElementById(`teste-s-${pino}`).value);
    });

    testesAtuais.push({ inputs, outputs });
    renderizarTestes();
}

function editarTeste(indice) {
    editandoIndice = indice;
    renderizarTestes();
}

function cancelarEdicao() {
    editandoIndice = -1;
    renderizarTestes();
}

function salvarEdicaoTeste(indice) {
    const inputs = {};
    pinosTesteEntrada.forEach(pino => {
        inputs[pino] = Number(document.getElementById(`edit-e-${pino}`).value);
    });

    const outputs = {};
    pinosTesteSaida.forEach(pino => {
        outputs[pino] = Number(document.getElementById(`edit-s-${pino}`).value);
    });

    testesAtuais[indice] = { inputs, outputs };
    editandoIndice = -1;
    renderizarTestes();
}

function excluirTeste(indice) {
    if (editandoIndice === indice) editandoIndice = -1;
    testesAtuais.splice(indice, 1);
    renderizarTestes();
}

// SALVAR TESTES (mantém nome/código/pinos como estão, só atualiza os testes)
async function salvarTestes() {
    const chip = {
        name:        nomeChip,
        code:        codigoChip,
        pin_count:   qtdPinosChip,
        input_pins:  pinosTesteEntrada,
        output_pins: pinosTesteSaida,
        tests:       testesAtuais
    };

    try {
        const resposta = await fetch(`/chips/${codigoChip}`, {
            method:  "PUT",
            headers: { "Content-Type": "application/json" },
            body:    JSON.stringify(chip)
        });

        if (!resposta.ok) { alert("Erro ao salvar os testes."); return; }

        alert("Testes salvos com sucesso!");
        window.location = "/";
    } catch (erro) {
        alert("Erro de conexão com o servidor.");
        console.error(erro);
    }
}

function formatarPinos(obj) {
    return Object.entries(obj)
        .map(([pino, val]) => `${pino}: ${val == 2 ? "NC" : val}`)
        .join(", ");
}

function renderizarSelectPino(idPrefix, pino, valorAtual) {
    const nome  = nomesPinos[pino] || nomesPinos[String(pino)] || "";
    const label = nome ? `${pino} (${nome})` : pino;
    const sel0  = valorAtual == 0 ? "selected" : "";
    const sel1  = valorAtual == 1 ? "selected" : "";
    const sel2  = valorAtual == 2 ? "selected" : "";
    return `<label>${label}</label>
        <select id="${idPrefix}-${pino}">
            <option value="0" ${sel0}>0</option>
            <option value="1" ${sel1}>1</option>
            <option value="2" ${sel2}>Não incluir</option>
        </select> `;
}

function renderizarFormEdicao(teste, indice) {
    let html = '<div style="margin-top:12px; border-top:1px solid #ccc; padding-top:12px;">';

    html += '<p>Valores dos pinos de entrada</p>';
    pinosTesteEntrada.forEach(pino => {
        const val = teste.inputs[pino] ?? teste.inputs[String(pino)] ?? 0;
        html += renderizarSelectPino("edit-e", pino, val);
    });

    html += '<p>Valores dos pinos de saída</p>';
    pinosTesteSaida.forEach(pino => {
        const val = teste.outputs[pino] ?? teste.outputs[String(pino)] ?? 0;
        html += renderizarSelectPino("edit-s", pino, val);
    });

    html += `<br><br>
        <button onclick="salvarEdicaoTeste(${indice})">Salvar alterações</button>
        <button onclick="cancelarEdicao()">Cancelar</button>`;
    html += '</div>';
    return html;
}

function renderizarTestes() {
    const div = document.getElementById("lista-testes");
    if (!div) return;

    if (testesAtuais.length === 0) {
        div.innerHTML = "<p>Nenhum teste cadastrado.</p>";
        return;
    }

    div.innerHTML = testesAtuais.map((teste, i) => {
        const editando = editandoIndice === i;
        return `
        <div class="cartao">
            Entradas: ${formatarPinos(teste.inputs)}<br>
            Saídas: ${formatarPinos(teste.outputs)}
            <br>
            ${!editando ? `
                <button onclick="editarTeste(${i})">Editar</button>
                <button class="btn-excluir" onclick="excluirTeste(${i})">Excluir</button>
            ` : ''}
            ${editando ? renderizarFormEdicao(teste, i) : ''}
        </div>`;
    }).join("");
}
