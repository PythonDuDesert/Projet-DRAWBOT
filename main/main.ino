#include <WiFi.h>
#include <WebServer.h>
#include <PID_v1.h>


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
const short int conversion_number = 33;
unsigned short int target_distance = 20;
volatile long int target_ticks = target_distance * conversion_number;
volatile long int nbr_ticks_left = 0, nbr_ticks_right = 0;
volatile long int error_left = target_ticks - nbr_ticks_left;
volatile long int error_right = target_ticks - nbr_ticks_right;
volatile long int last_error_left = target_ticks - nbr_ticks_left;
volatile long int last_error_right = target_ticks - nbr_ticks_right;
double input_left, output_left, setpoint_left;
double input_right, output_right, setpoint_right;

// Coefficients PID
const float kp = 1, ki = 1, kd = 1;
double P = 0, I = 0, D = 0;
bool in_sequence1 = false;

void IRAM_ATTR onTickGauche() {
  nbr_ticks_left++;
}

void IRAM_ATTR onTickDroite() {
  nbr_ticks_right++;
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
                background-color: #b61d1d;
            }

            #button_reset {
                background-color: #8a8a8a;
            }

            #button_reset:hover{
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
        function reset_ticks() {
            fetch("/reset_ticks");
        }
        function sequence_1() {
            fetch("/sequence1");
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
                            <button class="buttons" id="button_stop" onclick="stop()">
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
                            <div class="empty-cell"></div>
                            <button class="buttons" id="button_reset" onclick="reset_ticks()">
                                <span>Reset ticks</span>
                            </button>

                        </div>
                    </div>
                    
                    <!-- Sequence Controls -->
                    <div class="section">
                        <h3 class="section-title">Séquences</h3>
                        <div class="sequence-buttons">
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
  server.on("/reset_ticks", reset_ticks);
  server.on("/sequence1", sequence1);
  server.begin();
  Serial.println("Serveur HTTP lancé.");
}

void loop() {
  server.handleClient();

  static unsigned long memo_time = 0;
  if (millis() - memo_time > 1000) {
    Serial.print("Ticks gauche: ");
    Serial.println(nbr_ticks_left);
    Serial.print("Ticks droite: ");
    Serial.println(nbr_ticks_right);
    memo_time = millis();
  }

  if (in_sequence1) {
    //https://projecthub.arduino.cc/anova9347/line-follower-robot-with-pid-controller-01813f
    error_left = target_ticks - nbr_ticks_left;
    error_right = target_ticks - nbr_ticks_right;

    P = kp * error_left;

    I = I + error_left;

    D = error_left - last_error_left;
    last_error_left = error_left;

    float PID_result = P * kp + I * ki + D * kd;

    short int speed_left = PID_result;

    // Commande moteurs gauche
    if (speed_left > 0) {
      ledcWrite(IN_1_G, speed_left);
      ledcWrite(IN_2_G, 0);
    } else {
      ledcWrite(IN_1_G, 0);
      ledcWrite(IN_2_G, -speed_left);
    }
    // Commande moteurs droit
    if (speed_left > 0) {
      ledcWrite(IN_1_D, 0);
      ledcWrite(IN_2_D, speed_left);
    } else {
      ledcWrite(IN_1_D, -speed_left);
      ledcWrite(IN_2_D, 0);
    }

    // Condition d'arrêt si erreur négligeable
    if (abs(error_left) < 5) {
      stop();
      in_sequence1 = false;
      Serial.println("Séquence 1 terminée avec précision");
      return;  // Stop PID
    }

    if (abs(error_left) > target_ticks+33 || abs(error_right) > target_ticks+33) { // Sécurité
      stop();
      in_sequence1 = false;
      Serial.println("Erreur!");
      return;  // Stop PID
    }

    Serial.print("Error left : ");
    Serial.println(error_left);
    Serial.print("Error right : ");
    Serial.println(error_right);
    Serial.print("P : ");
    Serial.println(P);
    Serial.print("I : ");
    Serial.println(I);
    Serial.print("D : ");
    Serial.println(D);
    /*
    Serial.print("Ticks gauche : ");
    Serial.println(nbr_ticks_left);
    Serial.print("Ticks droite : ");
    Serial.println(nbr_ticks_right);
    */
  }
}

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

void reset_ticks() {
  stop();
  nbr_ticks_left = 0;
  nbr_ticks_right = 0;
  Serial.println(nbr_ticks_left);
  Serial.println(nbr_ticks_right);
}

void sequence1() {
  reset_ticks();
  in_sequence1 = true;
  server.send(200, "text/plain", "Sequence 1 started");
}
