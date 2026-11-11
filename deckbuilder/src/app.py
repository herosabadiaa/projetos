import sys
from pathlib import Path
import streamlit as st
from sqlalchemy.exc import IntegrityError, SQLAlchemyError

APP_DIR = Path(__file__).resolve().parent
if str(APP_DIR) not in sys.path:
    sys.path.insert(0, str(APP_DIR))

from database import engine
from deck_builder import (
    MAX_DECK_SIZE,
    Card,
    deck_size,
    draw_hand,
    export_deck_csv,
    load_catalog,
    parse_deck_csv,
)
from deck_catalog import (
    CREATE_DECK_CATALOG_SQL,
    DeckCatalogSetupError,
    ensure_deck_catalog,
    list_saved_decks,
    save_deck,
    search_saved_decks,
    update_saved_deck,
)


ROOT = APP_DIR.parents[1]


@st.cache_data
def get_catalog() -> tuple[Card, ...]:
    return load_catalog(ROOT / "cards.csv")


st.set_page_config(page_title="Construtor de Deck", layout="wide")
st.title("Construtor de Deck")
st.caption("Explore as cartas, monte seu deck e teste uma mão inicial.")

try:
    catalog = get_catalog()
except (OSError, ValueError) as error:
    st.error(f"Não foi possível carregar o catálogo: {error}")
    st.stop()

try:
    ensure_deck_catalog(engine)
    saved_decks = list_saved_decks(engine)
except DeckCatalogSetupError as error:
    st.error(f"Não foi possível inicializar o catálogo de decks: {error}")
    st.code(CREATE_DECK_CATALOG_SQL, language="sql")
    st.stop()
except (SQLAlchemyError, ValueError) as error:
    st.error(f"Não foi possível acessar o catálogo de decks no servidor: {error}")
    st.stop()

if "deck" not in st.session_state:
    st.session_state.deck = {}

catalog_names = {card.name for card in catalog}
cards_by_name = {card.name: card for card in catalog}
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
    sort_options = {
        "Nome (A-Z)": lambda card: card.name.casefold(),
        "Mana (menor primeiro)": lambda card: (card.mana, card.name.casefold()),
        "ATK (maior primeiro)": lambda card: (-card.atk, card.name.casefold()),
        "HP (maior primeiro)": lambda card: (-card.hp, card.name.casefold()),
    }
    sort_label = st.selectbox("Ordenar por", list(sort_options))
    mana_values = [card.mana for card in catalog]
    minimum_mana, maximum_mana = st.slider(
        "Faixa de mana",
        min_value=min(mana_values),
        max_value=max(mana_values),
        value=(min(mana_values), max(mana_values)),
    )

    filtered_cards = sorted(
        (
            card
            for card in catalog
            if search.casefold() in card.name.casefold()
            and (selected_type == "Todos" or card.card_type == selected_type)
            and (selected_subtype == "Todos" or card.subtype == selected_subtype)
            and minimum_mana <= card.mana <= maximum_mana
        ),
        key=sort_options[sort_label],
    )
    st.caption(f"{len(filtered_cards)} de {len(catalog)} cartas")
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

        remaining_capacity = MAX_DECK_SIZE - deck_size(st.session_state.deck)
        add_quantity = st.number_input(
            "Quantidade a adicionar",
            min_value=1,
            max_value=max(1, remaining_capacity),
            value=1,
            step=1,
            disabled=remaining_capacity == 0,
            key="add_card_quantity",
        )
        if st.button(
            "Adicionar ao deck",
            type="primary",
            disabled=remaining_capacity == 0,
        ):
            deck = st.session_state.deck
            deck[selected_card.name] = (
                deck.get(selected_card.name, 0) + add_quantity
            )
            st.session_state.pop("test_hand", None)
            st.rerun()

