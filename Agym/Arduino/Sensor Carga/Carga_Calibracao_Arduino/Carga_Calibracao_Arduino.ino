/* 
Tutorial: Calibração de uma Célula de Carga 
Autor: Curto Circuito 
Descrição: Programa para calibrar célula com o uso de um peso conhecido. */
 
#include "HX711.h" 

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 balanca;                                                                        

float calibration_factor = 48011.00;  // Colocar o fator de carga encontrado na calibração                                                      
float peso;                                                                                 

void setup() {                                                                              
  Serial.begin(9600);
  Serial.println("Iniciando a balanca");
  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  balanca.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);                                                                      
  Serial.println("Remova todos os pesos da balança");                                       
  delay(1000);                                                                              
  Serial.println("Após estabilização das leituras, coloque o peso conhecido na balança");   
  delay(1000); 
  Serial.println("Pressione + para incrementar o fator de calibração");                     
  Serial.println("Pressione - para decrementar o fator de calibração");                     
  delay(1000);                                                                                                                                                     
  balanca.set_scale();                                                                      
  balanca.tare();                                                                           

  long zero_factor = balanca.read_average();                                                
}

void loop() {                                                                               

  balanca.set_scale(calibration_factor);                                                    

  Serial.print("Peso: ");                                                                   
  peso = balanca.get_units(10); // Lê o peso                                                          
  if (peso < 0)                                                                             
  {
    peso = 0.00;                                                                            
  }                                                  
  Serial.print(peso);                                                                       
  Serial.print(" kg");                                                                      
  Serial.print(" Fator de calibração: ");                                                   
  Serial.print(calibration_factor);                                                         
  Serial.println();                                                                         
  delay(500);                                                                               

  if(Serial.available())                                                                    
  {
    char temp = Serial.read();
    if(temp == '+')                                                                         
      calibration_factor += 1;                                                              
    else if(temp == '-')                                                                    
      calibration_factor -= 1;                                                              
  }
}
