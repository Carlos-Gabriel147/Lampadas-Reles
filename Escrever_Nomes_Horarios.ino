#include <EEPROM.h>

void escrever_nome(String nome, int posicao_ini);
void escrever_horario(int h, int m, int posicao_ini);
int ler_horario(int posicao_ini);
String ler_nome(int posicao_ini);

void setup() {
  EEPROM.begin(4096);
  Serial.begin(57600);

  //Lâmpada 1
  escrever_horario(21, 42, 256); //Ligar
  escrever_horario(21, 44, 258); //Desligar
  escrever_nome("Lâmpada_1", 288); //Nome

  //Lâmpada 2
  escrever_horario(21, 42, 260); //Ligar
  escrever_horario(21, 44, 262); //Desligar
  escrever_nome("Lâmpada_2", 309); //Nome

  //Lâmpada 3
  escrever_horario(21, 42, 264); //Ligar
  escrever_horario(21, 44, 266); //Desligar
  escrever_nome("Lâmpada_3", 330); //Nome

  //Lâmpada 4
  escrever_horario(21, 42, 268); //Ligar
  escrever_horario(21, 44, 270); //Desligar
  escrever_nome("Lâmpada_4", 351); //Nome

  //Lâmpada 5
  escrever_horario(21, 42, 272); //Ligar
  escrever_horario(21, 44, 274); //Desligar
  escrever_nome("Lâmpada_5", 372); //Nome

  //Lâmpada 6
  escrever_horario(21, 42, 276); //Ligar
  escrever_horario(21, 44, 278); //Desligar
  escrever_nome("Lâmpada_6", 393); //Nome

  //Lâmpada 7
  escrever_horario(21, 42, 280); //Ligar
  escrever_horario(21, 44, 282); //Desligar
  escrever_nome("Lâmpada_7", 414); //Nome

  //Lâmpada 8
  escrever_horario(21, 42, 284); //Ligar
  escrever_horario(21, 44, 286); //Desligar
  escrever_nome("Lâmpada_8", 435); //Nome
}

void loop() {
  int i = 0;
  int lamp = 0;
  for(i=0; i<=30; i += 4){
    Serial.print("Lâmpada ");
    Serial.println(lamp+1);
    Serial.print(ler_horario(i+256));
    Serial.print(':');
    Serial.println(ler_horario(i+257));
    Serial.print(ler_horario(i+258));
    Serial.print(':');
    Serial.println(ler_horario(i+259));
    Serial.println();
    lamp += 1;
    }
  Serial.println("Nomes Salvos:");
  Serial.println(ler_nome(288));
  Serial.println(ler_nome(309));
  Serial.println(ler_nome(330));
  Serial.println(ler_nome(351));
  Serial.println(ler_nome(372));
  Serial.println(ler_nome(393));
  Serial.println(ler_nome(414));
  Serial.println(ler_nome(435));
  delay(5000);
}

void escrever_nome(String nome, int posicao_ini){
  char tam = nome.length();
  char i;
  for(i=0;i<tam;i++){
    EEPROM.write(posicao_ini+i , nome[i]);
  }
  EEPROM.write(posicao_ini+tam,'\0');
  EEPROM.commit();
}

void escrever_horario(int h, int m, int posicao_ini){
    EEPROM.write(posicao_ini, h);
    EEPROM.write(posicao_ini + 1, m);
    EEPROM.commit();
}

String ler_nome(int posicao_ini){
  char nome[21];
  unsigned char c;
  char i = 0;
  while(c != '\0' && i < 21){
    c = EEPROM.read(posicao_ini+i);
    nome[i] = c;
    i++;
  }
  nome[i] = '\0';

  return String(nome);
}

int ler_horario(int posicao_ini){
   return EEPROM.read(posicao_ini);
}
