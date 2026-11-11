import streamlit as st
from sqlalchemy import create_engine
from urllib.parse import quote_plus

config = st.secrets["database"]

url = (
    f"mysql+pymysql://{config['user']}:"
    f"{quote_plus(config['password'])}@"
    f"{config['host']}:{config['port']}/"
    f"{config['database']}"
)

engine = create_engine(
    url,
    pool_pre_ping=True
)