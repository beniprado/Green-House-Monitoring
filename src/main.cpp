#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "Greenhouse-Monitoring";
const char* password = "kitkats9";

#define DHTPIN 18 // DHT11 conectado ao pino 18 (GPIO18)
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
        body {
            background-color: #BBE1D6;
            text-align: center;
            font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
        }

        header {
            margin-bottom: 10px;
            padding: 10px;
            background-color: #74DBBC;
            color: #315C4F;
        }

        h1 {
            margin: 0;
            font-size: 2em;
        }

        div#info{
            padding: 10px;
        }

        span#tempodat{
            font-weight: bold;
        }
        span#tim{
            color: #193029;
            font-weight: bold;
            font-size: 16px;
        }

        .container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            padding: 20px;
        }

        article {
            width: 200px;
            padding: 15px;
            margin: 10px;
            border-radius: 10px;
            background-color: #315C4F;
            color: #FFFFFF;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: transform 0.2s;
        }

        article:hover {
            transform: scale(1.05);
        }

        h2 {
            font-size: 1.2em;
            color: #74DBBC;
            margin-bottom: 10px;
        }

        p {
            margin: 5px 0;
        }

        button {
            padding: 10px 20px;
            margin: 20px;
            border: none;
            border-radius: 5px;
            background-color: #74DBBC;
            color: #315C4F;
            font-size: 12px;
            cursor: pointer;
            transition: background-color 0.3s;
        }

        button:hover {
            background-color: #62c0a7;
        }

        footer {
            padding: 10px;
            background-color: #315C4F;
            color: #FFFFFF;
            position: fixed;
            bottom: 0;
            width: 100%;
            text-align: center;
        }

        @media screen and (max-width: 760px) {
            body{
                margin: 0 auto;
                text-align: center;
            }

            .container {
                flex-direction: column;
                padding: 0;
            }

            article {
                width: 70%;
                margin: 10px auto;
            }

            footer {
                position: static;
                padding: 20px 0;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1 id="title">Green-house Monitoring</h1>
        <div id="info">
            <span id="tempodat">Tempo de Atividade</span>
            <br>
            <span id="timerst">%TIMERST%</span>
        </div>
    </header>
    <div class="container">
        <article>
            <h2 id="temptitle">Temperatura</h2>
            <p id="temp_atual">Temp: %TEMPERATURA%&#8451;</p>
            <p id="temp_max">Max: %TEMP%&#8451;</p>
        </article>
        <article>
            <h2 id="humititle">Umidade</h2>
            <p id="umidade_atual">Hum: %UMIDADE%%</p>
            <p id="umidade_max">Max: %UMID%%</p>
        </article>
          <article>
            <h2 id="lumititle">Luminosidade</h2>
            <p id="lumiinfo">Lumens: N/A</p>
            <p id="lumimax">Max: N/A</p>
        </article>
        <article>
            <h2 id="soiltitle">Umidade do solo</h2>
            <p id="soilinfo">Hsolo: N/A%</p>
            <p id="soilmax">Max: N/A%</p>
        </article>
    </div>
    <footer>
        <p>© 2024 Green-house Monitoring Versão 1.0</p>
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
                        document.getElementById('umidade_atual').textContent = "Hum: " + data.humidity + "%";
                        document.getElementById('umidade_max').textContent = "Max: " + data.hum_max + "%";
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
float max_valueU = 0.0; 

unsigned long tempstart = 0;
unsigned long interval = 1000;

int seconds = 0;
int minutes = 0;
String tims = "00:00";

void setup(){
    Serial.begin(115200);
    dht.begin();

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    tims = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);

    server.on("/", HTTP_GET, [](){
        String html = htmlPage;
        html.replace("%TIMERST%", tims);
        html.replace("%TEMPERATURA%", String(dht.readTemperature()));
        html.replace("%TEMP%", String(max_valueT));
        html.replace("%UMIDADE%", String(dht.readHumidity()));
        html.replace("%UMID%", String(max_valueU));
        server.send(200, "text/html", html);
    });

    server.on("/data", HTTP_GET, [](){
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        String json = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + ", \"temp_max\": " + String(max_valueT) + ", \"hum_max\": " + String(max_valueU) + ", \"timerst\": \"" + tims + "\"}";
        server.send(200, "application/json", json);
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

void loop(){
    server.handleClient();

    float h = dht.readHumidity();
    float t = dht.readTemperature();    

    if (isnan(h) || isnan(t)) {
        Serial.println("Falha ao ler o sensor DHT!");
        return;
    }

    // Atualiza os valores máximos
    if (t > max_valueT) {
        max_valueT = t;
    }
    if (h > max_valueU) {
        max_valueU = h;
    }
    
    if (millis() - tempstart >= interval) {
        tempstart = millis();
        seconds++; // incrementa os segundos

        // se passou 60 segundos, incrementa os minutos e reseta os segundos
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
        }
    }

    tims = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);

    if (millis() - tempstart >= 2000) {
        tempstart = millis(); 
        Serial.print("Umidade: ");
        Serial.println(h);
        Serial.print("Temperatura: ");
        Serial.println(t);
        Serial.print("Temp: ");
        Serial.print(minutes);
        Serial.print(":");
        if (seconds < 10) {
            Serial.print("0");
        }
        Serial.println(seconds);
        Serial.print("HTML PAGE: ");
        Serial.println(tims);
    }
}
