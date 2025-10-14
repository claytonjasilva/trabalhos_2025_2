// Integrantes: 
// - Bruno Norton 202503452041 TA
// - João Arthur 202307164501 TA
// - Lucas Alcântara 202501001629 TA
// - Micael Dali 202501000975 TA
// - Roger Pires 202501001556 TA
// - Vinícios Machado 202503006725 TA

// --- Incluidndo biblioteca do keypad ---
#include <Keypad.h>

// --- Definindo entradas ---
const int pinoJoystickX = A0;
const int pinoJoystickY = A1;
const int pinoJoystickSW = 12;

const int pinoTrig = 11;
const int pinoEcho = 10;

const int pinoSensorIR = 13;

// --- Configurações do funcionamento do joystick ---
const int centroJoystick = 512;
const int limiarJoystick = 400;

// --- Keypad ---
const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinosLinhas[LINHAS] = {9, 8, 7, 6};
byte pinosColunas[COLUNAS] = {5, 4, 3, 2};
Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// --- Variáveis de controle de modo ---
enum EstadoSistema { AGUARDANDO_MODO, MODO_SELECIONADO, OPERANDO };
EstadoSistema estadoAtual = AGUARDANDO_MODO;
enum ModoOperacao { NENHUM, CONTINUO, SOB_DEMANDA, TESTE };
ModoOperacao modoAtual = NENHUM;

// --- Variáveis de controle de tempo (sem delay) ---
unsigned long previousMillis = 0;
const long intervaloTeste = 3000;
int etapaTeste = 0;

// --- Outras variáveis globais ---
int distanciaCritica = 20;
String bufferEntradaNumerica = ""; // Para a configuração inicial
String bufferDemanda = "";         // Buffer exclusivo para o modo sob demanda
bool instrucaoDemandaImpressa = false; // Controla a exibição da instrução

// --- Variáveis de memória ---
float ultimaDistancia = -1.0;
int ultimoEstadoIR = -1;

// --- Protótipos das funções ---
void leJoystick();
void leTeclado(char tecla);
void verificaCliqueJoystick();
void confirmarModo();
void gerenciaModoContinuo();
void gerenciaModoSobDemanda(char tecla);
void gerenciaModoTeste();
void voltarAoMenu();

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  pinMode(pinoJoystickSW, INPUT_PULLUP);
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoSensorIR, INPUT);
  
  Serial.println("\n===============================");
  Serial.println("Sistema de Monitoramento iniciado");
  Serial.println("===============================");
  
  voltarAoMenu();
}

// --- LOOP ---
void loop() {
  char teclaPressionada = teclado.getKey();

  if (teclaPressionada == '*') {
    voltarAoMenu();
    return;
  }

  switch (estadoAtual) {
    case AGUARDANDO_MODO:
      leJoystick();
      break;
    case MODO_SELECIONADO:
      leTeclado(teclaPressionada);
      verificaCliqueJoystick();
      break;
    case OPERANDO:
      switch (modoAtual) {
        case CONTINUO:      gerenciaModoContinuo(); break;
        case SOB_DEMANDA:   gerenciaModoSobDemanda(teclaPressionada); break;
        case TESTE:         gerenciaModoTeste(); break;
        case NENHUM:        break;
      }
      break;
  }
}

// --- Implementação das funções ---

void voltarAoMenu() {
  Serial.println("\n--- MENU PRINCIPAL ---");
  Serial.println("Use o joystick para escolher um modo. Pressione '*' a qualquer momento para voltar aqui.");
  Serial.println("Esquerda para modo contínuo, Direita para modo sob demanda, e Para cima para modo teste.");
  estadoAtual = AGUARDANDO_MODO;
  modoAtual = NENHUM;
}

void leJoystick() {
  int valorX = analogRead(pinoJoystickX);
  int valorY = analogRead(pinoJoystickY);

  if (valorX < (centroJoystick - limiarJoystick)) {
    modoAtual = CONTINUO;
    Serial.println("\nModo pré-selecionado: Monitoramento Contínuo. Confirme com '#' ou clique.");
    estadoAtual = MODO_SELECIONADO;
    delay(500);
  } else if (valorX > (centroJoystick + limiarJoystick)) {
    modoAtual = SOB_DEMANDA;
    Serial.println("\nModo pré-selecionado: Monitoramento Sob Demanda. Confirme com '#' ou clique.");
    estadoAtual = MODO_SELECIONADO;
    delay(500);
  } else if (valorY < (centroJoystick - limiarJoystick)) {
     modoAtual = TESTE;
     Serial.println("\nModo pré-selecionado: Teste de Sensores. Confirme com '#' ou clique.");
     estadoAtual = MODO_SELECIONADO;
     delay(500);
  }
}

void leTeclado(char tecla) {
  if (tecla) {
    Serial.println("\nTecla pressionada: " + String(tecla));
    if (isDigit(tecla) && modoAtual != TESTE) {
      bufferEntradaNumerica += tecla;
      Serial.println("Digite a distância crítica (cm): " + bufferEntradaNumerica);
    } 
    else if (tecla == '#') {
      confirmarModo();
    }
  }
}

