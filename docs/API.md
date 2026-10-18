
# SISTRA - Documentación de la API REST

## Proyecto 18 - Intersección Semaforizada

### Variante A

SISTRA es un sistema de simulación y control de tráfico vehicular desarrollado en C++20.

El backend utiliza Crow para proporcionar servicios HTTP y MySQL para almacenar vehículos, simulaciones, movimientos, multas y resultados históricos.

Este documento describe los endpoints implementados y utilizados durante el desarrollo del proyecto hasta la Fase 11.

## 1. Tecnologías utilizadas

- **C++20:** lenguaje principal del backend.
- **Crow:** framework para desarrollar la API REST.
- **MySQL:** sistema de base de datos relacional.
- **libmariadb:** biblioteca utilizada para conectar C++ con MySQL.
- **CMake:** configuración y compilación del proyecto.
- **vcpkg:** administración de dependencias.
- **MinGW:** compilador utilizado en Windows.
- **CLion:** entorno de desarrollo.

## 2. Servidor HTTP

El backend se ejecuta localmente en:

`http://localhost:18080`

La API utiliza JSON para intercambiar información con el frontend y otros clientes HTTP.

### Encabezado para solicitudes JSON

```http
Content-Type: application/json
```

### Códigos HTTP utilizados

| Código | Significado |
|---|---|
| 200 | Solicitud procesada correctamente |
| 201 | Recurso registrado correctamente |
| 400 | Datos inválidos o solicitud mal formada |
| 404 | Recurso inexistente |
| 409 | Conflicto con el estado o los datos existentes |
| 500 | Error interno del servidor |
| 503 | Servicio de base de datos no disponible |

Los códigos específicos dependen de cada endpoint y del resultado de la operación.

## 3. Endpoints generales

### 3.1. GET /api/health

Comprueba que el servidor HTTP se encuentra funcionando.

**Respuesta esperada:**

```json
{
  "success": true,
  "message": "Servidor SISTRA funcionando correctamente",
  "system": "SISTRA",
  "version": "1.0.0"
}
```

**Código HTTP:** 200.

### 3.2. GET /api/sistema

Devuelve información general del proyecto.

**Respuesta esperada:**

```json
{
  "nombre": "SISTRA",
  "descripcion": "Sistema Inteligente de Simulacion y Control de Trafico Vehicular",
  "proyecto": 18,
  "variante": "A",
  "tipoInterseccion": "Semaforizada",
  "cantidadCalles": 4
}
```

**Código HTTP:** 200.

### 3.3. GET /api/calles

Consulta las calles activas registradas en MySQL.

La intersección dispone de cuatro calles:

- Norte.
- Sur.
- Este.
- Oeste.

**Ejemplo de respuesta:**

```json
{
  "success": true,
  "message": "Calles consultadas correctamente",
  "total": 4,
  "data": [
    {
      "idCalle": 1,
      "nombre": "Norte",
      "direccionEntrada": "Norte",
      "direccionSalida": "Sur",
      "estadoRegistro": "A"
    },
    {
      "idCalle": 2,
      "nombre": "Sur",
      "direccionEntrada": "Sur",
      "direccionSalida": "Norte",
      "estadoRegistro": "A"
    },
    {
      "idCalle": 3,
      "nombre": "Este",
      "direccionEntrada": "Este",
      "direccionSalida": "Oeste",
      "estadoRegistro": "A"
    },
    {
      "idCalle": 4,
      "nombre": "Oeste",
      "direccionEntrada": "Oeste",
      "direccionSalida": "Este",
      "estadoRegistro": "A"
    }
  ]
}
```

**Código HTTP:** 200.

Si ocurre un error de conexión o consulta, el servidor puede responder con HTTP 500.

### 3.4. GET /api/database/status

Comprueba la disponibilidad de la conexión con MySQL.

**Respuesta exitosa:**

```json
{
  "success": true,
  "connected": true,
  "database": "sistra_db",
  "message": "Conexion con MySQL funcionando correctamente"
}
```

**Código HTTP:** 200.

**Respuesta cuando la base de datos no está disponible:**

```json
{
  "success": false,
  "connected": false,
  "message": "No se pudo conectar con MySQL"
}
```

**Código HTTP:** 503.

## 4. Gestión de vehículos

### 4.1. GET /api/vehiculos

Consulta los vehículos activos registrados en MySQL.

**Código HTTP exitoso:** 200.

La respuesta contiene:

- `success`.
- `message`.
- `total`.
- `data`.

