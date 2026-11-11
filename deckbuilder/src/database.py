import streamlit as st
from sqlalchemy import create_engine
from urllib.parse import quote_plus

# Limpeza e validação dos dados
if "database" not in st.secrets:
    st.error("Erro: O bloco [database] não foi encontrado nos Secrets.")
    st.stop()

config = st.secrets["database"]

# Garante que a porta seja limpa de espaços e convertida para um inteiro puro
try:
    porta_limpa = int(str(config['port']).strip())
except (ValueError, KeyError):
    st.error("Erro: A porta configurada no [database] é inválida.")
    st.stop()

user_encoded = quote_plus(config['user'])
password_encoded = quote_plus(config['password'])

# Montando a URL usando a porta tratada como inteiro
url = (
    f"mysql+pymysql://{user_encoded}:{password_encoded}@"
    f"{config['host']}:{porta_limpa}/{config['database']}"
)

# Configuração estável para TiDB Cloud
engine = create_engine(
    url,
    pool_pre_ping=True,
    connect_args={"ssl": {"fake_user": "true"}}
)
