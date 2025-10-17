//carrega a biblioteca do ultrasonico
#include <Ultrasonic.h>

//carrega a biblioteca do teclado
#include <Key.h>
#include <Keypad.h>

//Define os pinos para o trigger e echo
#define pino_trigger 48
#define pino_echo 50

//definindo int, bools, floats e char que serão utilizados no codigo
const int MAX_DIGITOS = 6; 
char numeroBuffer[MAX_DIGITOS + 2];
int indiceBuffer = 0;
float nivelCriticoRua = 0.0;
float nivelCriticoRio = 0.0;
bool aguardandoEntradaNumerica = false;
char modoCriticoSelecionado = ' ';
int pinoSensorAgua = A10;
int joystickX = A4;
int joystickY = A2;
int nivelRua;
int nivelRio;
bool sobdemanda = false;
bool critico = false;
Ultrasonic ultrasonic(pino_trigger, pino_echo);

//Configurações do teclado
const byte LINHAS = 4; 
const byte COLUNAS = 4;

const char TECLAS_MATRIZ[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const byte PINOS_LINHAS[LINHAS] = {22, 24, 26, 28};
const byte PINOS_COLUNAS[COLUNAS] = {30, 32, 34, 36};

Keypad teclado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS);

// --------------------------------------------------------------------------------
// FUNÇÕES AUXILIARES PARA LEITURA DE FLOAT
// --------------------------------------------------------------------------------

void resetBuffer() {
    memset(numeroBuffer, 0, sizeof(numeroBuffer));// Limpa o array
    indiceBuffer = 0;
    aguardandoEntradaNumerica = false;
    modoCriticoSelecionado = ' ';
}
// Processar a entrada dos números digitados no critico
void processarEntradaNumerica(float & variavelDestino, const char* nomeVariavel) {
    if (indiceBuffer > 0) {
        float valorDigitado = atof(numeroBuffer);
        // Armazena o valor na variável de destino
        variavelDestino = valorDigitado;

        Serial.print("\n[CONFIRMADO] Valor '");
        Serial.print(nomeVariavel);
        Serial.print("' salvo: ");
        Serial.println(variavelDestino, 3);
        Serial.println("------------------------------------------");
    } else {
        Serial.println("\n[AVISO] Nenhum numero digitado. Valor anterior mantido.");
    }
    // Reseta o buffer e flags para a próxima operação
    resetBuffer();
}

void setup() {
  Serial.begin(9600); // Inicia a comunicação serial com a velocidade de 9600 bps
  //Texto menu do sistema de monitoriamento
  Serial.println("Sistema de Monitoramento Inicializado.");
  Serial.println("Selecione o modo:");
  Serial.println("DIREITA: Sob demanda");
  Serial.println("ESQUERDA: Continuo");
  Serial.println("PARA CIMA: Teste");
  Serial.println("PARA BAIXO: Digite outro modo critico");
}

