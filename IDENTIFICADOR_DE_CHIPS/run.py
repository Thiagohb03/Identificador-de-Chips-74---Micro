# Importa o Flask e as funções que vamos usar
from flask import Flask, render_template, request, jsonify

# Importa a classe Chip (define o que é um chip) e as funções de armazenamento
from app.models import Chip, TestCase
from app.storage import (
    load_chips, add_chip, get_chip_by_code, update_chip, delete_chip,
    add_test, update_test, delete_test
)


# Cria o servidor Flask, apontando onde ficam os templates e arquivos estáticos
app = Flask(__name__,
            template_folder="app/templates",
            static_folder="app/templates/static")


# Rota principal: abre a página inicial
# Lê o parâmetro ?qtd_pinos= da URL para saber quantos pinos desenhar
@app.route("/")
def index():
    qtd_pinos = int(request.args.get("qtd_pinos", 14))
    if qtd_pinos not in (14, 16):
        qtd_pinos = 14
    return render_template("index.html", qtd_pinos=qtd_pinos)


# Retorna a lista de todos os chips salvos (em formato JSON)
@app.route("/chips", methods=["GET"])
def listar_chips():
    chips = load_chips()
    return jsonify([chip.to_dict() for chip in chips])


# Recebe um novo chip (em formato JSON) e salva no arquivo
@app.route("/chips", methods=["POST"])
def criar_chip():
    data = request.get_json()
    chip = Chip.from_dict(data)
    add_chip(chip)
    return jsonify({"message": "Chip criado com sucesso"}), 201


# Retorna um chip específico pelo código (ex: /chips/74283)
@app.route("/chips/<int:code>", methods=["GET"])
def buscar_chip(code):
    chip = get_chip_by_code(code)
    if not chip:
        return jsonify({"message": "Chip não encontrado"}), 404
    return jsonify(chip.to_dict())


# Atualiza os dados de um chip existente
@app.route("/chips/<int:code>", methods=["PUT"])
def atualizar_chip(code):
    data = request.get_json()
    chip = Chip.from_dict(data)
    update_chip(code, chip)
    return jsonify({"message": "Chip atualizado com sucesso"})


# Página de edição de um chip
@app.route("/editar/<int:code>")
def editar_chip(code):
    chip = get_chip_by_code(code)
    if not chip:
        return "Chip não encontrado", 404
    qtd_pinos = int(request.args.get("qtd_pinos", chip.pin_count))
    chip_tests = [test.to_dict() for test in chip.tests]
    return render_template("editar.html", chip=chip, qtd_pinos=qtd_pinos, chip_tests=chip_tests)


# Página de testes de um chip
@app.route("/testes/<int:code>")
def testes_chip(code):
    chip = get_chip_by_code(code)
    if not chip:
        return "Chip não encontrado", 404
    chip_tests = [test.to_dict() for test in chip.tests]
    return render_template("testes.html", chip=chip, chip_tests=chip_tests)


# Deleta um chip pelo código
@app.route("/chips/<int:code>", methods=["DELETE"])
def deletar_chip(code):
    delete_chip(code)
    return jsonify({"message": "Chip deletado com sucesso"})


# Adiciona um novo teste a um chip
@app.route("/chips/<int:code>/tests", methods=["POST"])
def criar_teste(code):
    data = request.get_json()
    test = TestCase.from_dict(data)
    add_test(code, test)
    return jsonify({"message": "Teste criado com sucesso"}), 201


# Atualiza um teste existente (pela posição na lista de testes do chip)
@app.route("/chips/<int:code>/tests/<int:index>", methods=["PUT"])
def atualizar_teste(code, index):
    data = request.get_json()
    test = TestCase.from_dict(data)
    update_test(code, index, test)
    return jsonify({"message": "Teste atualizado com sucesso"})


# Remove um teste (pela posição na lista de testes do chip)
@app.route("/chips/<int:code>/tests/<int:index>", methods=["DELETE"])
def deletar_teste(code, index):
    delete_test(code, index)
    return jsonify({"message": "Teste deletado com sucesso"})




# Envia todos os chips salvos para a serial do Arduino
@app.route("/chips/enviar-todos", methods=["POST"])
def enviar_todos_serial():
    chips = load_chips()
    if not chips:
        return jsonify({"message": "Nenhum chip cadastrado"}), 404

    try:
        from serial import Serial
        meu_serial = Serial("/dev/serial0", baudrate=9600)
        for chip in chips:
            texto = chip.to_arduino_protocol() + "\n"
            meu_serial.write(texto.encode("UTF-8"))
        meu_serial.close()
        return jsonify({"message": f"{len(chips)} chip(s) enviado(s) com sucesso"})
    except Exception as e:
        return jsonify({"message": f"Erro ao enviar: {str(e)}"}), 500


# Envia o protocolo do chip para a serial do Arduino
@app.route("/chips/<int:code>/enviar-serial", methods=["POST"])
def enviar_serial(code):
    chip = get_chip_by_code(code)
    if not chip:
        return jsonify({"message": "Chip não encontrado"}), 404

    try:
        from serial import Serial
        meu_serial = Serial("/dev/serial0", baudrate=9600)
        texto = chip.to_arduino_protocol() + "\n"
        meu_serial.write(texto.encode("UTF-8"))
        meu_serial.close()
        return jsonify({"message": "Enviado para o Arduino com sucesso"})
    except Exception as e:
        return jsonify({"message": f"Erro ao enviar: {str(e)}"}), 500


# Inicia o servidor quando o arquivo é executado diretamente
if __name__ == "__main__":
    app.run(debug=True)
