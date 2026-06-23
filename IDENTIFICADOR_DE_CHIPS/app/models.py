from typing import List, Dict


# Representa um caso de teste: quais valores entram e quais saídas são esperadas
class TestCase:
    def __init__(self, inputs: Dict[int, int], outputs: Dict[int, int]):
        self.inputs = inputs
        self.outputs = outputs

    def to_dict(self) -> Dict:
        return {
            "inputs": self.inputs,
            "outputs": self.outputs
        }

    def from_dict(data: Dict) -> "TestCase":
        return TestCase(
            inputs=data["inputs"],
            outputs=data["outputs"]
        )


# Representa um chip 74xx com seus pinos e casos de teste
class Chip:
    def __init__(
        self,
        name: str,
        code: int,
        pin_count: int,
        input_pins: List[int],
        output_pins: List[int],
        tests: List[TestCase],
        pin_names: Dict[str, str] = None
    ):
        self.name = name
        self.code = code
        self.pin_count = pin_count
        self.input_pins = input_pins
        self.output_pins = output_pins
        self.tests = tests
        self.pin_names = pin_names or {}

    # Converte o objeto para dicionário (usado para salvar no JSON)
    def to_dict(self) -> Dict:
        return {
            "name": self.name,
            "code": self.code,
            "pin_count": self.pin_count,
            "input_pins": self.input_pins,
            "output_pins": self.output_pins,
            "tests": [test.to_dict() for test in self.tests],
            "pin_names": self.pin_names
        }

    # Reconstrói um objeto Chip a partir de um dicionário lido do JSON
    def from_dict(data: Dict) -> "Chip":
        return Chip(
            name=data["name"],
            code=data["code"],
            pin_count=data["pin_count"],
            input_pins=data["input_pins"],
            output_pins=data["output_pins"],
            tests=[
                TestCase.from_dict(test)
                for test in data.get("tests", [])
            ],
            pin_names=data.get("pin_names", {})
        )

    # VCC é sempre o último pino; 
    def vcc_pin(self) -> int:
        return self.pin_count - 1

    def gnd_pin(self) -> int:
        if self.pin_count == 14:
            return 6
        return 7

    def add_test(self, test: TestCase):
        self.tests.append(test)

    def to_arduino_protocol(self) -> str:
        parts = ["CHIP", str(self.code), self.name, str(self.pin_count)]

        # Nomes dos pinos em ordem (0 a pin_count-1)
        for i in range(self.pin_count):
            if i == self.vcc_pin():
                parts.append("VCC")
            elif i == self.gnd_pin():
                parts.append("GND")
            else:
                parts.append(self.pin_names.get(str(i), str(i)))

        # DIR: pinos da direita, do topo para baixo
        parts.append("DIR")
        for i in range(self.pin_count // 2):
            pino = self.pin_count - 1 - i
            if pino in (self.vcc_pin(), self.gnd_pin()):
                parts.append("2")
            elif pino in self.input_pins:
                parts.append("0")
            elif pino in self.output_pins:
                parts.append("1")
            else:
                parts.append("2")

        # ESQ: pinos da esquerda, do topo para baixo
        parts.append("ESQ")
        for i in range(self.pin_count // 2):
            pino = i
            if pino in (self.vcc_pin(), self.gnd_pin()):
                parts.append("2")
            elif pino in self.input_pins:
                parts.append("0")
            elif pino in self.output_pins:
                parts.append("1")
            else:
                parts.append("2")

        # TESTES
        parts.append("TESTES")
        parts.append(str(len(self.tests)))
        for test in self.tests:
            test_str = ""
            for i in range(self.pin_count):
                if i in (self.vcc_pin(), self.gnd_pin()):
                    test_str += "0"
                elif i in self.input_pins:
                    test_str += str(test.inputs.get(str(i), test.inputs.get(i, 0)))
                elif i in self.output_pins:
                    test_str += str(test.outputs.get(str(i), test.outputs.get(i, 0)))
                else:
                    test_str += "0"
            parts.append(test_str)

        return ":".join(parts)
