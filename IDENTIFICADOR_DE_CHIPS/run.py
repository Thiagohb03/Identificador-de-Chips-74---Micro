# Importa o Flask e as funções que vamos usar
from flask import Flask, render_template, request, jsonify

# Importa a classe Chip e as funções de armazenamento
from app.models import Chip, TestCase
from app.storage import (
    load_chips, add_chip, get_chip_by_code, update_chip, delete_chip,
    add_test, update_test, delete_test
)


# Cria o servidor Flask, apontando onde ficam os templates e arquivos estáticos
app = Flask(
    __name__,
    template_folder="app/templates",
    static_folder="app/templates/static"
)


# Serial global para manter a COM5 aberta
# Assim o Arduino não reseta toda vez que um chip é enviado
arduino_serial = None


def enviar_texto_arduino(texto): 
    global arduino_serial

    from serial import Serial
    import time

    # Abre a serial só uma vez
    if arduino_serial is None or not arduino_serial.is_open:
        arduino_serial = Serial("COM5", baudrate=9600, timeout=0.5)

        # Espera o Arduino reiniciar e terminar o setup
        time.sleep(4.0)


    # Envia a string completa para o Arduino
    arduino_serial.write(texto.encode("UTF-8"))
    arduino_serial.flush()

    # Espera resposta do Arduino
    inicio = time.time()

    while time.time() - inicio < 8:
        linha = arduino_serial.readline().decode("UTF-8", errors="ignore").strip()

        if "SALVO" in linha:
            return True, "Chip enviado e salvo no Arduino."

        if "ERRO" in linha:
            return False, linha

    return False, "Arduino não confirmou o recebimento."


# Rota principal
@app.route("/")
def index():
    return render_template("index.html")


# Página de cadastro
@app.route("/cadastrar")
def cadastrar():
    qtd_pinos = int(request.args.get("qtd_pinos", 14))

    if qtd_pinos not in (14, 16):
        qtd_pinos = 14

    return render_template("cadastrar.html", qtd_pinos=qtd_pinos)


# Lista todos os chips
@app.route("/chips", methods=["GET"])
def listar_chips():
    chips = load_chips()
    return jsonify([chip.to_dict() for chip in chips])


# Cria novo chip
@app.route("/chips", methods=["POST"])
def criar_chip():
    data = request.get_json()
    chip = Chip.from_dict(data)
    add_chip(chip)

    return jsonify({"message": "Chip criado com sucesso"}), 201


# Busca chip pelo código
@app.route("/chips/<int:code>", methods=["GET"])
def buscar_chip(code):
    chip = get_chip_by_code(code)

    if not chip:
        return jsonify({"message": "Chip não encontrado"}), 404

    return jsonify(chip.to_dict())


# Atualiza chip existente
@app.route("/chips/<int:code>", methods=["PUT"])
def atualizar_chip(code):
    data = request.get_json()
    chip = Chip.from_dict(data)
    update_chip(code, chip)

    return jsonify({"message": "Chip atualizado com sucesso"})


# Página de edição
@app.route("/editar/<int:code>")
def editar_chip(code):
    chip = get_chip_by_code(code)

    if not chip:
        return "Chip não encontrado", 404

    qtd_pinos = int(request.args.get("qtd_pinos", chip.pin_count))
    chip_tests = [test.to_dict() for test in chip.tests]

    return render_template(
        "editar.html",
        chip=chip,
        qtd_pinos=qtd_pinos,
        chip_tests=chip_tests
    )


# Página de testes
@app.route("/testes/<int:code>")
def testes_chip(code):
    chip = get_chip_by_code(code)

    if not chip:
        return "Chip não encontrado", 404

    chip_tests = [test.to_dict() for test in chip.tests]

    return render_template(
        "testes.html",
        chip=chip,
        chip_tests=chip_tests
    )


# Deleta chip
@app.route("/chips/<int:code>", methods=["DELETE"])
def deletar_chip(code):
    delete_chip(code)

    return jsonify({"message": "Chip deletado com sucesso"})


# Cria novo teste para um chip
@app.route("/chips/<int:code>/tests", methods=["POST"])
def criar_teste(code):
    data = request.get_json()
    test = TestCase.from_dict(data)
    add_test(code, test)

    return jsonify({"message": "Teste criado com sucesso"}), 201


# Atualiza um teste existente
@app.route("/chips/<int:code>/tests/<int:index>", methods=["PUT"])
def atualizar_teste(code, index):
    data = request.get_json()
    test = TestCase.from_dict(data)
    update_test(code, index, test)

    return jsonify({"message": "Teste atualizado com sucesso"})


# Remove um teste
@app.route("/chips/<int:code>/tests/<int:index>", methods=["DELETE"])
def deletar_teste(code, index):
    delete_test(code, index)

    return jsonify({"message": "Teste deletado com sucesso"})


# Envia um chip individual para o Arduino
@app.route("/chips/<int:code>/enviar-serial", methods=["POST"])
def enviar_serial(code):
    chip = get_chip_by_code(code)

    if not chip:
        return jsonify({"message": "Chip não encontrado"}), 404

    try:
        texto = chip.to_arduino_protocol() + "\n"
        ok, mensagem = enviar_texto_arduino(texto)

        if ok:
            return jsonify({"message": mensagem})

        return jsonify({"message": mensagem}), 500

    except Exception as e:
        return jsonify({"message": f"Erro ao enviar: {str(e)}"}), 500


# Inicia o servidor
if __name__ == "__main__":
    # use_reloader=False evita o Flask abrir dois processos e disputar a COM5
    app.run(debug=True, use_reloader=False)
