/*
PROJETO ARDUINO  SISTEMA DE MONITORAMENTO COM JOYSTICK E TECLADO
ARQUITETURA DE COMPUTADORES

Integrantes do grupo
Guilherme da Silva Pinon - 202501000061 - TA
Bruno Carvalho Pessoa - 202503440964 - TA
Caio Cunha - 202502578369 - TA
Henrique Cals - 202501537839 - TA
- Giovanna Sales Nunes de Lima - 20250423670 - TA
*/

// ========== DEFINIÇÃO DOS PINOS ==========
// Mapeamento de hardware utilizado pelo sketch
#define VRX_PIN A0        // Joystick eixo X, leitura analógica
#define VRY_PIN A1        // Joystick eixo Y, leitura analógica
#define SOUND_ANALOG_PIN A5 // Sensor de som, leitura analógica
#define IR_SENSOR_PIN 5   // Sensor infravermelho, entrada digital

// ========== VARIÁVEIS GLOBAIS ==========
// Leituras atuais dos sensores
int soundAnalog = 0;      // Amplitude do som entre 0 e 1023
int xValue = 0;           // Joystick X entre 0 e 1023
int yValue = 0;           // Joystick Y entre 0 e 1023

// Controle de estado do sistema
int modoSelecionado = 0;           // 0 sem modo, 1 contínuo, 2 sob demanda, 3 teste
int distanciaLimite = 50;          // Limiar de alerta em centímetros
bool modoConfirmado = false;       // Confirmação com tecla cardinal
unsigned long ultimaLeituraJoystick = 0; // Antirruído e ritmo de amostragem do joystick

// ========== BIBLIOTECA E CONFIGURAÇÃO DO TECLADO ==========
#include <Keypad.h>  // Interface para teclado matricial 4x4

// Dimensão do teclado
const byte ROWS = 4;     // Quantidade de linhas
const byte COLS = 4;     // Quantidade de colunas

// Mapa de teclas no arranjo linha por coluna
char hexaKeys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},     // Linha superior, funções especiais
  {'#', '9', '6', '3'},     // Linha seguinte, dígitos e confirmar
  {'0', '8', '5', '2'},     // Linha seguinte, dígitos
  {'*', '7', '4', '1'}      // Linha inferior, dígitos e leitura sob demanda
};

// Pinos ligados às linhas do teclado
byte rowPins[ROWS] = {10, 11, 12, 13}; 

// Pinos ligados às colunas do teclado
byte colPins[COLS] = {6, 7, 8, 9}; 

// Instância do teclado com o mapeamento definido
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// ========== FUNÇÃO SETUP  EXECUTADA UMA VEZ ==========
void setup(){
  Serial.begin(9600);                 // Inicializa a porta serial
  pinMode(IR_SENSOR_PIN, INPUT);      // Sensor IR como entrada
  
  // Mensagens de orientação no Monitor Serial
  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
  Serial.println("Esquerda: Monitoramento Continuo");
  Serial.println("Direita: Monitoramento Sob Demanda");
  Serial.println("Cima: Modo Teste de Sensores");
  Serial.println("Confirme com #");
}

// ========== FUNÇÕES DE LEITURA DOS SENSORES ==========

// Proximidade com sensor infravermelho
void lerProximidade() {
  int sensorState = digitalRead(IR_SENSOR_PIN);  // Nível lógico atual
  
  if (sensorState == LOW) {  
    // Nível baixo indica obstáculo para o modelo de sensor utilizado
    Serial.println("IR: Obstaculo detectado!");
  } else {
    Serial.println("IR: Sem obstaculo.");
  }
}

// Nível de som por entrada analógica
void lerSom() {
  soundAnalog = analogRead(SOUND_ANALOG_PIN); // Amostra entre 0 e 1023
  Serial.print("Som (Analogico): ");
  Serial.println(soundAnalog);
}

// Posição atual do joystick
void lerJoystick() {
  xValue = analogRead(VRX_PIN);  // Eixo X
  yValue = analogRead(VRY_PIN);  // Eixo Y
  Serial.print("Joystick - X: ");
  Serial.print(xValue);
  Serial.print(" | Y: ");
  Serial.println(yValue);
}

// ========== SELEÇÃO DE MODO COM JOYSTICK ==========
void selecionarModo() {
  // Intervalo mínimo entre leituras para reduzir ruído e falso disparo
  if (millis() - ultimaLeituraJoystick > 300) {
    // Nova amostragem dos eixos
    xValue = analogRead(VRX_PIN);
    yValue = analogRead(VRY_PIN);
    ultimaLeituraJoystick = millis(); // Marca temporal da leitura
    
    // Para depuração, a linha a seguir exibe as leituras do joystick
    // Serial.print("X: "); Serial.print(xValue); Serial.print(" Y: "); Serial.println(yValue);
    
    // Zonas de decisão do joystick para cada direção
    if (xValue < 300) { // Esquerda
      if (modoSelecionado != 1) {
        Serial.println(">>> Modo selecionado: Monitoramento Continuo");
        modoSelecionado = 1;
      }
    } 
    else if (xValue > 700) { // Direita
      if (modoSelecionado != 2) {
        Serial.println(">>> Modo selecionado: Monitoramento Sob Demanda");
        modoSelecionado = 2;
      }
    } 
    else if (yValue > 700) { // Cima
      if (modoSelecionado != 3) {
        Serial.println(">>> Modo selecionado: Teste de Sensores");
        modoSelecionado = 3;
      }
    }
  }
}

