/* 2014/10/07 Versão apenas com os botões ZeroMetro e DozeMetro funcionando */
/* 2015/01/10 Botões de distância sem precisar retornar a zero. Na inicialização obrigatoriamente ele irá ao zero */

#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
#include "Wire.h"

//Definiçao necessarias
LiquidCrystal_I2C lcd(0x27,16,2);
#define DS1307_ADDRESS 0x68
#define PARA_FRENTE 0
#define PARA_TRAS   1
byte zero = 0x00; 

//Portas Utilizadas pelos Botões
const int ZeroMetro  = 2;
const int CincoMetro = 3;
const int SeteMetro  = 4;
const int DozeMetro  = 5;
const int SomaMetro  = 7;
const int SubMetro   = 6;

//Portas Utilizadas pelo Motor
const int MotorDoze  = 8;  // anda em direção ao 12
const int MotorZero =  9;  // anda em direção ao zero

//Portas Utilizadas pelos Sensores
const int SnsFimdeCurso  = 10;
const int SnsdePulso     = 11;

//Portas Disponiveis 12 e 13

//Variáveis Globais
unsigned int distancia_global = 0;

void setup()
{
  //Iniciando o LCD e o RTC
  Wire.begin();
  lcd.init();
  lcd.backlight(); 

  //Definindo portas de entradas
  pinMode(ZeroMetro,INPUT);
  pinMode(CincoMetro,INPUT);
  pinMode(SeteMetro,INPUT); 
  pinMode(DozeMetro,INPUT); 
  pinMode(SomaMetro,INPUT); 
  pinMode(SubMetro,INPUT); 
  pinMode(SnsFimdeCurso,INPUT);  
  pinMode(SnsdePulso,INPUT);

  //Definindo portas de saida
  pinMode(MotorDoze,OUTPUT); 
  pinMode(MotorZero,OUTPUT);
  
  // Ao iniciar move para zero
  move_para_zero(); // move para zero
}

void loop()
{
  Mostrarelogio();
  Metros();
  Botao();
  delay(200);
}

byte ConverteParaBCD(byte val){ //Converte o número de decimal para BCD
  return ( (val/10*16) + (val%10) );
}

byte ConverteparaDecimal(byte val)  { //Converte de BCD para decimal
  return ( (val/16*10) + (val%16) );
}

void Mostrarelogio()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  int segundos = ConverteparaDecimal(Wire.read());
  int minutos = ConverteparaDecimal(Wire.read());
  int horas = ConverteparaDecimal(Wire.read() & 0b111111); 
  int diadasemana = ConverteparaDecimal(Wire.read()); 
  int diadomes = ConverteparaDecimal(Wire.read());
  int mes = ConverteparaDecimal(Wire.read());
  int ano = ConverteparaDecimal(Wire.read());

  //Hora: xx:xx:xx  

  //Mostra a data no Serial Monitor
  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.setCursor(6, 0);
  if(horas<10)
  {      
    lcd.print("0");
    lcd.setCursor(7, 0);
  }
  lcd.print(horas);
  lcd.setCursor(8, 0);
  lcd.print(":");
  lcd.setCursor(9, 0);
  if(minutos<10)
  {      
    lcd.print("0");
    lcd.setCursor(10, 0);
  }
  lcd.print(minutos);
  lcd.setCursor(11, 0);
  lcd.print(":");
  lcd.setCursor(12, 0);
  if(segundos<10)
  {      
    lcd.print("0");
    lcd.setCursor(13, 0);
  }
  lcd.print(segundos);
  lcd.setCursor(14, 0);
  lcd.print("  ");
}

void Metros()
{
  //Dist.: xxx mts   
  // Exibe valor da distancia_global   
  lcd.setCursor(0,1);
  lcd.print("Dist.: ");
  lcd.setCursor(7,1);
  if (distancia_global < 100)
  {
      lcd.print(" ");
      lcd.setCursor(8,1);
  }
  if (distancia_global < 10)
  {
      lcd.print(" ");
      lcd.setCursor(9,1);
  }
  lcd.print(distancia_global);
  lcd.setCursor(10,1);
  lcd.print(" mts         ");
}

