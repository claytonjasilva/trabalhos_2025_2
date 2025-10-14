/*
 * --------------------------------------------------------------------
 Integrantes: 202503251665 Pedro dos Santos TA
 202501027288 Gabriel pereira TA
 202502898381 Matheus Paes Leme TA
 202501001203 mariana faria NT
 202503341583 Isis Tavares NT
 * --------------------------------------------------------------------
 */

// Inclusão da biblioteca para o teclado de membrana
#include <Keypad.h>

// --- Mapeamento de Hardware (Pinos) ---

// Joystick
const int pinoJoystickX = A0; // Pino para o eixo X do joystick
const int pinoJoystickY = A1; // Pino para o eixo Y do joystick

// Sensor Ultrassônico
const int pinoTrig = 9;      // Pino de Trigger
const int pinoEcho = 10;     // Pino de Echo

// Sensor de Nível de Água
const int pinoNivelAgua = A2; // Pino para o sensor de nível

// Teclado de Membrana
const byte LINHAS = 4; // Quatro linhas
const byte COLUNAS = 4; // Quatro colunas
// Mapeamento das teclas
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinosLinhas[LINHAS] = {46, 44, 42, 40}; // Conectado às linhas do teclado
byte pinosColunas[COLUNAS] = {38, 36, 34, 32}; // Conectado às colunas do teclado

// Inicialização do objeto Keypad
Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// --- Variáveis Globais de Controle ---

enum ModoOperacao {
  INICIAL, MONITOR_SELECIONADO, ENTRADA_DISTANCIA, ENTRADA_NIVEL,
  MONITOR_CONTINUO, MONITOR_SOB_DEMANDA, TESTE_SENSORES
};

ModoOperacao modoAtual = INICIAL;
ModoOperacao modoPreSelecionado = INICIAL;
int posXJoystick, posYJoystick;
long distanciaCm;
int nivelAgua;
int distanciaCritica = 20;
int nivelCritico = 500;
String entradaNumerica = "";
unsigned long tempoAnterior = 0;
const long intervaloLeitura = 2000;
bool mensagemModoDemandaExibida = false;


// ====================================================================
//                 >>> PROTÓTIPOS DAS FUNÇÕES <<<
//      Este bloco corrige o erro de compilação "'leJoystick' was not declared"
// ====================================================================
void leJoystick();
char leTeclado();
void leSensorDistancia();
void leSensorNivelAgua();
void leSensores();
void exibeResultados();
void verificaAlertas();
void gerenciaSelecaoModo();
void aguardaConfirmacao();
void gerenciaEntradaParametros(String parametro);
void executaModoContinuo();
void executaModoSobDemanda();
void executaModoTeste();
// ====================================================================


// --- Função de Configuração (executa uma vez) ---
void setup() {
  Serial.begin(9600);
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  Serial.println("=========================================");
  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
  Serial.println("=========================================");
}

// --- Loop Principal (executa continuamente) ---
void loop() {
  switch (modoAtual) {
    case INICIAL:               gerenciaSelecaoModo();      break;
    case MONITOR_SELECIONADO:   aguardaConfirmacao();       break;
    case ENTRADA_DISTANCIA:     gerenciaEntradaParametros("distancia"); break;
    case ENTRADA_NIVEL:         gerenciaEntradaParametros("nivel");   break;
    case MONITOR_CONTINUO:      executaModoContinuo();      break;
    case MONITOR_SOB_DEMANDA:   executaModoSobDemanda();    break;
    case TESTE_SENSORES:        executaModoTeste();         break;
  }
}

// --- Funções de Lógica e Controle ---

void gerenciaSelecaoModo() {
  leJoystick();
  if (posXJoystick > 900) {
    Serial.println("-> Modo Monitoramento Sob Demanda pre-selecionado. Pressione '#' para confirmar.");
    modoPreSelecionado = MONITOR_SOB_DEMANDA;
    modoAtual = MONITOR_SELECIONADO;
    delay(500);
  } else if (posXJoystick < 100) {
    Serial.println("-> Modo Monitoramento Continuo pre-selecionado. Pressione '#' para confirmar.");
    modoPreSelecionado = MONITOR_CONTINUO;
    modoAtual = MONITOR_SELECIONADO;
    delay(500);
  } else if (posYJoystick > 900) {
    Serial.println("-> Modo Teste de Sensores pre-selecionado. Pressione '#' para confirmar.");
    modoPreSelecionado = TESTE_SENSORES;
    modoAtual = MONITOR_SELECIONADO;
    delay(500);
  }
}

void aguardaConfirmacao() {
  char tecla = leTeclado();
  if (tecla == '#') {
    modoAtual = modoPreSelecionado;
    if (modoAtual == MONITOR_CONTINUO || modoAtual == MONITOR_SOB_DEMANDA) {
      Serial.print("\nModo ");
      Serial.print(modoAtual == MONITOR_CONTINUO ? "Monitoramento Continuo" : "Monitoramento Sob Demanda");
      Serial.println(" selecionado.");
      Serial.println("Digite a distancia critica (cm) e pressione '#':");
      modoAtual = ENTRADA_DISTANCIA;
    } else if (modoAtual == TESTE_SENSORES) {
      Serial.println("\nModo Teste de Sensores selecionado.");
    }
  }
}

