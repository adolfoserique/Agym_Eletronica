//Escrito por: Adolfo de Souza Serique, Engenhriro Eletrônico

//--- Bibliotecas ---
#include <avr/io.h>
#include <SoftwareSerial.h>

//--- Pinos no HX711 ---
#define ADD0 PB1 //Data
#define ADSK PB2 //SCK

//--- Pinos para comunicação UART do módulo Bluetooth ---
#define TX_BT PB0   // Pino 2 como TX
#define RX_BT PB3   // Pino 5 como RX

//--- Protótipos das funções ---
unsigned long read_count(); //Confersor A/D do HX711
void power_down(); //Modo de low power do HX711
void power_up(); //Acordar HX711 do mode de low power

//--- Instância da função ---
SoftwareSerial BTserial(RX_BT, TX_BT);

//--- Variáveis ---
float peso = 0; //Sáida de dados do HX711
float SCALE = 1; //Escala para calibração
float OFFSET = 0; //Offeset para zerar a medida
int analog = 0; //Nivél da bateria


// ------ Configurações iniciais ------
void setup(){
  
  // --- Configuração dos pinos ---
  DDRB |= (0 << ADD0); //Define ADD0 como INPUT de dados
  PORTB |= (1 << ADD0); //Define um resistor de pullup no pino de entrada
  DDRB |= (1 << ADSK); //Define ADSK com saída de clock

  // --- Iniciar cominicação serial ---
  BTserial.begin(9600);

  // --- Configuração do conversor A/D do Attiny para leitura de tensão da bateria ---  
  ADMUX = 0b00100010; // Vref = VCC, ADC2(pino 3 ou PB4) como input e ajustes a esquerda
  ADCSRA = 0b10000011; // Ativa o ADC, deixa o ADC no modo single conversion e o fator de divisão = 8 para um clock de 125kHz
  
}


// ------ Loop infinito ------
void loop(){
  
  peso = read_count(); //Lê os dados HX711
  
  ADCSRA |= (1 << ADSC); // Inicia a conversão e guarda o valor na variável    
  analog = ADCH; //Guarda o valor lido sobre a tensão da bateria

  // --- Dados para enviar ---
  BTserial.print(peso);
  BTserial.print(",");
  BTserial.print(analog);
  BTserial.println(";");

  power_down();
  delay(500);
  power_up();
  
}


// ------ Funções ------
unsigned long read_count(){
  
  unsigned long Count = 0;
  unsigned char i;
  
  PORTB &= ~(1 << ADSK); //Pino ADSK como LOW
  
  while(PINB & (1 << ADD0)); //Lê o pino ADD0

  for(i=0;i<24;i++){
    PORTB |= (1 << ADSK); //Pino ADSK como HIGH
    Count = Count << 1;
    PORTB &= ~(1 << ADSK);
    if(PINB & (1 << ADD0)) Count++;
    delay(10);
  }

  PORTB |= (1 << ADSK);
  Count = Count^0x800000;
  PORTB &= ~(1 << ADSK);

  return(Count);
  
}


void power_down() {
  
  PORTB &= ~(1 << ADSK);
  delayMicroseconds(61); //O pino em HIGH por pelo menos 60us, abilita o modo de low power do HX711
  PORTB |= (1 << ADSK);
  
}


void power_up() {
  
  PORTB &= ~(1 << ADSK);
  
}
