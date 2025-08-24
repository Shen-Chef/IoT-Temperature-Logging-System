from flask import Flask, request, jsonify
from datetime import datetime, timezone
from flask_cors import CORS
import sqlite3

app = Flask(__name__)
CORS(app)  # enable CORS

# POST endpoint: log new data in UTC
@app.route('/api/logs', methods=['POST'])
def log_data():
    data = request.get_json()
    temperature = data.get('temperature')
    humidity = data.get('humidity')

    # Store timestamp in UTC in ISO format
    timestamp = datetime.now(timezone.utc).isoformat()  # e.g., 2025-08-22T10:30:00+00:00

    conn = sqlite3.connect('iot_data.db')
    cursor = conn.cursor()
    cursor.execute(
        "INSERT INTO logs (temperature, humidity, timestamp) VALUES (?, ?, ?)",
        (temperature, humidity, timestamp)
    )
    conn.commit()
    conn.close()

    return jsonify({"status": "success"}), 201


# GET endpoint: return logs
@app.route('/api/logs', methods=['GET'])
def get_logs():
    try:
        conn = sqlite3.connect('iot_data.db')
        conn.row_factory = sqlite3.Row  # Enable dictionary-like access
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM logs ORDER BY timestamp ASC")  # oldest first
        rows = cursor.fetchall()
    finally:
        conn.close()

    logs = [
        {
            "id": row["id"],
            "temperature": row["temperature"],
            "humidity": row["humidity"],
            "timestamp": row["timestamp"]  # still UTC ISO format
        }
        for row in rows
    ]
    return jsonify(logs)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
