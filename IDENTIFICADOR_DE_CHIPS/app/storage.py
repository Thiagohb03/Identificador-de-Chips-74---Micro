import json
from pathlib import Path
from typing import List, Optional

from app.models import Chip, TestCase


DATA_FILE = Path("data/chips.json")


# Lê o JSON e converte cada entrada em um objeto Chip
def load_chips() -> List[Chip]:
    if not DATA_FILE.exists():
        return []

    with open(DATA_FILE, "r", encoding="utf-8") as f:
        data = json.load(f)
        return [Chip.from_dict(chip) for chip in data]


# Converte a lista de Chips para dicionários e sobrescreve o JSON
def save_chips(chips: List[Chip]):
    DATA_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(DATA_FILE, "w", encoding="utf-8") as f:
        json.dump(
            [chip.to_dict() for chip in chips],
            f,
            indent=4, 
            ensure_ascii=False
        )


def get_chip_by_code(code: int) -> Optional[Chip]:
    chips = load_chips()

    for chip in chips:
        if chip.code == code:
            return chip

    return None


def add_chip(chip: Chip):
    chips = load_chips()
    chips.append(chip)
    save_chips(chips)


# Substitui o chip cujo código bate com o do chip atualizado
def update_chip(original_code: int, updated_chip: Chip):
    chips = load_chips()

    for i, chip in enumerate(chips):
        if chip.code == original_code:
            chips[i] = updated_chip
            break

    save_chips(chips)


def delete_chip(code: int):
    chips = load_chips()
    chips = [chip for chip in chips if chip.code != code]
    save_chips(chips)


# Adiciona um teste a lista de testes do chip
def add_test(code: int, test: TestCase):
    chip = get_chip_by_code(code)
    if not chip:
        return

    chip.tests.append(test)
    update_chip(code, chip)


# Substitui o teste numa posição (índice) da lista d    e testes do chip
def update_test(code: int, index: int, updated_test: TestCase):
    chip = get_chip_by_code(code)
    chip.tests[index] = updated_test
    update_chip(code, chip)


# Remove o teste numa posição (índice) da lista de testes do chip
def delete_test(code: int, index: int):
    chip = get_chip_by_code(code)
    del chip.tests[index]
    update_chip(code, chip)