Cada vehículo incluye información como su identificador, placa, tipo, fecha de registro y estado.

### 4.2. GET /api/vehiculos/{id}

Consulta un vehículo activo mediante su identificador.

**Posibles respuestas:**

| Código | Descripción |
|---|---|
| 200 | Vehículo encontrado |
| 400 | Identificador inválido |
| 404 | Vehículo inexistente o inactivo |
| 503 | Conexión con MySQL no disponible |
| 500 | Error interno |

### 4.3. POST /api/vehiculos

Registra un vehículo nuevo.

**Ejemplo de solicitud:**

```json
{
  "placa": "P123ABC",
  "tipoVehiculo": "Automovil"
}
```

**Tipos de vehículo permitidos:**

- `Automovil`.
- `Motocicleta`.
- `Camion`.
- `Bus`.

**Validaciones:**

- La placa es obligatoria.
- Debe contener entre 5 y 10 caracteres alfanuméricos.
- Se convierte a mayúsculas.
- El tipo de vehículo debe ser válido.
- No se permiten placas duplicadas.

**Posibles respuestas:**

| Código | Descripción |
|---|---|
| 201 | Vehículo registrado |
| 400 | Datos inválidos |
| 409 | Placa duplicada |
| 503 | Base de datos no disponible |
| 500 | Error interno |

## 5. Gestión de simulaciones

### 5.1. GET /api/simulaciones

Consulta las simulaciones registradas en MySQL.

**Código HTTP exitoso:** 200.

La respuesta contiene:

- `success`.
- `message`.
- `total`.
- `data`.

Cada simulación puede incluir:

- `idSimulacion`.
- `nombre`.
- `descripcion`.
- `fechaCreacion`.
- `fechaInicio`.
- `fechaFin`.
- `horaSimuladaInicio`.
- `duracionSimuladaSegundos`.
- `segundoActual`.
- `estado`.
- `totalVehiculos`.
- `totalVehiculosFinalizados`.

**Ejemplo de respuesta con una simulación:**

```json
{
  "success": true,
  "message": "Simulaciones consultadas correctamente",
  "total": 1,
  "data": [
    {
      "idSimulacion": 9,
      "nombre": "Prueba Fase 11 - Multas",
      "descripcion": "Verificar que las multas no se dupliquen despues de recuperar el motor",
      "fechaCreacion": "2026-10-18 04:33:45",
      "fechaInicio": "2026-10-18 04:34:46",
      "fechaFin": "2026-10-18 04:37:49",
      "horaSimuladaInicio": "08:00:00",
      "duracionSimuladaSegundos": 120,
      "segundoActual": 120,
      "estado": "Finalizada",
      "totalVehiculos": 1,
      "totalVehiculosFinalizados": 1
    }
  ]
}
```

Los datos del ejemplo corresponden a una simulación utilizada durante las pruebas. El listado real puede contener varios registros.

### 5.2. GET /api/simulaciones/{id}

Consulta una simulación específica.

**Ejemplo:**

```http
GET /api/simulaciones/9
```

**Ejemplo de respuesta:**

```json
{
  "success": true,
  "message": "Simulacion consultada correctamente",
  "data": {
    "idSimulacion": 9,
    "nombre": "Prueba Fase 11 - Multas",
    "descripcion": "Verificar que las multas no se dupliquen despues de recuperar el motor",
    "fechaCreacion": "2026-10-18 04:33:45",
    "fechaInicio": "2026-10-18 04:34:46",
    "fechaFin": "2026-10-18 04:37:49",
    "horaSimuladaInicio": "08:00:00",
    "duracionSimuladaSegundos": 120,
    "segundoActual": 120,
    "estado": "Finalizada",
    "totalVehiculos": 1,
    "totalVehiculosFinalizados": 1
  }
}
```

**Posibles respuestas:**

| Código | Descripción |
|---|---|
| 200 | Simulación encontrada |
| 400 | Identificador inválido |
| 404 | Simulación inexistente |
| 503 | Base de datos no disponible |
| 500 | Error interno |

### 5.3. POST /api/simulaciones/{id}/vehiculos

Incorpora un vehículo existente a una simulación configurada.

**Ejemplo de solicitud:**

```http
POST /api/simulaciones/9/vehiculos
Content-Type: application/json
```

**Cuerpo JSON:**

```json
{
  "idVehiculo": 2,
  "idCalle": 1,
  "velocidad": 95,
  "segundoLlegada": 0
}
```

**Campos:**

