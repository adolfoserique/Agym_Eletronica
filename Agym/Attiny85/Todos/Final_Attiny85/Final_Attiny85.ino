// Perfil de longo alcance
// sensor.setSignalRateLimit(0.1);
// sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
// sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

// Os pinos PB0 e PB2 são usados para o I2C, onde PB0 é o clock e o PB2 é a sáida dos dados

// O pino PB5 não será utilizado, logo não é necessário a gravação dele
// PB5 é o pino de RESET

#include <Wire.h>
#include <SoftwareSerial.h>
#include <VL53L0X.h>

// Define os pinos UART do Attiny85 
#define TX_VX PB1   // Pino PB1(6) como TX
#define RX_VX PB5   // Pino PB5(1) como RX (não é utilizado)
#define LOADCELL_DOUT_PIN PB3   // Pino PB3(2) como entrada de dados dos sensores de carga
#define LOADCELL_SCK_PIN PB4   // Pino PB4(3) como clock para o sensor de carga

// Cria uma instancia do sensor
VL53L0X sensor;

SoftwareSerial BTserial(RX_VX ,TX_VX); // RX | TX

// Variaveis para armazenar a distancia atual e o ultimo valor lido
int dist = 0, dist_old = 0;
// Variavel para armazenar o tempo na parte do timeout
unsigned long timeout = 0;

void setup()
{ 
  // Inicializa a comunicação I2C
  Wire.begin();
  // Inicializa o sensor
  sensor.init();
  // Define um timeout de 500mS para a leitura do sensor
  // Em caso de erro, este será o tempo máximo de espera da resposta do sensor
  sensor.setTimeout(500);
  // Perfil de alta precisão do sensor
  sensor.setMeasurementTimingBudget(200000);
  // Inicializa a comunicação serial
  BTserial.begin(9600);

}

void loop()
{
  // Faz a medição da distância e retorna um valor em milímetros
  int dist = sensor.readRangeSingleMillimeters();

  // Filtra o valor de distancia
  filtrar_sinal(); 
 
  // Imprime no monitor serial
  BTserial.print(dist);
  BTserial.print(";");

  delay(20);
}

// Função para filtrar o valor medido
void filtrar_sinal()
{
  // Se a distância medida for maior que 8000 e ainda não tiver passado 1 segundo de timeout
  if (dist > 8000 && ((millis() - timeout) < 1000))
  {
    // Descarta a medição feita e iguala ela à anterior
    dist = dist_old;
  }
  else // Caso contrário (medição < 8000 ou passou do timeout)
  {
    // Não descarta a medição atual e atualiza a medição antiga para a atual
    dist_old = dist;
    timeout = millis(); // Reseta o valor da variável do timeout
  }
}
