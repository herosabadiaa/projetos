import json
from dataclasses import dataclass
from datetime import datetime
from typing import Mapping

from sqlalchemy import (
    Column,
    DateTime,
    Integer,
    MetaData,
    String,
    Table,
    Text,
    func,
    inspect,
    select,
    update,
)
from sqlalchemy.engine import Engine
from sqlalchemy.exc import OperationalError

from deck_builder import MAX_DECK_SIZE, deck_size


CREATE_DECK_CATALOG_SQL = """\
CREATE TABLE deck_catalog (
    id INT NOT NULL AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    cards TEXT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_deck_catalog_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
"""


class DeckCatalogSetupError(RuntimeError):
    pass


metadata = MetaData()
decks_table = Table(
    "deck_catalog",
    metadata,
    Column("id", Integer, primary_key=True, autoincrement=True),
    Column("name", String(100), nullable=False, unique=True),
    Column("cards", Text, nullable=False),
    Column("created_at", DateTime, nullable=False, server_default=func.now()),
)


@dataclass(frozen=True)
class SavedDeck:
    id: int
    name: str
    cards: dict[str, int]
    created_at: datetime


def ensure_deck_catalog(engine: Engine) -> None:
    if inspect(engine).has_table(decks_table.name):
        return
    try:
        metadata.create_all(engine)
    except OperationalError as error:
        error_code = getattr(error.orig, "args", (None,))[0]
        if engine.dialect.name == "mysql" and error_code == 1142:
            if inspect(engine).has_table(decks_table.name):
                return
            raise DeckCatalogSetupError(
                "O usuário do banco não tem permissão para criar a tabela. "
                "Peça ao administrador para executar o SQL abaixo uma vez."
            ) from error
        raise


def list_saved_decks(engine: Engine) -> tuple[SavedDeck, ...]:
    with engine.connect() as connection:
        rows = connection.execute(
            select(
                decks_table.c.id,
                decks_table.c.name,
                decks_table.c.cards,
                decks_table.c.created_at,
            ).order_by(decks_table.c.name)
        ).mappings()
        return tuple(
            SavedDeck(
                id=row["id"],
                name=row["name"],
                cards=_decode_deck(row["cards"]),
                created_at=row["created_at"],
            )
            for row in rows
        )


def search_saved_decks(
    saved_decks: tuple[SavedDeck, ...], query: str
) -> tuple[SavedDeck, ...]:
    normalized_query = query.strip().casefold()
    if not normalized_query:
        return saved_decks
    return tuple(
        saved_deck
        for saved_deck in saved_decks
        if normalized_query in saved_deck.name.casefold()
        or any(normalized_query in card_name.casefold() for card_name in saved_deck.cards)
    )


def save_deck(engine: Engine, name: str, cards: Mapping[str, int]) -> int:
    normalized_name = _validate_deck(name, cards)
    with engine.begin() as connection:
        result = connection.execute(
            decks_table.insert().values(
                name=normalized_name,
                cards=json.dumps(dict(cards), ensure_ascii=False),
            )
        )
        return int(result.inserted_primary_key[0])


def update_saved_deck(engine: Engine, deck_id: int, cards: Mapping[str, int]) -> None:
    _validate_deck("deck salvo", cards)
    with engine.begin() as connection:
        result = connection.execute(
            update(decks_table)
            .where(decks_table.c.id == deck_id)
            .values(cards=json.dumps(dict(cards), ensure_ascii=False))
        )
        if result.rowcount != 1:
            raise LookupError("O deck selecionado não existe mais no catálogo.")


def _validate_deck(name: str, cards: Mapping[str, int]) -> str:
    normalized_name = name.strip()
    if not normalized_name:
        raise ValueError("Informe um nome para o deck.")
    if len(normalized_name) > 100:
        raise ValueError("O nome do deck deve ter no máximo 100 caracteres.")
    if any(
        not isinstance(card_name, str)
        or not card_name.strip()
        or not isinstance(quantity, int)
        or isinstance(quantity, bool)
        or quantity < 1
        for card_name, quantity in cards.items()
    ):
        raise ValueError("O deck contém nomes ou quantidades de cartas inválidos.")
    if deck_size(cards) != MAX_DECK_SIZE:
        raise ValueError(f"O deck precisa ter exatamente {MAX_DECK_SIZE} cartas.")
    return normalized_name


def _decode_deck(serialized_cards: str) -> dict[str, int]:
    try:
        cards = json.loads(serialized_cards)
    except json.JSONDecodeError as error:
        raise ValueError("Há um deck inválido salvo no catálogo.") from error
    if not isinstance(cards, dict):
        raise ValueError("Há um deck inválido salvo no catálogo.")
    _validate_deck("deck salvo", cards)
    return cards
