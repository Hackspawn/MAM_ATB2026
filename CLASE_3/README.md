# Clase 3 · Sensores, MIDI y Sonic Pi 🎹

> **Objetivo de la clase:** Comprender cómo una señal física puede convertirse en información y utilizarse para controlar sonido mediante Arduino, MIDI y Sonic Pi.

---

## 🎯 Resultados de aprendizaje

Al finalizar la clase, las y los estudiantes podrán:

1. Relacionar los conceptos de señales, bits y programación de la clase anterior con la lectura de sensores.
2. Leer una entrada analógica y observar sus valores en el monitor serial.
3. Distinguir entre lectura ADC, distancia física y nota MIDI.
4. Programar secuencias de notas, duraciones y silencios en Sonic Pi.
5. Diferenciar MIDI por serial, USB-MIDI, BLE-MIDI y OSC.
6. Identificar qué vía de comunicación permite la placa disponible.
7. Construir un controlador que transforme lecturas de un sensor en notas.
8. Evaluar cómo el mapeo entre gesto y sonido modifica la experiencia de interacción.

---

# 📦 Materiales

- 1 × computador con **Arduino IDE** y **Sonic Pi de escritorio**.
- 1 × placa de desarrollo y cable USB **de datos**.
- 1 × sensor de distancia **SHARP GP2Y0A21YK0F** o **GP2Y0A02YK0F** u otro sensor de distancia Analógico.
- 1 × protoboard y cables de conexión.
- Audífonos o parlantes.
- Opcional: potenciómetro de 10 kΩ para probar primero una entrada analógica simple.

## Elegir la placa antes de comenzar

Para realizar directamente el proyecto USB-MIDI de esta guía:

- **Ruta A:** Arduino con ATmega32u4 y biblioteca `MIDIUSB`.
- **Ruta B:** ESP32-S2 o ESP32-S3 con puerto USB nativo y un core Arduino-ESP32 que incluya `USBMIDI.h`.

> [!IMPORTANT]
> La UNO R3/R4, ESP32 clásica y ESP8266 permiten realizar la lectura de sensores y el ejercicio de Sonic Pi. Sin embargo, no deben usarse con los sketches USB-MIDI de esta guía sin adaptar la comunicación. Revisa la tabla del apartado 11 antes de montar el proyecto.

---

# 🗺️ Ruta de la clase

1. Comprender MIDI y producir sonido con Sonic Pi.
2. Construir una secuencia con notas, duraciones y silencios.
3. Leer un sensor y observar el ADC en el monitor serial.
4. Mapear la lectura a un rango musical.
5. Elegir el transporte según la placa.
6. Conectar el controlador con Sonic Pi.
7. Experimentar, documentar y discutir los resultados.

---

# 1. 🎹 Antes del sonido: ¿qué es MIDI?

**MIDI** significa *Musical Instrument Digital Interface*. Permite comunicar instrucciones musicales entre dispositivos y programas.

Un mensaje puede indicar qué nota activar, con qué intensidad y en qué canal.

> **MIDI no es audio:** el controlador envía instrucciones; Sonic Pi genera el sonido.

## Mensajes que utilizaremos

| Mensaje / dato | Función | Ejemplo |
|---|---|---|
| `Note On` | Activar una nota | Nota 60, velocidad 64 |
| `Note Off` | Desactivar una nota | Apagar la nota 60 |
| Nota | Identificar la altura | 60, 62, 64… |
| Velocidad (*velocity*) | Intensidad del evento | 1 a 127 para una activación |
| Canal | Separar flujos de mensajes | Canales 1 a 16 |
| Control Change —CC— | Modificar un parámetro | Control continuo de un efecto |

Un `Note On` con velocidad 0 se interpreta como desactivación de la nota.

## Notas MIDI y bits

Las notas MIDI 1.0 utilizan valores de **0 a 127**: 128 posibilidades, representables con 7 bits.

| Nota MIDI | Relación musical |
|---:|---|
| 60 | Do central |
| 62 | Re |
| 64 | Mi |
| 65 | Fa |
| 67 | Sol |
| 69 | La de referencia, 440 Hz |
| 71 | Si |
| 72 | Do, una octava sobre 60 |

