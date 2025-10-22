// Trabalho Arquiteturas de Computadores - Turma Quarta-Feira Manhã
// Membros da Equipe:
// 202503341672 Gabriel -TA
// 202507010701 Maurício - TA
// 202407095917 Enzo - TA 
// 202307164607 Ricardo - TA
// 202503798371 Victor - TA
// BIBLIOTECA NECESSÁRIA - Keypad


#include <Keypad.h>

// Pinagem

// Joystick
#define JOY_X A0
#define JOY_Y A1

// Sensor ultrassônico
#define TRIG_PIN 8
#define ECHO_PIN 9

// Sensor de obstáculos
#define IR_PIN 12

// Variáveis globais

int modo = 0; // 0 = não selecionado, 1 = contínuo, 2 = sob demanda, 3 = teste
int distanciaCritica = 50; // distância limite em cm

// Variáveis para temporização não bloqueante (Modo Contínuo)
unsigned long tempoAnterior = 0;
const long intervalo = 1000; // 1 segundo (1000 ms) entre leituras

// Setup do teclado

const byte LINHAS = 4;
const byte COLUNAS = 4;

char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinosLinhas[LINHAS] = {2, 3, 4, 5};
byte pinosColunas[COLUNAS] = {6, 7, 10, 11};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// Funções

// Leitura do joystick
void leJoystick(int &x, int &y) {
  x = analogRead(JOY_X);
  y = analogRead(JOY_Y);
}

// Leitura do teclado
char leTeclado() {
  return teclado.getKey();
}

// Leitura do sensor ultrassônico
long leDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);
  long distancia = duracao * 0.034 / 2; // em cm
  return distancia;
}

// Leitura do sensor de obstáculo
bool leObstaculo() {
  // sensor retorna LOW quando detecta obstáculo
  return digitalRead(IR_PIN) == LOW;
}

// Exibe todas as informações no Serial Monitor
void exibeStatus(int joyX, int joyY, char tecla, long distancia, bool obstaculo) {
  Serial.println("=======================================");
  Serial.print("Joystick -> X: ");
  Serial.print(joyX);
  Serial.print(" | Y: ");
  Serial.println(joyY);

  Serial.print("Tecla pressionada: ");
  // A variável 'tecla' agora carrega a última tecla válida pressionada (ou NO_KEY)
  if (tecla != NO_KEY) Serial.println(tecla);
  else Serial.println("Nenhuma");

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.print(" cm | Obstáculo: ");
  Serial.println(obstaculo ? "DETECTADO" : "Nenhum");

  // Exibe alertas
  if (distancia < distanciaCritica)
    Serial.println("⚠️ ALERTA: distância inferior ao limite!");
  if (obstaculo)
    Serial.println("⚠️ ALERTA: obstáculo detectado!");

  Serial.println("=======================================");
  Serial.println();
}

// Teste de sensores
void testeSensores() {
  Serial.println("=== Teste de Sensores ===");
  Serial.print("Sensor Ultrassônico OK - ");
  Serial.print(leDistancia());
  Serial.println(" cm");

  Serial.print("Sensor de Obstáculo OK - ");
  if (leObstaculo()) Serial.println("Objeto detectado!");
  else Serial.println("Nenhum obstáculo.");
  Serial.println("=========================");
}

// Atualiza parâmetros
void atualizaParametro() {
  String valorStr = "";
  char tecla;

  Serial.print("\n--- Modo Parâmetros ---\n");
  Serial.print("Digite nova distância crítica (cm) e pressione #: ");

  while (true) {
    tecla = leTeclado();
    if (tecla != NO_KEY) {
      if (tecla == '#') break;
      
      if (isDigit(tecla)) {
        valorStr += tecla;
        Serial.print(tecla);
      }
    }
    delay(50); 
  }

  int valor = valorStr.toInt();
  if (valor > 0) { 
    distanciaCritica = valor;
    Serial.print("\n✅ Parâmetro atualizado: distância crítica = ");
  } else {
    Serial.print("\n❌ Entrada inválida. Mantendo o valor anterior: ");
  }
  Serial.print(distanciaCritica);
  Serial.println(" cm");
  Serial.println("------------------------\n");
}

// Configuração inicial

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
}

// Loop principal

void loop() {
  int x, y;
  leJoystick(x, y);
  
  // Leitura da tecla neste ciclo
  char teclaAtual = leTeclado();

  // Variável para armazenar a tecla pressionada entre as exibições do Modo Contínuo
  static char teclaParaExibir = NO_KEY; 

  // Verifica se D foi apertado para acionar a opção de inserir limite
  if (teclaAtual == 'D') {
    atualizaParametro();
    return; 
  }
  
  // Captura a tecla para exibi-lá no serial
  // Se uma tecla foi pressionada (e não foi 'D'), armazena para exibição
  if (teclaAtual != NO_KEY) {
      teclaParaExibir = teclaAtual;
  }

  // Seleção de modo
  static int modoSelecionado = 0; 

  if (modo == 0) {
    if (x < 400) {
      if (modoSelecionado != 1) {
        modoSelecionado = 1;
        Serial.println("Modo CONTÍNUO selecionado (esquerda). Pressione # para confirmar.");
      }
    } else if (x > 600) {
      if (modoSelecionado != 2) {
        modoSelecionado = 2;
        Serial.println("Modo SOB DEMANDA selecionado (direita). Pressione # para confirmar.");
      }
    } else if (y < 400) {
      if (modoSelecionado != 3) {
        modoSelecionado = 3;
        Serial.println("Modo TESTE selecionado (cima). Pressione # para confirmar.");
      }
    }

    if (teclaAtual == '#') {
      modo = modoSelecionado;
      switch (modo) {
        case 1: Serial.println("✅ Modo CONTÍNUO confirmado."); break;
        case 2: Serial.println("✅ Modo SOB DEMANDA confirmado."); break;
        case 3: Serial.println("✅ Modo TESTE confirmado."); break;
        default: Serial.println("❌ Seleção de modo inválida."); break;
      }
      delay(500); 
    }

    delay(100);
    return; 
  }

  // Modos de operação
  
  // Modo Contínuo (Uando millis para não bloquear o programa)
  if (modo == 1) {
    unsigned long tempoAtual = millis();

    if (tempoAtual - tempoAnterior >= intervalo) {
      tempoAnterior = tempoAtual; 

      long dist = leDistancia();
      bool obstaculo = leObstaculo();
      
      // Passa a tecla armazenada
      exibeStatus(x, y, teclaParaExibir, dist, obstaculo);
      
      // Limpa a tecla após a exibição para que não repita na próxima vez
      teclaParaExibir = NO_KEY; 
    }
  }

  // Modo sob demanda
  else if (modo == 2) {
    if (teclaAtual == '*') {
      long dist = leDistancia();
      bool obstaculo = leObstaculo();
      // Neste modo, exibimos o status imediatamente após pressionar '*'
      exibeStatus(x, y, teclaAtual, dist, obstaculo); 
    }
  }

  // Modo de teste
  else if (modo == 3) {
    testeSensores();
    delay(2000); 
  }
}
