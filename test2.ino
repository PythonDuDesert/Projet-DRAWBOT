#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "TrojanHorse";
const char* password = "F18hornet";
WebServer server(80);


// User led
#define LEDU1 25
#define LEDU2 26

// Enable moteurs droit et gauche
#define EN_D 23
#define EN_G 4

// Commande PWM moteur droit
#define IN_1_D 19
#define IN_2_D 18

// Commande PWM moteur gauche
#define IN_1_G 17
#define IN_2_G 16

// Encodeur gauche
#define ENC_G_CH_A 32
#define ENC_G_CH_B 33

// Encodeur droit
#define ENC_D_CH_A 27
#define ENC_D_CH_B 14

// I2C
#define SDA 21
#define SCL 22

// Adresse I2C
#define ADDR_IMU 0x6B
#define ADDR_MAG 0x1E


void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Contrôle ESP32</title>";
  html += "<style>button{width:120px;height:50px;margin:10px;font-size:18px;}</style></head><body>";
  html += "<h2>Contrôle du Robot</h2>";
  html += "<button onclick=\"location.href='/avancer'\">Avancer</button><br>";
  html += "<button onclick=\"location.href='/gauche'\">Gauche</button>";
  html += "<button onclick=\"location.href='/droite'\">Droite</button><br>";
  html += "<button onclick=\"location.href='/reculer'\">Reculer</button>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("\nConnexion au WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnecté !");
  Serial.println(WiFi.localIP());

  // Initialisation des broches comme sorties
  pinMode(EN_D, OUTPUT);
  pinMode(EN_G, OUTPUT);
  pinMode(IN_1_D, OUTPUT);
  pinMode(IN_2_D, OUTPUT);
  pinMode(IN_1_G, OUTPUT);
  pinMode(IN_2_G, OUTPUT);

  // Activer les moteurs (mettre les broches EN à HIGH)
  digitalWrite(EN_D, HIGH);
  digitalWrite(EN_G, HIGH);

  // Route principale (page HTML)
  server.on("/", handleRoot);

  // Routes de commande
  server.on("/avancer", []() {
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, HIGH);
    digitalWrite(IN_1_G, HIGH);
    digitalWrite(IN_2_G, LOW);
    delay(500); // Mouvement de 0.5s
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, LOW);
    server.sendHeader("Location", "/"); server.send(303);
  });

  server.on("/reculer", []() {
    digitalWrite(IN_1_D, HIGH);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, HIGH);
    delay(500);
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, LOW);
    server.sendHeader("Location", "/"); server.send(303);
  });

  server.on("/gauche", []() {
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, HIGH);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, HIGH);
    delay(500);
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, LOW);
    server.sendHeader("Location", "/"); server.send(303);
  });

  server.on("/droite", []() {
    digitalWrite(IN_1_D, HIGH);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, HIGH);
    digitalWrite(IN_2_G, LOW);
    delay(500);
    digitalWrite(IN_1_D, LOW);
    digitalWrite(IN_2_D, LOW);
    digitalWrite(IN_1_G, LOW);
    digitalWrite(IN_2_G, LOW);
    server.sendHeader("Location", "/"); server.send(303);
  });

  server.begin();
  Serial.println("Serveur HTTP lancé.");
}

void loop() {
  server.handleClient();
}
