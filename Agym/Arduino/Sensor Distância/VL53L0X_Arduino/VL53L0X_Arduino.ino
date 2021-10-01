#include <Wire.h>
#include <VL53L0X.h>

// Cria uma instancia do sensor
VL53L0X sensor;

// Variaveis para armazenar a distancia atual e o ultimo valor lido
int dist_max = 0, dist_min = 30, dist = 0, dist_old = 0;
// Variavel para armazenar o tempo na parte do timeout
unsigned long timeout = 0;

void setup()
{
  // Perfil de longo alcance
// sensor.setSignalRateLimit(0.1);
// sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
// sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  
  // Inicializa a comunicação serial
  Serial.begin(115200);
  // Inicializa a comunicação I2C
  Wire.begin();
  // Inicializa o sensor
  sensor.init();
  // Perfil de alta precisão
  sensor.setMeasurementTimingBudget(200000); 
  // Define um timeout de 500mS para a leitura do sensor
  // Em caso de erro, este será o tempo máximo de espera da resposta do sensor
  sensor.setTimeout(500);
  // Inicializacao do valor maximo
  for (int i = 0; i < 5; i++) {
    dist_max = sensor.readRangeSingleMillimeters();
    dist = sensor.readRangeSingleMillimeters();
    delay(50);
  }
}

void loop()
{
  
  // Faz a medição da distância e retorna um valor em milímetros
  dist = sensor.readRangeSingleMillimeters();
  // Filtra o valor de distancia
  filtrar_sinal();
  // Imprime no monitor serial
  Serial.println(dist);
  Serial.println(dist_max);
  delay(50);
}

// Função para filtrar o valor medido
void filtrar_sinal()
{
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
    // Descarta a medição feita e iguala ela ao mínimo
    dist = dist_min;
  }
  else {
    // Não descarta a medição atual e atualiza a medição antiga para a atual
    dist_old = dist;
    timeout = millis(); // Reseta o valor da variável do timeout
  }
}
