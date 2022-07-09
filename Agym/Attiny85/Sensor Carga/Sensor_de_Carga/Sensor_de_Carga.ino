//Escrito por: Adolfo de Souza Serique, Engenhriro Eletrônico

//--- Bibliotecas ---
#include <avr/io.h>
#include <SoftwareSerial.h>

//--- Pinos no HX711 ---
#define ADD0 PB1 //Data
#define ADSK PB2 //SCK

//--- Pinos para comunicação UART do módulo Bluetooth ---
#define TX_BT PB0   // Pino como TX
#define RX_BT PB3   // Pino como RX

//--- Protótipos das funções ---
unsigned long read_count(); //Confersor A/D do HX711
unsigned long read_average(byte); //Média de leituras
void set_offset(float); //Define o offset
void tare(byte); //Zerar leitura
void power_down(); //Modo de low power do HX711
void power_up(); //Acordar HX711 do mode de low power
float get_value(byte); //Leitura menos offset
float get_units(byte); //Leitura dividida pela escala
void set_scale(float); //Define a escala
float get_scale(); //Recupera o valor da escala
float get_offset(); //Recupera o valor de offset
void calibration(); //Calibra o sensor com um peso conhecido
void eeprom_write(unsigned short, unsigned char); //Escreve o dado na EEPROM
unsigned char eeprom_read(unsigned char); //Lê o dado da EEPROM

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

  // --- Leitura e gravação da EEPROM ---
  //eeprom_write(0x0A, get_scale()); //Mémoria máxima = 511 e dado máximo = 255 (por registrador)
  //eeprom_write(0xFF, get_offset());
  //eeprom_read(0x0A);
  //eeprom_read(0xFF);

  calibration(); //Realiza a calibração do sensor utilizando um peso conhecido
  
}


// ------ Loop infinito ------
void loop(){
  
  peso = get_units(2); //Lê os dados HX711
  
  ADCSRA |= (1 << ADSC); // Inicia a conversão e guarda o valor na variável    
  analog = ADCH; //Guarda o valor lido sobre a tensão da bateria

  // --- Dados para enviar ---
  BTserial.print(peso);
  BTserial.print(",");
  BTserial.print(analog);
  BTserial.println(";");
  
  delay(500);
  
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

unsigned long read_average(byte times) {
  
  unsigned long sum = 0;
  for (byte i = 0; i < times; i++) {
    sum += read_count();
    delay(10);
  }

  sum = sum / times;
  return sum;
  
}

void set_offset(float offset) {
  
  OFFSET = offset;
  
}

void tare(byte times) {
  
 unsigned long sum = read_average(times);
  set_offset(sum);
  
}

void power_down() {
  
  PORTB &= ~(1 << ADSK);
  delayMicroseconds(60); //O pino em HIGH por pelo menos 60us, abilita o modo de low power do HX711
  PORTB |= (1 << ADSK);
  
}

void power_up() {
  
  PORTB &= ~(1 << ADSK);
  
}

float get_value(byte times) {
  
  unsigned long sum = read_average(times);
  float sum_f = (float)sum;
  sum_f = sum_f - OFFSET;
  delay(10);

  return sum_f;
  
}

float get_units(byte times) {
  
  float sum = get_value(times) / SCALE;
  delay(10);
  
  return sum;
  
}

void set_scale(float scale) {
  
  SCALE = scale;
  
}

float get_scale() {
  
  return SCALE;
  
}

float get_offset() {
  
  return OFFSET;
  
}


void calibration(){

  float weight = 0;
  float new_scale = 0;
  set_scale(1);
  set_offset(0);
  tare(10);
  BTserial.println("Iniciando calibração...");
  tare(10);
  delay(2000);
  tare(10);
  BTserial.println("Tire todo o peso do sensor");
  delay(2000);
  tare(10);
  BTserial.println("Coloque o peso conhecido e digite seu peso em gramas(ex: 100.0):");
  
  while(BTserial.available() == 0) { // apenas sai do loop quando dados são recebidos
    
  }
  
  weight = BTserial.parseFloat();
  float sum = get_units(30);
  new_scale = sum / weight;
  delay(10);
  set_scale(new_scale);
  delay(1000);
  BTserial.println("Calibração finalizada!");
}

void eeprom_write(unsigned short address, unsigned char data){
  
  while(EECR & (1<<EEPE)); //Espera até uma escrita anterior ser completada
  
  EECR = (0<<EEPM1) | (0<<EEPM0); //Modo de programação(ver datasheet para saber as configurações)

   if (address < 512){ //Verifica o endereço (não pode ser maior de 511)
      EEAR = address;
   }
   else{
      EEAR = 511;
   }

   EEDR = data; //Registrador de dados da EEPROM

   EECR |= (1<<EEMPE); //Abilita a gravação
   EECR |= (1<<EEPE); //Começa a gravação (o bit vai a 0 automaticamente após a gravação)
   
}

unsigned char eeprom_read(unsigned char address){
  
  while(EECR & (1<<EEPE));
  
  EEAR = address; //Define o endereço
  
  EECR |= (1<<EERE); //Começa a leitura da EEPROM
  return(EEDR); //Retorna o dado
  
}
