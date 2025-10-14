/*  
INTEGRANTES DO GRUPO GOAT:

Bernardo Brandão - 202501000339 - TA
João Gabriel de Oliveira - 202401667145 - TA
João Gabriel Teodósio - 202502876378 - TA
Letícia Valladão - 202501001637 - TA

*/

#include <Keypad.h> // Biblioteca teclado
#include <Ultrasonic.h> // Biblioteca sensor ultrassônico

#define pino_out 8 // sensor de obstáculo (infravermelho)

// Pinos do joystick
const int xPin = A0; // Vertical (cima/baixo)
const int yPin = A1; // Horizontal (esquerda/direita)

// Teclado matricial
const byte LINHAS = 4;
const byte COLUNAS = 4;
const char TECLAS_MATRIZ[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const byte PINOS_LINHAS[LINHAS] = {30, 32, 34, 36};
const byte PINOS_COLUNAS[COLUNAS] = {38, 40, 42, 44};
Keypad teclado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS);

// Sensor ultrassônico
Ultrasonic ultrassom(4, 5);
long distancia = 0;

// Modos
enum Modo { MODO_IDLE, MODO_CONTINUO, MODO_SOB_DEMANDA, MODO_TESTE };
Modo modoAtual = MODO_IDLE;
Modo ultimoProposto = MODO_IDLE;

// Timing
unsigned long ultimaLeituraMillis = 0;
const unsigned long intervaloContinuo = 1000;

// Parâmetros
int limiteDistancia = 50; // padrão, caso o usuário não queira trocar
bool emModoEntradaParametro = false;
String bufferParametro = "";

// Joystick thresholds
const int JOY_LOW = 100;
const int JOY_HIGH = 900;


// Evitar spam do joystick: lembrar último valor significativo
int ultimoEixoX = 512;
int ultimoEixoY = 512;

void setup() {
  pinMode(pino_out, INPUT);
  Serial.begin(9600);
  delay(200);
  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
  Serial.print("Limite de distância atual = ");
  Serial.print(limiteDistancia);
  Serial.println(" cm");
}

void loop() {
  
  // Leitura do teclado
  leTeclado();

  // Mostrar sensores no modo contínuo
  if (modoAtual == MODO_CONTINUO) {
    unsigned long now = millis();
    if (now - ultimaLeituraMillis >= intervaloContinuo) {
      ultimaLeituraMillis = now;
      leSensores();
      exibeResultados(true);
    }
  }

  // Sempre checamos joystick para permitir ao usuário a mudança de modo
  leJoystick();
}

// ========== Funções ==========

void leJoystick() {
  int eixoX = analogRead(xPin); // vertical
  int eixoY = analogRead(yPin); // horizontal

  // Só imprimimos valores brutos quando houve movimento significativo (evita spam)
  if (abs(eixoX - ultimoEixoX) > 30 || abs(eixoY - ultimoEixoY) > 30) {
    Serial.print("Joystick X: ");
    Serial.print(eixoX);
    Serial.print(" | Y: ");
    Serial.println(eixoY);
    ultimoEixoX = eixoX;
    ultimoEixoY = eixoY;
    delay(200);
  }

  // Detectar proposta de modo segundo especificação:
  // Esquerda -> MODO_CONTINUO
  // Direita  -> MODO_SOB_DEMANDA
  // Cima     -> MODO_TESTE
  Modo proposto = MODO_IDLE;

  // Horizontal: esquerda / direita 
  bool movedYLow  = eixoY < JOY_LOW;
  bool movedYHigh = eixoY > JOY_HIGH;
  
  if (movedYLow)  proposto = MODO_CONTINUO;     // esquerda
  else if (movedYHigh) proposto = MODO_SOB_DEMANDA; // direita

  // Vertical: cima (MODO_TESTE)
  bool movedXLow  = eixoX < JOY_LOW;
  bool movedXHigh = eixoX > JOY_HIGH;
  
  if (movedXHigh) { // cima tem prioridade se você estiver empurrando diagonal
    proposto = MODO_TESTE;
  }

  // Se proposta mudou, informe o usuário 
  if (proposto != ultimoProposto) {
    ultimoProposto = proposto;
    switch (proposto) {
      case MODO_CONTINUO:
        Serial.println();
        Serial.println("Seleção proposta: MODO_CONTINUO (esquerda). Pressione '#' para confirmar.");
        break;
      case MODO_SOB_DEMANDA:
        Serial.println();
        Serial.println("Seleção proposta: MODO_SOB_DEMANDA (direita). Pressione '#' para confirmar.");
        break;
      case MODO_TESTE:
        Serial.println();
        Serial.println("Seleção proposta: MODO_TESTE (cima). Pressione '#' para confirmar.");
        break;
      default:
        // sem seleção
        break;
    }
  }
}