void verificaCliqueJoystick() {
  if (digitalRead(pinoJoystickSW) == LOW) {
    Serial.println("\nClique do Joystick detectado. Confirmando...");
    confirmarModo();
    delay(500);
  }
}

void confirmarModo() {
  if (bufferEntradaNumerica.length() > 0) {
    distanciaCritica = bufferEntradaNumerica.toInt();
    Serial.println("Parâmetro padrão atualizado: distância crítica = " + String(distanciaCritica) + " cm.");
    bufferEntradaNumerica = "";
  }
  
  String modoTexto = "";
  if(modoAtual == CONTINUO) modoTexto = "Contínuo";
  if(modoAtual == SOB_DEMANDA) modoTexto = "Sob Demanda";
  if(modoAtual == TESTE) modoTexto = "Teste de Sensores";
  
  Serial.println("--- MODO " + modoTexto + " INICIADO ---");
  
  estadoAtual = OPERANDO;
  previousMillis = 0;
  etapaTeste = 0;
  ultimaDistancia = -1.0;
  ultimoEstadoIR = -1;
  instrucaoDemandaImpressa = false; // Reseta a instrução para o modo sob demanda
  bufferDemanda = "";               // Limpa o buffer do modo sob demanda
}

void gerenciaModoContinuo() {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH);
  float distanciaAtual = duracao * 0.034 / 2.0;
  int estadoIRAtual = digitalRead(pinoSensorIR);

  bool mudouDistancia = abs(distanciaAtual - ultimaDistancia) > 0.5;
  bool mudouIR = estadoIRAtual != ultimoEstadoIR;

  if (mudouDistancia || mudouIR) {
    String irTexto = (estadoIRAtual == LOW) ? "Detectado!" : "Livre";
    String output = "Status -> Dist: " + String(distanciaAtual) + " cm | IR: " + irTexto;
    Serial.println(output);

    if ((distanciaAtual < distanciaCritica && distanciaAtual > 0) || estadoIRAtual == LOW) {
      Serial.println(">>> ALERTA: Condição crítica padrão detectada!");
    }
    
    ultimaDistancia = distanciaAtual;
    ultimoEstadoIR = estadoIRAtual;
  }
}

void gerenciaModoSobDemanda(char tecla) {
  // Mostra a instrução apenas uma vez ou após cada leitura
  if (!instrucaoDemandaImpressa) {
    Serial.println("\nDigite a distância máxima para a leitura e pressione 'A' para medir.");
    Serial.print("Distância: ");
    instrucaoDemandaImpressa = true;
  }

  // Se a tecla for um dígito, adiciona ao buffer específico deste modo
  if (isDigit(tecla)) {
    bufferDemanda += tecla;
    Serial.print(tecla); // Ecoa o dígito no monitor
  }
  
  // Se a tecla de medição for pressionada
  if(tecla == 'A') {
    Serial.println("\n\nMedindo...");
    int limiteDistanciaDemanda = bufferDemanda.toInt();
    if (bufferDemanda.length() == 0) {
      limiteDistanciaDemanda = distanciaCritica; // Usa o padrão se nada for digitado
      Serial.println("Nenhum limite digitado. Usando o padrão: " + String(limiteDistanciaDemanda) + " cm.");
    }

    // Realiza a leitura do sensor
    digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
    digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
    digitalWrite(pinoTrig, LOW);
    long duracao = pulseIn(pinoEcho, HIGH);
    float distanciaAtual = duracao * 0.034 / 2.0;

    // Mostra o resultado
    Serial.println("Distância atual do obstáculo: " + String(distanciaAtual) + " cm.");
    
    // Acusa se há um obstáculo dentro do limite definido
    if (distanciaAtual < limiteDistanciaDemanda && distanciaAtual > 0) {
      Serial.println(">>> RESULTADO: Obstáculo detectado dentro do limite de " + String(limiteDistanciaDemanda) + " cm!");
    } else {
      Serial.println(">>> RESULTADO: Caminho livre no limite de " + String(limiteDistanciaDemanda) + " cm.");
    }
    Serial.println("------------------------------------");

    // Prepara para a próxima leitura
    bufferDemanda = "";
    instrucaoDemandaImpressa = false;
  }
}

void gerenciaModoTeste() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervaloTeste) {
    previousMillis = currentMillis;
    
    if (etapaTeste == 0) {
      digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
      digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
      digitalWrite(pinoTrig, LOW);
      long duracao = pulseIn(pinoEcho, HIGH);
      float distancia = duracao * 0.034 / 2.0;
      Serial.println("Sensor de distância OK – leitura atual: " + String(distancia) + " cm");
      etapaTeste = 1;
    } else {
      int statusIR = digitalRead(pinoSensorIR);
      String irTexto = (statusIR == LOW) ? "Obstáculo Presente" : "Livre";
      Serial.println("Sensor de obstáculos IR OK – status: " + irTexto);
      Serial.println("------------------------------------");
      etapaTeste =0;
}
}
}