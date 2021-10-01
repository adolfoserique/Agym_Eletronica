#include <Wire.h>
#include <SoftwareSerial.h>
#include <VL53L0X.h>

// Cria uma instancia do sensor
VL53L0X sensor;

// Muda os pinos UART do Arduino 
SoftwareSerial BTserial(10, 11); // RX | TX

// Variaveis para armazenar a distancia atual, distancia inicial, o ultimo valor lido e a carga
int dist_max = 0, dist_min = 30, dist = 0, dist_old = 0;
float carga = 0, defor = 0;
// Variavel para armazenar o tempo na parte do timeout
unsigned long timeout = 0;

void setup()
{
  // Inicializa a comunicação I2C
  Wire.begin();
  // Inicializa o sensor
  sensor.init();
  // Perfil de alta precisão
  sensor.setMeasurementTimingBudget(200000); 
  // Define um timeout de 500mS para a leitura do sensor
  // Em caso de erro, este será o tempo máximo de espera da resposta do sensor
  sensor.setTimeout(500);
  // Inicialização do valor maximo (5 medidas iniciais)
  for (int i = 0; i < 5; i++) {
    dist_max = sensor.readRangeSingleMillimeters();
    dist = sensor.readRangeSingleMillimeters();
    delay(50);
  }
  // Inicializa a comunicação serial
  BTserial.begin(9600);
}

void loop() {
  // Faz a medição da distância e retorna um valor em milímetros
  dist = sensor.readRangeSingleMillimeters();

  // Filtra o valor de distancia
  filtrar_sinal();

  defor = dist_max - dist;
  defor = defor / 10;
  carga = 0.75 * defor;
 
  // Imprime no monitor serial
  BTserial.print(defor, 1);
  BTserial.print(" cm");
  BTserial.print(","); // "," para o proximo sensor ou ";" para finalizar a lista de sensores
  BTserial.print(carga, 2);
  BTserial.print(" kg");
  BTserial.print(";");
  
  delay(500);
}

// Função para filtrar o valor medido
void filtrar_sinal() {
  // Se a distância medida for maior que o máximo e ainda não tiver passado 1 segundo de timeout
  if ((millis() - timeout) < 1000) {
    // Descarta a medição feita e iguala ela à anterior
    dist = dist_old;
  }
  else if (dist >= dist_max) {
    // Descarta a medição feita e iguala ela ao máximo
    dist = dist_max;
  }
  else if (dist <= dist_min) {
    // Descarta a medição feita e iguala ela a zero
    dist = 0;
  }
  else {
    // Não descarta a medição atual e atualiza a medição antiga para a atual
    dist_old = dist;
    timeout = millis(); // Reseta o valor da variável do timeout
  }
}
