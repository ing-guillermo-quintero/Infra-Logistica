import cv2
import numpy as np
import urllib.request
import time
import requests

def iniciar_lector_qr():
    # ========================================================
    # PON AQUÍ LA IP QUE TE APARECE EN EL MONITOR SERIE DEL IDE
    # ========================================================
    ESP32_IP = "192.168.2.98"  
    
    CAPTURE_URL = f"http://{ESP32_IP}/capture"
    SERIAL_URL = f"http://{ESP32_IP}/serial?val="

    detector = cv2.QRCodeDetector()
    ultimo_tiempo_lectura = 0

    print(f"Conectando a ESP32 en {CAPTURE_URL}...")
    print("Presiona 'q' en la ventana de video para salir.")

    while True:
        try:
            # 1. Petición HTTP directa para descargar 1 solo frame (foto)
            img_resp = urllib.request.urlopen(CAPTURE_URL, timeout=2)
            
            # 2. Convertir los bytes descargados a una matriz de OpenCV
            imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
            frame = cv2.imdecode(imgnp, -1)
            
            if frame is None:
                continue

            # 3. Detectar y decodificar el QR
            data, bbox, _ = detector.detectAndDecode(frame)

            # Si detecta algo, dibujar el recuadro
            if bbox is not None and len(bbox) > 0:
                bbox = np.int32(bbox).reshape(-1, 2)
                cv2.polylines(frame, [bbox], isClosed=True, color=(0, 255, 0), thickness=3)

                if data:
                    tiempo_actual = time.time()
                    # Bloqueo de 2 segundos para no hacer spam al serial de la ESP32
                    if (tiempo_actual - ultimo_tiempo_lectura) >= 2.0:
                        print(f"QR Detectado: '{data}'")
                        ultimo_tiempo_lectura = tiempo_actual
                        
                        # Enviar a la ESP32
                        try:
                            requests.get(SERIAL_URL + data, timeout=1)
                        except requests.exceptions.RequestException as e:
                            print("-> Error enviando a ESP32:", e)
                    
                    # Mostrar texto en pantalla
                    cv2.putText(frame, data, (bbox[0][0], bbox[0][1] - 10), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            # Ampliar el cuadro de 240p para que se vea más grande en tu PC
            frame_ampliado = cv2.resize(frame, (640, 480))
            cv2.imshow("Lector QR - ESP32", frame_ampliado)

        except urllib.error.URLError as e:
            print(f"Error de red leyendo la ESP32: {e}. Reintentando...")
            time.sleep(1)
        except Exception as e:
            print(f"Error inesperado: {e}")
            time.sleep(1)

        # Salir con la tecla 'q'
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    iniciar_lector_qr()