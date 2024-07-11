#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "Greenhouse-Monitoring";
const char* password = "kitkats9";

#define DHTPIN 18  // DHT11 connected to pin 18
#define HMSOLO 34  // Soil humidity connected to pin 34
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Green-house Monitoring</title>
    <style>
        body { background-color: #BBE1D6; text-align: center; font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif; margin: 0; padding: 0; }
        header { margin-bottom: 10px; padding: 10px; background-color: #74DBBC; color: #315C4F; }
        h1 { margin: 0; font-size: 2em; }
        div#info { padding: 10px; }
        span#tempodat { font-weight: bold; }
        span#tim { color: #193029; font-weight: bold; font-size: 16px; }
        .container { display: flex; flex-wrap: wrap; justify-content: center; padding: 20px; }
        article { width: 200px; padding: 15px; margin: 10px; border-radius: 10px; background-color: #315C4F; color: #FFFFFF; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); transition: transform 0.2s; }
        article:hover { transform: scale(1.05); }
        h2 { font-size: 1.2em; color: #74DBBC; margin-bottom: 10px; }
        p { margin: 5px 0; }
        button { padding: 10px 20px; margin: 20px; border: none; border-radius: 5px; background-color: #74DBBC; color: #315C4F; font-size: 12px; cursor: pointer; transition: background-color 0.3s; }
        button:hover { background-color: #62c0a7; }
        footer { padding: 10px; background-color: #315C4F; color: #FFFFFF; position: fixed; bottom: 0; width: 100%; text-align: center; }
        @media screen and (max-width: 760px) {
            body { margin: 0 auto; text-align: center; }
            .container { flex-direction: column; padding: 0; }
            article { width: 70%; margin: 10px auto; }
            footer { position: static; padding: 20px 0; }
        }
    </style>
</head>
<body>
    <header>
        <h1 id="title">Green-house Monitoring</h1>
        <div id="info">
            <span id="tempodat">Tempo de Atividade</span><br>
            <span id="timerst">%TIMERST%</span>
        </div>
    </header>
    <main>
        <div class="container">
            <article>
                <h2 id="temptitle">Temperatura</h2>
                <p id="temp_atual"class="atual">Temp: %TEMPERATURA%&#8451;</p>
                <p id="temp_max">Max: %TEMP_MAX%&#8451;</p>
                <p id="temp_min">Min: %TEMP_MIN%&#8451;</p>
            </article>
            <article>
                <h2 id="humititle">Umidade</h2>
                <p id="umidade_atual"class="atual">Hum: %UMIDADE%%</p>
                <p id="umidade_max">Max: %UMID_MAX%%</p>
                <p id="umidade_min">Min: %UMID_MIN%%</p>
            </article>
            <article>
                <h2 id="soiltitle">Umidade do solo</h2>
                <p id="soilinfo"class="atual">Hsolo: %HSOLO%%</p>
            </article>
            <article>
                <h2 id="lumititle">Luminosidade</h2>
                <p id="lumiinfo"class="atual">Lumens: N/A</p>
                <p id="lumimax">Status: N/A</p>
            </article>
        </div>
    </main>
    <footer>
        <p>© 2024 Green-house Monitoring Versão 1.2</p><code>Feito por <a href="https://www.instagram.com/beni.prado/" style="color: white;" target="_blank">Beni</a></code>
    </footer>
    <script>
        function atualizarDados() {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        var data = JSON.parse(xhr.responseText);
                        document.getElementById('temp_atual').textContent = "Temp: " + data.temperature + "℃";
                        document.getElementById('temp_max').textContent = "Max: " + data.temp_max + "℃";
                        document.getElementById('temp_min').textContent = "Min: " + data.temp_min + "℃";
                        document.getElementById('umidade_atual').textContent = "Hum: " + data.humidity + "%";
                        document.getElementById('umidade_max').textContent = "Max: " + data.hum_max + "%";
                        document.getElementById('umidade_min').textContent = "Min: " + data.hum_min + "%";
                        document.getElementById('soilinfo').textContent = "Hsolo: " + data.HMSOLO + "%";
                        document.getElementById('timerst').textContent = data.timerst;
                    } else {
                        console.error('Erro ao carregar dados:', xhr.status);
                    }
                }
            };
            xhr.open("GET", "/data", true);
            xhr.send();
        }
        atualizarDados();
        setInterval(atualizarDados, 1000);
    </script>
</body>
</html>
)rawliteral";

float max_valueT = 0.0;
float min_valueT = 100.0;  // Valor inicial alto para encontrar o mínimo
float max_valueU = 0.0;
float min_valueU = 100.0;  // Valor inicial alto para encontrar o mínimo
int max_valueHS = 0;
int min_valueHS = 100;     // Inicializado com 100 para encontrar o mínimo

unsigned long tempstart = 0;
unsigned long interval = 1000;
int seconds = 0;
int minutes = 0;
String tims = "00:00";

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(HMSOLO, INPUT);

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    server.on("/", HTTP_GET, []() {
        String html = htmlPage;
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int soilHumidity = analogRead(HMSOLO);
        soilHumidity = map(soilHumidity, 0, 4095, 0, 100);  // Ajuste para melhorar a precisão
        
        if (isnan(temperature) || isnan(humidity) || isnan(soilHumidity)) {
            temperature = 0.0;
            humidity = 0.0;
            soilHumidity = 0;
        }
        
        // Atualiza valores máximos e mínimos
        if (temperature > max_valueT) {
            max_valueT = temperature;
        }
        if (temperature < min_valueT) {
            min_valueT = temperature;
        }
        if (humidity > max_valueU) {
            max_valueU = humidity;
        }
        if (humidity < min_valueU) {
            min_valueU = humidity;
        }
        
        html.replace("%TIMERST%", tims);
        html.replace("%TEMPERATURA%", String(temperature));
        html.replace("%TEMP_MAX%", String(max_valueT));
        html.replace("%TEMP_MIN%", String(min_valueT));
        html.replace("%UMIDADE%", String(humidity));
        html.replace("%UMID_MAX%", String(max_valueU));
        html.replace("%UMID_MIN%", String(min_valueU));
        
        // Garante que a umidade do solo não exceda 100%
        if (soilHumidity > 100) {
            soilHumidity = 100;
        }
        
        html.replace("%HSOLO%", String(soilHumidity));
        server.send(200, "text/html", html);
    });

    server.on("/data", HTTP_GET, []() {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int soilHumidity = analogRead(HMSOLO);
        soilHumidity = map(soilHumidity, 0, 4095, 0, 100);  // Ajuste para melhorar a precisão
        
        if (isnan(temperature) || isnan(humidity) || isnan(soilHumidity)) {
            temperature = 0.0;
            humidity = 0.0;
            soilHumidity = 0;
        }
        
        // Atualiza valores máximos e mínimos
        if (temperature > max_valueT) {
            max_valueT = temperature;
        }
        if (temperature < min_valueT) {
            min_valueT = temperature;
        }
        if (humidity > max_valueU) {
            max_valueU = humidity;
        }
        if (humidity < min_valueU) {
            min_valueU = humidity;
        }
        
        String json = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"temp_max\": " + String(max_valueT) + ", \"temp_min\": " + String(min_valueT) + ", \"hum_max\": " + String(max_valueU) + ", \"hum_min\": " + String(min_valueU) + ", \"HMSOLO\": " + String(soilHumidity) + ", \"HMAX\": " + String(max_valueHS) + ", \"HMIN\": " + String(min_valueHS) + ", \"timerst\": \"" + tims + "\"}";
        server.send(200, "application/json", json);
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int soilHumidity = analogRead(HMSOLO);
    soilHumidity = map(soilHumidity, 0, 4095, 0, 100);  // Ajuste para melhorar a precisão

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Atualiza valores máximos e mínimos
    if (t > max_valueT) {
        max_valueT = t;
    }
    if (t < min_valueT) {
        min_valueT = t;
    }
    if (h > max_valueU) {
        max_valueU = h;
    }
    if (h < min_valueU) {
        min_valueU = h;
    }

    // Atualiza tempo
    if (millis() - tempstart >= interval) {
        tempstart = millis();
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
        }
    }

    tims = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);

    // Saída Serial para debug
    if (millis() - tempstart >= 2000) {
        tempstart = millis();
        Serial.print("Humidity: ");
        Serial.println(h);
        Serial.print("Temperature: ");
        Serial.println(t);
        Serial.print("Time: ");
        Serial.print(minutes);
        Serial.print(":");
        if (seconds < 10) {
            Serial.print("0");
        }
        Serial.println(seconds);
        Serial.print("Soil Humidity: ");
        Serial.print(soilHumidity);
        Serial.println("%");
    }
}
