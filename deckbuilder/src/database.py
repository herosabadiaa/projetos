import streamlit as st
from sqlalchemy import create_engine
from urllib.parse import quote_plus

# Verifica se a chave existe antes de tentar usá-la
if "database" not in st.secrets:
    st.error("Erro: O bloco [database] não foi encontrado nos Secrets do Streamlit Cloud!")
    st.stop()

config = st.secrets["database"]

# Validação para evitar o TypeError caso o bloco esteja vazio
if config is None or not hasattr(config, "get") or 'user' not in config:
    st.error("Erro: As credenciais dentro de [database] estão incompletas ou ausentes.")
    st.stop()

# Realiza o encode seguro dos caracteres especiais
user_encoded = quote_plus(config['user'])
password_encoded = quote_plus(config['password'])

url = (
    f"mysql+pymysql://{user_encoded}:{password_encoded}@"
    f"{config['host']}:{config['port']}/{config['database']}"
)

# Configuração com suporte a SSL para o TiDB Cloud
engine = create_engine(
    url,
    pool_pre_ping=True,
    connect_args={"ssl": {"fake_user": "true"}}
)
