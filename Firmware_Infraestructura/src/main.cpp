#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// CONFIGURACIÓN WIFI Y MQTT
// ==========================================
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD";
const char* mqtt_server = "IP_DE_TU_BROKER";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Mutex para proteger la librería MQTT entre los dos núcleos
SemaphoreHandle_t mqttMutex;

// ==========================================
// DEFINICIÓN DE PINES (ESP32-S3)
// ==========================================
// Zona de Recolección - Motor
const int PIN_ENA = 4;
const int PIN_IN1 = 5;
const int PIN_IN2 = 6;

// Zona de Recolección - Sensores IR
const int PIN_SENSOR_IR1 = 7;
const int PIN_SENSOR_IR2 = 8;

// Zona de Almacenamiento - Registro de Desplazamiento
const int LATCH_PIN = 9;
const int CLOCK_PIN = 10;
const int DATA_PIN  = 11;

// Zona de Despacho
const int PIN_SENSOR_DESPACHO = 12;

// ==========================================
// VARIABLES DE ESTADO
// ==========================================
enum EstadoBanda { BANDA_CORRIENDO, ESPERANDO_PARADA, CAJA_LISTA };
EstadoBanda estadoActualBanda = BANDA_CORRIENDO;

unsigned long tiempoDeteccionIR1 = 0;
const unsigned long TIEMPO_ESPERA_PARADA = 2000;
unsigned long ultimoEnvioSensores = 0;
const unsigned long INTERVALO_ENVIO_SENSORES = 1000;
int estadoAnteriorDespacho = -1;

// ==========================================
// PROTOTIPOS DE FUNCIONES (Requerido en C++/PlatformIO)
// ==========================================
void setup_wifi();
void reconnect();
void arrancarBanda();
void detenerBanda();
uint16_t leerRegistroDesplazamiento();
void TareaSensoresYRed(void *pvParameters);
void TareaRecepcionYDespacho(void *pvParameters);

// ==========================================
// FUNCIONES DE RED
// ==========================================
void setup_wifi() {
  vTaskDelay(pdMS_TO_TICKS(10));
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
}

void reconnect() {
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32_S3_Almacen")) {
      Serial.println("Conectado al broker");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}

// ==========================================
// FUNCIONES DE CONTROL FÍSICO
// ==========================================
void arrancarBanda() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 200);
}

void detenerBanda() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 0);
}

uint16_t leerRegistroDesplazamiento() {
  digitalWrite(LATCH_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(LATCH_PIN, HIGH);
  
  uint16_t valores = 0;
  for (int i = 0; i < 16; i++) {
    int bitLeido = digitalRead(DATA_PIN);
    valores |= (bitLeido << (15 - i));
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(CLOCK_PIN, LOW);
  }
  return valores;
}

// ==========================================
// TAREAS DE LOS NÚCLEOS (FreeRTOS)
// ==========================================

// --- NÚCLEO 0: Sensores de Almacenamiento y Red MQTT ---
void TareaSensoresYRed(void *pvParameters) {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  for (;;) {
    if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      xSemaphoreGive(mqttMutex);
    }

    if (millis() - ultimoEnvioSensores >= INTERVALO_ENVIO_SENSORES) {
      ultimoEnvioSensores = millis();
      uint16_t sensoresAlmacen = leerRegistroDesplazamiento();
      
      String payload = "{";
      payload += "\"FilaA\": [";
      for(int i=0; i<5; i++) {
        payload += String(bitRead(sensoresAlmacen, i));
        if(i < 4) payload += ",";
      }
      payload += "], \"FilaB\": [";
      for(int i=0; i<5; i++) {
        payload += String(bitRead(sensoresAlmacen, i + 8));
        if(i < 4) payload += ",";
      }
      payload += "]}";
      
      if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
        client.publish("almacen/almacenamiento/celdas", payload.c_str());
        xSemaphoreGive(mqttMutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

// --- NÚCLEO 1: Recepción (Banda) y Despacho ---
void TareaRecepcionYDespacho(void *pvParameters) {
  arrancarBanda();

  for (;;) {
    bool deteccionIR1 = (digitalRead(PIN_SENSOR_IR1) == LOW);
    bool deteccionIR2 = (digitalRead(PIN_SENSOR_IR2) == LOW);

    switch (estadoActualBanda) {
      case BANDA_CORRIENDO:
        if (deteccionIR1) {
          tiempoDeteccionIR1 = millis();
          estadoActualBanda = ESPERANDO_PARADA;
        }
        break;

      case ESPERANDO_PARADA:
        if (millis() - tiempoDeteccionIR1 >= TIEMPO_ESPERA_PARADA) {
          detenerBanda();
          if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
            client.publish("almacen/recoleccion/estado", "CAJA_EN_ESPERA");
            xSemaphoreGive(mqttMutex);
          }
          estadoActualBanda = CAJA_LISTA;
        }
        break;

      case CAJA_LISTA:
        if (!deteccionIR2) { 
          arrancarBanda();
          if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
            client.publish("almacen/recoleccion/estado", "CAJA_RECOGIDA_BANDA_ACTIVA");
            xSemaphoreGive(mqttMutex);
          }
          estadoActualBanda = BANDA_CORRIENDO;
        }
        break;
    }

    int estadoActualDespacho = digitalRead(PIN_SENSOR_DESPACHO);
    if (estadoActualDespacho != estadoAnteriorDespacho) {
      if (xSemaphoreTake(mqttMutex, portMAX_DELAY) == pdTRUE) {
        if (estadoActualDespacho == LOW) {
          client.publish("almacen/despacho/estado", "CAJA_LISTA_PARA_DESPACHO");
        } else {
          client.publish("almacen/despacho/estado", "CELDA_DESPACHO_VACIA");
        }
        xSemaphoreGive(mqttMutex);
      }
      estadoAnteriorDespacho = estadoActualDespacho;
    }

    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

// ==========================================
// CONFIGURACIÓN INICIAL (SETUP)
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_SENSOR_IR1, INPUT_PULLUP);
  pinMode(PIN_SENSOR_IR2, INPUT_PULLUP);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);
  pinMode(PIN_SENSOR_DESPACHO, INPUT_PULLUP);

  mqttMutex = xSemaphoreCreateMutex();
  if (mqttMutex == NULL) {
    Serial.println("Error al crear el Mutex");
    while(1);
  }

  xTaskCreatePinnedToCore(
    TareaSensoresYRed, "TareaRed", 8192, NULL, 1, NULL, 0
  );

  xTaskCreatePinnedToCore(
    TareaRecepcionYDespacho, "TareaMotores", 4096, NULL, 1, NULL, 1
  );

  vTaskDelete(NULL); 
}

void loop() {
  // Vacío intencionalmente (Controlado por FreeRTOS)
}