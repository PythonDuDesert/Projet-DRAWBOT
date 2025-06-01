#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <math.h>


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

// Registres du LIS3MDL
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define OUT_X_L   0x28

#define conversion_number 36 // Nombre de ticks des encodeurs (36 ticks = 1cm)


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

unsigned short int target_distance = 0;
unsigned short int target_angle = 0;
float last_angle_error = 0.0;
float current_angle;

// PID distance
float kp_dist = 2, ki_dist = 0, kd_dist = 0.9;
double P_dist = 0, I_dist = 0, D_dist = 0;

// PID écart
float kp_diff = 1.2, ki_diff = 0, kd_diff = 0.2;
float P_diff = 0, I_diff = 0, D_diff = 0;

bool in_sequence1 = false, in_sequence2 = false, in_sequence3 = false, in_bonus1 = false, in_bonus2 = false, in_bonus3 = false;

// Compteur de ticks
void IRAM_ATTR onTickGauche() {
    if (digitalRead(ENC_G_CH_A) == HIGH) {
        wheel_left.nbr_ticks++;
    }
    else {
        wheel_left.nbr_ticks--;
    }
}

void IRAM_ATTR onTickDroite() {
    if (digitalRead(ENC_D_CH_B) == HIGH) {
        wheel_right.nbr_ticks++;
    }
    else {
        wheel_right.nbr_ticks--;
    }
}


// Pour le WiFi
const char* ssid = "TrojanHorse";  //nom du partage de co ou du wifi sur lequel il se connecte
const char* password = "F18hornet"; //mot de passe de ce partage ou de ce wifi
//puis aller sur internet et taper : 192.168.x.x (IP affiché dans la console)
WebServer server(80);


/*
const char* ssid = "SFR_9F7F";  //nom du partage de co ou du wifi sur lequel il se connecte
const char* password = "y4pw4r5lcwji97qrq5w4"; //mot de passe de ce partage ou de ce wifi
//puis aller sur internet et taper : 192.168.x.x (IP affiché dans la console)
WebServer server(80);
*/

void handleRoot() { //Interface web
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
                function start_sequence_1() {
                    fetch("/start_sequence1");
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
                        </div>

                        <!-- Sequence Controls -->
                        <div class="section">
                            <h3 class="section-title">Séquences</h3>
                            <div id="sequence-buttons">
                                <button class="buttons" onclick="start_sequence_1()">
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
                                <button class="buttons" style="cursor: not-allowed;">
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


// setup
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
    attachInterrupt(digitalPinToInterrupt(ENC_G_CH_B), onTickGauche, RISING);
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
    server.on("/start_sequence1", start_sequence1);
    server.begin();
    Serial.println("Serveur HTTP lancé.");

    wheel_left.target_ticks = 0;
    wheel_left.nbr_ticks = 0;
    wheel_left.error = 0;
    wheel_left.last_error = 0;
    wheel_left.speed = 0;
    wheel_right.target_ticks = 0;
    wheel_right.nbr_ticks = 0;
    wheel_right.error = 0;
    wheel_right.last_error = 0;
    wheel_right.speed = 0;

    Wire.begin();  // SDA = 21, SCL = 22 par défaut

    // Initialisation du magnétomètre
    Wire.beginTransmission(ADDR_MAG);
    Wire.write(CTRL_REG1);
    Wire.write(0b11110000); // Ultra-high performance, 80 Hz ODR
    Wire.endTransmission();

    Wire.beginTransmission(ADDR_MAG);
    Wire.write(CTRL_REG2);
    Wire.write(0b00000000); // ±4 gauss
    Wire.endTransmission();

    Wire.beginTransmission(ADDR_MAG);
    Wire.write(CTRL_REG3);
    Wire.write(0b00000000); // Mode continu
    Wire.endTransmission();
}

// loop
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
        sequence1();
        delay(50);
    }

    get_angle();
}


void tracing() {
    float distance = (float)((wheel_left.nbr_ticks + wheel_right.nbr_ticks) / 2) / (float)conversion_number;
    Serial.print("Distance:\t");
    Serial.print(distance);
    Serial.print("\tSpeed_L:\t");
    Serial.print(wheel_left.speed);
    Serial.print("\tSpeed_R:\t");
    Serial.println(wheel_right.speed);
}

void stop() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 0);
    in_sequence1 = false;
    in_sequence2 = false;
    in_sequence3 = false;
    in_bonus1 = false;
    in_bonus2 = false;
    in_bonus3 = false;
}

void avancer() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 200);
    ledcWrite(IN_1_G, 200);
    ledcWrite(IN_2_G, 0);
}

void reculer() {
    ledcWrite(IN_1_D, 200);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 200);
}

void gauche() {
    ledcWrite(IN_1_D, 0);
    ledcWrite(IN_2_D, 160);
    ledcWrite(IN_1_G, 0);
    ledcWrite(IN_2_G, 160);
}

void droite() {
    ledcWrite(IN_1_D, 160);
    ledcWrite(IN_2_D, 0);
    ledcWrite(IN_1_G, 160);
    ledcWrite(IN_2_G, 0);
}

void reset() {
    stop();
    wheel_left.nbr_ticks = 0;
    wheel_right.nbr_ticks = 0;
}

void start_sequence1() {
    reset();
    in_sequence1 = true;
    server.send(200, "text/plain", "Sequence 1 started");
} 

