/*
  Semaforo vial y peatonal - Aplicaciones Tecnologicas B

  Basado en la secuencia observada en Semaforo_051026.mp4.
  Los tiempos son APROXIMADOS y se pueden modificar para la clase.

  LEDs viales: rojo (12), amarillo (11), verde (10).
  LEDs peatonales: rojo (9), verde (8).

  Cada LED fisico necesita su resistencia limitadora de corriente.
  Ejemplo educativo: no utilizar para controlar semaforos reales.
*/

// 1. Definir los pines de los LEDs
const int LED_1 = 12;  // Vial rojo
const int LED_2 = 11;  // Vial amarillo
const int LED_3 = 10;  // Vial verde
const int LED_4 = 9;   // Peatonal rojo
const int LED_5 = 8;   // Peatonal verde

// 2. Definir los tiempos (en milisegundos)
const unsigned long TIEMPO_PEATON_VERDE = 43000UL;  // 43 segundos
const int PARPADEOS_PEATON_VERDE = 8;               // 8 parpadeos
const unsigned long TIEMPO_PARPADEO = 500UL;       // 0,5 s apagado / 0,5 s encendido
const unsigned long TIEMPO_AMBOS_ROJO = 2000UL;    // 2 segundos
const unsigned long TIEMPO_VIAL_VERDE = 55000UL;   // 55 segundos
const unsigned long TIEMPO_VIAL_AMARILLO = 3000UL; // 3 segundos

void setup() {
  // 3. Configurar los cinco LEDs como salidas digitales
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);

  // Comenzar con ambos semaforos en rojo
  digitalWrite(LED_1, HIGH);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, HIGH);
  digitalWrite(LED_5, LOW);
}

void loop() {
  // ETAPA 1: Autos detenidos, peatones pueden cruzar
  digitalWrite(LED_1, HIGH); // Vial rojo ENCENDIDO
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);  // Peatonal rojo APAGADO
  digitalWrite(LED_5, HIGH); // Peatonal verde ENCENDIDO
  delay(TIEMPO_PEATON_VERDE);

  // ETAPA 2: Verde peatonal intermitente (aviso de termino)
  for (int i = 0; i < PARPADEOS_PEATON_VERDE; i++) {
    digitalWrite(LED_5, LOW);
    delay(TIEMPO_PARPADEO);
    digitalWrite(LED_5, HIGH);
    delay(TIEMPO_PARPADEO);
  }

  // ETAPA 3: Ambos en rojo antes de dar paso a los autos
  digitalWrite(LED_5, LOW);  // Peatonal verde APAGADO
  digitalWrite(LED_4, HIGH); // Peatonal rojo ENCENDIDO
  delay(TIEMPO_AMBOS_ROJO);  // Vial sigue en rojo

  // ETAPA 4: Autos avanzan; peatones esperan
  digitalWrite(LED_1, LOW);  // Vial rojo APAGADO
  digitalWrite(LED_3, HIGH); // Vial verde ENCENDIDO
  delay(TIEMPO_VIAL_VERDE);

  // ETAPA 5: Precaucion para los autos
  digitalWrite(LED_3, LOW);  // Vial verde APAGADO
  digitalWrite(LED_2, HIGH); // Vial amarillo ENCENDIDO
  delay(TIEMPO_VIAL_AMARILLO);

  // ETAPA 6: Ambos en rojo antes de habilitar el cruce peatonal
  digitalWrite(LED_2, LOW);  // Vial amarillo APAGADO
  digitalWrite(LED_1, HIGH); // Vial rojo ENCENDIDO
  delay(TIEMPO_AMBOS_ROJO);  // Peatonal sigue en rojo

  // Se repite loop(): rojo vial + verde peatonal
}
