# PROYECTO 18 Intersección Semaforizada

## VARIANTE A

**Curso:** Algoritmos  
**Estudiante:** Jefferson Gadiel Yuman Pérez  
**Carné:** 2294-24-10258

## Descripción del proyecto

El proyecto consiste en desarrollar un sistema web de simulación y control de tráfico vehicular en una intersección semaforizada de cuatro calles: Norte, Sur, Este y Oeste.

El sistema permitirá registrar vehículos con sus respectivas placas, velocidades, direcciones de procedencia y horarios de llegada. También permitirá generar vehículos automáticamente para simular diferentes niveles de tráfico durante el día.

Mediante una representación gráfica en dos dimensiones (2D), se visualizará el movimiento de los vehículos, la formación de colas y el funcionamiento de los semáforos.

El sistema contará con un algoritmo de control inteligente que analizará la cantidad de vehículos en cada calle para ajustar los tiempos de los semáforos y favorecer las rutas con mayor congestión.

Además, incluirá cámaras de seguridad virtuales para detectar excesos de velocidad, generar multas y almacenar los resultados en una base de datos MySQL.

## Objetivo general

Desarrollar una aplicación web que permita simular y controlar el flujo vehicular de una intersección semaforizada mediante algoritmos implementados en C++, utilizando una interfaz gráfica 2D y una base de datos MySQL.

## Funcionalidades principales

- Registro individual de vehículos con placa, velocidad, dirección y horario.
- Generación automática de vehículos por calle.
- Simulación de una intersección de cuatro direcciones.
- Representación gráfica 2D desde una vista superior.
- Control de semáforos mediante ciclos configurables.
- Administración de colas de vehículos.
- Detección de las calles con mayor congestión.
- Asignación inteligente de tiempos de luz verde.
- Identificación de los horarios con mayor tráfico.
- Simulación de cámaras de seguridad.
- Generación de multas por exceso de velocidad.
- Cálculo de tiempos de espera y estadísticas acumuladas.
- Almacenamiento de información en MySQL.
- Consulta del historial de simulaciones.

## Listado de tecnologías utilizadas

### 1. C++20

Lenguaje de programación principal del proyecto. Se utilizará para desarrollar el backend, los algoritmos de simulación, la administración de colas, el control de semáforos, el cálculo de estadísticas y la generación de multas.

### 2. Crow

Framework de C++ que permite desarrollar aplicaciones web y servicios HTTP. Se utilizará para construir la API REST que permitirá la comunicación entre el frontend y la lógica implementada en C++.

### 3. CMake

Herramienta de configuración y compilación de proyectos C++. Permitirá organizar los archivos del backend, administrar la compilación y facilitar la ejecución desde CLion.

### 4. vcpkg

Administrador de paquetes para C++. Se utilizará para instalar y administrar las bibliotecas externas necesarias, evitando configurar manualmente cada dependencia.

### 5. MySQL 8

Sistema de gestión de bases de datos relacionales. Permitirá almacenar vehículos, calles, simulaciones, configuraciones de semáforos, multas y estadísticas.

### 6. MySQL Connector/C++

Biblioteca que permite conectar aplicaciones desarrolladas en C++ con bases de datos MySQL. Se utilizará para ejecutar consultas y realizar operaciones de almacenamiento y recuperación de información.

### 7. HTML5

Lenguaje de marcado utilizado para construir la estructura de las páginas web, formularios, menús, botones y paneles del sistema.

### 8. CSS3

Tecnología utilizada para diseñar la apariencia visual de la aplicación, incluyendo colores, tamaños, distribución de elementos, estilos y adaptación a diferentes resoluciones.

### 9. JavaScript

Lenguaje de programación utilizado en el frontend para gestionar las interacciones del usuario, comunicarse con la API REST y actualizar visualmente la simulación.

### 10. HTML5 Canvas 2D

Tecnología gráfica que permite dibujar elementos y animaciones dentro del navegador. Se utilizará para representar la intersección, las calles, los vehículos, los semáforos y sus movimientos desde una vista superior.

### 11. Chart.js

Biblioteca de JavaScript utilizada para crear gráficas estadísticas. Permitirá visualizar la cantidad de vehículos, los horarios de mayor tráfico, los tiempos de espera y otros resultados de las simulaciones.

### 12. Git

Sistema de control de versiones que permitirá registrar los cambios realizados durante el desarrollo, organizar el trabajo mediante ramas y conservar el historial del proyecto.

### 13. GitHub

Plataforma utilizada para almacenar el repositorio del proyecto, compartir el código fuente, administrar las ramas de desarrollo y facilitar su descarga y revisión.

### 14. CLion

Entorno de desarrollo integrado (IDE) utilizado para escribir, organizar, compilar, ejecutar y depurar el código C++ del proyecto.

## Arquitectura del sistema

El proyecto se organizará en tres componentes principales:

**Frontend:** desarrollado con HTML5, CSS3, JavaScript y Canvas 2D. Se encargará de mostrar la interfaz gráfica y las animaciones.

**Backend:** desarrollado en C++20 con Crow. Contendrá los algoritmos, las reglas de negocio, el motor de simulación y los servicios de la aplicación.

**Base de datos:** implementada en MySQL. Almacenará los datos necesarios para el funcionamiento del sistema y los resultados de las simulaciones.

## Estructura del proyecto

- `backend/`: código fuente y configuración del servidor C++.
- `frontend/`: páginas web, estilos, scripts y recursos gráficos.
- `database/`: scripts para la creación y configuración de MySQL.
- `docs/`: documentación técnica e instrucciones de instalación.
- `CMakeLists.txt`: configuración principal de compilación.
- `vcpkg.json`: declaración de dependencias de C++.
- `.gitignore`: archivos y carpetas excluidos del repositorio.

## Instalación y ejecución

Las instrucciones de instalación y configuración del proyecto estarán disponibles en el archivo `docs/INSTALACION.md`.

Se documentará cómo clonar el repositorio desde GitHub, abrirlo en CLion, configurar las dependencias, preparar MySQL, compilar el backend y ejecutar la aplicación web.


## API REST

SISTRA cuenta con un backend desarrollado en C++20 utilizando Crow.

El servidor HTTP se ejecuta localmente en:

http://localhost:18080

### Endpoints disponibles

| Metodo | Endpoint | Descripcion |
|--------|----------|-------------|
| GET | /api/health | Verifica el funcionamiento del servidor |
| GET | /api/sistema | Devuelve informacion general del sistema |
| GET | /api/calles | Consulta las calles registradas en MySQL |
| GET | /api/database/status | Verifica la conexion con MySQL |

La documentacion completa de la API se encuentra en:

[Documentacion de la API](docs/API.md)

Los endpoints para vehiculos, simulaciones, semaforos, multas y estadisticas se incorporaran en las siguientes fases.

## Estado del proyecto

**Fase 1 de 13 — Configuración inicial.**

Actualmente se está preparando la estructura base del proyecto y el entorno de desarrollo.

Las funcionalidades de simulación, conexión a MySQL e interfaz web se implementarán progresivamente en las siguientes fases.