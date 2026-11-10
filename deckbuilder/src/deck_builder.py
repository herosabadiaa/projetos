import csv
import io
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping


MAX_DECK_SIZE = 60
CSV_COLUMNS = ("name", "hp", "atk", "mana", "image", "type", "subtype")


@dataclass(frozen=True)
class Card:
    name: str
    hp: int
    atk: int
    mana: int
    image: str
    card_type: str
    subtype: str

    @classmethod
    def from_row(cls, row: list[str], line_number: int) -> "Card":
        if len(row) != len(CSV_COLUMNS):
            raise ValueError(
                f"Linha {line_number}: esperadas {len(CSV_COLUMNS)} colunas, "
                f"mas foram encontradas {len(row)}."
            )

        name, hp, atk, mana, image, card_type, subtype = (
            value.strip() for value in row
        )
        if not name or not image:
            raise ValueError(f"Linha {line_number}: nome ou imagem vazio.")

        try:
            return cls(
                name=name,
                hp=int(hp),
                atk=int(atk),
                mana=int(mana),
                image=image,
                card_type=card_type,
                subtype=subtype,
            )
        except ValueError as error:
            raise ValueError(
                f"Linha {line_number}: HP, ATK e mana devem ser números inteiros."
            ) from error

    def to_row(self) -> list[str]:
        return [
            self.name,
            str(self.hp),
            str(self.atk),
            str(self.mana),
            self.image,
            self.card_type,
            self.subtype,
        ]


def load_catalog(path: Path) -> tuple[Card, ...]:
    cards: list[Card] = []
    names: set[str] = set()

    with path.open(encoding="utf-8-sig", newline="") as catalog_file:
        for line_number, row in enumerate(csv.reader(catalog_file), start=1):
            if not row or not any(value.strip() for value in row):
                continue
            card = Card.from_row(row, line_number)
            if card.name in names:
                raise ValueError(f"Linha {line_number}: carta duplicada '{card.name}'.")
            names.add(card.name)
            cards.append(card)

    if not cards:
        raise ValueError(f"O catálogo {path} não contém cartas.")
    return tuple(cards)


def deck_size(deck: Mapping[str, int]) -> int:
    return sum(deck.values())


def parse_deck_csv(content: bytes, catalog: tuple[Card, ...]) -> dict[str, int]:
    try:
        text = content.decode("utf-8-sig")
    except UnicodeDecodeError as error:
        raise ValueError("O arquivo deve estar codificado em UTF-8.") from error

    rows = [
        (line_number, row)
        for line_number, row in enumerate(csv.reader(io.StringIO(text, newline="")), 1)
        if row and any(value.strip() for value in row)
    ]
    if not rows:
        raise ValueError("O arquivo CSV está vazio.")

    first_row = [value.strip().lower() for value in rows[0][1]]
    has_header = "name" in first_row and (
        "quantity" in first_row
        or all(column in first_row for column in CSV_COLUMNS)
    )
    if has_header:
        header_line, header_values = rows.pop(0)
        header = [value.strip().lower() for value in header_values]
        if "name" not in header or not (
            "quantity" in header or all(column in header for column in CSV_COLUMNS)
        ):
            raise ValueError(
                f"Linha {header_line}: use as colunas name,quantity ou o formato "
                "completo do catálogo."
            )
        name_index = header.index("name")
        quantity_index = header.index("quantity") if "quantity" in header else None
    else:
        name_index = 0
        quantity_index = None

    catalog_by_name = {card.name.casefold(): card.name for card in catalog}
    deck: dict[str, int] = {}
    for line_number, row in rows:
        if has_header:
            if name_index >= len(row):
                raise ValueError(f"Linha {line_number}: coluna name ausente.")
            if quantity_index is not None and quantity_index >= len(row):
                raise ValueError(f"Linha {line_number}: coluna quantity ausente.")
            name = row[name_index].strip()
            raw_quantity = row[quantity_index].strip() if quantity_index is not None else "1"
        else:
            if len(row) != len(CSV_COLUMNS):
                raise ValueError(
                    f"Linha {line_number}: o CSV sem cabeçalho deve ter "
                    f"{len(CSV_COLUMNS)} colunas."
                )
            name = row[0].strip()
            raw_quantity = "1"

        canonical_name = catalog_by_name.get(name.casefold())
        if canonical_name is None:
            raise ValueError(f"Linha {line_number}: carta '{name}' não encontrada no catálogo.")
        try:
            quantity = int(raw_quantity)
        except ValueError as error:
            raise ValueError(
                f"Linha {line_number}: a quantidade deve ser um número inteiro."
            ) from error
        if quantity < 1:
            raise ValueError(f"Linha {line_number}: a quantidade deve ser pelo menos 1.")

        deck[canonical_name] = deck.get(canonical_name, 0) + quantity
        if deck_size(deck) > MAX_DECK_SIZE:
            raise ValueError(f"O deck importado ultrapassa o limite de {MAX_DECK_SIZE} cartas.")

    if not deck:
        raise ValueError("O arquivo CSV não contém cartas.")
    return deck


def export_deck_csv(deck: Mapping[str, int], catalog: tuple[Card, ...]) -> bytes:
    cards_by_name = {card.name: card for card in catalog}
    output = io.StringIO(newline="")
    writer = csv.writer(output, lineterminator="\n")

    for name, quantity in deck.items():
        card = cards_by_name.get(name)
        if card is None:
            raise ValueError(f"A carta '{name}' não existe no catálogo.")
        if quantity < 1:
            raise ValueError(f"A quantidade da carta '{name}' deve ser pelo menos 1.")
        for _ in range(quantity):
            writer.writerow(card.to_row())

    return output.getvalue().encode("utf-8")
