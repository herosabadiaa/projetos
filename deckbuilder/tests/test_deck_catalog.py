import sys
import unittest
from pathlib import Path

from sqlalchemy import create_engine

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from deck_catalog import (
    ensure_deck_catalog,
    list_saved_decks,
    save_deck,
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


if __name__ == "__main__":
    unittest.main()
