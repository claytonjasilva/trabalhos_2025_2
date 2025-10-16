// 202501441581 Carlos Vinicius Oiticica Jund TA
// 202501511031 Heitor Gonçalves Lima TA
// 202502579519 Pietro Baldo Albuquerque TA
// 202501001254 Gianluca Leonardi TA
// 202503545189 Rafael Tomaz TA
// 202407321976 Vinícius Marinho Queiroz TA

#include <Keypad.h> //Usado com teclado membrana
const int TRIG_SOM = 10;
const int ECHO_SOM = 11;
int modo = 0;


// Mapeamento das teclas (teclado membrana)
const byte LINHAS = 4;
const byte COLUNAS = 4;
char TECLAS_MATRIZ[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Define os pinos do Arduino conectados às linhas e colunas do teclado
const byte PINOS_LINHAS[LINHAS] = {9, 8, 7, 6}; // Pinos 9, 8, 7, 6 do Arduino
const byte PINOS_COLUNAS[COLUNAS] = {5, 4, 3, 2}; // Pinos 5, 4, 3, 2 do Arduino
// Cria o objeto do teclado
Keypad teclado_personalizado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS);

int x;
int y;

int pulso;
int distancia;
int agua;
char leitura_teclas;
long limite_agua;
long limite_distancia;
bool setagua = false;
bool setdistancia = false;

void setup() {
  Serial.begin(9600); // Inicia a comunicação serial
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(TRIG_SOM,OUTPUT);
  pinMode(ECHO_SOM,INPUT);
  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
  delay(1000);
}
void lerultrasom() {
  digitalWrite(TRIG_SOM,HIGH);
  delay(10);
  digitalWrite(TRIG_SOM,LOW);
  pulso = pulseIn(ECHO_SOM,HIGH);
  distancia = (pulso*0.034)/2; //Calcula distância
}
void leragua() {
  agua = analogRead(A2);
}
void alerta(){
  if (agua > limite_agua){
    Serial.println("Alerta! Água passou do limite.");
  }
  if (distancia < limite_distancia) {
    Serial.println("Alerta! distância inferior ao limite");
  }
}

void teste(){
  leragua();
  lerultrasom();
  Serial.print("água       : ");
  if(agua > 0 && agua < 1024){//Sensor não lê além de 1024 e apenas lê 0 quando desligado
    Serial.print("sensor funcional | valor lido: ");
    Serial.println(agua);
  } else{
    Serial.println("sensor não ok");
  }
  Serial.print("ultrasônico: ");
  if (distancia > 0) {//Nunca lê 0
    Serial.print("sensor funcional | valor lido: ");
    Serial.println(distancia);
  }else{
    Serial.println("sensor não ok");
  }
  delay(500);

}
void sob_demanda(){
  if(leitura_teclas == '*' ) {
    leragua();
    lerultrasom();
    Serial.print("valor água:");
    Serial.println(agua);
    Serial.print("valor distância:");
    Serial.println(distancia);
    alerta();
  }
}
void monitoramento_continuo(){
  leragua();
  lerultrasom();
  Serial.print("agua: ");
  Serial.println(agua);
  Serial.print("Distancia(cm): ");
  Serial.println(distancia);
  delay(200);
}

void loop() {

  leitura_teclas = teclado_personalizado.getKey(); 

  if (leitura_teclas) { // Lê teclas do teclado de membrana
    Serial.println(leitura_teclas);
  }
  x = analogRead(A0);
  y = analogRead(A1);

  if(modo == 0){//Determina o modo com o joystick
    if (y < 400){
      Serial.print("Pressione # para entrar no modo de teste");
      if(leitura_teclas == '#') {
        Serial.println("modo de teste inicializado");
        Serial.println("Selecione um limite para a água máxima.");
        modo = 1;
      }
    } else if (x > 600) {
        Serial.print("Pressione # para entrar no modo de monitoramento sob demanda");
        if(leitura_teclas == '#') {
          Serial.println("modo de monitoramento sob demanda");
          Serial.println("Selecione um limite para a água máxima.");
          modo = 2;
        }  
      } else if (x < 400) {
          Serial.print("Pressione # para entrar no modo de monitoramento contínuo");
          if(leitura_teclas == '#') {
            Serial.println("modo de monitoramento contínuo");
            Serial.println("Selecione um limite para a água máxima.");
            modo = 3;
          }
      } else {
        Serial.print("nenhum modo selecionado");
      } 
        Serial.print(" x: ");
        Serial.print(x);
        Serial.print(" y: ");
        Serial.println(y);//mostrar posição do joystick
  } else {
  if(!setagua){//Lê primeiro limite
    if (isdigit(leitura_teclas)) {
      limite_agua*= 10;
      limite_agua += leitura_teclas - '0';
      Serial.print("-> ");
      Serial.println(limite_agua);
    } else if(leitura_teclas == '#'){
      Serial.print("Valor selecionado para a agua: ");
      Serial.println(limite_agua);
      Serial.println("Selecione um limite para a distância mínima.");
      setagua=true;
    }
  }else if(!setdistancia) {//Lê segundo limite
    if (isdigit(leitura_teclas)) {
      limite_distancia*= 10;
      limite_distancia += leitura_teclas - '0';
      Serial.print("-> ");
      Serial.println(limite_distancia);
    } else if(leitura_teclas == '#'){
      Serial.print("Valor selecionado para a distancia: ");
      Serial.println(limite_distancia);
      if (modo == 2){Serial.println("Pressione * para receber os valores.");}
      setdistancia=true;
    }
  } else {//loop função
    if(modo == 1){
      teste();
    alerta();
    }else if(modo == 2) {
      sob_demanda();
    }else if(modo == 3) {
      monitoramento_continuo();
    alerta();
    }
  }
  }
}
