# dashboard_final.py
# Flask dashboard: camera on right, minimal left status, CO2 graph below.
# Adjust SERIAL_PORT if your ESP32 appears at a different device.

from flask import Flask, Response, render_template_string, jsonify
import threading, time, io
import serial
import cv2
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# === CONFIG ===
SERIAL_PORT = '/dev/ttyUSB0'   # change if needed
SERIAL_BAUD = 115200
CAM_INDEX = 0                  # camera device index for cv2.VideoCapture
POLL_INTERVAL = 1.0            # UI poll interval (client-side JS also uses 1s)

# === GLOBALS ===
co2_values = []
co2_lock = threading.Lock()

status_text = "Standby"
status_lock = threading.Lock()

latest_human = 0
latest_distance = 0.0
result_lock = threading.Lock()

# === SERIAL THREAD ===
def serial_reader():
    global co2_values, status_text, latest_human, latest_distance
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
        time.sleep(2)
    except Exception as e:
        print("Serial open error:", e)
        return

    while True:
        try:
            raw = ser.readline()
            if not raw:
                continue
            # decode with replacement to avoid crashes on bad bytes
            line = raw.decode('utf-8', errors='replace').strip()
            if not line:
                continue

            # Parse messages
            if line.startswith("CO2,"):
                try:
                    _, val = line.split(",", 1)
                    v = float(val)
                    with co2_lock:
                        co2_values.append(v)
                        # keep last 100 points
                        co2_values = co2_values[-100:]
                except:
                    pass

            elif line == "SCAN_INITIATING":
                with status_lock:
                    status_text = "Scan Initiating..."

            elif line == "HOLD_STILL":
                with status_lock:
                    status_text = "Hold still..."

            elif line == "SCANNING":
                with status_lock:
                    status_text = "Scanning..."

            elif line.startswith("HUMAN,"):
                try:
                    parts = line.split(",")
                    if len(parts) >= 3:
                        h = int(parts[1])
                        d = float(parts[2])
                        with result_lock:
                            latest_human = h
                            latest_distance = d
                        with status_lock:
                            status_text = "Result ready"
                except:
                    pass
            else:
                # ignore unknown lines
                pass

        except Exception as e:
            print("Serial read error:", e)
            time.sleep(0.2)


# === CAMERA STREAM ===
camera = cv2.VideoCapture(CAM_INDEX)
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

def gen_camera():
    while True:
        success, frame = camera.read()
        if not success:
            time.sleep(0.1)
            continue
        # JPEG encode
        ret, jpeg = cv2.imencode('.jpg', frame)
        if not ret:
            continue
        frame_bytes = jpeg.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
        time.sleep(0.03)


# === CO2 GRAPH ROUTE ===
def make_co2_image():
    fig, ax = plt.subplots(figsize=(6,2.5))
    with co2_lock:
        data = co2_values.copy()
    ax.clear()
    if data:
        ax.plot(data)
        ax.set_ylim(0, max(max(data) * 1.1, 500))
    ax.set_title("CO2 Levels")
    ax.set_ylabel("scaled")
    ax.set_xlabel("samples")
    buf = io.BytesIO()
    fig.tight_layout()
    fig.savefig(buf, format='png')
    plt.close(fig)
    buf.seek(0)
    return buf.read()

# === FLASK APP ===
app = Flask(__name__)

INDEX_HTML = """
<!doctype html>
<html>
<head>
  <title>Dashboard — Minimal</title>
  <style>
    body{ font-family: Arial, sans-serif; margin:20px; background:#f7f7f7; color:#111; }
    .container{ display:flex; gap:20px; }
    .left{ width:48%; min-width:280px; }
    .card{ background:white; padding:16px; border-radius:8px; box-shadow:0 2px 6px rgba(0,0,0,0.08); }
    .status-line{ font-size:18px; margin:8px 0; }
    .big { font-weight:700; font-size:26px; margin-top:6px; }
    .co2 { margin-top:16px; }
    .camera{ width:52%; }
    img.cam { width:100%; border-radius:8px; box-shadow:0 2px 6px rgba(0,0,0,0.08); }
  </style>
</head>
<body>
  <h2>Live Monitor</h2>
  <div class="container">
    <div class="left">
      <div class="card">
        <div class="status-line">Status: <span id="status">Loading...</span></div>
        <div class="status-line">Human: <span id="human">—</span></div>
        <div class="status-line">Distance: <span id="distance">—</span></div>
      </div>

      <div class="card co2">
        <h3>CO₂ Graph</h3>
        <img id="co2img" src="/co2_graph" alt="co2 graph" style="width:100%; height:auto;">
      </div>
    </div>

    <div class="camera">
      <div class="card">
        <h3>Camera</h3>
        <img class="cam" src="/camera_feed">
      </div>
    </div>
  </div>

<script>
async function fetchStatus(){
  try{
    let res = await fetch('/status');
    let j = await res.json();
    document.getElementById('status').innerText = j.status;
    document.getElementById('human').innerText = (j.human == 1) ? "Yes" : "No";
    document.getElementById('distance').innerText = (j.human == 1) ? j.distance.toFixed(2) + " m" : "—";
  }catch(e){
    // ignore
  }
}

setInterval(fetchStatus, 1000);
setInterval(() => {
  // refresh co2 image periodically (cache-bust)
  document.getElementById('co2img').src = '/co2_graph?cb=' + Date.now();
}, 2000);

window.onload = fetchStatus;
</script>
</body>
</html>
"""

@app.route('/')
def index():
    return render_template_string(INDEX_HTML)

@app.route('/camera_feed')
def camera_feed():
    return Response(gen_camera(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/co2_graph')
def co2_graph():
    img = make_co2_image()
    return Response(img, mimetype='image/png')

@app.route('/status')
def status():
    with status_lock:
        s = status_text
    with result_lock:
        h = latest_human
        d = latest_distance
    return jsonify({"status": s, "human": int(h), "distance": float(d)})

# === START THREADS & APP ===
if __name__ == '__main__':
    t = threading.Thread(target=serial_reader, daemon=True)
    t.start()
    # start flask (production use: use gunicorn or systemd wrapper)
    app.run(host='0.0.0.0', port=8080, threaded=True)
