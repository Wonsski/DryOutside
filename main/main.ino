#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <DHT22.h>
#include <PMserial.h>

#define dhtPIN 23
#define pmTXPIN 18
#define pmRXPIN 5
#define YL83_ANALOG_PIN 15
#define YL83_DIGITAL_PIN 2

const char* ssid = "ESP32-AP";
const char* password = "12345678";

DHT22 dht22(dhtPIN);
SerialPM pms(PMSx003, pmRXPIN, pmTXPIN);

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int maxDataPoints = 50;
float tempHistory[maxDataPoints];
float humHistory[maxDataPoints];
float pm1History[maxDataPoints];
float pm25History[maxDataPoints];
float pm10History[maxDataPoints];
float rainHistory[maxDataPoints];
int dataIndex = 0;

void setup() {
  Serial.begin(9600);
  pms.init();

  pinMode(YL83_ANALOG_PIN, INPUT);
  pinMode(YL83_DIGITAL_PIN, INPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/history", handleHistory);
  server.on("/error", handleError);

  server.begin();
  Serial.println("HTTP server started");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Odczyty czujników</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            background-color: #f0f0f0;
        }
        .container {
            width: 90%;
            max-width: 1200px;
            background: #fff;
            padding: 20px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            border-radius: 8px;
            margin-top: 20px;
        }
        .tiles {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-around;
            margin-bottom: 20px;
        }
        .tile {
            width: 150px;
            height: 100px;
            background: #fff;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            margin: 10px;
            transition: background-color 0.3s;
            text-align: center;
        }
        .tile.good {
            background-color: #4caf50;
        }
        .tile.moderate {
            background-color: #ffeb3b;
        }
        .tile.bad {
            background-color: #f44336;
        }
        .tile span {
            font-size: 24px;
            font-weight: bold;
        }
        .tile p {
            margin: 0;
            font-size: 16px;
        }
        #qualityIndicator {
            width: 100%;
            height: 50px;
            background: linear-gradient(to right, green, yellow, red);
            position: relative;
            margin-bottom: 20px;
        }
        #pointer {
            width: 10px;
            height: 50px;
            background: black;
            position: absolute;
            top: 0;
            left: 50%;
            transform: translateX(-50%);
        }
        .chartContainer {
            width: 100%;
            height: 400px;
            margin-bottom: 20px;
            position: relative;
        }
        canvas {
            width: 100%;
            height: 100%;
        }
        .point {
            position: absolute;
            width: 7px;
            height: 7px;
            border-radius: 50%;
            transform: translate(-50%, -50%);
            cursor: pointer;
        }
        .point::before {
          content: '';
          position: absolute;
          top: -180px;
          left: -10px;
          width: 30px;
          height: 360px;
          background-color: transparent;
        }
        .point:hover::after {
            content: attr(data-value);
            position: absolute;
            top: -50px;
            left: 50%;
            transform: translateX(-50%);
            background: rgba(0, 0, 0, 0.7);
            color: #fff;
            padding: 5px;
            border-radius: 5px;
            white-space: nowrap;
        }
        .chartTitle {
            text-align: center;
            font-size: 18px;
            margin-bottom: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Odczyty czujników</h1>
        <div class="tiles">
            <div class="tile" id="tempTile">
                <span id="tempValue"></span>
                <p>Temperatura</p>
            </div>
            <div class="tile" id="humTile">
                <span id="humValue"></span>
                <p>Wilgotność</p>
            </div>
            <div class="tile" id="pm1Tile">
                <span id="pm1Value"></span>
                <p>PM1.0</p>
            </div>
            <div class="tile" id="pm25Tile">
                <span id="pm25Value"></span>
                <p>PM2.5</p>
            </div>
            <div class="tile" id="pm10Tile">
                <span id="pm10Value"></span>
                <p>PM10</p>
            </div>
            <div class="tile" id="rainTile">
                <span id="rainValue"></span>
                <p>Deszcz</p>
            </div>
        </div>
        <div id="qualityIndicator">
            <div id="pointer"></div>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">Temperatura</div>
            <canvas id="tempChart" width="1000" height="400"></canvas>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">Wilgotność</div>
            <canvas id="humChart" width="1000" height="400"></canvas>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">PM1.0</div>
            <canvas id="pm1Chart" width="1000" height="400"></canvas>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">PM2.5</div>
            <canvas id="pm25Chart" width="1000" height="400"></canvas>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">PM10</div>
            <canvas id="pm10Chart" width="1000" height="400"></canvas>
        </div>
        <div class="chartContainer">
            <div class="chartTitle">Deszcz</div>
            <canvas id="rainChart" width="1000" height="400"></canvas>
        </div>
        <button onclick='testAlert()'>Test Alert</button>
    </div>
    <script>
        var tempData = [];
        var humData = [];
        var pm1Data = [];
        var pm25Data = [];
        var pm10Data = [];
        var rainData = [];

        function drawChart(canvasId, data, color, label, yMin, yMax) {
            var canvas = document.getElementById(canvasId);
            var ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            // Set canvas dimensions to avoid pixelation
            canvas.width = canvas.clientWidth;
            canvas.height = canvas.clientHeight;

            // Draw axes
            ctx.beginPath();
            ctx.moveTo(50, 10);
            ctx.lineTo(50, canvas.height - 50);
            ctx.lineTo(canvas.width - 10, canvas.height - 50);
            ctx.stroke();

            // Draw axis labels
            ctx.font = '12px Arial';
            ctx.fillText('Wartość', 10, 20);
            ctx.fillText('Czas', canvas.width - 40, canvas.height - 10);

            // Draw min and max values on y-axis
            ctx.fillText(yMin, 10, canvas.height - 50);
            ctx.fillText(yMax, 10, 40);

            // Draw vertical lines for timestamps
            for (var i = 0; i < data.length; i++) {
                var x = 50 + (canvas.width - 60) * (i / (data.length - 1));
                ctx.beginPath();
                ctx.moveTo(x, canvas.height - 50);
                ctx.lineTo(x, 10);
                ctx.strokeStyle = '#e0e0e0';
                ctx.stroke();
            }

            // Draw data
            ctx.beginPath();
            ctx.strokeStyle = color;
            ctx.moveTo(50, canvas.height - 50 - ((data[0] - yMin) / (yMax - yMin)) * (canvas.height - 60));
            for (var i = 1; i < data.length; i++) {
                ctx.lineTo(50 + (canvas.width - 60) * (i / (data.length - 1)), canvas.height - 50 - ((data[i] - yMin) / (yMax - yMin)) * (canvas.height - 60));
            }
            ctx.stroke();

            // Add points
            var chartContainer = canvas.parentElement;
            chartContainer.querySelectorAll('.point').forEach(point => point.remove());
            for (var i = 0; i < data.length; i++) {
                var x = 50 + (canvas.width - 60) * (i / (data.length - 1));
                var y = 30 + canvas.height - 50 - ((data[i] - yMin) / (yMax - yMin)) * (canvas.height - 60);
                var point = document.createElement('div');
                point.className = 'point';
                point.style.left = x + 'px';
                point.style.top = y + 'px';
                point.style.backgroundColor = color;
                point.setAttribute('data-value', label + ': ' + data[i] + getUnit(label));
                chartContainer.appendChild(point);
            }
        }

        function getUnit(label) {
            switch (label) {
                case 'Temperatura':
                    return ' °C';
                case 'Wilgotność':
                    return ' %';
                case 'PM1.0':
                case 'PM2.5':
                case 'PM10':
                    return ' µg/m³';
                case 'Deszcz':
                    return ' %';
                default:
                    return '';
            }
        }

        function updateData() {
            fetch('/data').then(response => response.json()).then(data => {
                document.getElementById('tempValue').innerText = data.temperature + ' °C';
                document.getElementById('humValue').innerText = data.humidity + ' %';
                document.getElementById('pm1Value').innerText = data.pm1 + ' µg/m³';
                document.getElementById('pm25Value').innerText = data.pm25 + ' µg/m³';
                document.getElementById('pm10Value').innerText = data.pm10 + ' µg/m³';
                document.getElementById('rainValue').innerText = data.rainDigital ? 'Brak deszczu' : 'Deszcz';

                updateTileColor('tempTile', data.temperature);
                updateTileColor('humTile', data.humidity);
                updateTileColor('pm1Tile', data.pm1);
                updateTileColor('pm25Tile', data.pm25);
                updateTileColor('pm10Tile', data.pm10);
                updateTileColor('rainTile', data.rainDigital ? 0 : 100);

                if (tempData.length >= 50) {
                    tempData.shift();
                    humData.shift();
                    pm1Data.shift();
                    pm25Data.shift();
                    pm10Data.shift();
                    rainData.shift();
                }

                tempData.push(data.temperature);
                humData.push(data.humidity);
                pm1Data.push(data.pm1);
                pm25Data.push(data.pm25);
                pm10Data.push(data.pm10);
                rainData.push(data.rainAnalog);

                drawChart('tempChart', tempData, 'red', 'Temperatura', Math.min(...tempData) - 5, Math.max(...tempData) + 5);
                drawChart('humChart', humData, 'blue', 'Wilgotność', 0, 100);
                drawChart('pm1Chart', pm1Data, 'green', 'PM1.0', 0, Math.max(...pm1Data) + 5);
                drawChart('pm25Chart', pm25Data, 'orange', 'PM2.5', 0, Math.max(...pm25Data) + 5);
                drawChart('pm10Chart', pm10Data, 'purple', 'PM10', 0, Math.max(...pm10Data) + 5);
                drawChart('rainChart', rainData, 'blue', 'Deszcz', 0, 100);

                updatePointer(data.pm25);
            });
        }

        function updateTileColor(tileId, value) {
            var tile = document.getElementById(tileId);
            if (value < 50) {
                tile.className = 'tile good';
            } else if (value < 100) {
                tile.className = 'tile moderate';
            } else {
                tile.className = 'tile bad';
            }
        }

        function updatePointer(value) {
            const percentage = value / 100;
            const pointer = document.getElementById('pointer');
            pointer.style.left = `${percentage * 100}%`;
        }

        function loadHistory() {
            fetch('/history').then(response => response.json()).then(history => {
                tempData = history.tempHistory;
                humData = history.humHistory;
                pm1Data = history.pm1History;
                pm25Data = history.pm25History;
                pm10Data = history.pm10History;
                rainData = history.rainHistory;

                drawChart('tempChart', tempData, 'red', 'Temperatura', Math.min(...tempData) - 5, Math.max(...tempData) + 5);
                drawChart('humChart', humData, 'blue', 'Wilgotność', 0, 100);
                drawChart('pm1Chart', pm1Data, 'green', 'PM1.0', 0, Math.max(...pm1Data) + 5);
                drawChart('pm25Chart', pm25Data, 'orange', 'PM2.5', 0, Math.max(...pm25Data) + 5);
                drawChart('pm10Chart', pm10Data, 'purple', 'PM10', 0, Math.max(...pm10Data) + 5);
                drawChart('rainChart', rainData, 'blue', 'Deszcz', 0, 100);
            });
        }

        function testAlert() {
            var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
            ws.onopen = function() {
                ws.send('testAlert');
            };
        }

        setInterval(updateData, 5000);
        loadHistory(); // Load history on page load
    </script>
</body>
</html>
)";
  server.send(200, "text/html", html);
}

