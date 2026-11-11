import streamlit as st
from sqlalchemy import create_engine
from urllib.parse import quote_plus

config = st.secrets["database"]

# Passando tanto o user quanto a password pelo quote_plus
user_encoded = quote_plus(config['user'])
password_encoded = quote_plus(config['password'])

url = (
    f"mysql+pymysql://{user_encoded}:{password_encoded}@"
    f"{config['host']}:{config['port']}/{config['database']}"
)

# TiDB Cloud exige uma conexão segura (SSL)
engine = create_engine(
    url,
    pool_pre_ping=True,
    connect_args={"ssl": {"fake_user": "true"}} # Ativa o suporte a SSL exigido pela AWS/TiDB
)
