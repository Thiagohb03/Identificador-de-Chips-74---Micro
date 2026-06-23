// Globais usados na tela de testes, para montar/listar os testes do chip
let pinosTesteEntrada = [];
let pinosTesteSaida   = [];
let testesAtuais      = [];

// Globais usados na tela de testes, identificam o chip sendo testado
let codigoChip   = null;
let nomeChip     = "";
let qtdPinosChip = 0;

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
    const teste = testesAtuais[indice];

    pinosTesteEntrada.forEach(pino => {
        document.getElementById(`teste-e-${pino}`).value = teste.inputs[pino];
    });
    pinosTesteSaida.forEach(pino => {
        document.getElementById(`teste-s-${pino}`).value = teste.outputs[pino];
    });

    testesAtuais.splice(indice, 1);
    renderizarTestes();
}

function excluirTeste(indice) {
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

function renderizarTestes() {
    const div = document.getElementById("lista-testes");
    if (!div) return;

    if (testesAtuais.length === 0) {
        div.innerHTML = "<p>Nenhum teste cadastrado.</p>";
        return;
    }

    div.innerHTML = testesAtuais.map((teste, i) => `
        <div class="cartao">
            Entradas: ${JSON.stringify(teste.inputs)}<br>
            Saídas: ${JSON.stringify(teste.outputs)}
            <br>
            <button onclick="editarTeste(${i})">Editar</button>
            <button class="btn-excluir" onclick="excluirTeste(${i})">Excluir</button>
        </div>
    `).join("");
}
