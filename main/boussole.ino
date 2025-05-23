#include <Wire.h>

// Adresse I2C du LIS3MDL
#define ADDR_MAG 0x1E

// Registres du LIS3MDL
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define OUT_X_L   0x28

void setup() {
  Serial.begin(115200);
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

void loop() {
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

    Serial.print("Heading: ");
    Serial.print(heading);
    Serial.println("°");
  }

  delay(200);
}