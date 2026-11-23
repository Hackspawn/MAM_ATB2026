# Clase 06 — Arduino + Processing, Kinect y comunicación audiovisual en tiempo real 🎥

## Objetivos de aprendizaje

Al finalizar la sesión, cada estudiante podrá:

1. Reconocer la diferencia entre **transmitir datos numéricos** y **compartir imágenes o video** entre aplicaciones.
2. Enviar lecturas de un sensor SHARP desde **Arduino** a **Processing** mediante comunicación **Serial**.
3. Recibir e interpretar esas lecturas en Processing y utilizar `map()` para modificar la **opacidad** de un elemento visual.
4. Identificar las posibilidades de captura **RGB**, **profundidad** y **movimiento** mediante Kinect y su visualización en Processing.
5. Diferenciar **Syphon**, para compartir imágenes entre aplicaciones compatibles en un mismo equipo macOS, de **NDI**, para transportar video mediante una red IP.
6. Reconocer aplicaciones de estos flujos en proyectos de creación medial e instalaciones interactivas.

## Materiales y programas

**Para el ejercicio principal (Arduino + Processing):**

- Computador con **Arduino IDE**, **Processing 4** y cable USB de datos.
- Placa **Arduino UNO** (R3 o R4; revisar compatibilidad y configuración de la placa).
- Sensor de distancia infrarrojo **SHARP GP2Y0A21** (10–80 cm) o **GP2Y0A02** (20–150 cm), cables y protoboard.
- Objeto que pueda acercarse o alejarse del sensor.
- Si no se dispone de SHARP, un **potenciómetro** conectado a `A0` sirve para practicar el envío de datos analógicos.
- Potenciómentro (Opcional).

**Para las demostraciones:**

