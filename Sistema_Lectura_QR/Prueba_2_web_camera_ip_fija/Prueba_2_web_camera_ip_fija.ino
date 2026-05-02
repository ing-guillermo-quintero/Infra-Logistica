#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ==========================================
// 1. Configuración de Red
// ==========================================
const char* ssid = "ESP_CAM_RED";
const char* password = "";

// --- NUEVO: Configuración de IP Fija ---
IPAddress local_IP(192, 168, 2, 98);
IPAddress gateway(192, 168, 2, 1);   // Asegúrate de que esta sea la IP de tu router
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);    // Opcional
IPAddress secondaryDNS(8, 8, 4, 4);  // Opcional
// ---------------------------------------

// Pines AI-Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

httpd_handle_t camera_httpd = NULL;

// ==========================================
// 2. Handlers del Servidor
// ==========================================

// Handler para enviar una sola FOTO (No streaming)
static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Fallo al capturar la imagen");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  
  esp_camera_fb_return(fb); // Liberar memoria inmediatamente
  return res;
}

// Handler para recibir la letra del QR
static esp_err_t serial_handler(httpd_req_t *req) {
  char buf[64];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char value[32];
    if (httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
      Serial.print("QR Detectado: ");
      Serial.println(value);
    }
  }
  
  // Agregar cabeceras CORS para evitar bloqueos
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, "OK", 2);
  return ESP_OK;
}

// Mensaje básico si entras por error a la raíz
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  const char* resp = "<h1>ESP32 Activa</h1><p>Usa Python para procesar la imagen desde /capture</p>";
  return httpd_resp_send(req, resp, strlen(resp));
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
  httpd_uri_t capture_uri = { .uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL };
  httpd_uri_t serial_uri = { .uri = "/serial", .method = HTTP_GET, .handler = serial_handler, .user_ctx = NULL };
  
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &serial_uri);
  }
}

// ==========================================
// 3. Setup y Loop
// ==========================================
void setup() {
  Serial.begin(115200);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM; config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM; config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM; config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  // Resolución baja (240p) para que las peticiones por WiFi vuelen
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Fallo de cámara: 0x%x", err);
    return;
  }

  // Subir contraste para mejor lectura B/N de los QR
  sensor_t * s = esp_camera_sensor_get();
  s->set_contrast(s, 2);

  // --- NUEVO: Aplicar configuración de IP Fija ---
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Fallo al configurar IP estática");
  }
  // -----------------------------------------------

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\nWiFi conectado");
  startCameraServer();
  Serial.print("IP de la ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(10000);
}