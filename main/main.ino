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

#define conversion_number 33 // Nombre de ticks des encodeurs (33 ticks = 1cm)


class Wheel{
    public:
    volatile long int target_ticks;
    volatile long int nbr_ticks;
    volatile long int error;
    volatile long int last_error;
    short int speed;
};

Wheel wheel_left;
Wheel wheel_right;

unsigned short int target_distance = 20;

// Coefficients PID
float kp = 3, ki = 0.1, kd = 0.5;
double P = 0, I = 0, D = 0;
bool in_sequence1 = false;

void IRAM_ATTR onTickGauche() {
    if (digitalRead(ENC_G_CH_B) == LOW) {
        wheel_left.nbr_ticks++;
    } else {
        wheel_left.nbr_ticks--;
    }
}

void IRAM_ATTR onTickDroite() {
    if (digitalRead(ENC_D_CH_B) == HIGH) {
        wheel_right.nbr_ticks++;
    } else {
        wheel_right.nbr_ticks--;
    }
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
                    font-family: arial, sans-serif;
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
                #movement-buttons {
                    display: grid;
                    grid-template-columns: repeat(3, 1fr);
                    gap: 1rem;
                    max-width: 24rem;
                    margin: 0 auto;
                }

                /* Paramètres */
                #parameters {
                    margin: 1.5rem 0 2rem;
                    display: flex;
                }

                #coeff {
                    background-color: rgba(138, 138, 138, 0.3);
                    border: solid 3px black;
                    border-radius: 5px;
                    padding: 10px;
                    font-weight: bold;
                }

                input {
                    width : 3rem;
                    margin-right: 0.75rem;
                }

                /* Sequence controls */
                #sequence-buttons {
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

                #button_stop:hover {
                    background-color: #b61d1d;
                }

                #button_reset {
                    background-color: #8a8a8a;
                }

                #button_reset:hover {
                    background-color: #6a6a6a;
                }
            </style>
            <script>
                function avancer() {
                    fetch("/avancer");
                }
                function reculer() {
                    fetch("/reculer");
                }
                function gauche() {
                    fetch("/gauche");
                }
                function droite() {
                    fetch("/droite");
                }
                function stop() {
                    fetch("/stop");
                }
                function reset() {
                    fetch("/reset");
                }
                function sequence_1() {
                    fetch("/sequence1");
                }
                function update_coeff() {
                    let kp = document.getElementById("coeff_kp").value;
                    let ki = document.getElementById("coeff_ki").value;
                    let kd = document.getElementById("coeff_kd").value;
                    fetch(`/update_coeff?coeff_kp=${kp}&coeff_ki=${ki}&coeff_kd=${kd}`);
                }
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
                            <div id="movement-buttons">

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
                                <button class="buttons" id="button_stop" onclick="stop()">
                                    <span>STOP</span>
                                </button>
                                <button class="buttons" onmousedown="droite()" onmouseup="stop()">
                                    <span>Droite</span>
                                </button>

                                <!-- Bottom row -->
                                <button class="buttons" id="button_reset" onclick="reset()">
                                    <span>Reset</span>
                                </button>
                                <button class="buttons" onmousedown="reculer()" onmouseup="stop()">
                                    <span>Reculer</span>
                                </button>
                                <div class="empty-cell"></div>
                            </div>

                            <div id="parameters">
                                <div id="coeff">
                                    <span>kp: <input type="number" name="coeff_kp" id="coeff_kp" value="2" min="0" step="0.1"></span>
                                    <span>ki: <input type="number" name="coeff_ki" id="coeff_ki" value="0.1" min="0" step="0.1"></span>
                                    <span>kd: <input type="number" name="coeff_kd" id="coeff_kd" value="0.5" min="0" step="0.1"></span>
                                    <span><input type="submit" value="SEND" id="send_coeff" onclick="update_coeff()"></span>
                                </div>
                            </div>
                        </div>

                        <!-- Sequence Controls -->
                        <div class="section">
                            <h3 class="section-title">Séquences</h3>
                            <div id="sequence-buttons">
                                <button class="buttons" onclick="sequence_1()">
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

    pinMode(ENC_G_CH_A, INPUT);
    pinMode(ENC_D_CH_A, INPUT);
    attachInterrupt(digitalPinToInterrupt(ENC_G_CH_A), onTickGauche, RISING);
    attachInterrupt(digitalPinToInterrupt(ENC_D_CH_A), onTickDroite, RISING);

    // Activer les moteurs (mettre les broches EN à HIGH)
    digitalWrite(EN_D, HIGH);
    digitalWrite(EN_G, HIGH);

    // Configure les broches PWM pour les moteurs
    //ledcAttach(pin, fréquence, résolution)
    //https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#ledc
    ledcAttach(IN_1_D, 5000, 8);
    ledcAttach(IN_2_D, 5000, 8);
    ledcAttach(IN_1_G, 5000, 8);
    ledcAttach(IN_2_G, 5000, 8);

    // Route principale (page HTML)
    server.on("/", handleRoot);
    server.on("/avancer", avancer);
    server.on("/reculer", reculer);
    server.on("/gauche", gauche);
    server.on("/droite", droite);
    server.on("/stop", stop);
    server.on("/reset", reset);
    server.on("/sequence1", sequence1);
    server.begin();
    Serial.println("Serveur HTTP lancé.");

    wheel_left.target_ticks = target_distance*conversion_number;
    wheel_left.nbr_ticks = 0;
    wheel_left.error = 0;
    wheel_left.last_error = 0;
    wheel_left.speed = 0;
    wheel_right.target_ticks = target_distance*conversion_number;
    wheel_right.nbr_ticks = 0;
    wheel_right.error = 0;
    wheel_right.last_error = 0;
    wheel_right.speed = 0;
}