| Campo | Descripción |
|---|---|
| `idVehiculo` | Identificador del vehículo |
| `idCalle` | Identificador de la calle |
| `velocidad` | Velocidad asignada al vehículo |
| `segundoLlegada` | Segundo en el que llega a la intersección |

**Validaciones:**

- La simulación debe existir.
- Debe encontrarse en estado `Configurada`.
- El vehículo debe existir y estar activo.
- La calle debe existir y estar activa.
- La velocidad no puede ser negativa.
- El segundo de llegada debe estar dentro de la duración de la simulación.
- Un mismo vehículo no puede registrarse dos veces en la misma simulación.

El servidor calcula la hora de llegada utilizando la hora simulada de inicio y el segundo de llegada.

El registro del vehículo y la actualización del total de vehículos se realizan mediante una transacción MySQL.

**Posibles respuestas:**

| Código | Descripción |
|---|---|
| 201 | Vehículo incorporado |
| 400 | Datos inválidos |
| 404 | Simulación, vehículo o calle inexistente |
| 409 | Vehículo duplicado o simulación no configurable |
| 503 | Base de datos no disponible |
| 500 | Error interno |

### 5.4. GET /api/simulaciones/{id}/vehiculos

Consulta los vehículos incorporados a una simulación.

**Ejemplo:**

```http
GET /api/simulaciones/9/vehiculos
```

**Respuesta obtenida durante las pruebas:**

```json
{
  "success": true,
  "message": "Vehiculos de la simulacion consultados correctamente",
  "idSimulacion": 9,
  "nombreSimulacion": "Prueba Fase 11 - Multas",
  "estadoSimulacion": "Finalizada",
  "total": 1,
  "data": [
    {
      "idSimulacionVehiculo": 43,
      "idVehiculo": 2,
      "placa": "P201ABC",
      "tipoVehiculo": "Automovil",
      "idCalle": 1,
      "calle": "Norte",
      "velocidad": 95,
      "segundoLlegada": 0,
      "horaLlegada": "08:00:00",
      "segundoInicioCruce": 0,
      "horaInicioCruce": "08:00:00",
      "segundoSalida": 5,
      "horaSalida": "08:00:05",
      "tiempoEsperaSegundos": 0,
      "estado": "Finalizado"
    }
  ]
}
```

Los vehículos se consultan desde la información almacenada en MySQL.

## 6. Estados de las simulaciones

SISTRA administra tres estados principales:

| Estado | Descripción |
|---|---|
| `Configurada` | La simulación puede recibir vehículos y prepararse para su ejecución |
| `Ejecutando` | La simulación se encuentra iniciada y puede avanzar |
| `Finalizada` | La simulación terminó y conserva sus resultados |

### Reglas de transición

- Una simulación configurada puede iniciarse.
- Una simulación en ejecución puede avanzar.
- Al alcanzar su duración, la simulación finaliza.
- No se pueden incorporar nuevos vehículos a una simulación iniciada.
- Una simulación finalizada no puede iniciarse nuevamente mediante el endpoint de inicio.

## 7. Inicio y avance de simulaciones

### 7.1. POST /api/simulaciones/{id}/iniciar

Inicia una simulación configurada.

**Ejemplo:**

```http
POST /api/simulaciones/9/iniciar
```

La operación prepara el motor de simulación para comenzar su ejecución.

**Regla principal:**

La simulación debe encontrarse en estado `Configurada`.

Si se intenta iniciar una simulación finalizada, el servidor devuelve:

**HTTP 409:**

```json
{
  "success": false,
  "message": "Solo se puede iniciar una simulacion Configurada"
}
```

### 7.2. POST /api/simulaciones/{id}/avanzar

Avanza un segundo del tiempo simulado.

**Ejemplo:**

```http
POST /api/simulaciones/9/avanzar
```

Durante el avance, el motor puede:

- Incorporar vehículos programados.
- Administrar colas.
- Actualizar los cruces.
- Modificar los estados de los vehículos.
- Registrar tiempos de espera.
- Actualizar los resultados de la simulación.
- Registrar multas cuando corresponde.

**Regla principal:**

La simulación debe encontrarse en estado `Ejecutando`.

Si se intenta avanzar una simulación finalizada:

**HTTP 409:**

```json
{
  "success": false,
  "message": "La simulacion no esta en estado Ejecutando"
}
```

### 7.3. POST /api/simulaciones/{id}/avanzar-multiples

Permite avanzar varios segundos mediante una sola solicitud HTTP.

**Ejemplo:**

