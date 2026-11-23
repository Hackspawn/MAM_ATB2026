# Clase 05 — Captura y almacenamiento de datos con Arduino y microSD 💾

## Objetivos de aprendizaje

Al finalizar la sesión, cada estudiante podrá:

1. Diferenciar la **visualización en el Monitor Serial** del **almacenamiento persistente** de información.
2. Reconocer las funciones generales de los protocolos **I²C** y **SPI**, identificando por qué se usa SPI con un módulo microSD.
3. Conectar un módulo microSD y un sensor de temperatura **TMP36** a Arduino UNO.
4. Convertir las lecturas de `analogRead()` en **voltios, grados Celsius y grados Fahrenheit**.
5. Crear y escribir un archivo `data.txt` en una tarjeta microSD mediante la biblioteca `SD.h`.
6. Recuperar el archivo y examinar la organización de sus datos en un computador.

## Materiales

- Computador con **Arduino IDE**, cable USB con transferencia de datos y lector de microSD (integrado o externo).
- Placa **Arduino UNO R3** como referencia de conexión y cálculo de voltaje.
- Protoboard y cables Dupont.
- Sensor de temperatura analógico **TMP36** (encapsulado TO-92, si es el disponible).
- Módulo lector de **microSD con interfaz SPI**, apto para trabajar con la placa utilizada.
- Tarjeta **microSD formateada en FAT32** (preferentemente de capacidad moderada, por ejemplo, 8 o 16 GB).

> **Precaución:** una tarjeta microSD opera internamente a **3,3 V**. Algunos módulos incorporan regulador y conversión de niveles y pueden conectarse a 5 V; otros **no**. Antes de alimentar el lector, comprobar sus especificaciones. Nunca aplicar directamente señales de 5 V a una tarjeta microSD sin adaptación de niveles.

---

## 01 · ¿Por qué capturar y almacenar datos?

En las clases anteriores utilizamos sensores para obtener información del entorno y **observar los valores en tiempo real** mediante `Serial.print()` y `Serial.println()`. Sin embargo, si cerramos el Monitor Serial, esa visualización no constituye por sí misma un registro permanente.

Al incorporar una memoria microSD podemos **guardar una sucesión de mediciones** y revisarlas posteriormente. Esta operación se conoce como *registro de datos* o *data logging*.

**Flujo de información de esta clase:**

```text
Temperatura ambiente
       ↓
Sensor TMP36 (voltaje analógico)
       ↓
Arduino UNO — analogRead(A0)
       ↓
Cálculo: voltios → °C → °F
       ├──→ Monitor Serial (visualización)
       └──→ Módulo SPI → tarjeta microSD → data.txt (archivo)
```

**Concepto importante:** la memoria de trabajo utilizada por el programa no es lo mismo que almacenar un archivo en la microSD. El archivo **permanece** en la tarjeta cuando se corta la alimentación, siempre que la escritura haya terminado correctamente.

## 02 · Protocolos de comunicación: I²C y SPI

Los microcontroladores utilizan diferentes protocolos para intercambiar información con sensores, memorias y otros periféricos. Dos habituales son:

| Protocolo | Líneas principales | Uso habitual |
| --- | --- | --- |
| **I²C** | `SDA` (datos), `SCL` (reloj) | Sensores, pantallas y periféricos conectados a un bus compartido. |
| **SPI** | `MOSI`/`COPI`, `MISO`/`CIPO`, `SCK`, `CS` | Memorias y otros periféricos; en esta actividad, el lector microSD. |

En SPI, la placa Arduino dirige el intercambio de datos con el módulo:

- **MOSI / COPI:** datos enviados desde el controlador hacia el periférico.
- **MISO / CIPO:** datos enviados desde el periférico hacia el controlador.
- **SCK:** señal de reloj que sincroniza la comunicación.
- **CS (Chip Select):** selecciona el periférico con el que se quiere comunicar.

> **No confundir protocolos:** el Monitor Serial se usa para comunicar datos entre Arduino y el computador; la conexión con la microSD utiliza **SPI**. Aunque ambas transfieren bits, su cableado y funcionamiento son diferentes.

## 03 · Preparación de la microSD y conexiones

### Preparación de la tarjeta

1. Utilizar una tarjeta microSD cuyo funcionamiento pueda verificarse en el computador.
2. Formatearla con un sistema de archivos **FAT32** si es necesario, después de respaldar su contenido: el formateo **borra los archivos existentes**.
3. Insertarla en el lector microSD antes de encender el circuito.
4. Mantenerla insertada durante la ejecución. Retirarla **después de detener el programa y desconectar la alimentación**.

### Conexión del módulo microSD — Arduino UNO R3