Subir **1** equivale a un semitono; subir **12**, a una octava. La nomenclatura de octavas puede variar entre programas: el número MIDI evita esa ambigüedad.

---

# 2. 💻 Primer acercamiento a Sonic Pi

Sonic Pi permite escribir código para producir y transformar sonido.

En un buffer vacío, ejecutar:

```ruby
use_bpm 120
use_synth :pulse

play 60
sleep 1
play 64
sleep 1
play 67
```

| Instrucción | Qué hace |
|---|---|
| `use_bpm 120` | Establece el tempo en pulsos por minuto |
| `use_synth :pulse` | Selecciona un sintetizador |
| `play 60` | Produce una nota en Sonic Pi |
| `sleep 1` | Espera un pulso |

> `play 60` utiliza el número MIDI para indicar la altura del sonido, pero **no envía por sí solo un mensaje MIDI externo**.

## Pulsos y segundos

A 120 BPM, un pulso dura 0,5 segundos. A 60 BPM, dura 1 segundo.

**Duración en segundos = pulsos × 60 / BPM**

A diferencia de `delay()` en Arduino, `sleep` en Sonic Pi trabaja en pulsos afectados por el tempo.

---

# 3. 🧪 Ejercicio 1 · Notas, duraciones y silencios

Conservamos la secuencia de la clase anterior del curso y corregimos la representación de las pausas.

```ruby
use_bpm 220
use_synth :pulse
use_synth_defaults sustain: 0.08, release: 0.12, amp: 1.2

# Lista intercalada: [nota, duración, nota, duración, ...]
# :r representa un silencio; 20 sería una nota real.
notes_and_durations = [
  76, 12, 76, 12, :r, 12, 76, 12, :r, 12, 72, 12, 76, 12, :r, 12,
  79, 12, :r, 36, 67, 12, :r, 36, 72, 12, :r, 24, 67, 12, :r, 24,
  64, 12, :r, 24, 69, 12, :r, 12, 71, 12, :r, 12, 70, 12, 69, 12,
  :r, 12, 67, 16, 76, 16, 79, 16, 81, 12, :r, 12, 77, 12, 79, 12,
  :r, 12, 76, 12, :r, 12, 72, 12, 74, 12, 71, 12, :r, 24
]

sequence = notes_and_durations.each_slice(2).to_a

define :play_sequence do |seq, factor: 0.07, transpose: 0|
  seq.each do |n, d|
    t = d * factor
    if n != :r
      play n + transpose
    end
    sleep t
  end
end

live_loop :mario_theme do
  play_sequence sequence, factor: 0.07, transpose: 0
end
```

## ¿Qué está ocurriendo?

- `each_slice(2)` agrupa la lista en parejas de nota y duración.
- `define` crea una función que podemos reutilizar.
- `seq.each` recorre cada pareja.
- `if n != :r` reproduce solamente las notas.
- `sleep t` establece el tiempo hasta el próximo evento, incluyendo los silencios.
- `live_loop` repite la secuencia.
- `transpose` suma semitonos a las notas.

El factor `0.07` conserva la interpretación temporal del ejemplo original: una duración de `12` se convierte en `0.84` pulsos. La duración entre eventos no es necesariamente igual a la duración audible de cada nota, que depende también de su envolvente.

## Experimentar

1. Cambiar `:pulse` por `:piano`.
2. Probar `transpose: 12` y `transpose: -12`.
3. Modificar el tempo.
4. Crear una secuencia propia de al menos ocho eventos, incluyendo silencios.

**Pregunta:** ¿Qué cambia al modificar el timbre y qué cambia al modificar la altura?

---

# 4. 📡 Sensores: del fenómeno físico al número

Un sensor permite obtener información sobre una condición del entorno.

En esta clase utilizaremos un sensor SHARP de distancia con salida analógica. La distancia modifica su voltaje de salida y el ADC convierte ese voltaje en un número.

| Etapa | Qué representa |
|---|---|
| Distancia | Separación física entre sensor y objeto |
| Voltaje | Señal eléctrica producida por el sensor |
| Lectura ADC | Representación digital de ese voltaje |
| Mapeo | Regla que asigna la lectura a una nota |
| Síntesis | Producción de sonido en Sonic Pi |

Los modelos SHARP no son intercambiables en su curva de respuesta:

| Sensor | Rango nominal de medición |
|---|---|
| GP2Y0A21YK0F | 10–80 cm |
| GP2Y0A02YK0F | 20–150 cm |

> [!IMPORTANT]
> La relación entre distancia y voltaje es **no lineal**. Fuera del rango útil, la lectura puede resultar ambigua. Para controlar notas no es obligatorio convertir a centímetros, pero sí observar y calibrar el intervalo de trabajo.

Un HC-SR04 requiere otro ejercicio de lectura: mide tiempo de eco mediante señales digitales. No se lee con `analogRead()`.

---

# 5. 🔌 Cableado y límites eléctricos

## Ejemplo para Leonardo / Micro de 5 V

Con la placa desconectada:

| Terminal del SHARP | Conexión |
|---|---|
| Alimentación —VCC— | 5 V, según ficha del modelo |
| Tierra —GND— | GND |
| Salida —Vo— | A0 |

Comprobar la orientación del conector en la ficha técnica. No deducir el pinout únicamente por el color de los cables. Seguir también las recomendaciones de desacoplo del fabricante.

## Si trabajamos con ESP32 o ESP8266

- Alimentación del sensor y tensión admitida por el ADC son datos diferentes.
- Las entradas ESP32 **no toleran 5 V**. El rango medible del ADC depende del chip y de la atenuación configurada; no debe suponerse que mide de 0 a 3,3 V completos.
- La entrada ADC del chip ESP8266 admite normalmente **0–1 V**. Algunas placas incorporan un divisor que amplía el rango del terminal A0: verificar el esquema de la placa concreta.
- Adaptar la salida del sensor mediante un divisor o acondicionamiento adecuado cuando sea necesario. No conectar una salida de rango desconocido al ADC.
- Compartir GND entre sensor y placa.

Para una primera comprobación puede utilizarse un potenciómetro, limitando su tensión máxima al rango admisible y medible del ADC elegido.

---

# 6. 🔢 ADC y resolución: ¿por qué 0–1023?

Un ADC con resolución de 10 bits puede representar:

**2¹⁰ = 1024 niveles → valores entre 0 y 1023.**

| Configuración de lectura | Resolución | Valores |
|---|---:|---:|
| UNO R3 / Leonardo / Micro | 10 bits | 0–1023 |
| UNO R4, configuración predeterminada | 10 bits | 0–1023 |
| ESP32, configurada con `analogReadResolution(12)` | 12 bits | 0–4095 |
| ESP8266, ADC externo | 10 bits | 0–1023 |

La UNO R4 permite configurar otras resoluciones. En ESP32 conviene establecer explícitamente la resolución del ejemplo para evitar depender de valores predeterminados o revisiones del chip.

> **Más niveles no garantizan una medición más precisa.** También influyen ruido, alimentación, calibración y características del sensor.

---

# 7. 🧪 Ejercicio 2 · Observar el sensor

## Código para UNO / Leonardo / Micro

```cpp
const int SENSOR_PIN = A0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int lectura = analogRead(SENSOR_PIN);
  Serial.println(lectura);
  delay(50);
}
```

1. Seleccionar placa y puerto en Arduino IDE.
2. Cargar el sketch.
3. Abrir el monitor serial a **9600 baudios**.
4. Mover un objeto dentro del rango útil del sensor.
5. Registrar valores mínimo y máximo del gesto que se utilizará.
6. Observar si la lectura fluctúa cuando el objeto está quieto.

Para ESP32-S2/S3, elegir un GPIO ADC disponible en la placa —por ejemplo GPIO4 cuando su pinout lo permita—, sustituir `A0` y agregar `analogReadResolution(12);` en `setup()`. Para ESP8266 puede mantenerse `A0`, respetando su rango eléctrico.

## RECORDAR: ¿Qué son los baudios?

La tasa en baudios expresa símbolos transmitidos por segundo. En una UART binaria habitual coincide numéricamente con la tasa de bits por segundo, pero no con caracteres por segundo: cada carácter lleva además bits de trama.

En una conexión UART, emisor y receptor deben usar la misma configuración. `9600` y `115200` son velocidades habituales para depuración; no son capacidades exclusivas de una familia de placas.

