import sqlite3

# Connect to (or create) the database file
conn = sqlite3.connect('iot_data.db')

# Create a cursor object to execute SQL commands
cursor = conn.cursor()

# 1. Create the table with a default timestamp in IST
cursor.execute('''
   CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL NOT NULL,
    humidity REAL NOT NULL,
    timestamp TEXT NOT NULL DEFAULT (DATETIME('now', '+5 hours', '+30 minutes'))
);
''')

# 2. Insert a sample log (timestamp will automatically be IST)
cursor.execute('''
    INSERT INTO logs (temperature, humidity)
    VALUES (?, ?)
''', (25.5, 60.2))

# 3. Query the logs to verify
cursor.execute('SELECT id, temperature, humidity, timestamp AS ist_time FROM logs')
rows = cursor.fetchall()
for row in rows:
    print(row)

# Commit changes and close the connection
conn.commit()
conn.close()

print("Database, table, and sample log created successfully.")