void gerenciaEntradaParametros(String parametro) {
  char tecla = leTeclado();
  if (tecla) {
    if (isDigit(tecla)) {
      entradaNumerica += tecla;
      Serial.print(tecla);
    } else if (tecla == '#' && entradaNumerica.length() > 0) {
      if (parametro == "distancia") {
        distanciaCritica = entradaNumerica.toInt();
        Serial.println("\nParametro atualizado: distancia critica = " + String(distanciaCritica) + " cm.");
        entradaNumerica = "";
        Serial.println("Digite o nivel de agua critico (0-1023) e pressione '#':");
        modoAtual = ENTRADA_NIVEL;
      } else if (parametro == "nivel") {
        nivelCritico = entradaNumerica.toInt();
        Serial.println("\nParametro atualizado: nivel de agua critico = " + String(nivelCritico) + ".");
        entradaNumerica = "";
        Serial.println("\nConfiguracao finalizada. Iniciando operacao...");
        modoAtual = modoPreSelecionado;
      }
    } else if (tecla == '*') {
      entradaNumerica = "";
      Serial.println("\nEntrada limpa. Digite novamente:");
    }
  }
}

void executaModoContinuo() {
  char tecla = leTeclado();
  if (tecla) {
    Serial.println("\n--- Monitoramento Continuo Interrompido ---");
    Serial.println("Retornando ao menu principal...");
    modoAtual = INICIAL;
    delay(500);
    Serial.println("\nUse o joystick para escolher o modo.");
    return;
  }
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloLeitura) {
    tempoAnterior = tempoAtual;
    Serial.println("\n--- Leitura Periodica (Pressione qualquer tecla para parar) ---");
    leSensores();
    exibeResultados();
    verificaAlertas();
  }
}

void executaModoSobDemanda() {
  if (!mensagemModoDemandaExibida) {
    Serial.println("Pressione '*' para ler os sensores ou 'D' para voltar ao menu.");
    mensagemModoDemandaExibida = true;
  }
  char tecla = leTeclado();
  if (tecla == '*') {
    Serial.println("\n--- Leitura Sob Demanda ---");
    leSensores();
    exibeResultados();
    verificaAlertas();
    mensagemModoDemandaExibida = false;
  } else if (tecla == 'D') {
    Serial.println("\nRetornando ao menu principal...");
    modoAtual = INICIAL;
    mensagemModoDemandaExibida = false;
    delay(500);
    Serial.println("Use o joystick para escolher o modo.");
  }
}

void executaModoTeste() {
  Serial.println("\n--- Iniciando Teste de Sensores ---");
  leSensorDistancia();
  if (distanciaCm > 0) {
    Serial.println("Sensor de distancia OK – leitura atual: " + String(distanciaCm) + " cm");
  } else {
    Serial.println("ERRO: Sensor de distancia nao responde.");
  }
  delay(500);
  leSensorNivelAgua();
  Serial.println("Sensor de nivel de agua OK – leitura atual: " + String(nivelAgua));
  delay(500);
  leJoystick();
  Serial.println("Joystick OK - Posicao (X, Y): (" + String(posXJoystick) + ", " + String(posYJoystick) + ")");
  delay(500);
  Serial.println("Pressione qualquer tecla para testar o teclado...");
  char tecla = NO_KEY;
  while (tecla == NO_KEY) {
    tecla = leTeclado();
  }
  Serial.println("Teclado OK - Tecla pressionada: " + String(tecla));
  Serial.println("\n--- Teste finalizado! Retornando ao menu principal. ---");
  modoAtual = INICIAL;
  delay(1000);
  Serial.println("\nUse o joystick para escolher o modo.");
}

// --- Funções de Leitura e Exibição ---

void leJoystick() {
  posXJoystick = analogRead(pinoJoystickX);
  posYJoystick = analogRead(pinoJoystickY);
}

char leTeclado() {
  return teclado.getKey();
}

void leSensorDistancia() {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH);
  distanciaCm = duracao * 0.034 / 2;
}

void leSensorNivelAgua() {
  nivelAgua = analogRead(pinoNivelAgua);
}

void leSensores() {
  leSensorDistancia();
  leSensorNivelAgua();
  leJoystick();
}

void exibeResultados() {
  Serial.println("--- Dados Atuais ---");
  Serial.println("Joystick (X,Y): (" + String(posXJoystick) + ", " + String(posYJoystick) + ")");
  Serial.println("Distancia: " + String(distanciaCm) + " cm");
  Serial.println("Nivel de Agua: " + String(nivelAgua));
  Serial.println("--------------------");
}

void verificaAlertas() {
  if (distanciaCm < distanciaCritica && distanciaCm > 0) {
    Serial.println("**************************************");
    Serial.println("ALERTA: Objeto muito proximo!");
    Serial.println("**************************************");
  }
  if (nivelAgua > nivelCritico) {
    Serial.println("**************************************");
    Serial.println("ALERTA: Nivel de agua critico!");
    Serial.println("**************************************");
  }
}