**31.250 baudios** corresponde al MIDI serial tradicional. No es una velocidad que debamos configurar para enviar USB-MIDI.

Cuando `Serial` utiliza USB CDC nativo, el número solicitado en `Serial.begin()` no determina la velocidad física del bus USB.

---

# 8. 🧮 Mapear: diseñar la relación entre gesto y sonido

Podemos convertir una lectura de 10 bits en una nota:

```cpp
int nota = map(lectura, 0, 1023, 40, 80);
```

| Parámetro | Significado |
|---|---|
| `lectura` | Valor observado |
| `0, 1023` | Intervalo de entrada |
| `40, 80` | Intervalo musical de salida |

`map()` realiza una transformación lineal con resultado entero. **No convierte automáticamente el sensor a centímetros ni corrige su curva.**

## Calibración del gesto

Si durante el movimiento obtenemos valores entre 180 y 650, podemos utilizar:

```cpp
// Valores ilustrativos: sustituir por las mediciones del propio montaje.
const int SENSOR_MIN = 180;
const int SENSOR_MAX = 650;

int limitada = constrain(lectura, SENSOR_MIN, SENSOR_MAX);
int nota = map(limitada, SENSOR_MIN, SENSOR_MAX, 40, 80);
```

`constrain()` limita valores fuera del intervalo. `map()` por sí solo extrapola; no limita.

**Condición:** `SENSOR_MAX` debe ser mayor que `SENSOR_MIN`.

Para invertir la relación:

```cpp
int nota = map(limitada, SENSOR_MIN, SENSOR_MAX, 80, 40);
```

**Pregunta de diseño:** ¿acercar la mano debería producir sonidos más graves o más agudos? La respuesta depende de la interacción que queremos construir.

---

# 9. 🔄 El mensaje y su transporte

| Vía | Qué requiere | ¿Es MIDI? |
|---|---|---|
| MIDI serial / DIN | UART y circuito eléctrico apropiado para DIN | Sí |
| USB-MIDI | USB de dispositivo configurable y biblioteca compatible | Sí |
| BLE-MIDI | Bluetooth LE, firmware y receptor compatibles | Sí |
| MIDI por red —RTP-MIDI— | Implementación y sesión de red compatibles | Sí |
| OSC por Wi-Fi | Emisor y receptor OSC | No: es otro protocolo |
| Serial → programa puente → MIDI | Programa que convierta los datos a MIDI | La salida del puente sí |

> [!IMPORTANT]
> Un conector USB puede servir únicamente para alimentar y programar una placa mediante un puente USB–serial. Eso no convierte automáticamente a la placa en un dispositivo USB-MIDI.

`MIDI.h` con su instancia serial predeterminada y `MIDIUSB.h` no son equivalentes. Para USB-MIDI tampoco necesitamos `Serial.begin(31250)`.

---

# 10. ⚙️ Compatibilidad de placas para USB-MIDI

La tabla distingue **capacidad USB-MIDI** y **biblioteca utilizada**. El soporte depende también del core y de su versión.

| Placa / familia | USB-MIDI directo | Implementación / observación |
|---|---|---|
| Leonardo / Micro / Pro Micro —ATmega32u4— | Sí | `MIDIUSB`; ruta A |
| Zero / MKR Zero / MKR1000 —SAMD21— | Sí | `MIDIUSB`; revisar puerto nativo |
| Nano 33 IoT —SAMD21— | Sí | `MIDIUSB` |
| Nano 33 BLE —nRF52840— | Sí, con implementación compatible | Control Surface / backend Mbed; no el mismo MIDIUSB |
| Due —SAM3X8E— | Sí | `MIDIUSB`; puerto **Native USB** |
| UNO R3 / Mega 2560 | No, de fábrica | Puente de software; algunos modelos permiten reprogramar el ATmega16U2 |
| Nano clásico | No, de fábrica | Puente serial–MIDI |
| UNO R4 Minima / WiFi | No asumir soporte directo | Control Surface documenta MIDI USB deshabilitado en el core Renesas; verificar versión antes de usar |
| Teensy 3.x / 4.x | Sí | Teensyduino; seleccionar tipo USB MIDI |
| ESP32 clásica —WROOM/WROVER— | No | Alternativas: serial, BLE-MIDI o red |
| ESP32-S2 | Sí | `USBMIDI.h` en core compatible; sin Bluetooth |
| ESP32-S3 | Sí | `USBMIDI.h` en core compatible; ruta B |
| Arduino Nano ESP32 —ESP32-S3— | Sí, con configuración compatible | Verificar core, biblioteca y numeración de pines |
| ESP32-C3 / C6 | No mediante USB Serial/JTAG | Ese periférico no puede convertirse en USB-MIDI |
| ESP8266 —NodeMCU / D1 mini— | No | Alternativas: serial o red; sin BLE |

