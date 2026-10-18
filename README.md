
# SISTRA - Sistema de Simulación de Tráfico

## PROYECTO 18 - Intersección Semaforizada

### VARIANTE A

**Curso:** Algoritmos  
**Estudiante:** Jefferson Gadiel Yuman Pérez  
**Carné:** 2294-24-10258

## Descripción del proyecto

SISTRA es un sistema web de simulación y control de tráfico vehicular desarrollado para representar el funcionamiento de una intersección semaforizada de cuatro calles: Norte, Sur, Este y Oeste.

El sistema permite registrar vehículos, asignarlos a simulaciones, establecer sus velocidades y segundos de llegada, y observar su comportamiento durante el desarrollo de una simulación.

La aplicación cuenta con un motor de simulación desarrollado en C++ que administra el avance del tiempo, los estados de los vehículos, las colas de tráfico y los cruces de la intersección.

También incorpora semáforos inteligentes, una representación gráfica bidimensional (2D), cámaras virtuales de control de velocidad, generación de multas y estadísticas del tráfico.

La información se almacena en una base de datos MySQL, permitiendo consultar simulaciones, vehículos y resultados incluso después de reiniciar el servidor.

## Objetivo general

Desarrollar una aplicación web que permita simular y controlar el flujo vehicular de una intersección semaforizada mediante algoritmos implementados en C++, utilizando una interfaz gráfica 2D y una base de datos MySQL.

## Objetivos específicos

- Registrar y administrar vehículos dentro del sistema.
- Simular el tránsito vehicular en una intersección de cuatro calles.
- Administrar los estados y movimientos de los vehículos.
- Implementar el funcionamiento de semáforos inteligentes.
- Representar gráficamente la intersección y el movimiento vehicular.
- Detectar infracciones por exceso de velocidad.
- Registrar multas mediante cámaras virtuales.
- Calcular estadísticas del comportamiento del tráfico.
- Almacenar los resultados de las simulaciones en MySQL.
- Permitir la consulta de información histórica.
- Recuperar el estado de simulaciones en ejecución después de reiniciar el backend.

## Funcionalidades desarrolladas

### 1. Administración de vehículos

SISTRA permite registrar vehículos y consultar la información almacenada.

Cada vehículo puede contar con información como:

- Identificador.
- Placa.
- Tipo de vehículo.
- Velocidad.
- Calle de procedencia.

Los vehículos pueden incorporarse a una simulación configurada indicando su velocidad y segundo de llegada.

### 2. Administración de simulaciones

El sistema permite consultar las simulaciones registradas y administrar su ejecución.

Las simulaciones utilizan los siguientes estados:

- Configurada.
- Ejecutando.
- Finalizada.

Cada simulación almacena información sobre su duración, hora simulada de inicio, progreso, vehículos y fechas de ejecución.

### 3. Motor de simulación

El motor de simulación está desarrollado en C++ y administra el comportamiento de los vehículos dentro de la intersección.

Entre sus responsabilidades se encuentran:

- Controlar el tiempo simulado.
- Incorporar vehículos según su segundo de llegada.
- Administrar las colas de vehículos.
- Controlar el cruce de la intersección.
- Actualizar los estados de los vehículos.
- Registrar los segundos de inicio de cruce y salida.
- Calcular tiempos de espera.
- Finalizar las simulaciones cuando corresponde.

El backend permite avanzar una simulación segundo a segundo o avanzar varios segundos mediante una solicitud HTTP.

### 4. Semáforos inteligentes

SISTRA incorpora lógica para administrar los semáforos de la intersección.

El sistema considera el tráfico de las calles para controlar el paso de vehículos y administrar los cruces.

### 5. Visualización gráfica 2D

La aplicación dispone de una interfaz web que representa la intersección desde una vista superior.

La visualización permite observar:

- Las cuatro calles de la intersección.
- Los vehículos.
- El movimiento vehicular.
- Los semáforos.
- El desarrollo de la simulación.

La representación utiliza tecnologías web y Canvas 2D.

### 6. Control de velocidad y multas

SISTRA incorpora cámaras virtuales para detectar excesos de velocidad.

Durante la simulación, el sistema verifica las condiciones de velocidad y registra las multas correspondientes.

Las multas se almacenan en MySQL y pueden incluir distintos montos según la infracción.

El sistema también evita volver a registrar una multa ya generada al recuperar y continuar una simulación.

### 7. Estadísticas de tráfico

El sistema permite consultar estadísticas de las simulaciones.

Entre los datos disponibles se encuentran:

- Total de vehículos.
- Vehículos programados.
- Vehículos en cola.
- Vehículos cruzando.
- Vehículos finalizados.
- Tiempo promedio de espera.
- Estadísticas por calle.
- Total de multas.
- Monto acumulado de multas.

