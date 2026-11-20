
#include <SPI.h>
#include <EthernetENC.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ==================================================
// NEXUS ESP32 ETHERNET CONTROL CENTER
// ESP32 + ENC28J60 + SH1106 OLED
// ==================================================


// ==================================================
// 1. ETHERNET CONFIGURATION
// ==================================================

#define ETH_CS   5
#define ETH_SCK  18
#define ETH_MISO 19
#define ETH_MOSI 23

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// STATIC IP CONFIGURATION

IPAddress ip(192, 168, 1, 100);
IPAddress dnsServer(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(80);


// ==================================================
// 2. OLED CONFIGURATION
// ==================================================

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// ==================================================
// 3. LED CONFIGURATION
// ==================================================

#define LED_PIN 2

bool ledState = false;

String receivedText = "Waiting for message...";


// ==================================================
// 4. DEVICE INFORMATION
// ==================================================

unsigned long bootTime = 0;

bool oledAvailable = false;


// ==================================================
// 5. NEXUS WEB INTERFACE
// ==================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width, initial-scale=1">

<title>NEXUS | ESP32 Control Center</title>

<style>

:root {

  --bg: #080d18;
  --panel: #111a2b;
  --panel2: #152238;
  --cyan: #00d9ff;
  --green: #00e69a;
  --red: #ff5574;
  --text: #e8f1ff;
  --muted: #8293ad;
  --border: rgba(255,255,255,.08);

}

* {
  box-sizing: border-box;
}

body {

  margin: 0;
  padding: 25px;

  background:
    radial-gradient(
      ellipse at 15% 0%,
      #102c43 0%,
      transparent 40%
    ),
    radial-gradient(
      ellipse at 100% 100%,
      #10203c 0%,
      transparent 40%
    ),
    var(--bg);

  color: var(--text);

  font-family: Arial, sans-serif;

  min-height: 100vh;

}

.container {

  max-width: 1100px;
  margin: auto;

}

/* HEADER */

header {

  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-wrap: wrap;

  gap: 15px;

  padding: 15px 0 30px;

  border-bottom: 1px solid var(--border);

  margin-bottom: 30px;

}

.brand {

  display: flex;
  align-items: center;
  gap: 14px;

}

.logo {

  width: 48px;
  height: 48px;

  border-radius: 14px;

  display: grid;
  place-items: center;

  background: linear-gradient(
    135deg,
    #00d9ff,
    #0066ff
  );

  color: white;
  font-size: 23px;
  font-weight: 800;

  box-shadow: 0 0 25px #00bfff44;

}

.brand h1 {

  margin: 0;
  font-size: 21px;
  letter-spacing: 2px;

}

.brand p {

  color: var(--muted);
  font-size: 11px;
  margin: 5px 0 0;
  letter-spacing: 2px;

}

.connection {

  display: flex;
  align-items: center;
  gap: 9px;

  background: #102b29;
  border: 1px solid #00e69a44;

  color: var(--green);

  padding: 10px 16px;

  border-radius: 30px;
  font-size: 12px;

}

.dot {

  width: 8px;
  height: 8px;

  background: var(--green);

  border-radius: 50%;

  box-shadow: 0 0 12px var(--green);

  animation: pulse 1.5s infinite;

}

@keyframes pulse {

  50% {
    opacity: .3;
  }

}

/* WELCOME */

.welcome {
  margin-bottom: 25px;
}

.welcome h2 {

  font-size: 27px;
  margin: 0 0 8px;

}

.welcome p {

  color: var(--muted);
  font-size: 13px;

}

/* GRID */

.grid {

  display: grid;
  grid-template-columns: repeat(2,1fr);
  gap: 20px;

}

.card {

  background: linear-gradient(
    145deg,
    rgba(25,40,64,.95),
    rgba(13,23,40,.95)
  );

  border: 1px solid var(--border);
  border-radius: 18px;

  padding: 25px;

  box-shadow: 0 15px 40px #00000025;

  transition: .3s;

}

.card:hover {

  border-color: #00d9ff55;
  transform: translateY(-3px);

}

.card-title {

  display: flex;
  justify-content: space-between;
  align-items: center;

  margin-bottom: 25px;

}

.card-title h3 {

  margin: 0;
  font-size: 15px;

}

.icon {

  font-size: 23px;
  color: var(--cyan);

}

.subtitle {

  color: var(--muted);
  font-size: 11px;
  margin-top: 7px;

}

/* LED */

.led-display {

  text-align: center;
  padding: 15px;

}

.led-circle {

  width: 105px;
  height: 105px;

  margin: 0 auto 18px;

  border-radius: 50%;

  background: #263449;
  border: 5px solid #34465f;

  display: grid;
  place-items: center;

  font-size: 35px;

  transition: .3s;

}

.led-circle.active {

  background: #00e69a22;

  border-color: var(--green);

  box-shadow:
    0 0 20px #00e69a55,
    0 0 55px #00e69a22;

}

.led-status {

  font-size: 20px;
  font-weight: 800;
  margin-bottom: 7px;

}

.led-status.on {
  color: var(--green);
}

.led-status.off {
  color: var(--muted);
}

/* BUTTONS */

.buttons {

  display: flex;
  gap: 12px;
  justify-content: center;
  margin-top: 20px;

}

button {

  border: none;

  padding: 13px 24px;

  border-radius: 9px;

  font-weight: 700;
  font-size: 12px;

  cursor: pointer;
  transition: .2s;

}

button:hover {

  transform: translateY(-2px);
  filter: brightness(1.2);

}

button:disabled {

  opacity: .5;
  cursor: wait;

}

.btn-on {

  background: var(--green);
  color: #061b17;

}

.btn-off {

  background: #ff5574;
  color: white;

}

/* OLED */

.oled-screen {

  background: #020d0c;

  border: 2px solid #294d43;

  border-radius: 12px;

  padding: 20px;

  min-height: 130px;

  color: #00ffae;

  font-family: monospace;

  box-shadow: inset 0 0 25px #00ffae0c;

}

.oled-label {

  font-size: 10px;
  color: #6b9b88;
  margin-bottom: 15px;
  letter-spacing: 2px;

}

.oled-message {

  font-size: 15px;
  line-height: 1.6;
  overflow-wrap: anywhere;

}

input {

  width: 100%;

  background: #091323;

  border: 1px solid #34445d;

  border-radius: 9px;

  padding: 14px;

  color: white;

  margin-top: 18px;

  outline: none;

  font-size: 14px;

}

input:focus {

  border-color: var(--cyan);

  box-shadow: 0 0 12px #00d9ff22;

}

.send-btn {

  background: linear-gradient(
    90deg,
    #008dff,
    #00c6df
  );

  color: white;

  width: 100%;
  margin-top: 12px;

}

.counter {

  text-align: right;

  font-size: 10px;

  color: var(--muted);

  margin-top: 7px;

}

/* DEVICE INFO */

.info {

  display: grid;
  grid-template-columns: repeat(3,1fr);
  gap: 12px;

}

.info-item {

  background: #0b1424;

  padding: 17px;

  border-radius: 12px;

  border: 1px solid var(--border);

}

.info-item small {

  color: var(--muted);
  font-size: 10px;

}

.info-item p {

  margin: 9px 0 0;

  font-size: 13px;
  font-weight: 700;

  color: var(--cyan);

}

/* FOOTER */

footer {

  text-align: center;

  color: #64758d;

  font-size: 11px;

  margin-top: 35px;

  padding: 20px;

  border-top: 1px solid var(--border);

}

/* TOAST */

.toast {

  position: fixed;

  bottom: 25px;
  right: 25px;

  background: #102b29;

  border: 1px solid var(--green);

  color: var(--green);

  padding: 15px 20px;

  border-radius: 10px;

  opacity: 0;

  transform: translateY(20px);

  transition: .3s;

  pointer-events: none;

  font-size: 12px;

}

.toast.show {

  opacity: 1;
  transform: translateY(0);

}

@media(max-width:700px) {

  body {
    padding: 15px;
  }

  .grid {
    grid-template-columns: 1fr;
  }

  .info {
    grid-template-columns: 1fr;
  }

  .welcome h2 {
    font-size: 22px;
  }

}

</style>

</head>

<body>

<div class="container">

<header>

  <div class="brand">

    <div class="logo">N</div>

    <div>

      <h1>NEXUS</h1>

      <p>ESP32 CONTROL CENTER</p>

    </div>

  </div>

  <div class="connection">

    <div class="dot"></div>

    ENC28J60 ETHERNET

  </div>

</header>


<div class="welcome">

  <h2>System Overview</h2>

  <p>
    Monitor and control your ESP32 device through Ethernet.
  </p>

</div>


<div class="grid">

  <!-- LED CONTROL -->

  <div class="card">

    <div class="card-title">

      <div>

        <h3>LED CONTROL</h3>

        <div class="subtitle">
          GPIO 2 • Digital Output
        </div>

      </div>

      <div class="icon">◉</div>

    </div>


    <div class="led-display">

      <div class="led-circle" id="ledCircle">
        ●
      </div>

      <div class="led-status off" id="ledStatus">
        LED OFF
      </div>

      <div class="subtitle" id="ledDescription">
        Output currently inactive
      </div>

      <div class="buttons">

        <button class="btn-on"
        onclick="controlLED(1)">

          TURN ON

        </button>

        <button class="btn-off"
        onclick="controlLED(0)">

          TURN OFF

        </button>

      </div>

    </div>

  </div>


  <!-- OLED MESSAGING -->

  <div class="card">

    <div class="card-title">

      <div>

        <h3>OLED MESSAGE CENTER</h3>

        <div class="subtitle">
          SH1106 • I2C Display
        </div>

      </div>

      <div class="icon">▣</div>

    </div>


    <div class="oled-screen">

      <div class="oled-label">
        ESP32 DISPLAY OUTPUT
      </div>

      <div class="oled-message" id="oledMessage">
        Waiting for incoming message...
      </div>

    </div>


    <input
      id="message"
      type="text"
      maxlength="100"
      placeholder="Type your message..."
      oninput="updateCounter()"
    >

    <div class="counter">

      <span id="count">0</span> / 100 characters

    </div>

    <button class="send-btn" onclick="sendMessage()">

      SEND TO OLED &nbsp; →

    </button>

  </div>

</div>


<!-- DEVICE INFORMATION -->

<div class="card" style="margin-top:20px">

  <div class="card-title">

    <div>

      <h3>DEVICE INFORMATION</h3>

      <div class="subtitle">
        Network and hardware configuration
      </div>

    </div>

    <div class="icon">⌘</div>

  </div>


  <div class="info">

    <div class="info-item">

      <small>DEVICE</small>

      <p>ESP32 DevKit V1</p>

    </div>


    <div class="info-item">

      <small>ETHERNET</small>

      <p>ENC28J60</p>

    </div>


    <div class="info-item">

      <small>STATIC IP ADDRESS</small>

      <p>192.168.1.100</p>

    </div>

  </div>

</div>


<footer>

  NEXUS CONTROL CENTER • POWERED BY ESP32

  <br><br>

  LOCAL ETHERNET MANAGEMENT INTERFACE

</footer>

</div>


<div class="toast" id="toast">

  Command executed successfully

</div>


<script>

// ============================================
// NOTIFICATIONS
// ============================================

let toastTimer;

function notify(message) {

  let toast = document.getElementById('toast');

  toast.innerText = message;

  toast.classList.add('show');

  clearTimeout(toastTimer);

  toastTimer = setTimeout(() => {

    toast.classList.remove('show');

  }, 2500);

}


// ============================================
// LED CONTROL
// ============================================

function controlLED(state) {

  fetch('/led?state=' + state)

  .then(response => {

    if (!response.ok) throw new Error();

    return response.text();

  })

  .then(data => {

    let active = data === 'ON';

    document.getElementById(
      'ledStatus'
    ).innerText = active ? 'LED ON' : 'LED OFF';

    document.getElementById(
      'ledStatus'
    ).className = active ?
      'led-status on' : 'led-status off';

    document.getElementById(
      'ledCircle'
    ).className = active ?
      'led-circle active' : 'led-circle';

    document.getElementById(
      'ledDescription'
    ).innerText = active ?
      'Output currently active' :
      'Output currently inactive';

    notify('LED switched ' + data);

  })

  .catch(() => {

    notify('Ethernet communication error');

  });

}


// ============================================
// MESSAGE COUNTER
// ============================================

function updateCounter() {

  let message = document.getElementById(
    'message'
  ).value;

  document.getElementById(
    'count'
  ).innerText = message.length;

}


// ============================================
// OLED MESSAGE
// ============================================

function sendMessage() {

  let message = document.getElementById(
    'message'
  ).value;

  if (!message.trim()) {

    notify('Please enter a message');

    return;

  }

  fetch('/message', {

    method: 'POST',

    headers: {

      'Content-Type': 'text/plain'

    },

    body: message

  })

  .then(response => {

    if (!response.ok) throw new Error();

    return response.text();

  })

  .then(data => {

    document.getElementById(
      'oledMessage'
    ).innerText = message;

    notify('Message delivered to ESP32');

  })

  .catch(() => {

    notify('Message delivery failed');

  });

}

</script>

</body>

</html>

)rawliteral";


