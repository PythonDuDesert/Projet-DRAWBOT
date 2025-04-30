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


void setup() {
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
}

void loop() {
  digitalWrite(IN_1_D, LOW);
  digitalWrite(IN_2_D, HIGH);
  digitalWrite(IN_1_G, HIGH);
  digitalWrite(IN_2_G, LOW);
}