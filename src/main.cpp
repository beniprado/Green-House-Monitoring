#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "Greenhouse-Monitoring";
const char* password = "kitkats1";

#define DHTPIN 18  // DHT11 conectado ao pino 18
#define HMSOLO 34  // Umidade do solo conectado ao pino 34
#define LDR 35     // LDR conectado ao pino 35
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char *htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Green-house Monitoring</title><style>
body { background-color: #BBE1D6; text-align: center; font-family: 'Lucida Sans', 'Lucida Sans Regular', 'Lucida Grande', 'Lucida Sans Unicode', Geneva, Verdana, sans-serif; margin: 0; padding: 0 } header { margin-bottom: 10px; padding: 10px; background-color: #74DBBC; color: #315C4F } main { padding: 0 0 80px } h1 { margin: 0; font-size: 2em } div#info { display: flex; padding: 10px; justify-content: space-evenly }.container { display: flex; flex-wrap: wrap; justify-content: center; align-content: center; padding: 20px } section { width: 225px; padding: 10px; margin: 10px; border-radius: 10px; background-color: #315C4F; color: #FFF; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); transition: transform .2s } h2 { font-size: 1.2em; color: #74DBBC; margin-bottom: 10px } p { margin: 5px 0 } .divstats{display: flex; flex-direction: column;} footer { padding: 10px; background-color: #315C4F; color: #FFF; position: fixed; bottom: 0; width: 100%; text-align: center } .atual { display: inline-block; padding: 11px; color: #74DBBC; font-weight: 700; border: 2px solid #193029; border-radius: 4px; text-align: center } .Min-Max { font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif; font-size: 16px } .divmm { display: flex; flex-direction: row; justify-content: space-around } @media screen and (max-width: 540px) { body { margin: 0 auto; text-align: center } h1#title { font-size: 6.1vw; } .container { padding: 0 } .statsV { font-size: 2.8vw; } section { margin: 20px auto } footer { position: static; padding: 15px 0 } footer>p { font-size: 13px } }</style>
</head>
<body>
    <header>
        <h1 id="title">Green-house Monitoring</h1>
        <div id="info">
            <div class="divstats">
                <span id="tempodat" class="stats">&#9201;</span>
                <span id="timerst" class="statsV">%TIMERST%</span>
            </div>
            <div class="divstats">
                <span id="Userstitle" class="stats">&#x1F464;</span>
                <span id="users_value" class="statsV">%USERS_ON%</span>
            </div>
        </div>
    </header>
    <main>
        <div class="container">
            <section>
                <h2 id="temptitle">Temperatura</h2>
                <p id="temp_atual" class="atual">%TEMPERATURA%&#8451;</p>
                <div class="divmm">
                    <p id="temp_min" class="Min-Max">Min: %TEMP_MIN%&#8451;</p>
                    <p id="temp_max" class="Min-Max">Max: %TEMP_MAX%&#8451;</p>
                </div>
            </section>
            <section>
                <h2 id="humititle">Umidade</h2>
                <p id="umidade_atual" class="atual">%UMIDADE%%</p>
                <div class="divmm">
                    <p id="umidade_min" class="Min-Max">Min: %UMID_MIN%%</p>
                    <p id="umidade_max" class="Min-Max">Max: %UMID_MAX%%</p>
                </div>
            </section>
            <section>
                <h2 id="soiltitle">Umidade do solo</h2>
                <p id="soilinfo" class="atual">%HSOLO%%</p>
                <p id="soilstatus">Status: %HSOLO_STATUS%</p>
            </section>
            <section>
                <h2 id="lumititle">Luminosidade</h2>
                <p id="lumiinfo" class="atual">%LDR%%</p>
                <p id="lumimax">Status: %LDR_STATUS%</p>
            </section>
        </div>
    </main>
    <footer>
        <p>© 2024 Green-house Monitoring Versão 1.4.3</p>
        <code>Feito por <a href="https://www.instagram.com/beni.prado/" style="color: white;" target="_blank">Beni</a></code>
    </footer>
    <script>
        function atualizarDados() {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        var data = JSON.parse(xhr.responseText);
                        document.getElementById('temp_atual').textContent = "" + data.temperature + "℃";
                        document.getElementById('temp_max').textContent = "Max: " + data.temp_max + "℃";
                        document.getElementById('temp_min').textContent = "Min: " + data.temp_min + "℃";
                        document.getElementById('umidade_atual').textContent = "" + data.humidity + "%";
                        document.getElementById('umidade_max').textContent = "Max: " + data.hum_max + "%";
                        document.getElementById('umidade_min').textContent = "Min: " + data.hum_min + "%";
                        document.getElementById('soilinfo').textContent = "" + data.HMSOLO + "%";
                        document.getElementById('soilstatus').textContent = "Status: " + data.HSOLO_STATUS;  // Atualiza o status do solo
                        document.getElementById('lumiinfo').textContent = "" + data.LDR + "%";
                        document.getElementById('lumimax').textContent = "Status: " + data.LDR_STATUS;  // Atualiza o status da luminosidade
                        document.getElementById('users_value').textContent = data.users_value;
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

int users = 0;

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(HMSOLO, INPUT);
    pinMode(LDR, INPUT);

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);

    server.on("/", HTTP_GET, []() {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int SOhumidity = analogRead(HMSOLO);
        SOhumidity = map(SOhumidity, 0, 4095, 100, 0); // Corrigido para reverter os valores
        int LDR_V = analogRead(LDR);
        LDR_V = map(LDR_V, 0, 4095, 0, 100);

        if (isnan(temperature) || isnan(humidity) || isnan(SOhumidity) || isnan(LDR_V)){
            temperature = 0.0;
            humidity = 0.0;
            SOhumidity = 0;
            LDR_V = 0;
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
        
        String html = htmlPage;
        html.replace("%TEMPERATURA%", String(temperature, 1));
        html.replace("%TEMP_MAX%", String(max_valueT, 1));
        html.replace("%TEMP_MIN%", String(min_valueT, 1));
        html.replace("%UMIDADE%", String(humidity, 1));
        html.replace("%HMSOLO%", String(SOhumidity));
        html.replace("%LDR%", String(LDR_V));
        html.replace("%UMID_MAX%", String(max_valueU, 1));
        html.replace("%UMID_MIN%", String(min_valueU, 1));

        if (SOhumidity < 30) {
            html.replace("%HSOLO_STATUS%", "Seco");
        } else if (SOhumidity >= 30 && SOhumidity < 70) {
            html.replace("%HSOLO_STATUS%", "Apropriado");
        } else {
            html.replace("%HSOLO_STATUS%", "Encharcado");
        }

        if (LDR_V < 30) {
            html.replace("%LDR_STATUS%", "Baixa luminosidade");
        } else if (LDR_V >= 30 && LDR_V < 70) {
            html.replace("%LDR_STATUS%", "Luminosidade adequada");
        } else {
            html.replace("%LDR_STATUS%", "Luminosidade intensa");
        }

        html.replace("%USERS_ON%", String(users));
        html.replace("%TIMERST%", tims);
        server.send(200, "text/html", html);
    });

   server.on("/data", HTTP_GET, []() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int SOhumidity = analogRead(HMSOLO);
    SOhumidity = map(SOhumidity, 0, 4095, 100, 0); 
    int LDR_V = analogRead(LDR);
    LDR_V = map(LDR_V, 0, 4095, 0, 100);

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

    if (LDR_V > 100) {
        LDR_V = 100;
    }

    if (isnan(temperature) || isnan(humidity) || isnan(SOhumidity) || isnan(LDR_V)){
        temperature = 0.0;
        humidity = 0.0;
        SOhumidity = 0;
        LDR_V = 0;
    }

    String soilStatus;
    if (SOhumidity < 30) {
        soilStatus = "Seco";
    } else if (SOhumidity >= 30 && SOhumidity < 70) {
        soilStatus = "Apropriado";
    } else {
        soilStatus = "Encharcado";
    }

    String lightStatus;
    if (LDR_V < 30) {
        lightStatus = "Baixa luminosidade";
    } else if (LDR_V >= 30 && LDR_V < 70) {
        lightStatus = "Luminosidade adequada";
    } else {
        lightStatus = "Luminosidade intensa";
    }

    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"HMSOLO\":" + String(SOhumidity) + ",";
    json += "\"LDR\":" + String(LDR_V) + ",";
    json += "\"temp_max\":" + String(max_valueT, 1) + ",";
    json += "\"temp_min\":" + String(min_valueT, 1) + ",";
    json += "\"hum_max\":" + String(max_valueU, 1) + ",";
    json += "\"hum_min\":" + String(min_valueU, 1) + ",";
    json += "\"HSOLO_STATUS\":\"" + soilStatus + "\",";
    json += "\"LDR_STATUS\":\"" + lightStatus + "\",";
    json += "\"users_value\":\""  + String(users) + "\",";
    json += "\"timerst\":\"" + tims + "\"";
    json += "}";

    server.send(200, "application/json", json);
});

    server.begin();
}

void loop() {
    unsigned long currentMillis = millis();
    Serial.available();
    users = WiFi.softAPgetStationNum();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int SOhumidity = analogRead(HMSOLO);
    int LDR_V = analogRead(LDR);
    if (isnan(h) || isnan(t) || isnan(SOhumidity) || isnan(LDR_V)) {
        Serial.println("Failed to read from sensor!");
        return;
    }

    if (currentMillis - tempstart >= interval) {
        tempstart = currentMillis;
        seconds++;
        if (seconds == 60) {
            seconds = 0;
            minutes++;
        }
        tims = (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
    }
    server.handleClient();
}