#include <WiFi.h>
#include <WebServer.h>

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


// Nombre de ticks des encodeurs (33 ticks = 1cm)
volatile long int nbr_ticks_gauche = 0, nbr_ticks_droite = 0;
volatile long int target_nbr_ticks_gauche = 0, target_nbr_ticks_droite = 0;

void IRAM_ATTR onTickGauche() {
    nbr_ticks_gauche++;
}

void IRAM_ATTR onTickDroite() {
    nbr_ticks_droite++;
}

// Pour le WiFi
const char* ssid = "TrojanHorse";
const char* password = "F18hornet";
WebServer server(80);

void handleRoot() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>DRAWBOT - InterfaceWeb</title>
        <style>
            * {
                margin: 0;
            }
            header {
                display: flex;
                align-items: center;
                justify-content: center;
                height: 150px;
                background-color: rgba(0, 113, 121, 0.7);
                position: relative;
            }
            .title-container {
                text-align: center;
            }
            h1 {
                font-size: 60px;
                color: white;
            }

            body {
                background-image: url("background.PNG");
                background-repeat: no-repeat;
                background-size: cover;
                background-color: rgba(0, 0, 0, 0.3);
                background-blend-mode: overlay;
            }

            main {
                max-width: 1200px;
                margin: 2rem auto;
                padding: 0 1rem;
            }

            .card {
                background-color: rgba(255, 255, 255, 0.8);
                border-radius: 0.5rem;
                overflow: hidden;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.7);
            }

            .card-header {
                background-color: #007179;
                color: white;
                padding: 1rem;
                border-bottom: 1px solid #e5e7eb;
                font-size: 1.25rem;
                font-weight: bold;
            }

            .card-body {
                padding: 1.5rem;
            }

            .section-title {
                font-size: 30px;
                color: rgba(0, 113, 121, 0.8);
                margin-bottom: 1rem;
            }

            /* Movement controls */
            .movement-buttons {
                display: grid;
                grid-template-columns: repeat(3, 1fr);
                gap: 1rem;
                max-width: 24rem;
                margin: 0 auto;
            }

            /* Sequence controls */
            .sequence-buttons {
                display: grid;
                grid-template-columns: 1fr;
                gap: 1rem;
                grid-template-columns: repeat(3, 1fr);
            }

            /* Button styles */
            .buttons {
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                padding: 1.5rem 1rem;
                background-color: #007179;
                color: white;
                border: none;
                border-radius: 0.375rem;
                font-size: 1rem;
                font-weight: 500;
                cursor: pointer;
                transition: all 0.2s ease;
            }

            .buttons:hover {
                background-color: #00aeba;
            }

            #button_stop {
                background-color: #ec2424;
            }

            #button_stop:hover{
                background-color: #be1d1d;
            }
        </style>
        <script>
        function avancer() { fetch("/avancer"); }
        function reculer() { fetch("/reculer"); }
        function gauche() { fetch("/gauche"); }
        function droite() { fetch("/droite"); }
        function stop() { fetch("/stop"); }
        </script>
    </head>

    <body>
        <header>
            <h1>DRAWBOT</h1>
        </header>
        <main>
            <div class="card">
                <div class="card-header">
                    <h2>Contrôles DRAWBOT</h2>
                </div>

                <div class="card-body">
                    <!-- Movement Controls -->
                    <div class="section">
                        <h3 class="section-title">Contrôles de mouvement</h3>
                        <div class="movement-buttons">

                            <!-- Top row -->
                            <div class="empty-cell"></div>
                            <button class="buttons" onmousedown="avancer()" onmouseup="stop()">
                                <span>Avancer</span>
                            </button>
                            <div class="empty-cell"></div>
                            
                            <!-- Middle row -->
                            <button class="buttons" onmousedown="gauche()" onmouseup="stop()">
                                <span>Gauche</span>
                            </button>
                            <button class="buttons" id="button_stop" onmousedown="stop()">
                                <span>STOP</span>
                            </button>
                            <button class="buttons" onmousedown="droite()" onmouseup="stop()">
                                <span>Droite</span>
                            </button>
                            
                            <!-- Bottom row -->
                            <div class="empty-cell"></div>
                            <button class="buttons" onmousedown="reculer()" onmouseup="stop()">
                                <span>Reculer</span>
                            </button>
                            <div class="empty-cell"></div>

                        </div>
                    </div>
                    
                    <!-- Sequence Controls -->
                    <div class="section">
                        <h3 class="section-title">Séquences</h3>
                        <div class="sequence-buttons">
                            <button class="buttons">
                                <span>Séquence 1</span>
                            </button>
                            <button class="buttons">
                                <span>Séquence 2</span>
                            </button>
                            <button class="buttons">
                                <span>Séquence 3</span>
                            </button>
                            <button class="buttons">
                                <span>Bonus 1</span>
                            </button>
                            <button class="buttons">
                                <span>Bonus 2</span>
                            </button>
                            <button class="buttons">
                                <span>Bonus 3</span>
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </main>
    </body>
    </html>)rawliteral";
    server.send(200, "text/html", html);
}


