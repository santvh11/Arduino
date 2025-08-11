CONTROL BANDA TRANSPORTADORA

// --- 1. DEFINICIÓN DE PINES ---
// --- Entradas ---
// Sensores de posición
const int SENSOR1_PIN = 30;
const int SENSOR2_PIN = 31;
const int SENSOR3_PIN = 32;
const int SENSOR4_PIN = 33;

// Switches de selección de destino
const int SW1_PIN = 37;
const int SW2_PIN = 36;

// Pulsadores
const int START_BUTTON_PIN = 35;
const int PAUSE_BUTTON_PIN = 34; // Pulsador de "Detener"

// --- Salidas ---
// Control del motor (Girar derecha/izquierda)
const int MOTOR_RIGHT_PIN = 22;
const int MOTOR_LEFT_PIN = 23;

// --- 2. MÁQUINA DE ESTADOS ---
enum State {
  IDLE,             // Esperando para iniciar
  MOVING_TO_DEST,   // Moviéndose hacia el destino
  WAITING,          // Esperando en el destino
  RETURNING,        // Regresando al origen
  PAUSED,           // Pausado por el usuario
  ERROR             // Estado de error (origen = destino)
};

State currentState = IDLE;
State lastStateBeforePause;

// --- 3. VARIABLES GLOBALES ---
int originSensor = 0;
int destinationSensor = 0;
unsigned long waitStartTime = 0;
const long waitDuration = 5000;

void setup() {
  Serial.begin(9600);

  // --- Configuración de Pines de Entrada ---
  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(SENSOR3_PIN, INPUT);
  pinMode(SENSOR4_PIN, INPUT);
  pinMode(SW1_PIN, INPUT);
  pinMode(SW2_PIN, INPUT);
  pinMode(START_BUTTON_PIN, INPUT);
  pinMode(PAUSE_BUTTON_PIN, INPUT);

  // --- Configuración de Pines de Salida ---
  pinMode(MOTOR_RIGHT_PIN, OUTPUT);
  pinMode(MOTOR_LEFT_PIN, OUTPUT);

  stopMotor();
  Serial.println("Estado: Detenido. Coloque la pieza y presione START.");
}

void loop() {
  // --- Lectura de Entradas ---
  bool startPressed = digitalRead(START_BUTTON_PIN);
  bool pausePressed = digitalRead(PAUSE_BUTTON_PIN);

  // --- Lógica de la Máquina de Estados ---
  switch (currentState) {
    case IDLE:
      if (startPressed) {
        handleStart();
        delay(200);
      }
      break;

    case MOVING_TO_DEST:
      if (isPieceAt(destinationSensor)) {
        changeState(WAITING);
      }
      if (pausePressed) {
        changeState(PAUSED);
        delay(200);
      }
      break;

    case WAITING:
      handleWaiting();
      break;

    case RETURNING:
      if (isPieceAt(originSensor)) {
        changeState(IDLE);
      }
      if (pausePressed) {
        changeState(PAUSED);
        delay(200);
      }
      break;

    case PAUSED:
      if (startPressed && !pausePressed) {
        resumeFromPause();
        delay(200);
      }
      break;

    case ERROR:
      // El sistema se queda aquí hasta un reinicio o una nueva orden válida.
      if (startPressed) {
        handleStart(); // Intenta iniciar de nuevo
        delay(200);
      }
      break;
  }
}

// --- 4. FUNCIONES DE LÓGICA PRINCIPAL ---

void handleStart() {
  originSensor = getCurrentPosition();
  destinationSensor = getDestination();


  if (originSensor == 0) {
    Serial.println("Error: No se detecta la pieza en ningun sensor.");
    return;
  }

  if (originSensor == destinationSensor) {
    changeState(ERROR);
    return;
  }
  
  changeState(MOVING_TO_DEST);
}

void handleWaiting() {
  // Imprimir tiempo restante cada segundo
  unsigned long currentTime = millis();
  if (currentTime - waitStartTime < waitDuration) {
    if ((currentTime / 1000) != ((currentTime - 10) / 1000)) {
       long remainingSeconds = (waitDuration - (currentTime - waitStartTime)) / 1000;
       Serial.print("Tiempo de espera restante: ");
       Serial.print(remainingSeconds + 1);
       Serial.println("s");
    }
  } else {
    changeState(RETURNING);
  }
}