void sequence1() {
    pid_distance(20);
    turn(90);
    pid_distance(10);
    turn(-90);
    pid_distance(40);

    stop();
    Serial.println("Séqunce 1 terminée !");

    tracing();
}

float pid_distance(int distance) {
    //https://projecthub.arduino.cc/anova9347/line-follower-robot-with-pid-controller-01813f
    wheel_left.target_ticks = distance*conversion_number;
    wheel_right.target_ticks = distance*conversion_number;
    wheel_left.error = wheel_left.target_ticks - wheel_left.nbr_ticks;
    wheel_right.error = wheel_right.target_ticks - wheel_right.nbr_ticks;

    while (abs(wheel_left.error) > 10 || abs(wheel_right.error) > 10 || wheel_left.speed != 0 || wheel_right.speed != 0) {
        wheel_left.error = wheel_left.target_ticks - wheel_left.nbr_ticks;
        wheel_right.error = wheel_right.target_ticks - wheel_right.nbr_ticks;
        int avg_error = (wheel_left.error + wheel_right.error)/2;

        P_dist = avg_error;
        I_dist = I_dist + avg_error;
        D_dist = avg_error - (wheel_left.last_error + wheel_right.last_error)/2;
        wheel_left.last_error = wheel_left.error;
        wheel_right.last_error = wheel_right.error;

        float PID_dist_result = kp_dist*P_dist + ki_dist*I_dist + kd_dist*D_dist;
        float PID_diff_result = pid_ecart();
        
        wheel_left.speed = PID_dist_result - PID_diff_result;
        wheel_right.speed = PID_dist_result + PID_diff_result;
        wheel_left.speed = constrain(wheel_left.speed, -180, 180); //Plafond
        wheel_right.speed = constrain(wheel_right.speed, -180, 180); //Plafond
        if (abs(wheel_left.speed) < 100 && abs(wheel_left.speed) >= 70) { //Plancher
            wheel_left.speed = copysign(100, wheel_left.speed);
        }
        else if (abs(wheel_left.speed) < 70 && abs(wheel_left.speed) >= 10) {
            wheel_left.speed = copysign(90, wheel_left.speed);
        }
        else if (abs(wheel_left.speed) < 10) {
            wheel_left.speed = 0;
        }
        if (abs(wheel_right.speed) < 100 && abs(wheel_right.speed) >= 70) { //Plancher
            wheel_right.speed = copysign(100, wheel_right.speed);
        }
        else if (abs(wheel_right.speed) < 70 && abs(wheel_right.speed) >= 10) {
            wheel_right.speed = copysign(90, wheel_right.speed);
        }
        else if (abs(wheel_right.speed) < 10) {
            wheel_right.speed = 0;
        }

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

        float distance = (float)((wheel_left.nbr_ticks+wheel_right.nbr_ticks)/2)/(float)conversion_number;
        Serial.print("Distance : ");
        Serial.println(distance);
    }

    bool distance_terminee = true;
    return distance_terminee;
}

float pid_ecart() {
    int error_diff = wheel_left.nbr_ticks - wheel_right.nbr_ticks;
    P_diff = error_diff;
    I_diff += error_diff;
    D_diff = error_diff - (wheel_left.last_error + wheel_right.last_error)/2;
    wheel_right.last_error = error_diff;

    float PID_diff_result = kp_diff*P_diff + ki_diff*I_diff + kd_diff*D_diff;

    return PID_diff_result;
}

void turn(int angle) {
    const float amplitude = 180; // valeur maximale du speed en module
    const float step = 0.1f;     // incrément de t à chaque itération
    const float delay_us = 10000; // 10 ms

    // t va de -π/2 à π/2, ce qui fait passer sin(t) de -1 à 1
    for (float t = -M_PI_2; t <= M_PI_2; t += step) {
        float sin_value = sin(t); // varie de -1 à 1
        wheel_right.speed = amplitude;
        wheel_left.speed = amplitude * sin_value;
        
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

        usleep(delay_us); // pause pour que l'évolution soit progressive
    }
}


void get_angle() { //Aide IA pour le magnétomètre
    int16_t mx, my, mz;

    // Lire 6 octets à partir de OUT_X_L
    Wire.beginTransmission(ADDR_MAG);
    Wire.write(OUT_X_L | 0x80); // auto-increment des registres
    Wire.endTransmission(false); // redémarrage

    Wire.requestFrom(ADDR_MAG, 6);
    if (Wire.available() == 6) {
        uint8_t xL = Wire.read();
        uint8_t xH = Wire.read();
        uint8_t yL = Wire.read();
        uint8_t yH = Wire.read();
        uint8_t zL = Wire.read();
        uint8_t zH = Wire.read();

        mx = (int16_t)(xH << 8 | xL);
        my = (int16_t)(yH << 8 | yL);
        mz = (int16_t)(zH << 8 | zL);

        // Calcul de l'offset à la volée (moyenne des extrêmes)
        float mx_offset = (-645 -2930) / 2.0;
        float my_offset = (1390 -1180) / 2.0;

        // Correction des données
        float mx_corr = mx - mx_offset;
        float my_corr = my - my_offset;

        // Calcul du cap
        float heading = atan2(my_corr, mx_corr) * 180.0 / PI;
        heading = 360.0 - heading;

        // Normalisation entre 0 et 360
        heading = fmod(heading + 360.0, 360.0);

        /*Serial.print("Heading: ");
        Serial.print(heading);
        Serial.println("°");*/
    }
}
