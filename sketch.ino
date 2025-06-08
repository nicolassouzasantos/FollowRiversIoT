#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h> // Para formatar a resposta JSON

// --- Configurações de Wi-Fi ---
const char* ssid = "Wokwi-GUEST"; // Para Wokwi, use "Wokwi-GUEST"
const char* password = "";        // Para Wokwi, deixe em branco

// --- Variáveis e Constantes ---
int pinoPotenciometro = 34; // Pino ADC1_CH6 no ESP32
float distanciaMaxima = 10.0;
float alturaMaximaDoRio = 10.0;

// Objeto WebServer na porta 80 (porta padrão HTTP)
WebServer server(80);

// --- Funções de Leitura e Cálculo ---
// Função para ler o potenciômetro e calcular as distâncias
void medirNivelRio(float &distanciaSimulada, float &alturaDoRio) {
  int valorLido = analogRead(pinoPotenciometro);
  distanciaSimulada = map(valorLido, 0, 4095, 0, distanciaMaxima * 100) / 100.0;
  alturaDoRio = alturaMaximaDoRio - distanciaSimulada;

  if (alturaDoRio < 0) {
    alturaDoRio = 0;
  }
}

// --- Funções do Servidor Web ---

// Função que será chamada quando o ESP32 receber uma requisição HTTP para a raiz ("/")
void handleRoot() {
  float distanciaSimulada;
  float alturaDoRio;
  medirNivelRio(distanciaSimulada, alturaDoRio); // Pega os valores atualizados

  // Cria um objeto JSON para a resposta
  StaticJsonDocument<200> doc; // Tamanho do buffer para o JSON

  doc["distancia_sensor_agua_m"] = String(distanciaSimulada, 2); // 2 casas decimais
  doc["altura_rio_m"] = String(alturaDoRio, 2); // 2 casas decimais

  String jsonResponse;
  serializeJson(doc, jsonResponse); // Converte o JSON em uma string

  // Envia a resposta HTTP com o conteúdo JSON
  server.send(200, "application/json", jsonResponse);
}

// --- Configuração Inicial ---
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando simulação de medição de nível do rio...");

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configura a rota "/" para chamar a função handleRoot
  server.on("/", handleRoot);

  // Inicia o servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();

  float distanciaSimulada;
  float alturaDoRio;
  medirNivelRio(distanciaSimulada, alturaDoRio);
  Serial.print("Distância do sensor à água: ");
  Serial.print(distanciaSimulada, 2);
  Serial.print(" m   |   ");
  Serial.print("Altura do Rio: ");
  Serial.print(alturaDoRio, 2);
  Serial.println(" m");

  delay(5000); // Pequeno delay para não sobrecarregar o ESP32
}
