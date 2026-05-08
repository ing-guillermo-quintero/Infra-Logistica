import cv2
import numpy as np
import urllib.request
import time
import requests
import json
import datetime
import os
import winsound

def generar_ficha_tecnica_estetica(producto, zona, espacio, fecha_hora, img_producto):
    # Crear un lienzo negro (Fondo de terminal industrial)
    ancho, alto = 800, 500
    ficha = np.zeros((alto, ancho, 3), dtype=np.uint8)
    ficha[:] = (18, 18, 18)  # Gris muy oscuro

    # Dibujar borde decorativo de neón azul
    color_neon = (255, 229, 0) # Cian/Neón en BGR es (255, 229, 0) o similar
    cv2.rectangle(ficha, (10, 10), (ancho-10, alto-10), color_neon, 2)
    cv2.line(ficha, (10, 70), (ancho-10, 70), color_neon, 2)

    # Título de la Ficha
    cv2.putText(ficha, "SISTEMA DE CONTROL MECATRONICO - FICHA TECNICA", (150, 45), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, color_neon, 2)

    # Insertar imagen del producto a la izquierda
    if img_producto is not None:
        img_res = cv2.resize(img_producto, (300, 300))
        # Crear un marco para la imagen
        cv2.rectangle(ficha, (30, 100), (330, 400), (255, 255, 255), 1)
        ficha[100:400, 30:330] = img_res

    # Escribir los datos a la derecha
    x_offset = 360
    y_start = 140
    separacion = 50

    datos = [
        ("ID PRODUCTO:", str(producto).upper()),
        ("ZONA:", str(zona)),
        ("ESPACIO:", str(espacio)),
        ("ESTADO:", "REGISTRADO"),
        ("FECHA/HORA:", str(fecha_hora))
    ]

    for i, (label, value) in enumerate(datos):
        y_pos = y_start + (i * separacion)
        cv2.putText(ficha, label, (x_offset, y_pos), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (150, 150, 150), 1)
        cv2.putText(ficha, value, (x_offset + 140, y_pos), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

    # Decoración inferior (Serial / Footer)
    cv2.putText(ficha, f"SCAN_ID: {int(time.time())} | PROYECTO DISENO 2", (30, alto-25), 
                cv2.FONT_HERSHEY_SIMPLEX, 0.4, (100, 100, 100), 1)

    return ficha

def iniciar_lector_qr():
    # ========================================================
    # CONFIGURACIÓN DE ORIGEN DE VIDEO
    # ========================================================
    USAR_ESP32 = False  # Cambiar a True cuando uses la ESP32
    CAMARA_LOCAL = 0    # 0 para webcam, o URL de DroidCam
    ESP32_IP = "192.168.2.98"  
    
    CAPTURE_URL = f"http://{ESP32_IP}/capture"
    SERIAL_URL = f"http://{ESP32_IP}/serial?val="
    IMG_DIR = "imagenes"

    detector = cv2.QRCodeDetector()
    ultimo_tiempo_lectura = 0

    if USAR_ESP32:
        print(f"Conectando a ESP32 en {CAPTURE_URL}...")
    else:
        print(f"Iniciando cámara local/DroidCam...")
        cap = cv2.VideoCapture(CAMARA_LOCAL)

    print("Presiona 'q' en la ventana de video para salir.")

    while True:
        try:
            if USAR_ESP32:
                img_resp = urllib.request.urlopen(CAPTURE_URL, timeout=2)
                imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
                frame = cv2.imdecode(imgnp, -1)
            else:
                ret, frame = cap.read()
                if not ret:
                    time.sleep(1)
                    continue
            
            if frame is None:
                continue

            data, bbox, _ = detector.detectAndDecode(frame)

            if bbox is not None and len(bbox) > 0:
                bbox = np.int32(bbox).reshape(-1, 2)
                cv2.polylines(frame, [bbox], isClosed=True, color=(0, 255, 0), thickness=3)

                if data:
                    tiempo_actual = time.time()
                    if (tiempo_actual - ultimo_tiempo_lectura) >= 2.0:
                        ultimo_tiempo_lectura = tiempo_actual
                        fecha_hora = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        
                        # Extraer datos
                        producto_detectado = data
                        zona = "N/A"
                        espacio = "N/A"
                        try:
                            datos_qr = json.loads(data)
                            producto_detectado = datos_qr.get("producto", "Desconocido")
                            zona = datos_qr.get("zona", "N/A")
                            espacio = datos_qr.get("espacio", "N/A")
                        except json.JSONDecodeError:
                            pass

                        # Buscar imagen
                        exts = [".jpg", ".png", ".jpeg", ".webp"]
                        img_producto = None
                        for ext in exts:
                            path = os.path.join(IMG_DIR, f"{producto_detectado}{ext}")
                            if os.path.exists(path):
                                img_producto = cv2.imread(path)
                                break
                        
                        # Sonido de escáner (Frecuencia 1000Hz, Duración 200ms)
                        winsound.Beep(1000, 200)

                        # Mostrar Ficha Técnica Estética
                        ficha_visual = generar_ficha_tecnica_estetica(
                            producto_detectado, zona, espacio, fecha_hora, img_producto
                        )
                        cv2.imshow("FICHA TECNICA PREMIUM", ficha_visual)

                        if USAR_ESP32:
                            try:
                                requests.get(SERIAL_URL + str(producto_detectado), timeout=1)
                            except:
                                pass
                    
                    cv2.putText(frame, "QR OK", (bbox[0][0], bbox[0][1] - 10), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            if USAR_ESP32:
                frame = cv2.resize(frame, (640, 480))
            cv2.imshow("Lector QR", frame)

        except Exception as e:
            time.sleep(1)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    if not USAR_ESP32:
        cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    iniciar_lector_qr()