// ==================================================
// 6. OLED DISPLAY FUNCTION
// ==================================================

void updateOLED() {

  if (!oledAvailable) {
    return;
  }

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("NEXUS CONTROL");

  display.println("----------------");

  display.print("IP: ");

  display.println(Ethernet.localIP());

  display.print("LED: ");

  display.println(ledState ? "ON" : "OFF");

  display.println("----------------");

  display.println("MESSAGE:");

  // OLED supports approximately 21
  // characters per line at text size 1.

  String text = receivedText;

  int cursorY = 48;

  for (int i = 0; i < (int)text.length(); i += 21) {

    if (cursorY > 56) {
      break;
    }

    display.setCursor(0, cursorY);

    display.print(text.substring(i, i + 21));

    cursorY += 8;

  }

  display.display();

}


// ==================================================
// 7. HTTP RESPONSE FUNCTION
// ==================================================

void sendResponse(
  EthernetClient &client,
  int statusCode,
  const char* statusText,
  const char* contentType,
  const String &body
) {

  client.print("HTTP/1.1 ");

  client.print(statusCode);

  client.print(" ");

  client.println(statusText);

  client.print("Content-Type: ");

  client.println(contentType);

  client.print("Content-Length: ");

  client.println(body.length());

  client.println("Cache-Control: no-store");

  client.println("Connection: close");

  client.println();

  client.print(body);

}