- **Kinect v1** (1414) y su adaptador de alimentación, más un computador previamente configurado con una librería compatible de Processing. Una Kinect v2 requiere un entorno diferente: no intercambiar librerías sin verificar compatibilidad.
- Para **Syphon**: computador **macOS**, Processing y un programa receptor compatible, con su biblioteca instalada y verificada.
- Para **NDI**: dos aplicaciones o equipos compatibles con NDI y acceso a una misma red local preparada para video; un receptor como **NDI Video Monitor** sirve para observar la transmisión.
- Adicionalmente cuentan con [**NDISyphon**]([https://docs.vidvox.net/ndisyphon/]).

> **Nota de compatibilidad:** las bibliotecas antiguas de Kinect y Syphon pueden presentar dificultades en macOS con **Apple Silicon (M1/M2/M3 y posteriores)** y determinadas versiones de Processing. Verificar la combinación de hardware, sistema y librería **antes de la clase**. Las demostraciones Kinect/Syphon/NDI no son requisitos del ejercicio individual si el equipamiento no está operativo.

> **Seguridad eléctrica:** los sensores SHARP indicados requieren alimentación cercana a 5 V. Confirmar en la ficha técnica los terminales `VCC`, `GND` y `VO` del modelo utilizado; **no confiar solo en el color de los cables**. En placas de 3,3 V, comprobar compatibilidad y adaptación de niveles.
---

## 01 · Arduino + Processing mediante comunicación Serial

En las clases anteriores utilizamos `Serial.print()` y `Serial.println()` para observar lecturas de sensores en el **Monitor Serial**. Ahora **Processing recibirá esos números** y los transformará en una imagen.

### Dos entornos con funciones distintas

| Arduino | Processing |
| --- | --- |
| Lee el sensor físico con `analogRead(A0)`. | Recibe texto por un puerto serial. |
| Entrega un valor ADC (normalmente **0–1023** con resolución de 10 bits). | Interpreta el número y lo transforma en una variable visual. |
| Envía una línea con `Serial.println()`. | Utiliza `map()` y `constrain()` para obtener una opacidad entre **0 y 255**. |

```text
Objeto frente al sensor
       ↓
SHARP (salida de voltaje)
       ↓
Arduino — analogRead(A0): 0 a 1023
       ↓
USB / comunicación Serial — 9600 baudios
       ↓
Processing — lectura y conversión del texto a número
       ↓
map() → opacidad (0 a 255) → visual en tiempo real
```

> Los valores **0–1023 no representan centímetros**. En esta actividad se utilizan para controlar una característica visual; la conversión a distancia requiere una calibración específica del sensor.

### Conexión del SHARP con Arduino UNO

| Terminal del sensor | Arduino UNO |
| --- | --- |
| `VCC` | `5V` |
| `GND` | `GND` |
| `VO` (señal analógica) | `A0` |

**Antes de conectar:** comprobar la disposición real de los terminales en la ficha técnica del sensor, porque puede variar su interpretación según la orientación del conector.

### Paso 1 — Código Arduino: enviar lecturas

Guardar como `01_SHARP_Serial_Arduino.ino`:

```cpp
const int PIN_SHARP = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int lectura = analogRead(PIN_SHARP);
  Serial.println(lectura); // Una lectura por línea
  delay(50);              // Aproximadamente 20 lecturas por segundo
}
```

1. Cargar el programa en Arduino.
2. Abrir brevemente el **Monitor Serial** a **9600 baudios** y comprobar que el número cambia al mover un objeto.
3. **Cerrar el Monitor Serial** antes de abrir el puerto desde Processing: normalmente **solo una aplicación puede usar ese puerto a la vez**.

### Paso 2 — Código Processing: controlar opacidad

Guardar como `02_Serial_Processing.pde`:

```java
import processing.serial.*;

Serial puerto;
int valorADC = 0;
int indicePuerto = 0; // Cambiar según la lista mostrada en consola

void setup() {
  size(640, 480);
  println(Serial.list()); // Revisar los puertos disponibles

  // Cambiar indicePuerto si Arduino aparece en otra posición
  puerto = new Serial(this, Serial.list()[indicePuerto], 9600);
  puerto.clear();
  puerto.bufferUntil('\n'); // Recibir una línea completa
}

void draw() {
  background(25);

  float opacidad = map(valorADC, 0, 1023, 0, 255);
  opacidad = constrain(opacidad, 0, 255);

  noStroke();
  fill(80, 180, 255, opacidad);
  ellipse(width/2, height/2, 260, 260);

  fill(255);
  textSize(16);
  text("Lectura ADC: " + valorADC, 20, 30);
  text("Opacidad: " + int(opacidad), 20, 55);
}

void serialEvent(Serial p) {
  String linea = p.readStringUntil('\n');
  if (linea == null) return;
  linea = trim(linea);

  try {
    int numero = Integer.parseInt(linea);
    if (numero >= 0 && numero <= 1023) valorADC = numero;
  } catch (NumberFormatException e) {
    // Ignorar una línea incompleta o no numérica
  }
}
```

**Importante sobre el puerto:** `indicePuerto = 0` es **solo un ejemplo**. Si el programa indica que no se puede abrir el puerto, revisar la lista de puertos impresa en la consola de Processing y modificar el índice al correspondiente a Arduino. Revisar que no continúe abierto el Monitor Serial y que ambos programas utilicen **9600 baudios**.

### ¿Qué estamos programando?

- `import processing.serial.*;` incorpora la biblioteca de comunicación serial de Processing.
- `Serial.list()` muestra los puertos detectados.
- `bufferUntil('\n')` espera hasta recibir el salto de línea enviado por `Serial.println()`.
- `serialEvent()` procesa las líneas que llegan desde Arduino.
- `Integer.parseInt()` transforma una cadena de texto numérica en un entero.
- `map(valorADC, 0, 1023, 0, 255)` convierte la lectura en un valor apropiado para la opacidad de Processing.
- `fill(r, g, b, opacidad)` modifica la transparencia de la figura: `0` es transparente y `255` es opaco.

### Actividad práctica

1. Ejecutar ambos programas y observar la figura de Processing.
2. Acercar y alejar un objeto frente al SHARP **dentro del intervalo nominal del sensor**.
3. Identificar cómo cambia la opacidad y observar simultáneamente el número ADC mostrado en pantalla.
4. Modificar el código para controlar otra propiedad visual: **tamaño, posición o color** de la figura.
5. Explicar qué información proviene del sensor y qué decisión de diseño se realiza al mapear esos datos.

**Pregunta de discusión:** ¿qué diferencias existen entre usar el mismo número para representar una distancia, una opacidad o una posición en pantalla?

---

## 02 · Kinect + Processing: imagen, profundidad y movimiento

Kinect permite explorar otras formas de captura del entorno. A diferencia del SHARP, que nos entrega **una medición de voltaje**, Kinect puede proporcionar **imágenes RGB y datos de profundidad distribuidos en una imagen**. Con software y técnicas adicionales, esas imágenes también permiten estudiar la presencia y el movimiento del cuerpo.

| Captura | Información obtenida | Posible uso |
| --- | --- | --- |
| **RGB** | Imagen de color de la escena. | Texturas y tratamiento de video. |
| **Profundidad** | Información de distancia por píxel. | Siluetas, segmentación por distancia y representaciones espaciales. |
| **Seguimiento corporal** | Estimaciones de posición/movimiento con herramientas compatibles. | Control gestual e interacción corporal. |

> **Profundidad no es lo mismo que seguimiento corporal.** Capturar una imagen de profundidad no implica disponer automáticamente de un esqueleto o articulaciones identificadas; para ello se requiere procesamiento o bibliotecas específicas.

### Demostración guiada — Kinect v1

Como referencia utilizaremos la biblioteca [OpenKinect for Processing](https://github.com/shiffman/OpenKinect-for-Processing), si la instalación es compatible con el equipo disponible. El siguiente ejemplo muestra **solo la imagen de profundidad**:

```java
import org.openkinect.processing.*;

Kinect kinect;

void setup() {
  size(640, 480);
  kinect = new Kinect(this);
  kinect.initDepth();
}

void draw() {
  background(0);
  image(kinect.getDepthImage(), 0, 0, width, height);
}
```

Guardar como `03_Kinect_Profundidad.pde`. El ejemplo está pensado para **Kinect v1** con la biblioteca adecuada y el hardware conectado; **no** funcionará únicamente copiándolo si faltan controladores, dependencias o compatibilidad con el sistema.

**Observación propuesta:** acercarse, alejarse y desplazar el cuerpo frente a Kinect. Discutir qué cambia en la imagen y cómo podrían utilizarse esos datos para modificar una visual en Processing.

**Si Kinect no está operativo:** revisar una captura de profundidad previamente registrada por el docente y desarrollar la discusión conceptual sin exigir instalación individual durante la sesión.

---

## 03 · Syphon y NDI: compartir imágenes entre aplicaciones

Hasta ahora transferimos **números** mediante Serial y capturamos **imágenes** con Kinect. El siguiente paso es compartir una salida visual con otro programa o computador.

| Tecnología | ¿Qué comparte? | Dónde funciona | Ejemplo de uso |
| --- | --- | --- | --- |
| **Syphon** | Imágenes/texturas entre aplicaciones compatibles. | **Dentro de un mismo Mac** (macOS). | Processing → VDMX / MadMapper, si ambas aplicaciones son compatibles. |
| **NDI (Network Device Interface)** | Flujos audiovisuales a través de IP. | **Entre aplicaciones o equipos conectados por red**. | Aplicación emisora NDI → receptor NDI en otro computador. |

**Importante:** Syphon y NDI **no son equivalentes a la comunicación Serial**. Serial transmite aquí valores del sensor; Syphon y NDI se usan para mover la salida visual. Además, **Syphon no envía automáticamente por NDI**: para enlazar ambos se necesita un programa o puente compatible que reciba Syphon y emita NDI.

### Demostración A — Processing → Syphon (macOS)

Con una biblioteca Syphon compatible instalada, se puede compartir la salida de un sketch de Processing con otra aplicación en el **mismo Mac**.

```java
import codeanticode.syphon.*;

SyphonServer servidor;

void setup() {
  size(640, 480, P3D);
  servidor = new SyphonServer(this, "Clase06_Processing");
}

void draw() {
  background(20);
  noStroke();
  fill(120, 210, 255);
  ellipse(mouseX, mouseY, 140, 140);

  servidor.sendScreen(); // Compartir la imagen dibujada
}
```

Guardar como `04_Syphon_Processing.pde`. En un programa receptor compatible, buscar el servidor **`Clase06_Processing`** y visualizar su salida.

> **Atención:** algunas distribuciones de la biblioteca Syphon para Processing presentan problemas de compatibilidad con Apple Silicon. Si no está validada en el Mac del taller, realizar esta parte como demostración en un equipo configurado o mostrar un registro previo.

### Demostración B — Video mediante NDI (red local)

1. Elegir una aplicación que **pueda emitir NDI** y una aplicación que **pueda recibir NDI** (por ejemplo, un monitor NDI compatible).
2. Conectar ambos equipos a una red local apropiada y habilitar la fuente NDI en el programa emisor.
3. Abrir la fuente desde el equipo receptor y observar latencia, resolución y fluidez.
4. Comparar con Syphon: ¿cuándo necesitamos intercambiar imágenes **dentro del mismo computador** y cuándo necesitamos una **red**?

**No es necesario programar NDI directamente desde Processing** en esta clase: la implementación depende de la aplicación o biblioteca compatible disponible. En caso de querer compartir una salida de Processing mediante NDI, se requerirá una ruta concreta de captura o conversión y un emisor NDI.

```text
FLUJO A — DATOS
SHARP → Arduino → Serial USB → Processing → imagen interactiva

FLUJO B — VIDEO LOCAL EN MAC
Processing → Syphon → aplicación receptora compatible (mismo equipo)

FLUJO C — VIDEO POR RED
Aplicación emisora NDI → red local → aplicación receptora NDI

FLUJO INTEGRADO (solo si existe un puente compatible)
Kinect → Processing → Syphon → puente Syphon/NDI → red → receptor NDI
```

---

## 04 · Cierre y registro de la actividad

**Resultado obligatorio:** un sketch de Processing cuyo comportamiento visual cambie a partir de los valores de un sensor conectado a Arduino mediante Serial.

Al finalizar, cada estudiante debería contar con:

- [ ] Código `.ino` que lee el sensor y envía **una lectura por línea** mediante `Serial.println()`.
- [ ] Código `.pde` que recibe esos datos e interpreta correctamente los números.
- [ ] Una visual en Processing cuya **opacidad** (o alguna propiedad modificada por el estudiante) responda al sensor.
- [ ] Captura de pantalla o breve registro en video del resultado.
- [ ] Una explicación breve del recorrido de los datos desde el sensor hasta la imagen.

**Demostraciones complementarias (no obligatorias):** Kinect + Processing y visuales compartidas por Syphon/NDI.

## Solución de problemas frecuentes

| Problema | Qué revisar |
| --- | --- |
| Processing no abre el puerto Serial | Cerrar Monitor Serial, comprobar cable USB, verificar `Serial.list()` e `indicePuerto`, confirmar permisos del sistema. |
| Llegan símbolos extraños o no se reciben lecturas | Comprobar que Arduino y Processing utilizan **9600 baudios** y que Arduino envía valores mediante `println()`. |
| La figura no cambia | Verificar el valor ADC mostrado; posición del objeto respecto del SHARP; que `serialEvent()` esté recibiendo números. |
| Kinect no entrega imagen | Alimentación, cableado, generación del dispositivo, controladores, biblioteca y compatibilidad de sistema/arquitectura. |
| La aplicación no encuentra Syphon | Ambas aplicaciones deben estar en **macOS y el mismo computador**, ser compatibles y la biblioteca debe funcionar. |
| No aparece la fuente NDI | Comprobar que existe un **emisor NDI**, que el receptor está conectado a la red adecuada y que la configuración de red permite descubrir la fuente. |
