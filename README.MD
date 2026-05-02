# 📦 Proyecto de Infraestructura y Logística

## 📖 Descripción General
Este repositorio contiene la documentación técnica, diseños mecánicos, planos generales y ruteo electrónico de un sistema de infraestructura logística automatizada.

El proyecto integra componentes mecánicos (estructura y banda transportadora), diseño eléctrico-electrónico y pruebas del sistema de lectura QR, permitiendo la identificación y transporte eficiente de elementos dentro del sistema.

---

## 🗂️ Estructura del Proyecto
.
├── CADs/
│ ├── Infraestructura.STEP
│ └── Banda_transportadora.STEP
│
├── Diseño_Plano_General/
│ └── (Planos generales del sistema)
│
├── Firmware_Infraestructura/
│
├── Pruebas_unitarias_Sistema_Lectura_QR/
│
├── Ruteo_eléctrico-electrónico/
│ └── (Archivo Proteus - conexiones electrónicas)
│
└── Sistema_Lectura_QR/


---

## 📁 Descripción de Carpetas

### 🔧 CADs/
Contiene los modelos 3D del sistema en formato `.STEP`, fundamentales para el diseño mecánico:

- **Infraestructura.STEP**  
  Representa la estructura completa del sistema logístico.

- **Banda_transportadora.STEP**  
  Modelo detallado de la banda transportadora, incluyendo soportes y sistema de desplazamiento.

Estos archivos permiten:
- Visualización tridimensional
- Validación dimensional
- Preparación para fabricación

---

### 📐 Diseño_Plano_General/
Incluye los planos generales del sistema, donde se define:

- Distribución de la infraestructura
- Ubicación de los módulos
- Relación entre componentes mecánicos

Sirve como referencia principal para el montaje físico del sistema.

---

### ⚙️ Firmware_Infraestructura/
Contiene el desarrollo del firmware encargado del control del sistema:

- Lógica de operación
- Control de actuadores
- Lectura de sensores
- Integración con el sistema de lectura QR

---

### 🧪 Pruebas_unitarias_Sistema_Lectura_QR/
Incluye pruebas orientadas a validar el funcionamiento del sistema de lectura QR:

- Precisión de lectura
- Tiempo de respuesta
- Integración con el sistema general

---

### 🔌 Ruteo_eléctrico-electrónico/
Contiene el diseño electrónico del sistema:

- Archivo de simulación en **Proteus**
- Conexiones del controlador
- Integración de sensores

Permite:
- Simulación del circuito
- Verificación de conexiones
- Validación del diseño electrónico

---

### 📡 Sistema_Lectura_QR/
Módulo encargado de la lectura e interpretación de códigos QR:

- Captura de datos
- Procesamiento de información
- Comunicación con el sistema principal

---

## 🧩 Componentes del Sistema

- Estructura mecánica modular
- Banda transportadora
- Sensores electrónicos
- Controlador principal
- Sistema de lectura QR

---

## 🎯 Objetivo del Proyecto

Desarrollar un sistema automatizado de logística que permita:

- Transporte eficiente de elementos
- Identificación mediante códigos QR
- Integración mecánica y electrónica
- Escalabilidad del sistema

---

## 🛠️ Herramientas Utilizadas

- Software CAD (archivos `.STEP`)
- Proteus (simulación electrónica)
- Sistemas embebidos (controlador)
- Herramientas de desarrollo de firmware

---

## 📌 Notas

- Los archivos `.STEP` pueden visualizarse en herramientas como SolidWorks, Fusion 360 o FreeCAD.
- El archivo de Proteus requiere el entorno instalado para su simulación.
- Este repositorio está orientado tanto a documentación técnica como a desarrollo del sistema.

---