Fuentes para comprobar soporte: [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB), [Control Surface](https://tttapa.github.io/Control-Surface/Doxygen/d8/d4a/md_pages_MIDI-over-USB.html) y [ejemplos USB-MIDI de Espressif](https://github.com/espressif/arduino-esp32/tree/master/libraries/USB/examples/MIDI).

## Si la placa tiene dos conectores USB

En una ESP32-S3, uno puede corresponder al puente UART y otro al USB nativo. Para USB-MIDI utilizar el conectado al periférico USB del chip. Consultar las etiquetas y el esquema de la placa.

La presencia de un ESP32-S3 auxiliar en una UNO R4 WiFi no hace que esta funcione como una placa ESP32-S3 de propósito general ni permite copiar directamente la ruta B.

---

# 11. 🎛️ Proyecto de clase · Sensor como controlador USB-MIDI

## Comportamiento esperado

1. Leer el sensor.
2. Convertir la lectura en una nota entre 40 y 80.
3. Enviar `Note On`.
4. Esperar 64 ms.
5. Enviar `Note Off` de esa misma nota.
6. Esperar 100 ms y repetir.

Esto produce **notas reiteradas cuya altura depende del sensor**. No es todavía un instrumento con notas sostenidas según el gesto.

Elegir **una** de las siguientes rutas. No mezclar sus bibliotecas en un mismo sketch.

---

# 12. 🧪 Ruta A · Leonardo / Micro / Pro Micro

Instalar **MIDIUSB** desde el gestor de bibliotecas. El código siguiente utiliza A0 y resolución de 10 bits.

```cpp
#include <MIDIUSB.h>

const int SENSOR_PIN = A0;
const int SENSOR_MIN = 0;
const int SENSOR_MAX = 1023;
const byte VELOCIDAD = 64;

void enviarNota(byte nota, bool encender) {
  // Cable virtual 0; canal MIDI 1 (codificado como 0).
  midiEventPacket_t mensaje = {
    (byte)(encender ? 0x09 : 0x08),
    (byte)(encender ? 0x90 : 0x80),
    nota,
    (byte)(encender ? VELOCIDAD : 0)
  };

  MidiUSB.sendMIDI(mensaje);
  MidiUSB.flush();
}

void setup() {
  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  int lectura = analogRead(SENSOR_PIN);
  int limitada = constrain(lectura, SENSOR_MIN, SENSOR_MAX);
  byte nota = (byte)map(limitada, SENSOR_MIN, SENSOR_MAX, 40, 80);

  enviarNota(nota, true);
  delay(64);

  enviarNota(nota, false);
  delay(100);
}
```

## Anatomía del programa

| Elemento | Función |
|---|---|
| `#include <MIDIUSB.h>` | Incorpora la biblioteca USB-MIDI |
| `byte` | Almacena un entero de 8 bits; aquí usamos valores válidos para MIDI |
| `bool encender` | Indica si enviaremos activación o desactivación |
| `midiEventPacket_t` | Representa un paquete USB-MIDI |
| `MidiUSB.sendMIDI()` | Prepara el envío del mensaje |
| `MidiUSB.flush()` | Envía los paquetes pendientes |

Sustituir `SENSOR_MIN` y `SENSOR_MAX` por los valores observados para aprovechar el gesto útil del sensor. El intervalo completo del ADC permite una primera prueba, pero un SHARP normalmente no lo recorre entero.

---

# 13. 🧪 Ruta B · ESP32-S2 / ESP32-S3

## Preparación

1. Instalar el paquete de placas **esp32 by Espressif Systems**.
2. Seleccionar el modelo exacto.
3. Comprobar que el core instalado contiene los ejemplos **USB → MIDI** y `USBMIDI.h`; actualizar a una versión estable compatible si faltan.
4. Si aparece la opción **USB Mode**, seleccionar **USB-OTG (TinyUSB)**.
5. Conectar el puerto USB nativo al computador.
6. Comprobar pin ADC, cableado y tensión de entrada antes de cargar.

> Este sketch usa **GPIO4**, no necesariamente el terminal rotulado D4. Verificar que GPIO4 sea un ADC disponible en la placa elegida. No está destinado a ESP32 clásica, C3, C6 o ESP8266.

```cpp
#include <Arduino.h>
#include "USB.h"
#include "USBMIDI.h"

USBMIDI MIDI;

const int SENSOR_PIN = 4;  // GPIO4: verificar pinout de la placa.
const int SENSOR_MIN = 0;
const int SENSOR_MAX = 4095;

void setup() {
  pinMode(SENSOR_PIN, INPUT);
  analogReadResolution(12);

  MIDI.begin();
  USB.begin();
}

void loop() {
  int lectura = analogRead(SENSOR_PIN);
  int limitada = constrain(lectura, SENSOR_MIN, SENSOR_MAX);
  uint8_t nota = (uint8_t)map(limitada, SENSOR_MIN, SENSOR_MAX, 40, 80);

  MIDI.noteOn(nota, 64);
  delay(64);

  MIDI.noteOff(nota, 0);
  delay(100);
}
```

El core define el canal predeterminado de estas llamadas. Para esta primera prueba el receptor de Sonic Pi acepta cualquier canal.

Calibrar `SENSOR_MIN` y `SENSOR_MAX` con lecturas reales. Si hay saturación, revisar primero el rango de tensión y la configuración del ADC: cambiar `map()` no corrige una entrada saturada.

---

# 14. 🔊 Recibir las notas en Sonic Pi

Detener la melodía del ejercicio 1 antes de probar el sensor.

1. Conectar la placa con el sketch MIDI cargado.
2. Revisar las entradas MIDI en las preferencias de Sonic Pi.
3. Activar la recepción MIDI si está deshabilitada.
4. Observar los mensajes `note_on` en el panel **Cues**.
5. Ejecutar el siguiente código en un buffer nuevo.

```ruby
live_loop :sensor_midi do
  use_real_time

  # Comodín para la primera prueba: cualquier dispositivo y canal.
  note, velocity = sync "/midi*/note_on"

  if velocity > 0
    synth :piano, note: note, amp: velocity / 127.0, release: 0.3
  end
end
```

Después de verificar la recepción, reemplazar el patrón `"/midi*/note_on"` por la ruta exacta que muestra Cues. Así otro controlador conectado no disparará el mismo instrumento.

## Relación entre MIDI y sonido

- `sync` espera un evento.
- `note` y `velocity` reciben los datos del mensaje.
- `use_real_time` permite responder a eventos externos con menor latencia de planificación.
- `velocity / 127.0` convierte la velocidad a una amplitud entre 0 y 1.

> [!NOTE]
> Este receptor crea un sonido con envolvente propia por cada `Note On`. No utiliza los mensajes `Note Off` para detenerlo. Para notas sostenidas habría que gestionar también esos eventos y los nodos de síntesis. La pareja On/Off sigue enviándose correctamente desde la placa para su uso con otros instrumentos MIDI.

No es obligatorio cerrar Arduino IDE para utilizar USB-MIDI. Si se utiliza un **programa puente serial**, cerrar el monitor y el plotter serial para liberar el puerto que necesita ese programa.

---

# 15. 🌐 Alternativas para UNO, ESP32 clásica y ESP8266

Estas placas pueden participar en un controlador sonoro, pero requieren una ruta de comunicación diferente de los sketches anteriores.

| Placa | Ruta posible | Requisito adicional |
|---|---|---|
| UNO R3 / UNO R4 | Serial USB → programa puente → MIDI u OSC | Programa en el computador que lea y traduzca los datos |
| ESP32 clásica | OSC por Wi-Fi | Emisor OSC y recepción de red en Sonic Pi |
| ESP32 clásica | BLE-MIDI | Biblioteca BLE-MIDI y conexión en el sistema operativo |
| ESP8266 | OSC por Wi-Fi | Emisor OSC y recepción de red en Sonic Pi |
| ESP32 / ESP8266 | RTP-MIDI | Biblioteca y sesión de red compatibles |

**OSC no es MIDI**, aunque ambos pueden controlar un mismo sintetizador. En esta clase puede desarrollarse la lectura y el mapeo con cualquier placa; la extensión por red requiere un sketch específico y configuración de red, fuera del montaje USB-MIDI principal.

Antes de la sesión práctica, acordar la ruta del grupo para que cada estudiante disponga de una vía de comunicación completa.

---

# 16. 🔍 Solución de problemas

| Problema | Qué revisar |
|---|---|
| No aparecen números en el monitor serial | Placa, puerto, sketch cargado y cable de datos |
| Texto ilegible por UART | Coincidencia entre baudios del sketch y del monitor |
| Lectura fija en mínimo o máximo | Cableado, GND, pin ADC, tensión y saturación |
| Lecturas inestables | Alimentación, conexiones, desacoplo y rango útil del sensor |
| `MIDIUSB.h` no existe | Instalar MIDIUSB y comprobar que la placa sea compatible |
| `USBMIDI.h` no existe | Comprobar core Espressif, versión y selección S2/S3 |
| ESP32-S3 aparece solo como puerto serial | Revisar puerto nativo, modo OTG y firmware cargado |
| La placa aparece en Arduino IDE, pero no en MIDI | Ser un puerto serial no equivale a ser un dispositivo MIDI |
| Hay MIDI en Cues, pero no sonido | Ruta de `sync`, velocidad, salida de audio y volumen |
| No hay MIDI en Cues | Recepción habilitada, dispositivo detectado y cable/puerto correctos |
| Las notas cambian demasiado | Calibración, filtrado y posibles umbrales con histéresis |
| El sonido no se corta al recibir Note Off | El receptor básico usa envolventes; no procesa Note Off |
| El puente serial informa puerto ocupado | Cerrar monitor/plotter u otra aplicación que use ese puerto |

Comprobar por etapas: primero lectura, luego mensajes MIDI y finalmente sonido. Así podemos localizar dónde se interrumpe el flujo.

---

# 17. 📝 Actividad y registro

1. Presentar una secuencia propia de Sonic Pi con notas y silencios.
2. Mostrar la lectura del sensor y registrar su intervalo útil.
3. Indicar placa, transporte y biblioteca utilizados.
4. Demostrar el controlador sonoro con la ruta elegida.
5. Explicar una decisión de mapeo entre gesto y sonido.

Pueden Registrar:

- Código de Arduino y de Sonic Pi.
- Fotografía del montaje y modelo exacto de placa/sensor.
- Versiones de core y biblioteca empleadas.
- Valores de calibración.
- Breve descripción del comportamiento y dificultades encontradas.

---

# 18. 📚 Documentación y referencias

- [Sonic Pi · Tutorial](https://sonic-pi.net/tutorial.html). Consultar también los apartados MIDI y OSC del tutorial integrado en la aplicación de escritorio.
- [Arduino · MIDIUSB](https://github.com/arduino-libraries/MIDIUSB).
- [Arduino MIDI Library · Transportes y ejemplos](https://github.com/FortySevenEffects/arduino_midi_library).
- [Control Surface · Compatibilidad USB-MIDI](https://tttapa.github.io/Control-Surface/Doxygen/d8/d4a/md_pages_MIDI-over-USB.html).
- [Espressif · Ejemplos USB-MIDI](https://github.com/espressif/arduino-esp32/tree/master/libraries/USB/examples/MIDI).
- [Espressif · USB API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb.html).
- [Espressif · ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html).
- [ESP8266 Arduino Core · Referencia](https://arduino-esp8266.readthedocs.io/en/latest/reference.html).
- [Arduino · UNO R4 Minima, resolución del ADC](https://docs.arduino.cc/tutorials/uno-r4-minima/adc-resolution/).
- [SHARP · GP2Y0A21YK0F, ficha técnica](https://global.sharp/products/device/lineup/data/pdf/datasheet/gp2y0a21yk_e.pdf).
- [SHARP · GP2Y0A02YK0F, ficha técnica](https://global.sharp/products/device/lineup/data/pdf/datasheet/gp2y0a02yk_e.pdf).
