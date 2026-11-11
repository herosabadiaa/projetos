import sys
import unittest
from datetime import datetime
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from sqlalchemy import create_engine
from sqlalchemy.exc import OperationalError

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from deck_catalog import (
    DeckCatalogSetupError,
    SavedDeck,
    ensure_deck_catalog,
    list_saved_decks,
    save_deck,
    search_saved_decks,
    update_saved_deck,
)


class DeckCatalogTests(unittest.TestCase):
    def setUp(self) -> None:
        self.engine = create_engine("sqlite:///:memory:")
        ensure_deck_catalog(self.engine)
        self.deck = {f"Carta {number}": 1 for number in range(60)}

    def tearDown(self) -> None:
        self.engine.dispose()

    def test_saves_and_lists_a_deck(self) -> None:
        deck_id = save_deck(self.engine, "  Meu deck  ", self.deck)

        saved_decks = list_saved_decks(self.engine)

        self.assertEqual(len(saved_decks), 1)
        self.assertEqual(saved_decks[0].id, deck_id)
        self.assertEqual(saved_decks[0].name, "Meu deck")
        self.assertEqual(saved_decks[0].cards, self.deck)

    def test_updates_a_saved_deck(self) -> None:
        deck_id = save_deck(self.engine, "Meu deck", self.deck)
        updated_deck = {
            "Carta alterada": 1,
            **{f"Carta {number}": 1 for number in range(59)},
        }

        update_saved_deck(self.engine, deck_id, updated_deck)

        self.assertEqual(list_saved_decks(self.engine)[0].cards, updated_deck)

    def test_rejects_decks_that_do_not_have_60_cards(self) -> None:
        with self.assertRaisesRegex(ValueError, "exatamente 60"):
            save_deck(self.engine, "Incompleto", {"Carta": 1})

    def test_filters_saved_decks_by_name_and_selected_card(self) -> None:
        created_at = datetime.now()
        first_deck = SavedDeck(1, "Dragões", {"Dragão Branco": 60}, created_at)
        second_deck = SavedDeck(2, "Guerreiros", {"Mago Negro": 60}, created_at)
        saved_decks = (first_deck, second_deck)

        self.assertEqual(
            search_saved_decks(saved_decks, name_query="DRAG"),
            (first_deck,),
        )
        self.assertEqual(
            search_saved_decks(saved_decks, card_name="mAgO nEgRo"),
            (second_deck,),
        )
        self.assertEqual(
            search_saved_decks(saved_decks, name_query="drag", card_name="Mago Negro"),
            (),
        )
        self.assertEqual(search_saved_decks(saved_decks), saved_decks)

    def test_explains_manual_setup_when_mysql_denies_table_creation(self) -> None:
        mysql_engine = SimpleNamespace(dialect=SimpleNamespace(name="mysql"))
        permission_error = OperationalError("CREATE TABLE", {}, Exception(1142, "denied"))

        with (
            patch("deck_catalog.inspect") as inspect_engine,
            patch("deck_catalog.metadata.create_all", side_effect=permission_error),
        ):
            inspect_engine.return_value.has_table.return_value = False

            with self.assertRaisesRegex(DeckCatalogSetupError, "administrador"):
                ensure_deck_catalog(mysql_engine)


if __name__ == "__main__":
    unittest.main()