// ========== LEITURA CONJUNTA DOS SENSORES ==========
void lerTodosSensores() {
  Serial.println("=== LEITURA DOS SENSORES ===");
  lerJoystick();        // Posição do joystick
  lerProximidade();     // Estado do sensor IR
  lerSom();             // Amplitude do som
  Serial.println("============================");
}

// ========== ROTINA DO MODO TESTE ==========
void modoTesteSensores() {
  Serial.println("=== MODO TESTE DE SENSORES ===");
  
  // Verificacao do sensor IR
  int sensorState = digitalRead(IR_SENSOR_PIN);
  if (sensorState == LOW) {
    Serial.println("Sensor IR: OK - Obstaculo detectado");
  } else {
    Serial.println("Sensor IR: OK - Sem obstaculo");
  }
  
  // Verificacao do sensor de som
  soundAnalog = analogRead(SOUND_ANALOG_PIN);
  Serial.print("Sensor de Som: OK - Leitura atual: ");
  Serial.println(soundAnalog);
  
  // Verificacao do joystick
  lerJoystick();
  Serial.println("Joystick: OK");
  
  Serial.println("==============================");`
}

// ========== TRATAMENTO DO TECLADO MATRICIAL ==========
void processarTeclado() {
  char customKey = customKeypad.getKey(); // Captura de tecla, caso exista
  
  if (customKey) {
    Serial.print("Tecla pressionada: ");
    Serial.println(customKey);
    
    // Ação por tecla
    switch(customKey) {
      case '#': // Confirmacao do modo
        if (modoSelecionado == 0) {
          Serial.println("ERRO: Selecione um modo com o joystick primeiro!");
        } else {
          modoConfirmado = true; // Marca que a escolha foi confirmada
          Serial.print(">>> Modo ");
          switch(modoSelecionado) {
            case 1: Serial.println("Monitoramento Continuo confirmado!"); break;
            case 2: Serial.println("Monitoramento Sob Demanda confirmado!"); break;
            case 3: Serial.println("Teste de Sensores confirmado!"); break;
          }
        }
        break;
        
      case '*': // Leitura pontual no modo sob demanda
        if (modoConfirmado && modoSelecionado == 2) {
          Serial.println(">>> Executando leitura sob demanda...");
          lerTodosSensores();
        } else if (!modoConfirmado) {
          Serial.println("ERRO: Confirme o modo com # primeiro!");
        }
        break;
        
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        // Ajuste de parametro quando um modo ja estiver confirmado
        if (modoConfirmado) {
          int numero = customKey - '0';   // Converte caractere para inteiro
          distanciaLimite = numero * 10;  // Passo de 10 cm por digito
          Serial.print(">>> Parametro atualizado: distancia critica = ");
          Serial.print(distanciaLimite);
          Serial.println(" cm");
        } else {
          Serial.println("ERRO: Confirme o modo com # primeiro!");
        }
        break;
        
      case 'A': // Reinicio lógico do estado do aplicativo
        Serial.println(">>> Funcao A: Reiniciar sistema");
        modoSelecionado = 0;
        modoConfirmado = false;
        Serial.println("Sistema reiniciado. Selecione o modo novamente.");
        break;
        
      case 'B': // Exibe parametros atuais para conferência
        Serial.println(">>> Funcao B: Exibir parametros atuais");
        Serial.print("Distancia limite: ");
        Serial.print(distanciaLimite);
        Serial.println(" cm");
        Serial.print("Modo atual: ");
        Serial.println(modoSelecionado);
        Serial.print("Modo confirmado: ");
        Serial.println(modoConfirmado ? "SIM" : "NAO");
        break;
        
      default: // Teclas não mapeadas
        Serial.println("Tecla nao configurada");
        break;
    }
  }
}

// ========== EXECUÇÃO DO MODO ATUAL ==========
void executarModo() {
  if (!modoConfirmado) return; // Evita executar antes da confirmacao
  
  // Rotina dedicada por modo ativo
  switch(modoSelecionado) {
    case 1: // Monitoramento continuo
      lerTodosSensores();
      delay(2000);        // Janela entre leituras
      break;
      
    case 2: // Sob demanda
      // Aguardando tecla asterisco para leitura
      break;
      
    case 3: // Teste de sensores
      modoTesteSensores();
      delay(5000);        // Intervalo entre ciclos de teste
      break;
  }
}

// ========== LOOP PRINCIPAL ==========
void loop(){
  // Verifica teclado a cada ciclo
  processarTeclado();
  
  // Seleção de modo enquanto não confirmado
  if (!modoConfirmado) {
    selecionarModo();
  } else {
    // Execução do modo escolhido
    executarModo();
  }
  
  delay(50); // Estabilização simples do laço principal
}
