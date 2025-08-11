MOTOR DC

// --- 1. DEFINICIÓN DE PINES
// Botones
const int P1_PIN = 35; // Pulsador para subir
const int P2_PIN = 34; // Pulsador para bajar (y pausar subida)
const int P3_PIN = 33; // Pulsador para pausar bajada

// Microswitches
const int USW1_PIN = 37; // Microswitch de arriba (uSW1)
const int USW2_PIN = 36; // Microswitch de abajo (uSW2)


// LEDs
const int LED1_PIN = 24; // LED indica "Subiendo"
const int LED2_PIN = 25; // LED indica "Bajando"
const int LED3_PIN = 26; // LED indica "Pausa"

// Motor
const int MOTOR_A_PIN = 22; // Pin para subir motor
const int MOTOR_B_PIN = 23; // Pin para bajar motor

// --- 2. MÁQUINA DE ESTADOS ---
enum State {
  IDLE,         // El motor está detenido (extremo o inicio).
  MOVING_UP,    // El motor subiendo.
  MOVING_DOWN,  // El motor bajando.
  PAUSED_UP,    // El motor estaba subiendo y pausó.
  PAUSED_DOWN   // El motor estaba bajando y pausó.
};

State currentState = IDLE;

void setup() {
  // Inicializa la comunicación serie para depuración y monitorización.
  Serial.begin(9600);

  // --- Configuración de Pines ---
  pinMode(P1_PIN, INPUT);
  pinMode(P2_PIN, INPUT);
  pinMode(P3_PIN, INPUT);
  pinMode(USW1_PIN, INPUT);
  pinMode(USW2_PIN, INPUT);

  // LEDs como salidas.
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  // Pines del motor como salidas.
  pinMode(MOTOR_A_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);

  // Al iniciar, nos aseguramos de que todo esté apagado.
  stopMotor();
  updateLedsAndSerial();
}

void loop() {
  // Leemos el estado de todos los botones y switches en cada ciclo.
  bool p1Pressed = !!digitalRead(P1_PIN);
  bool p2Pressed = !!digitalRead(P2_PIN);
  bool p3Pressed = !!digitalRead(P3_PIN);
  bool usw1Active = !!digitalRead(USW1_PIN); // Switch de arriba activado
  bool usw2Active = !!digitalRead(USW2_PIN); // Switch de abajo activado

  // El bloque switch-case ejecuta el código correspondiente al estado actual.
  switch (currentState) {
    case IDLE:
      if (p1Pressed && !usw1Active) { // Si se presiona P1 y no estamos ya arriba...
        changeState(MOVING_UP); // ...empezamos a subir.
      } else if (p2Pressed && !usw2Active) { // Si se presiona P2 y no estamos ya abajo...
        changeState(MOVING_DOWN); // ...empezamos a bajar.
      }
      break;

    case MOVING_UP:
      // Si estamos subiendo, comprobamos si llegamos al final o si se pausa.
      if (usw1Active) { // Si se activa el switch de arriba...
        changeState(IDLE); // ...nos detenemos.
      } else if (p2Pressed) { // P2 pausa la subida.
        changeState(PAUSED_UP); // ...entramos en modo pausa.
        delay(200); // Pequeño retardo para evitar rebotes o dobles lecturas.
      }
      break;

    case MOVING_DOWN:
      if (usw2Active) { // Si se activa el switch de abajo...
        changeState(IDLE); // ...nos detenemos.
      } else if (p3Pressed) { // P3 pausa la bajada.
        changeState(PAUSED_DOWN); // ...entramos en modo pausa.
        delay(200); // Pequeño retardo para evitar rebotes.
      }
      break;

    case PAUSED_UP:
      if (p1Pressed) { // Si se presiona P1 mientras estamos en pausa de subida...
        changeState(MOVING_UP); // ...continuamos subiendo.
        delay(200);
      }
      break;

    case PAUSED_DOWN:
      if (p2Pressed) { // Si se presiona P2 mientras estamos en pausa de bajada...
        changeState(MOVING_DOWN); // ...continuamos bajando.
        delay(200);
      }
      break;
  }
}

// --- 3. FUNCIONES AUXILIARES ---

// Cambia el estado de la máquina y actualiza los LEDs y el monitor serie.
void changeState(State newState) {
  currentState = newState;
  updateLedsAndSerial();

  // Activa el motor según el nuevo estado
  switch (currentState) {
    case MOVING_UP:
      moveUp();
      break;
    case MOVING_DOWN:
      moveDown();
      break;
    case IDLE:
    case PAUSED_UP:
    case PAUSED_DOWN:
      stopMotor();
      break;
  }
}

// Actualiza el estado de los LEDs y envía el estado actual al monitor serie. 
// Se asegura de que solo un LED esté encendido a la vez.
void updateLedsAndSerial() {
  // Primero, apaga todos los LEDs para asegurar un estado limpio.
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  // Luego, enciende el LED correspondiente y envía el mensaje por el puerto serie.
  switch (currentState) {
    case IDLE:
      Serial.println("Accion: Detenido");
      // Ningún LED se enciende cuando está detenido.
      break;
    case MOVING_UP:
      digitalWrite(LED1_PIN, HIGH); // Enciende LED de subida
      Serial.println("Accion: Subiendo");
      break;
    case MOVING_DOWN:
      digitalWrite(LED2_PIN, HIGH); // Enciende LED de bajada
      Serial.println("Accion: Bajando");
      break;
    case PAUSED_UP:
    case PAUSED_DOWN:
      digitalWrite(LED3_PIN, HIGH); // Enciende LED de pausa
      Serial.println("Accion: Pausa");
      break;
  }
}

// --- Funciones de Control del Motor ---

void stopMotor() {
  digitalWrite(MOTOR_A_PIN, LOW);
  digitalWrite(MOTOR_B_PIN, LOW);
}

void moveUp() {
  digitalWrite(MOTOR_A_PIN, HIGH);
  digitalWrite(MOTOR_B_PIN, LOW);
}

void moveDown() {
  digitalWrite(MOTOR_A_PIN, LOW);
  digitalWrite(MOTOR_B_PIN, HIGH);
}