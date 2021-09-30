#include "HX711.h"

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN PB3   // Pino PB3(2) como entrada de dados dos sensores de carga
#define LOADCELL_SCK_PIN PB4   // Pino PB4(3) como clock para o sensor de carga

HX711 scale;

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando a balanca...");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("Remova todos os pesos da balança");
  delay(1000);
  scale.set_scale(2280.f);  // Valor obtido a partir a calibração
  scale.tare();  // Zera a balança
  Serial.println("Balanca iniciada!");   
  delay(1000);
}

void loop() {

  if (scale.is_ready()) {
    long carga = scale.read();
    Serial.print("Peso: ");
    Serial.println(carga);
  } else {
    Serial.println("Peso não encontrado");
  }

  delay(1000);
  
}
