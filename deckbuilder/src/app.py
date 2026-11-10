from pathlib import Path

import streamlit as st

from deck_builder import (
    MAX_DECK_SIZE,
    Card,
    deck_size,
    export_deck_csv,
    load_catalog,
    parse_deck_csv,
)


ROOT = Path(__file__).resolve().parents[2]


@st.cache_data
def get_catalog() -> tuple[Card, ...]:
    return load_catalog(ROOT / "cards.csv")


st.set_page_config(page_title="Construtor de Deck", layout="wide")
st.title("Construtor de Deck")
st.caption("Monte seu deck com as cartas do catálogo do jogo e importe ou exporte em CSV.")

try:
    catalog = get_catalog()
except (OSError, ValueError) as error:
    st.error(f"Não foi possível carregar o catálogo: {error}")
    st.stop()

if "deck" not in st.session_state:
    st.session_state.deck = {}

catalog_names = {card.name for card in catalog}
st.session_state.deck = {
    name: quantity
    for name, quantity in st.session_state.deck.items()
    if name in catalog_names and quantity > 0
}

catalog_column, deck_column = st.columns([1.2, 1], gap="large")

with catalog_column:
    st.header("Catálogo")
    if "show_card_images" not in st.session_state:
        st.session_state.show_card_images = True
    if st.button(
        "Ocultar imagens das cartas"
        if st.session_state.show_card_images
        else "Mostrar imagens das cartas"
    ):
        st.session_state.show_card_images = not st.session_state.show_card_images
        st.rerun()

    search = st.text_input("Buscar por nome")
    type_options = ["Todos"] + sorted({card.card_type for card in catalog})
    selected_type = st.selectbox("Tipo", type_options)
    subtype_options = ["Todos"] + sorted({card.subtype for card in catalog})
    selected_subtype = st.selectbox("Subtipo", subtype_options)
    mana_values = [card.mana for card in catalog]
    minimum_mana, maximum_mana = st.slider(
        "Faixa de mana",
        min_value=min(mana_values),
        max_value=max(mana_values),
        value=(min(mana_values), max(mana_values)),
    )

    filtered_cards = [
        card
        for card in catalog
        if search.casefold() in card.name.casefold()
        and (selected_type == "Todos" or card.card_type == selected_type)
        and (selected_subtype == "Todos" or card.subtype == selected_subtype)
        and minimum_mana <= card.mana <= maximum_mana
    ]
    if not filtered_cards:
        st.info("Nenhuma carta corresponde aos filtros.")
    else:
        card_by_label = {
            f"{card.name} | ATK {card.atk} | HP {card.hp} | Mana {card.mana}": card
            for card in filtered_cards
        }
        selected_label = st.selectbox("Carta", list(card_by_label))
        selected_card = card_by_label[selected_label]

        preview, details = st.columns([1, 2])
        image_path = ROOT / selected_card.image
        with preview:
            if not st.session_state.show_card_images:
                st.container(height=240, border=True)
            elif image_path.is_file():
                st.image(str(image_path), width=160)
            else:
                st.warning(f"Imagem não encontrada: {selected_card.image}")
        with details:
            st.subheader(selected_card.name)
            st.write(f"**Tipo:** {selected_card.card_type}")
            st.write(f"**Subtipo:** {selected_card.subtype}")
            st.write(f"**HP:** {selected_card.hp} · **ATK:** {selected_card.atk}")
            st.write(f"**Mana:** {selected_card.mana}")

        if st.button(
            "Adicionar ao deck",
            type="primary",
            disabled=deck_size(st.session_state.deck) >= MAX_DECK_SIZE,
        ):
            deck = st.session_state.deck
            deck[selected_card.name] = deck.get(selected_card.name, 0) + 1
            st.rerun()

with deck_column:
    st.header("Meu deck")
    total_cards = deck_size(st.session_state.deck)
    st.write(f"**{total_cards} / {MAX_DECK_SIZE} cartas**")
    st.progress(total_cards / MAX_DECK_SIZE)

    deck_upload = st.file_uploader("Importar deck em CSV", type=["csv"])
    if st.button("Importar deck", disabled=deck_upload is None):
        try:
            imported_deck = parse_deck_csv(deck_upload.getvalue(), catalog)
        except ValueError as error:
            st.error(f"Não foi possível importar o deck: {error}")
        else:
            st.session_state.deck = imported_deck
            st.success(f"Deck importado com {deck_size(imported_deck)} cartas.")

    st.download_button(
        "Exportar deck em CSV",
        data=export_deck_csv(st.session_state.deck, catalog),
        file_name="deck.csv",
        mime="text/csv",
        disabled=not st.session_state.deck,
    )

    if st.session_state.deck:
        st.divider()
        for name, quantity in list(st.session_state.deck.items()):
            card = next(card for card in catalog if card.name == name)
            name_column, minus_column, count_column, plus_column, remove_column = st.columns(
                [5, 0.7, 0.8, 0.7, 1.5]
            )
            name_column.write(f"**{name}**  \nATK {card.atk} · HP {card.hp} · Mana {card.mana}")
            if minus_column.button("-", key=f"minus_{name}", disabled=quantity <= 1):
                st.session_state.deck[name] = quantity - 1
                st.rerun()
            count_column.write(f"x {quantity}")
            if plus_column.button(
                "+",
                key=f"plus_{name}",
                disabled=total_cards >= MAX_DECK_SIZE,
            ):
                st.session_state.deck[name] = quantity + 1
                st.rerun()
            if remove_column.button("Remover", key=f"remove_{name}"):
                del st.session_state.deck[name]
                st.rerun()
    else:
        st.info("Seu deck está vazio. Adicione cartas pelo catálogo ou importe um CSV.")

    if st.button("Limpar deck", disabled=not st.session_state.deck):
        st.session_state.deck = {}
        st.rerun()