void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.println("\nConnexion au WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
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
    server.on("/avancer", avancer);
    server.on("/reculer", reculer);
    server.on("/gauche", gauche);
    server.on("/droite", droite);
    server.on("/stop", stop);
    server.begin();
    Serial.println("Serveur HTTP lancé.");

    pinMode(ENC_G_CH_A, INPUT);
    pinMode(ENC_D_CH_A, INPUT);
    attachInterrupt(digitalPinToInterrupt(ENC_G_CH_A), onTickGauche, RISING);
    attachInterrupt(digitalPinToInterrupt(ENC_D_CH_A), onTickDroite, RISING);

    // Configure les broches PWM pour les moteurs
    //https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#ledc
    ledcAttach(IN_1_D, 5000, 8);
    ledcAttach(IN_2_D, 5000, 8);
    ledcAttach(IN_1_G, 5000, 8);
    ledcAttach(IN_2_G, 5000, 8);
}

void loop() {
    server.handleClient();

    static unsigned long memo_time = 0;
    if (millis() - memo_time > 1000) {
        Serial.print("Ticks gauche: ");
        Serial.println(nbr_ticks_gauche);
        Serial.print("Ticks droite: ");
        Serial.println(nbr_ticks_droite);
        memo_time = millis();
    }

}

/*
void stop() {
    analogWrite(IN_1_D, 0);
    analogWrite(IN_2_D, 0);
    analogWrite(IN_1_G, 0);
    analogWrite(IN_2_G, 0);
}

void avancer() {
    analogWrite(IN_1_D, 0);
    analogWrite(IN_2_D, 128);
    analogWrite(IN_1_G, 128);
    analogWrite(IN_2_G, 0);
}

void reculer() {
    analogWrite(IN_1_D, 128);
    analogWrite(IN_2_D, 0);
    analogWrite(IN_1_G, 0);
    analogWrite(IN_2_G, 128);
}

void gauche() {
    analogWrite(IN_1_D, 0);
    analogWrite(IN_2_D, 96);
    analogWrite(IN_1_G, 0);
    analogWrite(IN_2_G, 96);
}

void droite() {
    analogWrite(IN_1_D, 96);
    analogWrite(IN_2_D, 0);
    analogWrite(IN_1_G, 96);
    analogWrite(IN_2_G, 0);
}
*/


void stop() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 0);
}

void avancer() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 192);
    ledcWrite(IN_1_G, 192);
    ledcWrite(IN_2_G, 0);
}

void reculer() {
    ledcWrite(IN_1_D, 192);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 192);
}

void gauche() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 128);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 128);
}

void droite() {
    ledcWrite(IN_1_D, 128);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 128);
    ledcWrite(IN_2_G, 0);
}
