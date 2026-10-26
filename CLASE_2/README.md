# Clase 2 · Arduino: señales analógicas, sensores y comunicación serial

> **Pregunta de la clase:** ¿Cómo puede Arduino recibir información del entorno físico, convertirla en datos y comunicarlos al computador?

---

## 🎯 Objetivos de aprendizaje

Al finalizar la sesión, cada estudiante podrá:

1. Transferir a una placa Arduino física el semáforo desarrollado en Tinkercad.
2. Distinguir una **salida digital** (`HIGH`/`LOW`) de la **lectura de una entrada analógica**.
3. Comprender por qué una lectura ADC de **10 bits** puede representarse mediante **1024 valores** (de `0` a `1023`).
4. Conectar y leer un sensor de distancia infrarrojo **SHARP** mediante `analogRead()`.
5. Enviar datos al computador usando `Serial.begin()`, `Serial.print()` y `Serial.println()` y observarlos en el **Monitor Serial**.
6. Reconocer, de manera introductoria, para qué sirven la comunicación serial, **I²C** y **SPI**.
---

## 🛠️ Materiales

- Computador con **Arduino IDE** y un cable USB **con transferencia de datos**.
- Placa **Arduino UNO R3 o UNO R4** (u otra placa compatible, revisando sus niveles de tensión).
- Protoboard, cables de conexión, LEDs y resistencias utilizadas en la Clase 01.
- **Sensor de distancia infrarrojo SHARP**, preferentemente `GP2Y0A21YK0F` (10–80 cm) o `GP2Y0A02YK0F` (20–150 cm).
- **Opcional:** sensor ultrasónico `HC-SR04`, para comparar un principio de medición diferente.
- Regla o huincha de medir y un objeto opaco para experimentar con la distancia.

**Antes de energizar el circuito:** verificar los terminales **VCC**, **GND** y **VO** del SHARP según el modelo, conector y hoja de datos. 
No asumir la función de un cable únicamente por su color. Los SHARP indicados requieren alimentación cercana a **5 V** (4,5–5,5 V). 
Si se utiliza una placa de **3,3 V**, revisar compatibilidad y adaptación de niveles. Con el `HC-SR04` convencional, 
su salida `ECHO` de 5 V tampoco debe conectarse directamente a una entrada que admita solo 3,3 V.

---

## 01 · Del simulador al circuito físico 🚦

La clase anterior construimos en Tinkercad un **semáforo vial sincronizado con un semáforo peatonal**. 
Utilizamos `pinMode()`, `digitalWrite()`, `HIGH`, `LOW` y `delay()`, además de calcular las resistencias de los LEDs mediante la ley de Ohm.

**Actividad de inicio**

1. Recuperar el código del semáforo elaborado en la Clase #1.
2. Replicar el montaje en una protoboard conectada a un Arduino físico.
3. En Arduino IDE, seleccionar la **placa** y el **puerto** correspondientes.
4. Cargar el código y comprobar que las dos secuencias se encuentren sincronizadas.
5. Si algo falla, revisar polaridad de LEDs, resistencias, tierra común (`GND`), numeración de pines y conexiones.

---

## 02 · De `HIGH/LOW` a las entradas analógicas 📈

En la Clase 01 usamos salidas digitales con dos estados: `HIGH` y `LOW`. 
Ahora exploraremos una **entrada analógica**: un pin que recibe un voltaje variable y que Arduino transforma en un número mediante un **conversor analógico-digital (ADC)**.

En un **Arduino UNO**, usando la resolución habitual de **10 bits**:

```text
2^10 = 1024 niveles posibles
Rango numérico: 0 a 1023
```

Por ejemplo, con una referencia de lectura de 5 V, un valor bajo representa un voltaje próximo a 0 V y uno alto un voltaje próximo a 5 V. La lectura es una **cuantización del voltaje**, no una medición directa en centímetros.

| Concepto | Ejemplo | Función |
| --- | --- | --- |
| Salida digital | Encender un LED | `digitalWrite(pin, HIGH)` |
| Entrada digital | Leer un pulsador | `digitalRead(pin)` |
| Entrada analógica | Leer la salida de un SHARP | `analogRead(A0)` |

**Importante:** un sensor analógico no tiene por qué producir todos los valores de 0 a 1023. 
El rango observado depende de su salida eléctrica, de la distancia y de la referencia de voltaje de la placa.