void loop() {
    server.handleClient();

    static unsigned long memo_time = 0;
    if (millis() - memo_time > 200) {
        Serial.print("Ticks gauche: ");
        Serial.println(wheel_left.nbr_ticks);
        Serial.print("Ticks droite: ");
        Serial.println(wheel_right.nbr_ticks);
        memo_time = millis();
    }

    if (in_sequence1) {
        //https://projecthub.arduino.cc/anova9347/line-follower-robot-with-pid-controller-01813f
        wheel_left.error = wheel_left.target_ticks - wheel_left.nbr_ticks;
        wheel_right.error = wheel_right.target_ticks - wheel_right.nbr_ticks;

        P = wheel_right.error;
        I = I + wheel_right.error;
        D = wheel_right.error - wheel_right.last_error;
        wheel_right.last_error = wheel_right.error;

        float PID_result = kp*P + ki*I + kd*D;
        wheel_left.speed = PID_result;
        wheel_right.speed = PID_result;

        Serial.print("Error left : ");
        Serial.println(wheel_left.error);
        Serial.print("Error right : ");
        Serial.println(wheel_right.error);

        Serial.print("Ticks gauche : ");
        Serial.println(wheel_left.nbr_ticks);
        Serial.print("Ticks droite : ");
        Serial.println(wheel_right.nbr_ticks);

        float distance = (float)wheel_right.nbr_ticks/(float)conversion_number;
        Serial.print("Distance : ");
        Serial.println(distance);

        // Commande moteurs gauche
        if (wheel_left.speed > 0) {
            ledcWrite(IN_1_G, wheel_left.speed);
            ledcWrite(IN_2_G, 0);
        } else {
            ledcWrite(IN_1_G, 0);
            ledcWrite(IN_2_G, -wheel_left.speed);
        }
        // Commande moteurs droit
        if (wheel_right.speed > 0) {
            ledcWrite(IN_1_D, 0);
            ledcWrite(IN_2_D, wheel_right.speed);
        } else {
            ledcWrite(IN_1_D, -wheel_right.speed);
            ledcWrite(IN_2_D, 0);
        }

        // Condition d'arrêt si erreur négligeable
        if (wheel_left.error==wheel_left.last_error && wheel_right.error==wheel_right.last_error) {
            stop();
            in_sequence1 = false;
            Serial.println("Séquence 1 terminée!");
            return;  // Stop PID
        }

        // Sécurité
        // if (abs(wheel_left.error) > wheel_left.target_ticks+33 || abs(wheel_right.error) > wheel_right.target_ticks+33) {
        //     stop();
        //     in_sequence1 = false;
        //     Serial.println("Erreur!");
        //     return;  // Stop PID
        // }
    }
}

void stop() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 0);
    in_sequence1 = false;
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

void reset() {
    stop();
    wheel_left.nbr_ticks = 0;
    wheel_right.nbr_ticks = 0;
    in_sequence1 = false;
}

void sequence1() {
    reset();
    in_sequence1 = true;
    server.send(200, "text/plain", "Sequence 1 started");
}

void update_coeff() {
    kp = server.arg("coeff_kp").toFloat();
    ki = server.arg("coeff_ki").toFloat();
    kd = server.arg("coeff_kd").toFloat();
    server.send(200, "text/plain", "Coefficients PID mis à jour");
}