### 8. Persistencia y recuperación

SISTRA utiliza MySQL para almacenar la información de las simulaciones y sus resultados.

El sistema permite:

- Consultar simulaciones almacenadas.
- Recuperar simulaciones en ejecución.
- Conservar el progreso de la simulación.
- Consultar el historial de vehículos.
- Conservar las multas registradas.
- Consultar estadísticas después de reiniciar el backend.

Las simulaciones finalizadas conservan sus resultados históricos y no pueden reanudarse como simulaciones activas.

### 9. Validación de solicitudes

La API REST valida los datos recibidos y responde con códigos HTTP según el resultado de las operaciones.

Entre los códigos utilizados se encuentran:

- `200 OK`: solicitud procesada correctamente.
- `400 Bad Request`: datos inválidos o incompletos.
- `404 Not Found`: recurso inexistente.
- `409 Conflict`: operación incompatible con el estado actual.

## Tecnologías utilizadas

### Backend

**C++20**

Lenguaje principal utilizado para implementar los algoritmos, el motor de simulación y la lógica del sistema.

**Crow**

Framework utilizado para desarrollar el servidor HTTP y los endpoints de la API REST.

**CMake**

Herramienta utilizada para configurar y compilar el proyecto C++.

**vcpkg**

Administrador de dependencias utilizado en la configuración del entorno C++.

### Base de datos

**MySQL / MariaDB**

Sistema de base de datos relacional utilizado para almacenar vehículos, simulaciones, calles, multas y demás información del proyecto.

El backend se comunica con la base de datos para realizar consultas, inserciones y actualizaciones.

### Frontend

**HTML5**

Define la estructura de la interfaz web.

**CSS3**

Permite establecer la apariencia visual de la aplicación.

**JavaScript**

Administra la interacción del usuario, las solicitudes a la API y la actualización de la interfaz.

**Canvas 2D**

Permite representar gráficamente la intersección y el movimiento de los vehículos.

### Herramientas de desarrollo

**CLion**

Entorno de desarrollo utilizado para trabajar con el proyecto C++.

**Git**

Sistema de control de versiones utilizado para registrar los cambios.

**GitHub**

Plataforma utilizada para almacenar y compartir el repositorio del proyecto.

## Arquitectura del sistema

SISTRA utiliza una arquitectura dividida en tres componentes principales.

### Frontend

Desarrollado con HTML, CSS y JavaScript.

Se encarga de presentar la información, permitir la interacción del usuario y representar gráficamente la simulación.

### Backend

Desarrollado en C++20 utilizando Crow.

Contiene:

- Rutas de la API REST.
- Servicios de lógica de negocio.
- Motor de simulación.
- Administración de vehículos.
- Control de semáforos.
- Control de velocidad.
- Generación de multas.
- Cálculo de estadísticas.
- Operaciones de persistencia.

### Base de datos

Implementada mediante MySQL.

Almacena la información necesaria para ejecutar las simulaciones y conservar sus resultados.

## Estructura general del proyecto

El repositorio está organizado en componentes para separar las responsabilidades del sistema.

```text
SISTRA/
├── backend/
├── frontend/
├── database/
├── docs/
├── CMakeLists.txt
├── vcpkg.json
├── .gitignore
└── README.md
```

Las carpetas cumplen las siguientes funciones:

- `backend/`: código fuente del servidor y lógica desarrollada en C++.
- `frontend/`: archivos de la interfaz web.
- `database/`: scripts relacionados con la base de datos.
- `docs/`: documentación técnica y de uso.
- `CMakeLists.txt`: configuración de compilación.
- `vcpkg.json`: declaración de dependencias.
- `.gitignore`: configuración de archivos excluidos del control de versiones.

## API REST

El backend de SISTRA utiliza Crow para exponer servicios HTTP.

La dirección local utilizada durante el desarrollo es:

`http://localhost:18080`

### Endpoints generales

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| GET | `/api/health` | Verifica el funcionamiento del servidor |
| GET | `/api/sistema` | Consulta información del sistema |
| GET | `/api/calles` | Consulta las calles de la intersección |
| GET | `/api/database/status` | Verifica la conexión con la base de datos |

### Endpoints de vehículos

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| GET | `/api/vehiculos` | Consulta los vehículos registrados |
| GET | `/api/vehiculos/{id}` | Consulta un vehículo específico |
| POST | `/api/vehiculos` | Registra un vehículo |

