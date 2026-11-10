import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from deck_builder import draw_hand


class DrawHandTests(unittest.TestCase):
    def test_draws_without_exceeding_card_copies(self) -> None:
        hand = draw_hand({"Dragão": 2, "Mago": 1}, 3)

        self.assertCountEqual(hand, ["Dragão", "Dragão", "Mago"])

    def test_rejects_hand_larger_than_deck(self) -> None:
        with self.assertRaisesRegex(ValueError, "maior que o deck"):
            draw_hand({"Dragão": 1}, 2)

    def test_rejects_empty_hand(self) -> None:
        with self.assertRaisesRegex(ValueError, "pelo menos 1"):
            draw_hand({"Dragão": 1}, 0)

    def test_rejects_negative_card_quantity(self) -> None:
        with self.assertRaisesRegex(ValueError, "não podem ser negativas"):
            draw_hand({"Dragão": -1}, 1)


if __name__ == "__main__":
    unittest.main()
