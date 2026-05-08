import qrcode
import json
import os

# Aquí puedes poner los nombres exactos de los 4 productos 
# de los cuales ya guardaste las imágenes en la carpeta "imagenes".
productos = [
    {"producto": "messi", "zona": "A", "espacio": 1},
    {"producto": "rosas", "zona": "A", "espacio": 2},
    {"producto": "superman", "zona": "B", "espacio": 1},
    {"producto": "wall-e", "zona": "B", "espacio": 2}
]

# Crear carpeta para guardar los QR generados si no existe
output_dir = "codigos_qr"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

for p in productos:
    # Convertir el diccionario a string en formato JSON
    data_json = json.dumps(p)
    
    # Configurar y generar el QR
    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=10,
        border=4,
    )
    qr.add_data(data_json)
    qr.make(fit=True)

    # Crear la imagen y guardarla
    img = qr.make_image(fill_color="black", back_color="white")
    filepath = os.path.join(output_dir, f"QR_{p['producto']}.png")
    img.save(filepath)
    
    print(f"✅ Generado: {filepath} -> {data_json}")