with deck_column:
    st.header("Meu deck")
    total_cards = deck_size(st.session_state.deck)
    stat_columns = st.columns(2)
    stat_columns[0].metric("Cartas", f"{total_cards} / {MAX_DECK_SIZE}")
    average_mana = (
        sum(cards_by_name[name].mana * quantity for name, quantity in st.session_state.deck.items())
        / total_cards
        if total_cards
        else 0
    )
    stat_columns[1].metric("Mana médio", f"{average_mana:.1f}")
    st.progress(total_cards / MAX_DECK_SIZE)

    if st.session_state.deck:
        type_counts: dict[str, int] = {}
        for name, quantity in st.session_state.deck.items():
            card_type = cards_by_name[name].card_type
            type_counts[card_type] = type_counts.get(card_type, 0) + quantity
        with st.expander("Ver composição do deck"):
            for card_type, quantity in sorted(type_counts.items()):
                st.write(f"**{card_type}:** {quantity} carta(s)")

    with st.expander("Testar mão inicial", expanded=False):
        st.caption("Compre cartas aleatórias sem reposição. O deck não será alterado.")
        initial_hand_size = min(5, total_cards) if total_cards else 1
        hand_size = st.number_input(
            "Tamanho da mão",
            min_value=1,
            max_value=max(1, total_cards),
            value=initial_hand_size,
            step=1,
            disabled=not st.session_state.deck,
        )
        if st.button("Comprar mão", disabled=not st.session_state.deck):
            st.session_state.test_hand = draw_hand(st.session_state.deck, hand_size)
        if st.session_state.get("test_hand"):
            st.write(" · ".join(st.session_state.test_hand))

    deck_upload = st.file_uploader("Importar deck em CSV", type=["csv"])
    if st.button("Importar deck", disabled=deck_upload is None):
        try:
            imported_deck = parse_deck_csv(deck_upload.getvalue(), catalog)
        except ValueError as error:
            st.error(f"Não foi possível importar o deck: {error}")
        else:
            st.session_state.deck = imported_deck
            st.session_state.pop("test_hand", None)
            st.success(f"Deck importado com {deck_size(imported_deck)} cartas.")

    st.download_button(
        "Exportar deck em CSV",
        data=export_deck_csv(st.session_state.deck, catalog),
        file_name="deck.csv",
        mime="text/csv",
        disabled=not st.session_state.deck,
    )

    st.divider()
    with st.expander("Catálogo de decks salvos"):
        if saved_decks:
            name_query = st.text_input(
                "Buscar deck por nome",
                key="saved_deck_name_search",
            )
            saved_card_names = sorted(
                {name for saved_deck in saved_decks for name in saved_deck.cards},
                key=str.casefold,
            )
            card_options = ["Todas as cartas", *saved_card_names]
            selected_card_name = st.selectbox(
                "Filtrar por carta (digite para buscar)",
                card_options,
                key="saved_deck_card_search",
            )
            matching_saved_decks = search_saved_decks(
                saved_decks,
                name_query=name_query,
                card_name=(
                    selected_card_name
                    if selected_card_name != "Todas as cartas"
                    else None
                ),
            )
            if matching_saved_decks:
                saved_decks_by_name = {
                    saved_deck.name: saved_deck for saved_deck in matching_saved_decks
                }
                selected_saved_name = st.selectbox(
                    "Deck salvo",
                    list(saved_decks_by_name),
                    key="selected_saved_deck_name",
                )
                selected_saved_deck = saved_decks_by_name[selected_saved_name]
                saved_deck_is_available = all(
                    name in cards_by_name for name in selected_saved_deck.cards
                )
                st.caption(
                    f"Criado em {selected_saved_deck.created_at:%d/%m/%Y %H:%M} · "
                    f"{deck_size(selected_saved_deck.cards)} cartas"
                )
                if not saved_deck_is_available:
                    st.error(
                        "Este deck contém cartas que não existem mais no catálogo atual."
                    )
                else:
                    st.download_button(
                        "Baixar CSV do deck salvo",
                        data=export_deck_csv(selected_saved_deck.cards, catalog),
                        file_name=f"{selected_saved_deck.name}.csv",
                        mime="text/csv",
                        key=f"download_saved_deck_{selected_saved_deck.id}",
                    )
                if st.button(
                    "Carregar para editar",
                    key="load_saved_deck",
                    disabled=not saved_deck_is_available,
                ):
                    st.session_state.deck = selected_saved_deck.cards.copy()
                    st.session_state.editing_saved_deck_id = selected_saved_deck.id
                    st.session_state.editing_saved_deck_name = selected_saved_deck.name
                    st.session_state.pop("test_hand", None)
                    st.rerun()
            else:
                st.info("Nenhum deck corresponde à busca.")
        else:
            st.info("Ainda não há decks salvos no catálogo.")

        deck_name = st.text_input("Nome para salvar como novo deck", max_chars=100)
        if st.button(
            "Salvar no catálogo",
            disabled=total_cards != MAX_DECK_SIZE,
            key="save_deck_to_catalog",
        ):
            try:
                save_deck(engine, deck_name, st.session_state.deck)
            except ValueError as error:
                st.error(f"Não foi possível salvar o deck: {error}")
            except IntegrityError:
                st.error("Já existe um deck com esse nome no catálogo.")
            except SQLAlchemyError as error:
                st.error(f"Falha ao salvar o deck no servidor: {error}")
            else:
                st.success(f"Deck '{deck_name.strip()}' salvo no catálogo.")
                st.rerun()
        if total_cards != MAX_DECK_SIZE:
            st.caption(f"Complete {MAX_DECK_SIZE} cartas para salvar o deck.")

        editing_deck_id = st.session_state.get("editing_saved_deck_id")
        editing_deck_name = st.session_state.get("editing_saved_deck_name")
        if editing_deck_id is not None:
            st.caption(f"Editando uma cópia de '{editing_deck_name}'.")
            if st.button(
                f"Atualizar deck salvo '{editing_deck_name}'",
                disabled=total_cards != MAX_DECK_SIZE,
                key="update_saved_deck",
            ):
                try:
                    update_saved_deck(engine, editing_deck_id, st.session_state.deck)
                except (ValueError, LookupError) as error:
                    st.error(f"Não foi possível atualizar o deck: {error}")
                except SQLAlchemyError as error:
                    st.error(f"Falha ao atualizar o deck no servidor: {error}")
                else:
                    st.success(f"Deck '{editing_deck_name}' atualizado.")
                    st.rerun()

    if st.session_state.deck:
        st.divider()
        for name, quantity in list(st.session_state.deck.items()):
            card = cards_by_name[name]
            name_column, minus_column, count_column, plus_column, remove_column = st.columns(
                [5, 0.7, 0.8, 0.7, 1.5]
            )
            name_column.write(f"**{name}**  \nATK {card.atk} · HP {card.hp} · Mana {card.mana}")
            if minus_column.button("-", key=f"minus_{name}", disabled=quantity <= 1):
                st.session_state.deck[name] = quantity - 1
                st.session_state.pop("test_hand", None)
                st.rerun()
            count_column.write(f"x {quantity}")
            if plus_column.button(
                "+",
                key=f"plus_{name}",
                disabled=total_cards >= MAX_DECK_SIZE,
            ):
                st.session_state.deck[name] = quantity + 1
                st.session_state.pop("test_hand", None)
                st.rerun()
            if remove_column.button("Remover", key=f"remove_{name}"):
                del st.session_state.deck[name]
                st.session_state.pop("test_hand", None)
                st.rerun()
    else:
        st.info("Seu deck está vazio. Adicione cartas pelo catálogo ou importe um CSV.")

    if st.button("Limpar deck", disabled=not st.session_state.deck):
        st.session_state.deck = {}
        st.session_state.pop("test_hand", None)
        st.rerun()