void leTeclado() {
  char key = teclado.getKey();
  if (!key) return;

  // Se no modo de entrada de parâmetro (pressionou 'A' antes)
  if (emModoEntradaParametro) {
    if (key >= '0' && key <= '9') {
      bufferParametro += key;
      Serial.print(key); 
    } else if (key == '#') {
      if (bufferParametro.length() > 0) {
        int novo = bufferParametro.toInt();
        limiteDistancia = max(0, novo);
        Serial.println();
        Serial.print("Parâmetro atualizado: distância crítica = ");
        Serial.print(limiteDistancia);
        Serial.println(" cm.");
      } else {
        Serial.println();
        Serial.println("Entrada vazia. Nenhuma alteração feita.");
      }
      bufferParametro = "";
      emModoEntradaParametro = false;
    } else if (key == 'D') { // cancelar com D
      bufferParametro = "";
      emModoEntradaParametro = false;
      Serial.println();
      Serial.println("Entrada de parâmetro cancelada.");
    }
    return;
  }

  // Saída referente às teclas pressionadas (exigência do trabalho);
  Serial.println();
  Serial.print("Tecla pressionada: ");
  Serial.println(key);

  // Confirmar modo com '#'
  if (key == '#') {
    if (ultimoProposto != MODO_IDLE) {
      modoAtual = ultimoProposto;
      switch (modoAtual) {

        case MODO_CONTINUO:
        Serial.println();
        Serial.println("Modo CONTÍNUO selecionado."); break;

        case MODO_SOB_DEMANDA: 
        Serial.println();
        Serial.println("Modo SOB DEMANDA selecionado.");
        Serial.println("Pressione 'A' para editar o limite seguro de distância.");
        Serial.println("Pressione '*' para realizar o monitoramento. ");
        Serial.println(); break;

        case MODO_TESTE:
        Serial.println();
        Serial.println("Modo TESTE selecionado.");
        realizaTesteSensores();

        // retorna para o idle após o teste 
        modoAtual = MODO_IDLE;
        Serial.println("Voltando para modo IDLE. Use joystick para escolher outro modo."); break;

        default: break;
      }

    } else {

      int eixoX = analogRead(xPin), eixoY = analogRead(yPin);

      // usar mesma lógica de mapeamento
      bool movedYLow  = eixoY < JOY_LOW;
      bool movedYHigh = eixoY > JOY_HIGH;
      bool movedXHigh = eixoX > JOY_HIGH;
      
      if (movedYLow) { modoAtual = MODO_CONTINUO; Serial.println("Modo CONTÍNUO selecionado (fallback)."); }
      else if (movedYHigh) { modoAtual = MODO_SOB_DEMANDA; Serial.println("Modo SOB DEMANDA selecionado (fallback)."); }
      else if (movedXHigh) { modoAtual = MODO_TESTE; Serial.println("Modo TESTE selecionado (fallback)."); realizaTesteSensores(); modoAtual = MODO_IDLE; }
      else Serial.println("Nenhuma seleção detectada no joystick. Não foi alterado.");
    }

    return;
  }

  // '*' para leitura sob demanda
  if (key == '*') {
    if (modoAtual == MODO_SOB_DEMANDA) {
      leSensores();
      exibeResultados(true);
    } else {
      Serial.println("'*' apenas realiza leitura quando em Modo Sob Demanda.");
    }
    return;
  }

  // 'A' entra em modo de edição do parâmetro distância
  if (key == 'A') {
    emModoEntradaParametro = true;
    bufferParametro = "";
    Serial.println("Entre com novo valor de distância crítica (cm). Digite números e confirme com '#'. Pressione 'D' para cancelar.");
    return;
  }
}

void leSensores() {
  distancia = ultrassom.Ranging(CM);
}

void exibeResultados(bool checarAlerta) {
  Serial.print("Distância (ultrassom): ");
  Serial.print(distancia);
  Serial.println(" cm");

  int estadoObst = digitalRead(pino_out);
  if (estadoObst == LOW) Serial.println("Sensor de obstáculo: OBJETO DETECTADO");
  else Serial.println("Sensor de obstáculo: Nenhum objeto detectado");
  
  if (checarAlerta) {
    if (distancia > 0 && distancia <= limiteDistancia) {
      Serial.println("ALERTA: distância inferior ao limite!");
    }
  }
  Serial.println("-------------------------------");
}

void realizaTesteSensores() {
  Serial.println();
  Serial.println("=== INICIANDO TESTE DE SENSORES ===");
  leSensores();
  Serial.print("Teste - Ultrassom: leitura atual = ");
  Serial.print(distancia);
  Serial.println(" cm");
  if (distancia > 0) Serial.println("Sensor de distância OK");
  else Serial.println("Sensor de distância: leitura inválida (verifique conexões)");

  int estadoObst = digitalRead(pino_out);
  if (estadoObst == LOW) Serial.println("Sensor de obstáculo OK - detecta objeto (estado LOW).");
  else Serial.println("Sensor de obstáculo OK - sem objeto (estado HIGH).");

  Serial.println();
  Serial.println("=== FIM DO TESTE ===");
}