| Módulo microSD (SPI) | Arduino UNO R3 | Función |
| --- | --- | --- |
| `GND` | `GND` | Tierra común. |
| `VCC` | `5V` **solo si el módulo lo admite**; de lo contrario, tensión y adaptación según su ficha técnica | Alimentación. |
| `MOSI` (o `DI`) | `D11` | Datos hacia la tarjeta. |
| `MISO` (o `DO`) | `D12` | Datos desde la tarjeta. |
| `SCK` (o `CLK`) | `D13` | Reloj. |
| `CS` (o `SS`) | `D10` | Selección del módulo. |

**Este cableado corresponde a Arduino UNO R3.** En otras placas, como ESP8266 o ESP32, los pines y niveles eléctricos pueden ser diferentes; no copiar esta tabla sin comprobar el pinout correspondiente.

### Conexión del sensor TMP36

Para un **TMP36 en encapsulado TO-92**, con la **cara plana mirando hacia nosotros** y las patillas hacia abajo, la disposición habitual es:

| Patilla | TMP36 | Arduino UNO R3 |
| --- | --- | --- |
| Izquierda | `+Vs` (alimentación) | `5V` |
| Centro | `Vout` (señal analógica) | `A0` |
| Derecha | `GND` | `GND` |

**Revisar siempre el encapsulado y la hoja de datos del componente real.** Una conexión invertida puede dañarlo.

## 04 · Primera lectura: del TMP36 a la temperatura

El TMP36 es un sensor analógico cuya salida cambia aproximadamente **10 mV por grado Celsius**. Su salida es de **0,5 V a 0 °C**, por lo que es necesario restar ese desplazamiento (*offset*) para convertir el voltaje a temperatura.

En un Arduino UNO R3 con referencia analógica predeterminada de aproximadamente **5 V**, `analogRead(A0)` devuelve un entero entre **0 y 1023**.

**Paso 1. Convertir la lectura ADC a voltios:**

```text
voltaje = lecturaADC × (5.0 / 1023.0)
```

**Paso 2. Calcular temperatura en Celsius:**

```text
temperaturaC = (voltaje - 0.5) × 100
```

**Paso 3. Convertir a Fahrenheit:**

```text
temperaturaF = (temperaturaC × 9.0 / 5.0) + 32.0
```

Estas fórmulas producen **estimaciones**. La precisión depende de la referencia de voltaje, del sensor y del montaje. Si se usa otra placa o referencia ADC, se debe adaptar la conversión.

### Código guiado: comprobar el sensor en el Monitor Serial

Guardar el sketch como `01_tmp36_serial.ino`:

```cpp
const int PIN_TMP36 = A0;
const float VREF = 5.0; // Referencia aproximada del UNO R3

void setup() {
  Serial.begin(9600);
}

void loop() {
  int lectura = analogRead(PIN_TMP36);
  float voltaje = lectura * (VREF / 1023.0);
  float tempC = (voltaje - 0.5) * 100.0;
  float tempF = (tempC * 9.0 / 5.0) + 32.0;

  Serial.print("ADC: ");
  Serial.print(lectura);
  Serial.print(" | Voltaje: ");
  Serial.print(voltaje, 3);
  Serial.print(" V | Celsius: ");
  Serial.print(tempC, 1);
  Serial.print(" C | Fahrenheit: ");
  Serial.println(tempF, 1);

  delay(1000);
}
```

**Prueba:** abrir el Monitor Serial a **9600 baudios**, observar los valores y comprobar cómo cambia la lectura al aproximar suavemente los dedos al sensor sin forzar sus conexiones.

## 05 · Ejercicio principal: guardar las lecturas en `data.txt`

La biblioteca **`SD.h`** permite inicializar la tarjeta y crear, abrir, escribir y cerrar archivos. Para utilizar la tarjeta conectada mediante SPI, incluimos también **`SPI.h`**.

| Instrucción | ¿Qué hace? |
| --- | --- |
| `#include <SPI.h>` | Habilita la comunicación SPI usada por el módulo. |
| `#include <SD.h>` | Permite trabajar con el sistema de archivos de la tarjeta. |
| `SD.begin(10)` | Inicializa la tarjeta usando el pin `10` como `CS`. |
| `SD.open("data.txt", FILE_WRITE)` | Abre el archivo para escribir; si no existe, normalmente lo crea. |
| `archivo.print()` / `archivo.println()` | Escribe valores y separadores dentro del archivo. |
| `archivo.close()` | Cierra el archivo y finaliza la escritura de los datos pendientes. |

### Código principal: sensor TMP36 + microSD

Guardar como `02_tmp36_microsd.ino`:

```cpp
#include <SPI.h>
#include <SD.h>

const int PIN_TMP36 = A0;
const int PIN_CS = 10;
const float VREF = 5.0; // Arduino UNO R3, referencia predeterminada aproximada

void setup() {
  Serial.begin(9600);

  Serial.println("Iniciando tarjeta microSD...");
  if (!SD.begin(PIN_CS)) {
    Serial.println("Error: no se pudo iniciar la microSD.");
    while (true) { // Detener el programa para revisar conexiones
      delay(1000);
    }
  }

  Serial.println("MicroSD preparada. Comenzando registro.");
}

void loop() {
  // 1. Capturar y transformar la lectura analogica
  int lectura = analogRead(PIN_TMP36);
  float voltaje = lectura * (VREF / 1023.0);
  float tempC = (voltaje - 0.5) * 100.0;
  float tempF = (tempC * 9.0 / 5.0) + 32.0;

  // 2. Crear/abrir el archivo y agregar una fila
  File archivo = SD.open("data.txt", FILE_WRITE);

  if (archivo) {
    archivo.print(lectura);
    archivo.print(";");
    archivo.print(voltaje, 3);
    archivo.print(";");
    archivo.print(tempC, 1);
    archivo.print(";");
    archivo.println(tempF, 1);
    archivo.close(); // Completar la escritura antes de retirar la tarjeta

    Serial.print("Dato guardado. Temperatura: ");
    Serial.print(tempC, 1);
    Serial.println(" C");
  } else {
    Serial.println("Error: no se pudo abrir data.txt");
  }

  delay(1000); // Intervalo aproximado entre lecturas
}
```

### ¿Qué contiene `data.txt`?

Cada fila presenta **cuatro valores separados por punto y coma**, en este orden:

```text
lecturaADC;voltaje_V;temperatura_C;temperatura_F
```

El archivo generado no incluye la línea anterior como encabezado: sirve aquí para explicar el orden de las columnas. El aspecto de las mediciones será similar a este **ejemplo ficticio**:

```text
155;0.758;25.8;78.4
154;0.753;25.3;77.5
155;0.758;25.8;78.4
```

> **Importante:** `FILE_WRITE` agrega nuevas líneas al final del archivo existente. Si se reinicia Arduino, las nuevas lecturas se añaden al mismo `data.txt`. Para comenzar un registro completamente nuevo, respaldar o eliminar el archivo anterior desde un computador.

### Actividad guiada

1. Cargar primero el sketch de lectura del TMP36 y comprobar sus valores en el Monitor Serial.
2. Desconectar la alimentación, conectar el módulo microSD e insertar la tarjeta.
3. Cargar el segundo sketch y verificar el mensaje `MicroSD preparada`.
4. Dejar funcionando el sistema durante **al menos un minuto**.
5. Apagar o desconectar el circuito **antes** de retirar la tarjeta.
6. Abrir la microSD en el computador, localizar `data.txt` y comprobar que contiene varias filas.
7. Abrir el archivo en un editor de texto o importarlo en una planilla, indicando que el separador de columnas es `;`.

**Preguntas para observar:** ¿las nuevas filas se agregan al final? ¿Qué ocurre con los datos después de desconectar Arduino? ¿Por qué es importante `archivo.close()`?

## 06 · Experimentación individual

Modificar el sketch para estudiar una de estas variables:

- **Frecuencia de muestreo:** comparar, por ejemplo, `delay(1000)` y `delay(2000)`. Observar cuántas filas aparecen durante un mismo tiempo total. Los intervalos son aproximados porque cada iteración también demora un tiempo en calcular y escribir.
- **Organización de datos:** cambiar el orden de las columnas y actualizar su descripción en el `README.md` o bitácora.
- **Condición ambiental:** registrar un minuto a temperatura ambiente y otro con una variación suave del entorno. No aplicar calor directo intenso al sensor.

## Solución de problemas frecuentes

| Problema | Qué revisar |
| --- | --- |
| `Error: no se pudo iniciar la microSD` | Formato de la tarjeta; orientación e inserción; alimentación; `CS` en D10; pines D11/D12/D13; compatibilidad eléctrica del módulo. |
| `Error: no se pudo abrir data.txt` | Estado de la tarjeta, conexiones, nombre del archivo y capacidad de escritura. |
| Datos de temperatura poco razonables | Cableado y orientación del TMP36; referencia `VREF`; alimentación; conexiones de A0 y GND. |
| No aparece información en Monitor Serial | Puerto, placa seleccionada, cable USB y velocidad **9600 baudios**. |
| El archivo está vacío o incompleto | Dejar que termine el ciclo y se ejecute `archivo.close()` antes de retirar la tarjeta o cortar la alimentación. |

## Cierre y registro de la actividad

Al finalizar, cada estudiante debería contar con:

- [ ] Sensor TMP36 funcionando y mostrando valores interpretables en el Monitor Serial.
- [ ] Módulo microSD conectado mediante SPI y reconocido por el programa.
- [ ] Sketch `.ino` que capture lecturas y las escriba en `data.txt`.
- [ ] Archivo `data.txt` recuperado desde el computador con al menos **10 mediciones**.
- [ ] Registro fotográfico del circuito y una observación breve sobre el proceso.

---
