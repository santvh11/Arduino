PLUMILLA DE PARQUEADERO

// --- 1. DEFINICIÓN DE PINES ---
// --- Entradas ---
const int S1_PIN = 33; // Sensor de entrada
const int S2_PIN = 32; // Sensor de salida
const int M1_PIN = 31; // Sensor de plumilla abajo
const int M2_PIN = 30; // Sensor de plumilla arriba

// --- Salidas ---
const int OPEN_BARRIER_PIN = 22; // Abrir Plumilla
const int CLOSE_BARRIER_PIN = 23; // Cerrar Plumilla
const int RED_LED_PIN = 24;
const int GREEN_LED_PIN = 25;

// --- 2. MÁQUINA DE ESTADOS ---
enum State {
  IDLE,
  ENTERING_OPENING,
  ENTERING_WAITING,
  ENTERING_CLOSING,
  EXITING_OPENING,
  EXITING_WAITING,
  EXITING_CLOSING
};

State currentState = IDLE;

// --- 3. VARIABLES GLOBALES ---
int carCount = 0;
unsigned long waitStartTime = 0;
const long waitDuration = 5000;

void setup() {
  Serial.begin(9600);

  // --- Configuración de Pines de Entrada ---
  pinMode(S1_PIN, INPUT);
  pinMode(S2_PIN, INPUT);
  pinMode(M1_PIN, INPUT);
  pinMode(M2_PIN, INPUT);

  // --- Configuración de Pines de Salida ---
  pinMode(OPEN_BARRIER_PIN, OUTPUT);
  pinMode(CLOSE_BARRIER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  stopBarrier();
  updateLeds();
  printStatus("Detenida");
}

void loop() {
  // --- Lectura de Sensores ---
  bool s1_active = digitalRead(S1_PIN);
  bool s2_active = digitalRead(S2_PIN);
  bool m1_active = digitalRead(M1_PIN); // Plumilla abajo
  bool m2_active = digitalRead(M2_PIN); // Plumilla arriba

  // --- Lógica de la Máquina de Estados ---
  switch (currentState) {
    case IDLE:
      // Si un carro llega al sensor de entrada y la plumilla está abajo
      if (s1_active && m1_active) {
        changeState(ENTERING_OPENING); // Comienza el flujo de entrada
      }
      // Si un carro llega al sensor de salida y la plumilla está abajo
      else if (s2_active && m1_active) {
        changeState(EXITING_OPENING); // Comienza el flujo de salida
      }
      break;

    // --- Flujo de Entrada ---
    case ENTERING_OPENING:
      printStatus("Subiendo");
      if (m2_active) { // Si la plumilla llega arriba
        changeState(ENTERING_WAITING);
      }
      break;
    case ENTERING_WAITING:
      printStatus("Detenida (Arriba)");
      // Si han pasado 5 segundos y el carro ya no está en el sensor S1
      if (millis() - waitStartTime >= waitDuration && !s1_active) {
        changeState(ENTERING_CLOSING);
      }
      break;
    case ENTERING_CLOSING:
      printStatus("Bajando");
      if (m1_active) { // Si la plumilla llega abajo
        carCount++; // Incrementa el contador de carros
        changeState(IDLE);
        printStatus("Espera");
      }
      break;

    // --- Flujo de Salida ---
    case EXITING_OPENING:
      printStatus("Subiendo");
      if (m2_active) { // Si la plumilla llega arriba
        changeState(EXITING_WAITING);
      }
      break;
    case EXITING_WAITING:
      printStatus("Detenida (Arriba)");
      // Si han pasado 5 segundos y el carro ya no está en el sensor S2
      if (millis() - waitStartTime >= waitDuration && !s2_active) {
        changeState(EXITING_CLOSING);
      }
      break;
    case EXITING_CLOSING:
      printStatus("Bajando");
      if (m1_active) { // Si la plumilla llega abajo
        carCount--; // Decrementa el contador de carros
        printStatus("Espera");
        changeState(IDLE);
      }
      break;
  }
}

// --- 4. FUNCIONES AUXILIARES ---

void changeState(State newState) {
  currentState = newState;
  
  // Acciones al entrar a un nuevo estado
  switch (currentState) {
    case IDLE:
      stopBarrier();
      break;
    case ENTERING_OPENING:
    case EXITING_OPENING:
      openBarrier();
      break;
    case ENTERING_WAITING:
    case EXITING_WAITING:
      stopBarrier();
      waitStartTime = millis(); // Inicia el temporizador de 5 segundos
      break;
    case ENTERING_CLOSING:
    case EXITING_CLOSING:
      closeBarrier();
      break;
  }
  updateLeds(); // Actualiza los LEDs en cada cambio de estado
}

void updateLeds() {
  // El bombillo rojo se enciende mientras la plumilla este arriba o bajando
  if (currentState == ENTERING_OPENING || currentState == ENTERING_CLOSING ||
      currentState == EXITING_OPENING || currentState == EXITING_CLOSING) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
  // El bombillo verde cuando la plumilla este arriba
  else if (currentState == ENTERING_WAITING || currentState == EXITING_WAITING) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  // Ambos apagados en reposo
  else { // IDLE
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }
}

void printStatus(String action) {
  Serial.print("Accion: ");
  Serial.print(action);
  Serial.print(" - Carros en el parqueadero: ");
  Serial.println(carCount);
}

// --- 5. FUNCIONES DE CONTROL DE LA PLUMILLA ---
void stopBarrier() {
  digitalWrite(OPEN_BARRIER_PIN, LOW);
  digitalWrite(CLOSE_BARRIER_PIN, LOW);
}

void openBarrier() {
  digitalWrite(OPEN_BARRIER_PIN, HIGH);
  digitalWrite(CLOSE_BARRIER_PIN, LOW);
}

void closeBarrier() {
  digitalWrite(OPEN_BARRIER_PIN, LOW);
  digitalWrite(CLOSE_BARRIER_PIN, HIGH);
}