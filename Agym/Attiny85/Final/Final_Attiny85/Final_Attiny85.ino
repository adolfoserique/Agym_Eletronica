// Criado por Adolfo de Souza Serique Engenheiro Eletrônico a serviço da Agym

// Observações:
// Os pinos PB0 e PB2 são usados para o I2C, onde PB0 é o clock e o PB2 é a sáida dos dados (não usar)
// O pino PB5 não será utilizado, logo não é necessário a gravação dele
// PB5 é o pino de RESET
// "," para o proximo sensor ou ";" para finalizar a lista de sensores no serial


// Bibliotecas
#include <Wire.h>
#include <SoftwareSerial.h>
#include <VL53L0X.h>

// Define os pinos UART do Attiny85 
#define TX_VX PB1   // Pino PB1(6) como TX
#define RX_VX PB5   // Pino PB5(1) como RX (não é utilizado)

// Cria uma instância do sensor
VL53L0X sensor;

// Define os pinos UART do Attiny85 
SoftwareSerial BTserial(RX_VX ,TX_VX); // RX | TX

// Variáveis para armazenar a distância maxima, distância minima, distância atual e o último valor lido
int dist_max = 0, dist_min = 30, dist = 0, dist_old = 0;
// Váriáveis para armazenar a carga e a deformação
float carga = 0, defor = 0;
// Variavel para armazenar o tempo na parte do timeout
unsigned long timeout = 0;

void setup()
{ 
  // Inicializa a comunicação I2C
  Wire.begin();
  // Inicializa o sensor
  sensor.init();
  // Define um timeout de 500 ms para a leitura do sensor
  // Em caso de erro, este será o tempo máximo de espera da resposta do sensor
  sensor.setTimeout(500);
  // Perfil de alta precisão do sensor
  sensor.setMeasurementTimingBudget(200000);
  // Inicialização do valor maximo (5 medidas iniciais)
  for (int i = 0; i < 5; i++) {
    dist_max = sensor.readRangeSingleMillimeters();
    dist = sensor.readRangeSingleMillimeters();
    delay(50);
  }
  // Inicializa a comunicação serial (UART)
  BTserial.begin(9600);
}

void loop()
{
  // Faz a medição da distância e retorna um valor em milímetros (mm)
  dist = sensor.readRangeSingleMillimeters();

  // Filtra o valor de distancia medido
  filtrar_sinal();
  
  // Cálculo da carga
  defor = dist_max - dist;
  defor = defor / 10;
  carga = 0.75 * defor;
 
  // Imprime no monitor serial
  BTserial.print(defor, 1);
  BTserial.print(" cm");
  BTserial.print(","); 
  BTserial.print(carga, 2);
  BTserial.print(" kg");
  BTserial.print(";");

  delay(500);
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
    // Descarta a medição feita e iguala ela a zero
    dist = 0;
  }
  else {
    // Não descarta a medição atual e atualiza a medição antiga para a atual
    dist_old = dist;
    // Reseta o valor da variável do timeout
    timeout = millis(); 
  }
}