void handleData() {
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();
  pms.read();
  int rainAnalog = analogRead(YL83_ANALOG_PIN);
  int rainDigital = digitalRead(YL83_DIGITAL_PIN);

  if (dataIndex >= maxDataPoints) {
    for (int i = 1; i < maxDataPoints; i++) {
      tempHistory[i - 1] = tempHistory[i];
      humHistory[i - 1] = humHistory[i];
      pm1History[i - 1] = pm1History[i];
      pm25History[i - 1] = pm25History[i];
      pm10History[i - 1] = pm10History[i];
      rainHistory[i - 1] = rainHistory[i];
    }
    dataIndex = maxDataPoints - 1;
  }

  tempHistory[dataIndex] = t;
  humHistory[dataIndex] = h;
  pm1History[dataIndex] = pms.pm01;
  pm25History[dataIndex] = pms.pm25;
  pm10History[dataIndex] = pms.pm10;
  rainHistory[dataIndex] = rainAnalog;
  dataIndex++;

  String json = "{";
  json += "\"temperature\":" + String(t) + ",";
  json += "\"humidity\":" + String(h) + ",";
  json += "\"pm1\":" + String(pms.pm01) + ",";
  json += "\"pm25\":" + String(pms.pm25) + ",";
  json += "\"pm10\":" + String(pms.pm10) + ",";
  json += "\"rainAnalog\":" + String(rainAnalog) + ",";
  json += "\"rainDigital\":" + String(rainDigital);
  json += "}";

  server.send(200, "application/json", json);
}

void handleHistory() {
  String json = "{";
  json += "\"tempHistory\":[" + joinArray(tempHistory, dataIndex) + "],";
  json += "\"humHistory\":[" + joinArray(humHistory, dataIndex) + "],";
  json += "\"pm1History\":[" + joinArray(pm1History, dataIndex) + "],";
  json += "\"pm25History\":[" + joinArray(pm25History, dataIndex) + "],";
  json += "\"pm10History\":[" + joinArray(pm10History, dataIndex) + "],";
  json += "\"rainHistory\":[" + joinArray(rainHistory, dataIndex) + "]";
  json += "}";

  server.send(200, "application/json", json);
}

String joinArray(float* array, int length) {
  String result = "";
  for (int i = 0; i < length; i++) {
    if (i > 0) result += ",";
    result += String(array[i]);
  }
  return result;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    if (strcmp((char*)payload, "testAlert") == 0) {
      webSocket.broadcastTXT("Alert: Test alert triggered!");
    }
  }
}

void handleTestAlert() {
  webSocket.broadcastTXT("Alert: Test alert triggered!");
  server.send(200, "text/plain", "Test alert sent");
}