```http
POST /api/simulaciones/9/avanzar-multiples
Content-Type: application/json
```

**Cuerpo JSON:**

```json
{
  "segundos": 10
}
```

### Validación del campo segundos

El campo `segundos` debe cumplir las siguientes condiciones:

- Ser obligatorio.
- Ser un número JSON.
- Ser un valor entero.
- Ser mayor o igual a 1.
- Ser menor o igual a 3600.

**Valores válidos:**

```json
{
  "segundos": 1
}
```

```json
{
  "segundos": 10
}
```

```json
{
  "segundos": 3600
}
```

**Valores inválidos:**

- `0`.
- `-1`.
- `3601`.
- `1.5`.
- `"10"`.
- `null`.
- Campo `segundos` ausente.

Cuando el campo es inválido, el servidor responde:

**HTTP 400:**

```json
{
  "success": false,
  "message": "El campo segundos debe ser un entero entre 1 y 3600"
}
```

### Validación del JSON

Si el cuerpo no contiene un objeto JSON válido, el servidor responde:

**HTTP 400:**

```json
{
  "success": false,
  "message": "Debe enviar un objeto JSON valido"
}
```

### Finalización durante el avance múltiple

Si una simulación alcanza su duración configurada, el motor finaliza su ejecución.

Por ejemplo, una simulación de 120 segundos puede pasar de `Ejecutando` a `Finalizada` cuando alcanza el segundo 120.

El motor no debe continuar ejecutando segundos después de la finalización.

## 8. Estados de los vehículos

Durante una simulación, los vehículos pueden encontrarse en los siguientes estados:

| Estado | Descripción |
|---|---|
| `Programado` | Todavía no ha llegado a la intersección |
| `EnCola` | Llegó y está esperando su turno |
| `Cruzando` | Está atravesando la intersección |
| `Finalizado` | Completó su recorrido |

El motor actualiza estos estados conforme avanza el tiempo simulado.

### Datos de seguimiento

Cada vehículo puede almacenar:

- Segundo de llegada.
- Hora de llegada.
- Segundo de inicio de cruce.
- Hora de inicio de cruce.
- Segundo de salida.
- Hora de salida.
- Tiempo de espera.
- Estado actual.

Estos datos permiten consultar el historial de circulación de los vehículos.

## 9. Consulta del estado del motor

### GET /api/simulaciones/{id}/estado

Consulta el estado del motor de una simulación en ejecución.

**Ejemplo:**

```http
GET /api/simulaciones/9/estado
```

El endpoint permite obtener información del progreso de una simulación activa.

### Recuperación después de un reinicio

Si el backend se reinicia mientras una simulación permanece en estado `Ejecutando`, el sistema puede reconstruir su estado a partir de los datos persistidos en MySQL.

La recuperación permite continuar la simulación sin comenzar nuevamente desde cero.

### Simulaciones finalizadas

Una simulación finalizada no puede recuperarse como motor activo.

En ese caso, el endpoint responde:

**HTTP 409:**

```json
{
  "success": false,
  "message": "La simulacion no esta en estado Ejecutando"
}
```

Los resultados históricos siguen disponibles mediante los endpoints de consulta de simulaciones, vehículos y estadísticas.

## 10. Estadísticas de simulaciones

### GET /api/simulaciones/{id}/estadisticas

Consulta las estadísticas de una simulación.

**Ejemplo:**

```http
GET /api/simulaciones/9/estadisticas
```

**Respuesta obtenida durante las pruebas de integración:**

```json
{
  "success": true,
  "idSimulacion": 9,
  "nombreSimulacion": "Prueba Fase 11 - Multas",
  "estadoSimulacion": "Finalizada",
  "duracionSegundos": 120,
  "estadisticas": {
    "totalVehiculos": 1,
    "programados": 0,
    "enCola": 0,
    "cruzando": 0,
    "finalizados": 1,
    "tiempoPromedioEsperaSegundos": 0,
    "calles": [
      {
        "idCalle": 1,
        "nombre": "Norte",
        "totalVehiculos": 1,
        "enCola": 0,
        "finalizados": 1,
        "tiempoPromedioEsperaSegundos": 0
      },
      {
        "idCalle": 2,
        "nombre": "Sur",
        "totalVehiculos": 0,
        "enCola": 0,
        "finalizados": 0,
        "tiempoPromedioEsperaSegundos": 0
      },
      {
        "idCalle": 3,
        "nombre": "Este",
        "totalVehiculos": 0,
        "enCola": 0,
        "finalizados": 0,
        "tiempoPromedioEsperaSegundos": 0
      },
      {
        "idCalle": 4,
        "nombre": "Oeste",
        "totalVehiculos": 0,
        "enCola": 0,
        "finalizados": 0,
        "tiempoPromedioEsperaSegundos": 0
      }
    ],
    "multas": {
      "totalMultas": 1,
      "montoTotal": 600
    }
  }
}
```