void resumeFromPause() {
  // Vuelve al estado en el que estaba antes de la pausa
  changeState(lastStateBeforePause);
}

// --- 5. FUNCIONES AUXILIARES ---

void changeState(State newState) {
  // Guarda el estado actual si vamos a pausar
  if (newState == PAUSED) {
    lastStateBeforePause = currentState;
  }
  
  currentState = newState;

  // Realiza acciones al entrar a un nuevo estado
  switch (currentState) {
    case IDLE:
      stopMotor();
      Serial.println("Estado: Detenido. Proceso completado. Listo para iniciar.");
      break;
    case MOVING_TO_DEST:
      Serial.print("Moviendo pieza del sensor ");
      Serial.print(originSensor);
      Serial.print(" al sensor ");
      Serial.println(destinationSensor);
      // Mover en dirección al destino
      (destinationSensor > originSensor) ? moveLeft() : moveRight();
      break;
    case WAITING:
      stopMotor();
      Serial.println("Estado: En destino. Iniciando espera de 5 segundos.");
      waitStartTime = millis(); // Inicia el temporizador
      break;
    case RETURNING:
      Serial.print("Regresando al origen (Sensor ");
      Serial.print(originSensor);
      Serial.println(")");
      // Mover en dirección opuesta
      (originSensor > destinationSensor) ? moveLeft() : moveRight();
      break;
    case PAUSED:
      stopMotor();
      Serial.println("Estado: PAUSADO. Presione START para continuar.");
      break;
    case ERROR:
      stopMotor();
      Serial.println("Error: El origen y el destino son el mismo. La banda no se movera.");
      break;
  }
}

int getCurrentPosition() {
  // Devuelve el número del sensor que está en HIGH (activo)
  if (digitalRead(SENSOR1_PIN)) return 1;
  if (digitalRead(SENSOR2_PIN)) return 2;
  if (digitalRead(SENSOR3_PIN)) return 3;
  if (digitalRead(SENSOR4_PIN)) return 4;
  return 0; // 0 si no hay pieza
}

int getDestination() {
  // Lee el estado de los switches. 'true' si está en HIGH (activo).
  bool sw1 = digitalRead(SW1_PIN);
  bool sw2 = digitalRead(SW2_PIN);

  // Lógica de destino basada en la tabla de verdad
  if (sw1 == false && sw2 == false) { // SW1=0, SW2=0
    return 1; // Destino S1
  } else if (sw1 == false && sw2 == true) { // SW1=0, SW2=1
    return 2; // Destino S2
  } else if (sw1 == true && sw2 == false) { // SW1=1, SW2=0
    return 3; // Destino S3
  } else if (sw1 == true && sw2 == true) { // SW1=1, SW2=1
    return 4; // Destino S4
  }
  
  return 0; // Nunca debería llegar aquí
}

bool isPieceAt(int sensorNumber) {
  // Comprueba si el sensor especificado está en HIGH (activo)
  switch(sensorNumber) {
    case 1: return digitalRead(SENSOR1_PIN);
    case 2: return digitalRead(SENSOR2_PIN);
    case 3: return digitalRead(SENSOR3_PIN);
    case 4: return digitalRead(SENSOR4_PIN);
    default: return false;
  }
}

// --- 6. FUNCIONES DE CONTROL DEL MOTOR ---
void stopMotor() {
  digitalWrite(MOTOR_RIGHT_PIN, LOW);
  digitalWrite(MOTOR_LEFT_PIN, LOW);
}

void moveRight() {
  digitalWrite(MOTOR_LEFT_PIN, LOW);
  digitalWrite(MOTOR_RIGHT_PIN, HIGH);
}

void moveLeft() {
  digitalWrite(MOTOR_RIGHT_PIN, LOW);
  digitalWrite(MOTOR_LEFT_PIN, HIGH);
}