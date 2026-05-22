# Guía de Programación del ATmega328P mediante Arduino como ISP

Este repositorio contiene la documentación técnica y el firmware del prototipo autónomo para la medición de transmitancia óptica a 630 nm. En este documento se detalla el procedimiento paso a paso para configurar y programar el microcontrolador **ATmega328P-AU** de montaje superficial (SMD) integrado en la PCB, utilizando una placa **Arduino UNO** convencional como programador físico externo a través del puerto **ICSP (In-Circuit Serial Programming)**.

## 🛠️ Requisitos y Herramientas
* **Arduino IDE** instalado en la computadora.
* 1x Placa **Arduino UNO**.
* Cables jumper (Macho-Hembra).
* Firmware de la aplicación (código del sensor y la pantalla OLED).

---

## 📌 Conexión de Pines (Mapeo ICSP)

Para realizar la programación, se debe interconectar el Arduino UNO con el header ICSP de la PCB objetivo siguiendo estrictamente la distribución de la siguiente tabla:

| Pin en el Header ICSP de la PCB | Conexión en la Placa Arduino UNO | Función Técnica |
| :--- | :--- | :--- |
| **VCC** | Pin 5V | Alimentación eléctrica de la PCB |
| **GND** | Pin GND | Referencia común de tierra |
| **RESET** | Pin 10 | Control de reinicio para forzar el modo de programación |
| **MOSI** | Pin 11 | Entrada de datos en serie del maestro (Master Out Slave In) |
| **MISO** | Pin 12 | Salida de datos en serie del esclavo (Master In Slave Out) |
| **SCK** | Pin 13 | Señal de reloj de sincronización (Serial Clock) |

---

## 🚀 Procedimiento de Programación Paso a Paso

### Fase 1: Configurar el Arduino UNO como Programador (ISP)
1. Conecte **únicamente** la placa Arduino UNO a su computadora utilizando el cable USB.
2. Abra el **Arduino IDE**.
3. Diríjase al menú superior: `Archivo` ➔ `Ejemplos` ➔ `11.ArduinoISP` ➔ `ArduinoISP`.
4. Suba este código al Arduino UNO de la forma habitual (presionando el botón de la flecha hacia la derecha). 
5. Una vez que el IDE muestre *"Subido"*, la placa habrá dejado de ser un Arduino común y se habrá convertido en un programador físico dedicado. **Desconecte el cable USB de la computadora antes de proceder al cableado.**

### Fase 2: Conexión del Hardware en Modo ISP
1. Con la alimentación desconectada, use los cables jumpers para unir los pines del header ICSP de su PCB con los pines del Arduino UNO, guiándose exclusivamente por la **Tabla de Mapeo ICSP** descrita anteriormente.
2. Revise detalladamente que ninguna conexión esté invertida (comúnmente se suele errar confundiendo MISO con MOSI).
3. Vuelva a conectar el Arduino UNO a la computadora mediante el cable USB. Notará que la PCB personalizada se encenderá de forma automática, puesto que toma los 5V y el GND directamente desde el Arduino.

### Fase 3: Configuración del Hardware del ATmega328P (Quemar Fusibles)
*Nota técnica: Como el chip ATmega328P-AU viene nuevo de fábrica, viene preconfigurado para usar un oscilador interno lento de 1 MHz. Este paso es obligatorio para programar los registros internos (Fuses) e indicarle al chip que trabaje con el cristal externo de 16 MHz ruteado en la PCB.*

1. En el Arduino IDE, vaya al menú superior `Herramientas` ➔ `Placa` y seleccione **"Arduino Uno"**. *(Esto le indica al compilador los parámetros del hardware final: 16 MHz, oscilador externo y memoria).*
2. Diríjase a `Herramientas` ➔ `Programador` y seleccione **"Arduino as ISP"** (Asegúrese de escoger la opción que incluye la palabra "**as**", evite seleccionar *ArduinoISP*).
3. Vaya nuevamente al menú `Herramientas` y haga clic en la opción **Quemar Bootloader** (Burn Bootloader).
4. El LED interno del Arduino UNO parpadeará intensamente durante unos segundos. Espere hasta leer el mensaje en la barra de estado: *"Bootloader quemado con éxito"*.

### Fase 4: Carga del Firmware del Proyecto
1. Abra el sketch de su firmware final (el código que realiza el promedio de muestras del sensor de luz y controla la interfaz de la pantalla OLED).
2. Asegúrese de mantener los mismos parámetros en el menú `Herramientas` (Placa: *Arduino Uno* y Programador: *Arduino as ISP*).
3. **¡ADVERTENCIA!** No presione el botón de flecha convencional para subir el código. Si lo hace, intentará programar el Arduino UNO y borrará el software de programador instalado en la Fase 1.
4. En su lugar, diríjase al menú superior **`Programa`** (Sketch) y haga clic en la opción **`Subir usando programador`** (Upload Using Programmer), o utilice el atajo de teclado `Ctrl + Shift + U`.
5. Una vez terminada la carga, el programador liberará el pin de RESET de su PCB. El ATmega328P-AU comenzará a ejecutar las instrucciones de inmediato, inicializando la interfaz I2C y mostrando los valores de transmitancia en el display OLED.

---

## 📝 Firmware del Sistema

A continuación, se detalla el código en C++ desarrollado para la inicialización de periféricos, el algoritmo de sobremuestreo (oversampling) para reducción de ruido en el ADC y el despliegue de datos en pantalla:

```cpp
#include <Wire.h>               // Librería para comunicación I2C (SDA/SCL)
#include <Adafruit_GFX.h>       // Librería para gráficos (dibujar letras, líneas)
#include <Adafruit_SSD1306.h>   // Librería específica para la pantalla OLED

// Configuración de la pantalla OLED (128x64 píxeles)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1        // No usamos pin de reset físico en la pantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pinSensor = A0;  // Pin analógico de lectura (Salida del amplificador MCP6001)
const int muestras = 20;   // Filtro digital: cantidad de lecturas para promediar

void setup() {
  // Inicializar comunicación I2C con la pantalla OLED (Dirección estándar: 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Detener ejecución si la pantalla no responde
  }

  display.clearDisplay();
  display.setTextColor(WHITE); // Texto blanco sobre fondo negro
}

void loop() {
  // ETAPA 1: ADQUISICIÓN DE DATOS Y FILTRADO DIGITAL (Oversampling)
  long suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += analogRead(pinSensor); // Lectura del voltaje digitalizado (0 a 1023)
    delay(2);                     // Pausa de estabilización para el ADC
  }
  int valorADC = suma / muestras;  // Promedio móvil para supresión de ruido

  // ETAPA 2: PROCESAMIENTO MATEMÁTICO (Cálculo de Transmitancia)
  // Conversión lineal del valor analógico de 10 bits a porcentaje de luz (0.00% - 100.00%)
  float transmitancia = (valorADC / 1023.0) * 100.0;

  // ETAPA 3: ACTUALIZACIÓN DE LA INTERFAZ GRÁFICA (Pantalla OLED)
  display.clearDisplay();      

  // Renderizado de etiqueta fija
  display.setTextSize(1);      
  display.setCursor(10, 10);   
  display.print("Transmitancia:");

  // Renderizado del valor numérico calculado
  display.setTextSize(2);      
  display.setCursor(15, 30);   
  display.print(transmitancia, 2); // Restringe el despliegue a 2 dígitos decimales (XXX.XX)
  display.print(" %");

  display.display(); // Envío de la matriz de píxeles a través del bus I2C

  delay(500); // Frecuencia de refresco del lazo de control
}