---

## 03 · ¿Qué es la comunicación serial? 📞

Hasta ahora, Arduino ejecutaba instrucciones para cambiar el estado de sus pines. 
El **Monitor Serial** permite observar datos enviados desde la placa al computador; es una herramienta de experimentación y diagnóstico.

```cpp
void setup() {
  Serial.begin(9600); // Iniciar comunicación serial
}

void loop() {
  Serial.print("Hola, ");   // Escribe sin terminar la línea
  Serial.println("Arduino"); // Escribe y añade un salto de línea
  delay(1000);
}
```

**Probar:** cargar el código, abrir **Herramientas → Monitor Serial** y configurar la velocidad a **9600 baudios**, de acuerdo con `Serial.begin(9600)`.

- `Serial.begin(9600)`: inicializa el enlace serial a la velocidad indicada.
- `Serial.print()`: envía un número o texto sin salto de línea al final.
- `Serial.println()`: envía un número o texto y termina la línea.
- El **Monitor Serial** muestra los mensajes recibidos; no es un sensor ni el lugar donde Arduino realiza la medición.

### Protocolos de comunicación — primera aproximación

Un **protocolo de comunicación** establece reglas para intercambiar datos entre dispositivos. Por ahora utilizaremos el enlace serial entre Arduino y el computador. Otros protocolos que aparecerán más adelante son:

| Comunicación | Idea general | Aplicación posible |
| --- | --- | --- |
| **Serial / UART** | Intercambio secuencial de datos | Comunicación y diagnóstico con el computador. |
| **I²C** | Bus compartido con datos y reloj | Sensores y pantallas compatibles. |
| **SPI** | Comunicación síncrona con reloj y selección de dispositivo | Lectores de tarjetas SD y otros periféricos. |

### ¿Qué son los baudios (Baud Rate)?

Los **baudios (Bd)** indican la cantidad de símbolos transmitidos por segundo en un sistema de comunicación. En una comunicación serial **UART** convencional, cada símbolo representa un bit; por ello, **9600 baudios equivalen a 9600 bits por segundo (bps)** en la línea de transmisión. Esto no significa que se transmitan 9600 bits de datos útiles por segundo: también se envían bits de inicio y parada.

Históricamente, esta unidad se utilizó en **telegrafía, teletipos y módems telefónicos**. Actualmente, sigue siendo relevante para configurar comunicaciones seriales entre computadores, microcontroladores, sensores y otros dispositivos electrónicos.

#### Velocidades habituales

| Dispositivo o tecnología | Velocidad habitual (baudios) |
|---|---:|
| Módems telefónicos antiguos | 300, 1200 |
| Arduino UNO R3 (Monitor Serial) | 9600, 115200 |
| ESP8266 (comunicación serial) | 115200 |
| ESP8266 (mensajes de arranque del chip) | 74880 |
| ESP32 (comunicación serial) | 115200 |
| MIDI tradicional mediante conexión DIN | 31250 |

> **Importante:** Estos valores son ejemplos frecuentes, **no límites de velocidad** de las placas. Arduino UNO, ESP32 y ESP8266 permiten configurar distintas velocidades. Además, en algunos sistemas de comunicación, como ciertos módems, un símbolo puede representar más de un bit: por eso **baudios y bps no son siempre equivalentes**.

#### ¿Cómo configuramos la velocidad en Arduino?

En el código utilizamos `Serial.begin()` dentro de `setup()`:

```cpp
void setup() {
  Serial.begin(9600); //fijamos la velocidad
}

void loop() {
  Serial.println("Hola mundo"); //fijamos el registro de dato o mensaje
  delay(1000);
}
```

El valor `9600` establece la velocidad de transmisión. Para leer correctamente los mensajes, seleccionamos **la misma velocidad en el Monitor Serial** de Arduino IDE.

Si las velocidades no coinciden, pueden aparecer caracteres ilegibles o no visualizarse correctamente los mensajes.

**Aclaración:** ni el SHARP analógico ni el HC-SR04 convencional necesitan que programemos I²C o SPI. No profundizaremos todavía en estos protocolos.

---

## 04 · Ejercicio principal: sensor infrarrojo SHARP ✋ 📹