### Estadísticas generales

El sistema permite conocer:

- Total de vehículos.
- Vehículos programados.
- Vehículos en cola.
- Vehículos cruzando.
- Vehículos finalizados.
- Tiempo promedio de espera.

### Estadísticas por calle

Para cada calle se puede consultar:

- Identificador.
- Nombre.
- Total de vehículos.
- Vehículos en cola.
- Vehículos finalizados.
- Tiempo promedio de espera.

### Estadísticas de multas

La respuesta también contiene:

- `totalMultas`: cantidad de multas registradas.
- `montoTotal`: suma de los montos de las multas.

Los montos se expresan en quetzales.

## 11. Control de velocidad y multas

SISTRA incorpora cámaras virtuales de control de velocidad.

Durante la ejecución de una simulación, el sistema puede detectar vehículos que superan el límite de velocidad configurado.

Cuando corresponde, se registra una multa en MySQL.

### Información de multas

Las estadísticas permiten consultar:

- Cantidad total de multas.
- Monto acumulado de las multas.

Durante las pruebas se verificó el registro de una multa de Q600 para un vehículo que circulaba a 95 km/h en la calle Norte.

### Prevención de duplicados

El sistema contempla la prevención de multas duplicadas al recuperar y continuar una simulación.

Durante las pruebas de integración se comprobó que:

1. El vehículo generó una multa.
2. Se reinició el backend.
3. Se recuperó la simulación.
4. Se continuó avanzando el tiempo.
5. La multa no volvió a registrarse.

De esta manera se conserva la integridad de los resultados almacenados.

## 12. Persistencia en MySQL

El backend utiliza MySQL para almacenar la información necesaria para el funcionamiento del sistema.

Entre los datos persistidos se encuentran:

- Vehículos.
- Calles.
- Simulaciones.
- Vehículos incorporados a simulaciones.
- Estados y movimientos.
- Multas.
- Resultados de ejecución.

### Integridad de los datos

El sistema utiliza mecanismos como:

- Consultas parametrizadas para operaciones con datos externos.
- Validación de solicitudes JSON.
- Restricciones de claves foráneas.
- Restricciones de unicidad.
- Transacciones MySQL.
- Configuración de credenciales mediante variables de entorno.

### Transacciones

Las operaciones que requieren actualizar varios registros relacionados utilizan transacciones para mantener la consistencia de los datos.

Por ejemplo, la incorporación de un vehículo a una simulación actualiza tanto el registro del vehículo asociado como el total de vehículos de la simulación.

## 13. Recuperación de simulaciones

La recuperación permite continuar una simulación en ejecución después de reiniciar el backend.

### Funcionamiento general

1. Una simulación se encuentra en estado `Ejecutando`.
2. El motor avanza y actualiza la información persistida.
3. El backend se detiene o reinicia.
4. El backend vuelve a iniciarse.
5. Se consulta o continúa la simulación.
6. El sistema reconstruye el estado necesario desde MySQL.

### Restricciones

- La simulación debe encontrarse en estado `Ejecutando`.
- Una simulación `Finalizada` no puede recuperarse como activa.
- Los resultados históricos permanecen disponibles.
- Las multas ya registradas no deben duplicarse.

## 14. Ejemplos de solicitudes desde PowerShell

Los siguientes comandos permiten utilizar la API desde Windows.

### Consultar simulaciones

```powershell
Invoke-RestMethod `
    -Uri "http://localhost:18080/api/simulaciones" `
    -Method GET |
    ConvertTo-Json -Depth 10
```

### Consultar una simulación

```powershell
Invoke-RestMethod `
    -Uri "http://localhost:18080/api/simulaciones/9" `
    -Method GET |
    ConvertTo-Json -Depth 10
```

### Consultar vehículos de una simulación

```powershell
Invoke-RestMethod `
    -Uri "http://localhost:18080/api/simulaciones/9/vehiculos" `
    -Method GET |
    ConvertTo-Json -Depth 10
```

### Consultar estadísticas

```powershell
Invoke-RestMethod `
    -Uri "http://localhost:18080/api/simulaciones/9/estadisticas" `
    -Method GET |
    ConvertTo-Json -Depth 10
```

