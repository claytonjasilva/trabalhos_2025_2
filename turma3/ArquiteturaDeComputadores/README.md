# Arquitetura de Computadores

Projeto prático da disciplina **Arquitetura de Computadores**.

## Integrantes

- **Raí Lamper de Avila** — 202402627279 — TA  
- **Guilherme Braz Martinelli Amaro** — 202502580835 — TA  
- **Ricardo Magalhães de Bello Costa** — 202501000053 — TA  
- **Davi C. dos Santos** — 202502165676 — TA  
- **Gabriel de Aguiar da Pureza** — 202501270001 — TA

---

## Descrição do Projeto

O projeto utiliza um **Arduino** com os seguintes componentes:

- Teclado 4x4
- Joystick analógico
- Sensor Ultrassônico
- Sensor de Nível de Água

Através do **joystick**, o usuário seleciona um modo de operação. O sistema faz a leitura dos sensores e exibe os dados no **monitor serial**. Modos disponíveis:

1. **Contínuo** — Leituras automáticas a cada segundo
2. **Sob Demanda** — Leituras manuais ao pressionar `*`
3. **Teste** — Diagnóstico dos sensores

---

##  Código-Fonte

```cpp
//Raí Lamper de Avila — 202402627279 — TA
//Guilherme Braz Martinelli Amaro — 202502580835 — TA
//Ricardo Magalhães de Bello Costa — 202501000053 — TA
//Davi C. dos Santos — 202502165676 — TA
//Gabriel de Aguiar da Pureza — 202501270001 — TA

#include <Keypad.h>

// --- Configuração do Teclado 4x4 ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Pinos das linhas
byte colPins[COLS] = {5, 4, 3, 2}; // Pinos das colunas
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Configuração do Joystick ---
const int joyXPin = A0; // Eixo X do joystick
const int joyYPin = A1; // Eixo Y do joystick

// --- Configuração do Sensor Ultrassônico ---
const int trigPin = 11;
const int echoPin = 12;

// --- Configuração do Sensor de Nível de Água ---
const int waterPin = A2;

// --- Variáveis Globais ---
int mode = 0; // 0: inicial, 1: contínuo, 2: sob demanda, 3: teste
float distanceLimit = 50.0; // Distância crítica em cm
int waterLevelLimit = 700; // Nível crítico de água (valor analógico)
unsigned long lastReadTime = 0; // Controle de tempo para modo contínuo

void setup() {
  Serial.begin(9600); // Inicia comunicação serial
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(waterPin, INPUT);
  Serial.println("Sistema iniciado. Use o joystick para escolher o modo.");
}

void loop() {
  leJoystick(); // Lê joystick e seleciona modo ou reseta modo
  if (mode != 0) {
    leTeclado(); // Lê teclado para confirmação, parâmetros e reset
    leSensores(); // Lê sensores conforme o modo
    exibeResultados(); // Exibe resultados no monitor serial
  }
  delay(1500); // Pequeno delay para estabilidade
}

// --- Função para ler o joystick ---
void leJoystick() {
  int xValue = analogRead(joyXPin); // Lê eixo X
  int yValue = analogRead(joyYPin); // Lê eixo Y
  
  Serial.print("Joystick X: ");
  Serial.print(xValue);
  Serial.print(" | Y: ");
  Serial.println(yValue);
  
  // Permite resetar para modo inicial movendo joystick para baixo
  if (yValue < 200 && mode != 0) {
    Serial.println("Resetando modo para inicial via joystick.");
    mode = 0;
    return;
  }
  
  // Seleciona modo apenas se estiver no modo inicial
  if (mode == 0) {
    if (xValue < 200) {
      Serial.println("Modo contínuo selecionado. Confirme com '#'.");
      mode = 1;
    } else if (xValue > 800) {
      Serial.println("Modo sob demanda selecionado. Confirme com '#'.");
      mode = 2;
    } else if (yValue > 800) {
      Serial.println("Modo teste selecionado. Confirme com '#'.");
      mode = 3;
    }
  }
}

// --- Função para ler o teclado ---
void leTeclado() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Tecla pressionada: ");
    Serial.println(key);
    
    // Reset do modo ao pressionar 'D'
    if (key == 'D') {
      Serial.println("Resetando modo para inicial via teclado.");
      mode = 0;
      return;
    }
    
    // Confirmação do modo
    if (key == '#' && mode != 0) {
      Serial.print("Modo ");
      if (mode == 1) Serial.println("contínuo selecionado.");
      else if (mode == 2) Serial.println("sob demanda selecionado.");
      else if (mode == 3) Serial.println("teste selecionado.");
    }
    
    // Configuração de parâmetros (exemplo: distância crítica)
    if (key >= '0' && key <= '9') {
      static String input = "";
      input += key;
      if (input.length() == 2) { // Assume entrada de 2 dígitos
        distanceLimit = input.toInt();
        Serial.print("Parâmetro atualizado: distância crítica = ");
        Serial.print(distanceLimit);
        Serial.println(" cm.");
        input = "";
      }
    }
    
    // Modo sob demanda: leitura ao pressionar '*'
    if (key == '*' && mode == 2) {
      leSensores();
    }
  }
}

// --- Função para ler os sensores ---
void leSensores() {
  if (mode == 1 && millis() - lastReadTime < 1000) return; // Modo contínuo: lê a cada 1s
  if (mode == 2 && keypad.getKey() != "") return; // Modo sob demanda: só lê com '*'

  // Sensor ultrassônico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2; // Distância em cm
  
  // Sensor de nível de água
  int waterLevel = analogRead(waterPin);
  
  // Verifica alertas
  if (distance < distanceLimit) {
    Serial.println("ALERTA: distância inferior ao limite!");
  }
  if (waterLevel > waterLevelLimit) {
    Serial.println("ALERTA: nível de água crítico!");
  }
  
  // Exibe resultados
  if (mode == 1 || mode == 2) {
    Serial.print("Distância: ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.print("Nível de água: ");
    Serial.println(waterLevel);
  } else if (mode == 3) {
    Serial.print("Sensor de distância OK – leitura atual: ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.print("Sensor de nível de água OK – leitura atual: ");
    Serial.println(waterLevel);
  }
  
  lastReadTime = millis(); // Atualiza tempo da última leitura
}

// --- Função para exibir resultados ---
void exibeResultados() {
  // Resultados já exibidos em leSensores() para manter a lógica clara
}

//Falta corrijir a parte sobe demanda.