Los sensores SHARP `GP2Y0A21YK0F` y `GP2Y0A02YK0F` utilizan luz infrarroja y **triangulación** para obtener una salida de voltaje relacionada con la distancia al objeto. La relación entre voltaje y distancia es **no lineal**.

| Sensor | Intervalo nominal de distancia | Tipo de salida |
| --- | --- | --- |
| `GP2Y0A21YK0F` | 10–80 cm | Voltaje analógico |
| `GP2Y0A02YK0F` | 20–150 cm | Voltaje analógico |

### Conexión básica a Arduino UNO

| Terminal del SHARP | Arduino |
| --- | --- |
| `VCC` | `5V` |
| `GND` | `GND` |
| `VO` (salida de voltaje) | `A0` |

> **Revisar el pinout del modelo concreto antes de conectar.** La disposición de terminales puede interpretarse de forma incorrecta si el conector se observa desde el frente o desde atrás.

### Código base: lectura de un sensor

```cpp
const int PIN_SENSOR = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int lectura = analogRead(PIN_SENSOR);

  Serial.print("Valor ADC: ");
  Serial.println(lectura);

  delay(100);
}
```

**¿Qué está pasando en el código?**

1. `const int PIN_SENSOR = A0;` define el pin del sensor como una constante.
2. `analogRead(PIN_SENSOR)` solicita al ADC el valor digital correspondiente al voltaje recibido.
3. `int lectura` almacena el número leído.
4. `Serial.print()` agrega una etiqueta reconocible.
5. `Serial.println(lectura)` muestra el valor y pasa a una línea nueva.
6. `delay(100)` hace una pequeña pausa entre lecturas.

### Experimentación

1. Abrir el **Monitor Serial** a 9600 baudios.
2. Colocar un objeto frente al sensor dentro de su intervalo de funcionamiento.
3. Acercar y alejar el objeto lentamente, observando los valores.
4. Repetir las mediciones con objetos de distintas superficies o colores.
5. Registrar al menos **tres distancias aproximadas** medidas con una regla y sus respectivas lecturas ADC.

| Distancia (cm) | Lectura ADC (0–1023) | Observaciones |
| ---: | ---: | --- |
| | | |
| | | |
| | | |

---

## 5. Convertir ADC a centímetros

La curva del sensor se puede aproximar mediante una **función potencial**:

```text
distancia_cm = a × (lecturaADC ^ b)
```

Donde `a` y `b` son constantes que se obtienen experimentalmente con lecturas a distancias conocidas.

En su tutorial, Naylamp en [Tutorial Sensor de Distancia SHARP](https://naylampmechatronics.com/blog/55_tutorial-sensor-de-distancia-sharp.html) utiliza dos puntos de referencia para el **GP2Y0A21**:

| Distancia medida con regla | Lectura ADC de ejemplo |
|---:|---:|
| 15 cm | 350 |
| 50 cm | 129 |

A partir de esas mediciones obtiene una relación aproximada:

```text
distancia_cm = 17569.7 × (lecturaADC ^ -1.2062)
```

En Arduino podemos escribirla con `pow()`, una función que calcula potencias:

```cpp
float distanciaCm = 17569.7 * pow(lecturaADC, -1.2062);
```

**Ejemplos aproximados con esta fórmula** (no son valores universales de todos los sensores):

| ADC | Distancia calculada |
|---:|---:|
| 350 | 15.0 cm |
| 250 | 22.5 cm |
| 200 | 29.5 cm |
| 129 | 50.0 cm |
| 100 | 68.0 cm |

```cpp
const int pinSharp = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int lecturaADC = analogRead(pinSharp);

  Serial.print("ADC: ");
  Serial.print(lecturaADC);

  // Evita calcular una potencia de exponente negativo con ADC = 0.
  if (lecturaADC > 0) {
    float distanciaCm = 17569.7 * pow(lecturaADC, -1.2062);

    Serial.print(" | Distancia estimada: ");
    Serial.print(distanciaCm, 1);  // Un decimal
    Serial.println(" cm");
  } else {
    Serial.println(" | Sin lectura valida");
  }

  delay(100);
}
```

**En el código:**

- `analogRead(A0)` captura el dato digital del ADC.
- `pow(lecturaADC, -1.2062)` aplica la curva matemática de calibración.
- `float` almacena la distancia con decimales.
- `Serial.print()` escribe en la misma línea, mientras que `Serial.println()` finaliza esa línea.
  
---