### Avanzar varios segundos

```powershell
$body = @{
    segundos = 10
} | ConvertTo-Json

Invoke-RestMethod `
    -Uri "http://localhost:18080/api/simulaciones/9/avanzar-multiples" `
    -Method POST `
    -ContentType "application/json" `
    -Body $body |
    ConvertTo-Json -Depth 10
```

**Importante:** El identificador `9` corresponde a una simulación utilizada durante las pruebas y que ya se encuentra finalizada.

Para iniciar o avanzar simulaciones, debe utilizarse una simulación que se encuentre en el estado requerido por cada operación.

## 15. Configuración de MySQL

El backend obtiene la configuración de conexión mediante las siguientes variables de entorno:

| Variable | Descripción |
|---|---|
| `SISTRA_DB_HOST` | Dirección del servidor |
| `SISTRA_DB_PORT` | Puerto de conexión |
| `SISTRA_DB_USER` | Usuario |
| `SISTRA_DB_PASSWORD` | Contraseña |
| `SISTRA_DB_NAME` | Nombre de la base de datos |
| `SISTRA_DB_PLUGIN_DIR` | Ruta de plugins de libmariadb |

La configuración de instalación y conexión se describe en:

[Guía de instalación](INSTALACION.md)

## 16. Organización del backend

El backend mantiene separados los componentes de la aplicación.

### Rutas HTTP

Los archivos de rutas administran las solicitudes y respuestas de la API REST.

Entre los archivos utilizados se encuentran:

- `ApiRoutes.h`.
- `ApiRoutes.cpp`.
- `VehiculoRoutes.h`.
- `VehiculoRoutes.cpp`.
- `SimulacionRoutes.h`.
- `SimulacionRoutes.cpp`.

### Servicios

Los servicios contienen la lógica de negocio del sistema, incluyendo el motor de simulación y el control de velocidad.

### Base de datos

La capa de acceso a datos administra las conexiones y operaciones con MySQL.

Entre sus componentes se encuentra `DatabaseConnection`.

### Archivo principal

El archivo `main.cpp` participa en la inicialización y ejecución del backend.

### Compilación

El archivo `CMakeLists.txt` configura la compilación del proyecto.

## 17. Pruebas de integración realizadas

Durante la Fase 11 se realizaron pruebas para verificar el funcionamiento conjunto de los componentes principales.

### Operaciones verificadas

- Incorporación de vehículos.
- Inicio de simulaciones.
- Avance individual.
- Avance múltiple.
- Finalización automática.
- Registro de multas.
- Prevención de multas duplicadas.
- Recuperación después de reiniciar el backend.
- Persistencia de vehículos.
- Persistencia de estadísticas.
- Consulta de simulaciones.
- Consulta de resultados históricos.
- Validación de solicitudes inválidas.
- Manejo de errores HTTP.

### Validaciones de avance múltiple

Se verificaron los siguientes casos:

| Solicitud | Resultado esperado y obtenido |
|---|---|
| `segundos = 0` | HTTP 400 |
| `segundos = 3601` | HTTP 400 |
| `segundos = 1.5` | HTTP 400 |
| `segundos = "10"` | HTTP 400 |
| JSON mal formado | HTTP 400 |
| Campo `segundos` ausente | HTTP 400 |

### Validaciones de estado

También se comprobó que:

- No puede iniciarse una simulación finalizada.
- No pueden agregarse vehículos a una simulación que ya no está configurada.
- No puede avanzarse una simulación finalizada.
- Una simulación inexistente devuelve HTTP 404.
- Una simulación finalizada no puede recuperarse como activa.

### Persistencia histórica

Después de reiniciar el backend se verificó que la simulación 9 conservara:

- Estado `Finalizada`.
- Duración de 120 segundos.
- Segundo actual 120.
- Un vehículo finalizado.
- Una multa registrada.
- Monto total de multas de Q600.

También se confirmó que sus estadísticas y el historial del vehículo continuaban disponibles.

## 18. Estado de la documentación

**Fase 12 de 13 - Documentación del proyecto.**

Las funcionalidades principales de la API REST y sus pruebas de integración corresponden a las fases desarrolladas previamente.

Este documento reúne las rutas, reglas, ejemplos y resultados conocidos hasta la Fase 11.

La Fase 13 corresponde a la preparación de la entrega final.

## 19. Repositorio

El código fuente del proyecto se encuentra disponible en:

https://github.com/jyuman-umg/SISTRA
