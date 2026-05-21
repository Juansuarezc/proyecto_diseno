/*
 * Proyecto: Medidor de transmitancia a 630 nm
 * Integrantes: Juan David Suarez Corzo y Michelle Garzon Campos
 * Hardware: ARDUINO UNO, ATmega328P
 * Asignatura: Diseño de Sistemas Electrónicos
 * Universidad Industrial de Santander
 */

#include <Wire.h>               // Librería para comunicación I2C (SDA/SCL)
#include <Adafruit_GFX.h>       // Librería para gráficos (dibujar letras, líneas)
#include <Adafruit_SSD1306.h>   // Librería específica para tu pantalla OLED

// Configuración de la pantalla OLED (128x64 píxeles)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1        // No usamos pin de reset en la pantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pinSensor = A0;  // Pin donde llega la señal de tu MCP6001
const int muestras = 20;   // Filtro: cantidad de lecturas para promediar

void setup() {
  // 1. Iniciar la comunicación con la pantalla (Dirección I2C estándar: 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Si no detecta la pantalla, el chip se queda congelado aquí (seguridad)
    for(;;); 
  }

  // 2. Limpiar cualquier basura de la pantalla al encender
  display.clearDisplay();
  display.setTextColor(WHITE); // Color del texto (blanco sobre negro)
}

void loop() {
  // ---------------------------------------------------------
  // ETAPA 1: LECTURA DEL SENSOR (Oversampling)
  // ---------------------------------------------------------
  long suma = 0;
  for (int i = 0; i < muestras; i++) {
    suma += analogRead(pinSensor); // Lee el voltaje (0 a 1023)
    delay(2);
  }
  int valorADC = suma / muestras; // Promedio limpio de ruido

  // ---------------------------------------------------------
  // ETAPA 2: MATEMÁTICA (Cálculo de Transmitancia)
  // ---------------------------------------------------------
  // Convertimos el valor crudo del ADC (0-1023) a porcentaje (0.00% - 100.00%)
  // Nota: Quizás necesites calibrar el "1023.0" por el valor máximo real de tu sensor.
  float transmitancia = (valorADC / 1023.0) * 100.0;

  // ---------------------------------------------------------
  // ETAPA 3: ACTUALIZAR LA PANTALLA
  // ---------------------------------------------------------
  display.clearDisplay();      // Borrar la lectura anterior

  // Imprimir la palabra "Transmitancia:"
  display.setTextSize(1);      // Tamaño de texto pequeño
  display.setCursor(10, 10);   // Coordenadas (X, Y) en la pantalla
  display.print("Transmitancia:");

  // Imprimir el valor "XXX.XX %"
  display.setTextSize(2);      // Tamaño de texto grande para el número
  display.setCursor(15, 30);   // Lo centramos un poco más abajo
  
  // El '2' le dice a la función print que solo muestre 2 decimales (XXX.XX)
  display.print(transmitancia, 2); 
  display.print(" %");

  // Esta función es la que realmente "empuja" los datos y enciende los píxeles
  display.display(); 

  // Esperar medio segundo antes de tomar otra medición
  delay(500); 
}