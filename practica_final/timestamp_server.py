from flask import Flask
from datetime import datetime

app = Flask(__name__)

@app.route('/timestamp', methods=['GET'])
def get_timestamp():
    """Devuelve la fecha y hora actual en formato DD/MM/AAAA HH:MM:SS"""
    now = datetime.now()
    return now.strftime("%d/%m/%Y %H:%M:%S")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)