// ==================================================
// 8. HTTP CLIENT HANDLER
// ==================================================

void handleClient() {

  EthernetClient client = server.available();

  if (!client) {
    return;
  }

  client.setTimeout(3000);

  String requestLine = client.readStringUntil('\n');

  requestLine.trim();

  int contentLength = 0;

  bool headersComplete = false;

  unsigned long headerStart = millis();

  // READ HTTP HEADERS

  while (
    client.connected() &&
    millis() - headerStart < 3000
  ) {

    if (!client.available()) {
      continue;
    }

    String header = client.readStringUntil('\n');

    header.trim();

    if (header.length() == 0) {

      headersComplete = true;

      break;

    }

    if (header.startsWith("Content-Length:")) {

      contentLength = header.substring(15).toInt();

    }

  }

  if (!headersComplete) {

    client.stop();

    return;

  }


  // ============================================
  // MAIN PAGE
  // ============================================

  if (requestLine.startsWith("GET / ")) {

    sendResponse(
      client,
      200,
      "OK",
      "text/html; charset=utf-8",
      String(webpage)
    );

  }


  // ============================================
  // LED CONTROL
  // ============================================

  else if (requestLine.startsWith("GET /led?")) {

    int position = requestLine.indexOf("state=");

    if (position >= 0) {

      String value = requestLine.substring(position + 6);

      int amp = value.indexOf('&');

      if (amp >= 0) {
        value = value.substring(0, amp);
      }

      ledState = (value.toInt() == 1);

      digitalWrite(
        LED_PIN,
        ledState ? HIGH : LOW
      );

      updateOLED();

    }

    sendResponse(
      client,
      200,
      "OK",
      "text/plain",
      ledState ? "ON" : "OFF"
    );

  }


  // ============================================
  // OLED MESSAGE
  // ============================================

  else if (requestLine.startsWith("POST /message")) {

    if (contentLength < 0 || contentLength > 100) {

      sendResponse(
        client,
        413,
        "Payload Too Large",
        "text/plain",
        "Message must be 100 bytes or less"
      );

    }

    else {

      String body = "";

      unsigned long bodyStart = millis();

      while (
        (int)body.length() < contentLength &&
        client.connected() &&
        millis() - bodyStart < 3000
      ) {

        while (
          client.available() &&
          (int)body.length() < contentLength
        ) {

          body += (char)client.read();

        }

      }

      if ((int)body.length() == contentLength) {

        receivedText = body;

        updateOLED();

        sendResponse(
          client,
          200,
          "OK",
          "text/plain",
          "Message displayed on OLED!"
        );

      }

      else {

        sendResponse(
          client,
          408,
          "Request Timeout",
          "text/plain",
          "Incomplete message received"
        );

      }

    }

  }


  // ============================================
  // NOT FOUND
  // ============================================

  else {

    sendResponse(
      client,
      404,
      "Not Found",
      "text/plain",
      "Endpoint not found"
    );

  }

  delay(2);

  client.stop();

}