void Botao()
{
  boolean estado;
  boolean estadoLed = 0;

  // Botão ZeroMetro
  estado = digitalRead(ZeroMetro); //estado recebe o valor da corrente que passa pelo botão  
  if(estado == HIGH) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
    move_para_zero(); // move para zero
  else     
  {
    // Botão CincoMetro
    estado = digitalRead(CincoMetro); //estado recebe o valor da corrente que passa pelo botão  
    if(estado == HIGH) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
      move_distancia_fixa(5);  // envia quantidade de pulsos correspondentes e lê a distância
    else     
    {
      // Botão SeteMetro
      estado = digitalRead(SeteMetro); //estado recebe o valor da corrente que passa pelo botão  
      if(estado == HIGH) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
         move_distancia_fixa(7);  // envia quantidade de pulsos correspondentes e lê a distância
      else     
      {
        // Botão DozeMetro
        estado = digitalRead(DozeMetro); //estado recebe o valor da corrente que passa pelo botão  
        if(estado == HIGH) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
          move_distancia_fixa(12);  // envia quantidade de pulsos correspondentes e lê a distância
        else     
        {
         //Botão SomaMetro
          estado = digitalRead(SomaMetro); //estado recebe o valor da corrente que passa pelo botão  
          if((estado == HIGH)&& (distancia_global <12)) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
             move_distancia_variavel(1);  // envia quantidade de pulsos correspondentes e lê a distância
          else     
          {
            // Botão SubMetro
            estado = digitalRead(SubMetro); //estado recebe o valor da corrente que passa pelo botão  
            if(estado == HIGH) //IF SIGNIFICA SE, então se(estado==HIGH) , ele executará o que está dentro dos cochetes
               move_distancia_variavel(-1);  // envia quantidade de pulsos correspondentes e lê a distância
          }
        }
      }
    }
  }
}

void move_para_zero()
{
  boolean estado;

  digitalWrite(MotorZero, HIGH);  // Ativa motor para trás
  
  do
  { 
    estado = digitalRead(SnsFimdeCurso); //estado recebe o valor da corrente que passa pelo botão
    Mostrarelogio();
  } while (estado == HIGH);

  digitalWrite(MotorZero, LOW);  // Desativa motor para trás

  distancia_global = 0;
  Metros();
}

void move_distancia_fixa(unsigned int dist)
{
  // sempre começa em zero
  //move_para_zero(); // move para zero
  //delay(500);  // para dar tempo do sinal do fim de curso resetar
  
  // Verifica a relação entre distancia_global e a dist  
  if (distancia_global < dist)
  {
    // Obs: cada metro de distância equivale a 10 pulsos
    digitalWrite(MotorDoze, HIGH);  // Ativa motor para frente
    le_infravermelho( 2 * (dist - distancia_global), PARA_FRENTE ); // verifica pulsos do sensor infravermelho
    digitalWrite(MotorDoze, LOW);  // Desativa motor para frente
  }
  else if (distancia_global > dist)
  {
    digitalWrite(MotorZero, HIGH);  // Ativa motor para trás
    le_infravermelho( 2 * (distancia_global - dist), PARA_TRAS ); // verifica pulsos do sensor infravermelho
    digitalWrite(MotorZero, LOW);  // Desativa motor para trás
  }
}

void move_distancia_variavel(int dist)  // move de 1 em 1 metro
{
  // Obs: cada metro de distância equivale a 10 pulsos

  if (dist >= 0) 
  {
    digitalWrite(MotorDoze, HIGH);  // Ativa motor para frente
    le_infravermelho(2, PARA_FRENTE);      // verifica pulsos do sensor infravermelho
  }  
  else
  {
    digitalWrite(MotorZero, HIGH);  // Ativa motor para trás
    le_infravermelho(2, PARA_TRAS);      // verifica pulsos do sensor infravermelho
  }  
  
  digitalWrite(MotorDoze, LOW);  // Desativa motor para frente
  digitalWrite(MotorZero, LOW);  // Desativa motor para trás
}

void le_infravermelho(int total_pulsos, boolean direcao)
{
  // Obs: cada metro de distância equivale a 2 pulsos
  int conta_pulsos = 0;
  boolean estado_fim_curso;
  boolean estado;
  boolean estado_aux;

  // verifica sensor infravermelho por pulsos e incrementa conta_pulsos    
  estado = digitalRead(SnsdePulso); //estado recebe o valor do sensor infravermelho    
  
  do
  {
    estado_aux = digitalRead(SnsdePulso);
    
    if (estado != estado_aux && estado_aux == HIGH)
    {
      ++conta_pulsos;
      if (conta_pulsos%2 == 0)
      {
        if (direcao == PARA_FRENTE)
          ++distancia_global;
        else
          --distancia_global;
      }
    }
      
    estado = estado_aux;    
    
    Mostrarelogio();
    Metros();
  } while (conta_pulsos < total_pulsos);
}