### Endpoints de simulaciones

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| GET | `/api/simulaciones` | Consulta el listado de simulaciones |
| GET | `/api/simulaciones/{id}` | Consulta una simulación |
| POST | `/api/simulaciones/{id}/vehiculos` | Agrega un vehículo a una simulación |
| GET | `/api/simulaciones/{id}/vehiculos` | Consulta los vehículos de una simulación |
| POST | `/api/simulaciones/{id}/iniciar` | Inicia una simulación configurada |
| POST | `/api/simulaciones/{id}/avanzar` | Avanza un segundo de simulación |
| POST | `/api/simulaciones/{id}/avanzar-multiples` | Avanza varios segundos |
| GET | `/api/simulaciones/{id}/estado` | Consulta el estado del motor de una simulación en ejecución |
| GET | `/api/simulaciones/{id}/estadisticas` | Consulta estadísticas de una simulación |

### Ejemplo: avanzar varios segundos

Solicitud:

```http
POST /api/simulaciones/9/avanzar-multiples
Content-Type: application/json
```

Cuerpo JSON:

```json
{
  "segundos": 10
}
```

El campo `segundos` debe contener un número entero entre 1 y 3600.

El backend valida el contenido de la solicitud y el estado de la simulación antes de ejecutar el avance.

### Ejemplo: consultar estadísticas

```http
GET /api/simulaciones/9/estadisticas
```

La respuesta incluye información sobre el estado de la simulación, la cantidad de vehículos, las estadísticas por calle y las multas registradas.

El identificador `9` corresponde a una simulación utilizada durante las pruebas de integración y debe sustituirse por el identificador que se desee consultar.

Para información adicional, consultar:

[Documentación de la API](docs/API.md)

## Instalación y ejecución

### Requisitos generales

Para trabajar con el proyecto se requiere:

- Un entorno compatible con C++20.
- CMake.
- CLion o un entorno de desarrollo compatible.
- Las dependencias C++ configuradas para el proyecto.
- MySQL o MariaDB.
- Un navegador web.
- Git para clonar el repositorio.

### Clonar el repositorio

```bash
git clone https://github.com/jyuman-umg/SISTRA.git
```

### Abrir el proyecto

1. Abrir CLion.
2. Seleccionar la opción para abrir un proyecto existente.
3. Seleccionar la carpeta de SISTRA.
4. Permitir que CMake cargue la configuración del proyecto.
5. Verificar que las dependencias estén disponibles.

### Configurar la base de datos

Preparar MySQL utilizando los scripts y la configuración del proyecto.

La base de datos debe encontrarse disponible para que el backend pueda realizar las operaciones de persistencia.

### Ejecutar el backend

Compilar y ejecutar el servidor C++ desde CLion utilizando la configuración correspondiente.

Durante el desarrollo, el servidor se ejecuta en:

`http://localhost:18080`

### Utilizar la interfaz web

Abrir la interfaz web del proyecto desde el entorno local configurado.

La interfaz se comunica con el backend para consultar información y controlar las simulaciones.

Las instrucciones técnicas adicionales se documentan en:

[Guía de instalación](docs/INSTALACION.md)

## Pruebas de integración

Durante la Fase 11 se realizaron pruebas para verificar el funcionamiento conjunto del backend, el motor de simulación y la base de datos.

Entre las comprobaciones realizadas se encuentran:

- Registro de vehículos en simulaciones.
- Inicio de simulaciones configuradas.
- Avance individual y múltiple del tiempo simulado.
- Finalización de simulaciones.
- Registro de multas por exceso de velocidad.
- Prevención de multas duplicadas.
- Recuperación de simulaciones después de reiniciar el servidor.
- Persistencia de resultados.
- Consulta de estadísticas.
- Consulta del historial de vehículos.
- Validación de solicitudes incorrectas.
- Manejo de errores HTTP 400, 404 y 409.

Estas pruebas permitieron comprobar las principales operaciones del sistema.

## Estado del proyecto

**Fase 12 de 13 - Documentación del proyecto.**

### Fases desarrolladas

Las Fases 1 a 11 comprenden la preparación del entorno, el desarrollo de los componentes del sistema, la implementación del motor de simulación, la interfaz gráfica, los semáforos, el control de velocidad, la persistencia y las pruebas de integración.

### Fase actual

**Fase 12 - Documentación**

Se está actualizando la documentación general y técnica del proyecto para facilitar su instalación, comprensión y utilización.

### Fase pendiente

**Fase 13 - Entrega final**

Comprenderá la preparación y revisión de los elementos necesarios para presentar el proyecto terminado.

## Repositorio

El código fuente de SISTRA se encuentra disponible en GitHub:

https://github.com/jyuman-umg/SISTRA

## Autor

**Jefferson Gadiel Yuman Pérez**  
**Curso:** Algoritmos  
**Proyecto:** 18 - Intersección Semaforizada  
**Variante:** A
