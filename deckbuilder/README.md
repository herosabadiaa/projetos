# Deck Builder

Aplicação web em Streamlit para montar decks usando o catálogo do jogo em
`cards.csv` e as imagens disponíveis em `resources/`.

## Requisitos

- [uv](https://docs.astral.sh/uv/)
- Python 3.10 ou superior

## Executar localmente

Na raiz do repositório, instale as dependências e inicie a aplicação:

```powershell
uv sync --project deckbuilder
uv run --project deckbuilder streamlit run deckbuilder/src/app.py
```

O Streamlit exibirá o endereço local da aplicação no terminal.

## Utilização

- Pesquise e filtre o catálogo por nome, tipo, subtipo e mana.
- Ordene os resultados por nome, mana, ATK ou HP.
- Adicione cartas ao deck e ajuste suas quantidades. O deck aceita até 60 cartas.
- Consulte a quantidade de cartas, a mana média e a composição por tipo.
- Simule uma mão inicial de tamanho configurável sem alterar o deck.
- Importe decks em CSV com as colunas originais do catálogo (com ou sem cabeçalho)
  ou com as colunas `name,quantity`.
- Exporte o deck em CSV no formato original do catálogo, sem cabeçalho e com uma
  linha por carta. Assim, cada quantidade aparece como linhas repetidas.
- Salve decks completos (60 cartas) no catálogo persistido no MySQL configurado
  no Streamlit Secrets. Decks salvos podem ser carregados para edição, atualizados
  ou baixados individualmente em CSV.

O arquivo do catálogo contém as colunas `name,hp,atk,mana,image,type,subtype`.
O caminho das imagens é relativo à raiz do projeto.

## Publicação no Streamlit Community Cloud

Conecte este repositório no [Streamlit Community Cloud](https://share.streamlit.io/)
e configure o arquivo principal como `deckbuilder/src/app.py`. Configure a
instalação de dependências para usar `deckbuilder/requirements.txt`. O projeto
também inclui `deckbuilder/pyproject.toml` e `deckbuilder/uv.lock` para uso local
com uv. O catálogo e as imagens continuam sendo lidos de `cards.csv` e
`resources/` na raiz do repositório. Configure `database.host`, `database.port`,
`database.user`, `database.password` e `database.database` nos Streamlit Secrets
para habilitar o catálogo de decks no servidor.

## Aplicação
https://projetos-9beqncb4osygzlurzqrxl9.streamlit.app/