import serial
import threading
from flask import Flask, jsonify, send_from_directory
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD = 115200

# States
status = "SCANNING"        # Current display state
last_event_time = 0        # Time of last HUMAN / NO HUMAN event
hold_mode = False          # If TRUE ? holding HUMAN or NO HUMAN for 5s

def read_serial():
    global status, last_event_time, hold_mode

    while True:
        now = time.time()

        # Release hold after 5 seconds ? back to SCANNING
        if hold_mode and (now - last_event_time >= 5):
            status = "SCANNING"
            hold_mode = False

        try:
            line = ser.readline().decode().strip()

            # ESP32 says HUMAN
            if line == "HUMAN":
                status = "HUMAN"
                last_event_time = now
                hold_mode = True

            # ESP32 says NO HUMAN
            elif line == "NO HUMAN":
                status = "NO HUMAN"
                last_event_time = now
                hold_mode = True

        except:
            pass

        time.sleep(0.1)

# Serial setup
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
threading.Thread(target=read_serial, daemon=True).start()

# Flask server for dashboard
app = Flask(__name__)

@app.get("/status")
def get_status():
    return jsonify({"status": status})

@app.get("/")
def serve_dashboard():
    return send_from_directory(".", "dashboard.html")

app.run(host="0.0.0.0", port=5000)
import serial
import threading
from flask import Flask, jsonify, send_from_directory
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD = 115200

# States
status = "SCANNING"        # Current display state
last_event_time = 0        # Time of last HUMAN / NO HUMAN event
hold_mode = False          # If TRUE ? holding HUMAN or NO HUMAN for 5s

def read_serial():
    global status, last_event_time, hold_mode

    while True:
        now = time.time()

        # Release hold after 5 seconds ? back to SCANNING
        if hold_mode and (now - last_event_time >= 5):
            status = "SCANNING"
            hold_mode = False

        try:
            line = ser.readline().decode().strip()

            # ESP32 says HUMAN
            if line == "HUMAN":
                status = "HUMAN"
                last_event_time = now
                hold_mode = True

            # ESP32 says NO HUMAN
            elif line == "NO HUMAN":
                status = "NO HUMAN"
                last_event_time = now
                hold_mode = True

        except:
            pass

        time.sleep(0.1)

# Serial setup
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
threading.Thread(target=read_serial, daemon=True).start()

# Flask server for dashboard
app = Flask(__name__)

@app.get("/status")
def get_status():
    return jsonify({"status": status})

@app.get("/")
def serve_dashboard():
    return send_from_directory(".", "dashboard.html")

app.run(host="0.0.0.0", port=5000)
import serial
import threading
from flask import Flask, jsonify, send_from_directory
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD = 115200

# States
status = "SCANNING"        # Current display state
last_event_time = 0        # Time of last HUMAN / NO HUMAN event
hold_mode = False          # If TRUE ? holding HUMAN or NO HUMAN for 5s

def read_serial():
    global status, last_event_time, hold_mode

    while True:
        now = time.time()

        # Release hold after 5 seconds ? back to SCANNING
        if hold_mode and (now - last_event_time >= 5):
            status = "SCANNING"
            hold_mode = False

        try:
            line = ser.readline().decode().strip()

            # ESP32 says HUMAN
            if line == "HUMAN":
                status = "HUMAN"
                last_event_time = now
                hold_mode = True

            # ESP32 says NO HUMAN
            elif line == "NO HUMAN":
                status = "NO HUMAN"
                last_event_time = now
                hold_mode = True

        except:
            pass

        time.sleep(0.1)

# Serial setup
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
threading.Thread(target=read_serial, daemon=True).start()

# Flask server for dashboard
app = Flask(__name__)

@app.get("/status")
def get_status():
    return jsonify({"status": status})

@app.get("/")
def serve_dashboard():
    return send_from_directory(".", "dashboard.html")

app.run(host="0.0.0.0", port=5000)
import serial
import threading
from flask import Flask, jsonify, send_from_directory
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD = 115200

# States
status = "SCANNING"        # Current display state
last_event_time = 0        # Time of last HUMAN / NO HUMAN event
hold_mode = False          # If TRUE ? holding HUMAN or NO HUMAN for 5s

def read_serial():
    global status, last_event_time, hold_mode

    while True:
        now = time.time()

        # Release hold after 5 seconds ? back to SCANNING
        if hold_mode and (now - last_event_time >= 5):
            status = "SCANNING"
            hold_mode = False

        try:
            line = ser.readline().decode().strip()

            # ESP32 says HUMAN
            if line == "HUMAN":
                status = "HUMAN"
                last_event_time = now
                hold_mode = True

            # ESP32 says NO HUMAN
            elif line == "NO HUMAN":
                status = "NO HUMAN"
                last_event_time = now
                hold_mode = True

        except:
            pass

        time.sleep(0.1)

# Serial setup
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
threading.Thread(target=read_serial, daemon=True).start()

# Flask server for dashboard
app = Flask(__name__)

@app.get("/status")
def get_status():
    return jsonify({"status": status})

@app.get("/")
def serve_dashboard():
    return send_from_directory(".", "dashboard.html")

app.run(host="0.0.0.0", port=5000)