void loop() {
    nivelRua = analogRead(pinoSensorAgua);// Lê o valor do sensor
    int valX = analogRead(joystickX); // Lê os valores dos eixos X
    int valY = analogRead(joystickY); // Lê os valores dos eixos Y
    long microsec = ultrasonic.timing(); //Conversão do ultrasonico
    nivelRio = ultrasonic.convert(microsec, Ultrasonic::CM); //Conversão do ultrasonico
    char teclaPressionada = teclado.getKey(); // Lê a tecla
    
    // ==========================================================
    // PRIORIDADE 1: ENTRADA NUMÉRICA (MODO CRÍTICO)
    // ==========================================================
    // if das condições de leitura do teclado no modo critico.
    if (aguardandoEntradaNumerica) {
        if (teclaPressionada) {
            // Se for dígito (0-9) ou ponto decimal ('*')
            if ((teclaPressionada >= '0' && teclaPressionada <= '9') || teclaPressionada == '*') {
                // Trata o ponto decimal ('*')
                if (teclaPressionada == '*') {
                    if (strchr(numeroBuffer, '.') == NULL && indiceBuffer < MAX_DIGITOS) {
                        numeroBuffer[indiceBuffer++] = '.';
                        numeroBuffer[indiceBuffer] = '\0';
                        Serial.print('.');
                    }
                }
                // Trata os dígitos
                else if (indiceBuffer < MAX_DIGITOS) {
                    numeroBuffer[indiceBuffer++] = teclaPressionada;
                    numeroBuffer[indiceBuffer] = '\0';
                    Serial.print(teclaPressionada);
                }
            }
            // Se for ENTER ('#')
            else if (teclaPressionada == '#') {
                if (modoCriticoSelecionado == 'A') {
                    processarEntradaNumerica(nivelCriticoRua, "Nivel Critico Rua");
                } else if (modoCriticoSelecionado == 'B') {
                    processarEntradaNumerica(nivelCriticoRio, "Nivel Critico Rio");
                }
                aguardandoEntradaNumerica = false;
                critico = false;
            }
            // Se for CANCELAR ('D')
            else if (teclaPressionada == 'D') {
                Serial.println("\n[CANCELADO] Entrada de nivel critico cancelada.");
                resetBuffer();
                aguardandoEntradaNumerica = false;
                critico = false; // Sai do Modo Crítico ao cancelar
            }
        }
        delay(1000);
        return;
    }
    
    // ==========================================================
    // PARTE 1: MUDANÇA DE ESTADO (Ativação dos Modos)
    // ==========================================================
    
    // Toque para a DIREITA (valX > 900) = Seleciona MODO SOB DEMANDA
    if (valX > 900) { 
        if (sobdemanda == false) {
            Serial.println("MODO SOB DEMANDA ATIVADO");
            Serial.println("Digite A no teclado para ter o monitoramento da rua e B para o monitoramento do rio.");
            sobdemanda = true;
        }
    }
    
    // Toque para a ESQUERDA (valX < 100) = Seleciona MODO CONTÍNUO
    if (valX < 100) {
        Serial.println("MODO CONTÍNUO ATIVADO");
        Serial.print("O nivel da rua em centimetros é: "); 
        Serial.println(nivelRua);
        Serial.print("O nivel do rio em centimetros é: ");
        Serial.println(nivelRio);
    }
    
    // Ativação do modo teste (valY < 100)
    if (valY < 100) {
        Serial.println("Modo de teste");
        Serial.print("Nivel Rio: ");
        Serial.println(nivelRio);
        Serial.print("Nivel Rua: ");
        Serial.println(nivelRua);
    }
    
    // Função editar nivel critico (valY > 900)
    if (valY > 900) { 
        if (critico == false){
            Serial.println("\n--- MODO CRÍTICO ATIVADO ---");
            Serial.print("Critico Rua atual: "); 
            Serial.println(nivelCriticoRua, 3);
            Serial.print("Critico Rio atual: "); 
            Serial.println(nivelCriticoRio, 3);
            Serial.println("Aperte A (Rua) ou B (Rio) para definir o novo nível.");
            critico = true;
        }
    }
    
    // ==========================================================
    // PARTE 2: EXECUÇÃO DO MODO ATUAL
    // ==========================================================
    
    // Condições do modo sobdemanda
    if(sobdemanda == true) {
        if (teclaPressionada == 'A') {
            Serial.print("Nível da rua em centimetros: ");
            Serial.println(nivelRua);
        }
        else if(teclaPressionada == 'B') {
            Serial.print("Nível do rio em centimetros: ");
            Serial.println(nivelRio);
        }
        else if (teclaPressionada == 'D') {
            Serial.println("MODO SOB DEMANDA DESATIVADO. Voltando ao modo normal.");
            sobdemanda = false;
        }
        else if (teclaPressionada != NO_KEY && teclaPressionada != 'A' && teclaPressionada != 'B') {
            Serial.println("Por favor, digite A ou B para continuar ou pressione 'D' para sair do modo.");
        }
    }
    
    // Modo Crítico, mas nenhum número foi digitado
    if(critico == true && !aguardandoEntradaNumerica) { 
        if (teclaPressionada == 'A') {  
            Serial.print("\n[RUA] Digite o novo nivel critico em cm (Use '*' para o ponto) considerando a distancia do inicio do sensor até a altura maxima e '#' para confirmar: ");
            resetBuffer();
            aguardandoEntradaNumerica = true;
            modoCriticoSelecionado = 'A';  
        }
        else if(teclaPressionada == 'B') {
            Serial.print("\n[RIO] Digite o novo nivel critico em cm (Use '*' para o ponto) considerando a distancia da margem do rio ate a superficie da água e '#' para confirmar: ");
            resetBuffer();
            aguardandoEntradaNumerica = true;
            modoCriticoSelecionado = 'B';
        }
        else if (teclaPressionada == 'D') {
            Serial.println("\nMODO CRITICO DESATIVADO. Voltando ao modo normal.");
            critico = false;
        }
        else if (teclaPressionada != NO_KEY) {
            Serial.println("Por favor, digite A (Rua), B (Rio) para entrar com o valor, ou D para sair.");
        }
    }
    
    
    delay(1000);
}

//Alunos e participação:
//João Pedro Nicacio - 202502813686 - TA
//Julia Valente - 202501000673 - TA
//Giovanna Perrone - 202504193111 - TA
//Maria Eduarda Perpetuo - 202501001262 - TA
