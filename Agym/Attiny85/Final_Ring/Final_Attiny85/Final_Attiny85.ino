// Criado por Adolfo de Souza Serique, Engenheiro Eletrônico a serviço da Agym

// Observações:
// Os pinos PB0 e PB2 são usados para o I2C, onde PB0 é o clock e o PB2 é a sáida dos dados (não usar)
// "," para o proximo sensor ou ";" para finalizar a lista de sensores no serial

//OBS: Avaliar como deixar o sensor mais estável e preciso:
    //1) Avaliar a função setMeasurementTimingBudget();
    //2) Colocar resistores de pull-up e capacitores;
    //3) Avaliar se a leitura contínua fica mais estável;
    //4) Testar diferentes tempos na leitura contínua startContinuous(period_ms);
    //5) Avaliar se precisa da função init();
    //6) Avaliar a função setSignalRateLimit();

//--------Anel de pilates inteligente--------//

// Bibliotecas
#include <Wire.h>
#include <SoftwareSerial.h>
#include <VL53L0X.h>

// Define os pinos UART do Attiny85 
#define TX_VX PB1   // Pino PB1(6) como TX
#define RX_VX PB3   // Pino PB3(2) como RX

// Cria uma instância do sensor
VL53L0X sensor;

// Define os pinos UART do Attiny85 
SoftwareSerial BTserial(RX_VX, TX_VX); // RX | TX

// Variáveis para armazenar a distância maxima, distância atual, o último valor lido e o dado analogico
int dist_max = 1000, dist = 0, dist_old = 0, analog = 0;
//int max_old = 295;
// Váriáveis para armazenar a carga, a deformação(unidades: cm e kg / gasta mais memória) e o valor de tensão
//float carga = 0, defor = 0;
float tensao = 0;
// Váriáveis para armazenar a carga e a deformação(unidades: mm e g / gasta menos memória)
//int carga = 0, defor = 0;
// Variavel para armazenar o tempo na parte do timeout
unsigned long timeout = 0;       

void setup()
{ 
  // Inicializa a comunicação serial(UART)
  BTserial.begin(9600);

  // Inicializa a comunicação I2C
  Wire.begin();
  
  // Inicializa o sensor
  sensor.init();
  
  // Define um timeout de 500 ms para a leitura do sensor
  // Em caso de erro, este será o tempo máximo de espera da resposta do sensor
  sensor.setTimeout(500);
  
  // Define o tempo de leitura do sensor, quanto maior, mais preciso(argumento em us, com o mínimo de 20000us)
  //sensor.setMeasurementTimingBudget(200000);

  // Sobe o retorno do sinal (padrão é 0.25 MCPS)
  //sensor.setSignalRateLimit(0.3);

  // Inicia o modo continuo de leitura
  //sensor.startContinuous();
  
  // Lê o sensor 5 vezes
  //for (int i = 0; i < 5; i++) {
    //dist_max = sensor.readRangeContinuousMillimeters();
    //filtrar_maximo();
    //delay(50);
  //}
  
  // Para o modo continuo de leitura
  //sensor.stopContinuous();
  
  // Vref = VCC, ADC2(pin 3 ou PB4) como input e  ajustes a esquerda
  ADMUX = 0b00100010;
  
  // Ativa o ADC, deixa o ADC no modo single conversion e o fator de divisão = 8 para um clock de 125kHz   
  ADCSRA = 0b10000011;  
}

void loop()
{
  // Faz a medição da distância e retorna um valor em milímetros(mm)
  dist = sensor.readRangeSingleMillimeters();

  // Filtra o valor de distancia medido
  filtrar_sinal();

  // Inicia a conversão e guarda o valor na variável
  ADCSRA |= (1 << ADSC);         
  analog = ADCH; 

 //-------------- cm, kg e V --------------//
  
  // Cálculo da carga
  //defor = dist_max - dist;
  //defor = defor / 10;
  //carga = 0.75 * defor;

  //Mapeamento de Tensão
  //tensao = (float)analog * 3.28 / 255;
 
  // Imprime no monitor serial
  BTserial.print(dist);
  BTserial.print(",");
  BTserial.print(analog);
  BTserial.print(";");
  //BTserial.print(defor, 1);
  //BTserial.print(" cm");
  //BTserial.print(",");
  //BTserial.print(carga, 2);
  //BTserial.print(" kg");
  //BTserial.print(",");
  //BTserial.print(tensao, 2);
  //BTserial.print(" V");
  //BTserial.print(";");
              

 //-------------- mm e g --------------//

//    // Cálculo da carga
//  defor = dist_max - dist;
//  carga = 75 * defor;
// 
//  // Imprime no monitor serial
//  BTserial.print(defor);
//  BTserial.print(" mm");
//  BTserial.print(",");
//  BTserial.print(carga);
//  BTserial.print(" g");
//  BTserial.print(";");

  delay(50);
}

// Função para filtrar o valor maximo
//void filtrar_maximo()
//{
//  // Se a distância medida for maior que o máximo e ainda não tiver passado 1 segundo de timeout
//  if ((millis() - timeout) < 1000) {
//    // Descarta a medição feita e iguala ela à anterior
//    dist_max = max_old;
//  }
//  else if (dist_max > 350) {
//    // Descarta a medição feita e iguala ela à anterior
//    dist_max = max_old;
//  }
//  else if (dist_max < 250) {
//    // Descarta a medição feita e iguala ela à anterior
//    dist_max = max_old;
//  }
//  else {
//    // Não descarta a medição atual e atualiza a medição antiga para a atual
//    max_old = dist_max;
//    // Reseta o valor da variável do timeout
//    timeout = millis(); 
//  }
//}

// Função para filtrar o valor medido
void filtrar_sinal()
{
  // Se a distância medida for maior que o máximo e ainda não tiver passado 1 segundo de timeout
  if ((millis() - timeout) < 1000) {
    // Descarta a medição feita e iguala ela à anterior
    dist = dist_old;
  }
  else if (dist > dist_max) {
    // Descarta a medição feita e iguala ela ao máximo
    dist = dist_max;
  } 
  else if (dist < 30) {
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
