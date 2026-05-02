# Sistema de Visión y Detección QR con ESP32

Este proyecto contiene un script en Python (`Prueba1Webservice.py`) que actúa como cliente para un módulo ESP32 equipado con cámara. El sistema captura imágenes enviadas por el ESP32, procesa las imágenes para detectar códigos QR y envía la información decodificada de vuelta al microcontrolador.

## Arquitectura del Sistema

La arquitectura está basada en un modelo **Cliente-Servidor** a través de red local (Wi-Fi), distribuido en dos componentes principales:

1. **Servidor (Edge Device - ESP32):**
   - El ESP32 actúa como un servidor web HTTP.
   - **Cámara:** Se encarga de capturar el entorno de manera continua.
   - **Endpoint `/capture`:** Proveedor de imágenes. Al hacer una petición GET, responde con el frame actual de la cámara (foto en formato binario/JPEG).
   - **Endpoint `/serial?val=<datos>`:** Receptor de comandos. Permite recibir cadenas de texto (en este caso, la información del QR) y procesarlas o mostrarlas por el monitor serie del microcontrolador.

2. **Cliente (Procesamiento de Visión - PC con Python):**
   - El script de Python realiza el procesamiento pesado que el ESP32 no puede o le costaría mucho hacer (Computer Vision).
   - Realiza _polling_ constante (peticiones HTTP directas) al endpoint `/capture` para simular un stream de video en tiempo real.
   - Decodifica la imagen utilizando `numpy` y `OpenCV`.
   - Utiliza `cv2.QRCodeDetector()` para analizar la imagen y buscar códigos QR.
   - Si detecta un código QR, envía el dato obtenido de regreso al ESP32 a través del endpoint `/serial`.

## Flujo de Funcionamiento del Código (`Prueba1Webservice.py`)

1. **Configuración y Conexión:**
   Se define la dirección IP del ESP32 (`ESP32_IP = "192.168.2.98"`) en la red local. A partir de ella se construyen las URLs para la captura de imágenes (`CAPTURE_URL`) y el envío de datos (`SERIAL_URL`).

2. **Bucle de Captura:**
   Entra en un ciclo infinito `while True` donde:
   - Se solicita la imagen al ESP32 mediante `urllib.request.urlopen`.
   - Los bytes recibidos se transforman en un arreglo (array) de `numpy`.
   - Se decodifica la imagen a un formato matricial (frame) compatible con OpenCV (`cv2.imdecode`).

3. **Detección y Decodificación del QR:**
   - El detector analiza el frame. Si encuentra un código QR, extrae las coordenadas (`bbox`) y la información en texto (`data`).
   - El código dibuja un recuadro verde alrededor del QR detectado utilizando `cv2.polylines`.

4. **Retorno de Información (Feedback) al ESP32:**
   - Se implementa una lógica de anti-spam (temporizador de 2 segundos mediante `time.time()`) para no sobrecargar de peticiones al ESP32 si el mismo QR permanece frente a la cámara.
   - Si hay una nueva lectura (o pasaron más de 2 segundos), la información es mostrada en la terminal local y enviada de vuelta al ESP32 utilizando la librería `requests` (`requests.get(SERIAL_URL + data)`).

5. **Visualización en PC:**
   - Para mejorar la visibilidad (dado que la resolución base parece ser baja, por ejemplo, 240p), el frame es redimensionado (`cv2.resize` a 640x480).
   - El flujo de video modificado (con los recuadros verdes y el texto) se muestra en una ventana interactiva (`cv2.imshow`).
   - El bucle puede detenerse en cualquier momento presionando la tecla **`q`**.

## Requisitos y Dependencias

Para ejecutar este script, es necesario contar con Python instalado y las siguientes librerías:

- `opencv-python` (cv2)
- `numpy`
- `requests`
- `urllib` (Librería estándar de Python)
- `time` (Librería estándar de Python)

Para instalar las dependencias externas (si no están instaladas), puedes ejecutar:

```bash
pip install opencv-python numpy requests
```

## Instrucciones de Uso

1. Cargar el firmware correspondiente en el ESP32 que levante el servidor web de la cámara y los endpoints (`/capture`, `/serial`).
2. Abrir el Monitor Serie del IDE de Arduino (u otro de preferencia) para observar la dirección IP asignada al ESP32.
3. Modificar la variable `ESP32_IP` en la **línea 10** del archivo `Prueba1Webservice.py` con la IP obtenida.
4. Ejecutar el script:
   ```bash
   python Prueba1Webservice.py
   ```
5. Colocar un código QR frente a la cámara del ESP32. Se podrá ver en la ventana del PC el recuadro de reconocimiento y el ESP32 recibirá la información.
6. Presionar la tecla `q` en la ventana del video para cerrar el programa.