// ==================================================
// 9. SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  bootTime = millis();


  // LED INITIALIZATION

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);


  // OLED INITIALIZATION

  Wire.begin(OLED_SDA, OLED_SCL);

  oledAvailable = display.begin(0x3C, true);

  if (oledAvailable) {

    display.clearDisplay();

    display.setTextSize(1);

    display.setTextColor(SH110X_WHITE);

    display.setCursor(0, 0);

    display.println("NEXUS");

    display.println("Starting system...");

    display.display();

    Serial.println("OLED initialized");

  }

  else {

    Serial.println("OLED initialization failed");

  }


  // ENC28J60 SPI INITIALIZATION

  SPI.begin(
    ETH_SCK,
    ETH_MISO,
    ETH_MOSI,
    ETH_CS
  );

  Ethernet.init(ETH_CS);

  Serial.println();

  Serial.println("==========================");

  Serial.println("NEXUS ETHERNET STARTING");

  Serial.println("==========================");


  // STATIC IP INITIALIZATION

  Ethernet.begin(
    mac,
    ip,
    dnsServer,
    gateway,
    subnet
  );

  delay(1000);


  // NETWORK INFORMATION

  Serial.print("IP Address: ");

  Serial.println(Ethernet.localIP());

  Serial.print("Gateway: ");

  Serial.println(gateway);

  Serial.print("Subnet: ");

  Serial.println(subnet);


  // START HTTP SERVER

  server.begin();

  updateOLED();


  Serial.println();

  Serial.println("==========================");

  Serial.println("NEXUS CONTROL CENTER READY");

  Serial.println("Open browser:");

  Serial.println("http://192.168.1.100");

  Serial.println("==========================");

}


// ==================================================
// 10. MAIN LOOP
// ==================================================

void loop() {

  handleClient();

}
