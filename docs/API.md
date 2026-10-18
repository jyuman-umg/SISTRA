
# SISTRA - Documentacion de la API REST

## Proyecto 18 - Interseccion Semaforizada
### Variante A

SISTRA es un sistema inteligente de simulacion y control de trafico vehicular desarrollado en C++20.

El backend utiliza Crow para proporcionar servicios HTTP y MySQL 8 para almacenar la informacion del sistema.

## 1. Tecnologias utilizadas

- C++20: lenguaje principal del backend.
- Crow: framework para crear la API REST.
- MySQL 8: base de datos relacional.
- libmariadb: biblioteca de conexion con MySQL.
- CMake: configuracion y compilacion del proyecto.
- vcpkg: administracion de dependencias.
- MinGW: compilador utilizado en Windows.

## 2. Servidor

Direccion local:

http://localhost:18080

Las respuestas de la API utilizan el formato JSON.

## 3. Endpoints disponibles

### GET /api/health

Comprueba que el servidor HTTP se encuentra funcionando.

Respuesta esperada:

```json
{
  "success": true,
  "message": "Servidor SISTRA funcionando correctamente",
  "system": "SISTRA",
  "version": "1.0.0"
}
```

Codigo HTTP: 200.

### GET /api/sistema

Devuelve informacion general del proyecto.

Respuesta esperada:

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

Codigo HTTP: 200.

### GET /api/calles

Consulta las calles activas registradas en MySQL.

Respuesta esperada:

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

Codigo HTTP: 200.

En caso de error de conexion o consulta, el servidor responde con codigo HTTP 500 y un mensaje de error.

### GET /api/database/status

Comprueba que la conexion con MySQL se encuentra disponible.

Respuesta exitosa:

```json
{
  "success": true,
  "connected": true,
  "database": "sistra_db",
  "message": "Conexion con MySQL funcionando correctamente"
}
```

Codigo HTTP: 200.

Si la base de datos no esta disponible:

```json
{
  "success": false,
  "connected": false,
  "message": "No se pudo conectar con MySQL"
}
```

Codigo HTTP: 503.

## 4. Configuracion de MySQL

El backend obtiene la configuracion de conexion mediante variables de entorno:

- SISTRA_DB_HOST
- SISTRA_DB_PORT
- SISTRA_DB_USER
- SISTRA_DB_PASSWORD
- SISTRA_DB_NAME
- SISTRA_DB_PLUGIN_DIR

## 5. Estructura del backend

```text
backend/
├── include/
│   ├── database/
│   │   └── DatabaseConnection.h
│   └── routes/
│       └── ApiRoutes.h
├── src/
│   ├── database/
│   │   └── DatabaseConnection.cpp
│   ├── routes/
│   │   └── ApiRoutes.cpp
│   └── main.cpp
└── CMakeLists.txt
```

## 6. Ejecucion

Compilar el proyecto mediante CMake desde CLion.

Ejecutar el programa:

```powershell
.\cmake-build-debug\backend\SISTRA.exe
```

El servidor escucha en el puerto 18080.

Para detenerlo, presionar Ctrl + C.

## 7. Proximas funcionalidades

En las siguientes fases se implementaran:

- Registro y consulta de vehiculos.
- Gestion de simulaciones.
- Control inteligente de semaforos.
- Camaras virtuales y generacion de multas.
- Estadisticas de trafico.
- Integracion con el frontend web.
```
---

# Fase 4 - Gestion de Vehiculos y Simulaciones

## 8. Gestion de vehiculos

### GET /api/vehiculos

Consulta todos los vehiculos activos registrados en MySQL.

Respuesta exitosa: HTTP 200.

La respuesta incluye:

- success
- message
- total
- data

Cada vehiculo contiene su identificador, placa, tipo, fecha de registro y estado.

### GET /api/vehiculos/{id}

Consulta un vehiculo activo por su identificador.

Respuestas:

- HTTP 200: vehiculo encontrado.
- HTTP 400: identificador invalido.
- HTTP 404: vehiculo inexistente o inactivo.
- HTTP 503: conexion con MySQL no disponible.
- HTTP 500: error interno.

### POST /api/vehiculos

Registra un vehiculo nuevo.

Ejemplo de solicitud:

```json
{
  "placa": "P123ABC",
  "tipoVehiculo": "Automovil"
}
```

Tipos permitidos:

- Automovil
- Motocicleta
- Camion
- Bus

Validaciones:

- La placa es obligatoria.
- La placa debe tener entre 5 y 10 caracteres alfanumericos.
- La placa se convierte a mayusculas.
- El tipo de vehiculo debe ser valido.
- No se permiten placas duplicadas.

Respuestas:

- HTTP 201: vehiculo registrado.
- HTTP 400: datos invalidos.
- HTTP 409: placa duplicada.
- HTTP 503: conexion no disponible.
- HTTP 500: error interno.

## 9. Gestion de simulaciones

### GET /api/simulaciones

Consulta todas las simulaciones registradas.

Respuesta exitosa: HTTP 200.

Cada simulacion incluye:

- Identificador
- Nombre
- Descripcion
- Fechas de creacion, inicio y finalizacion
- Hora simulada de inicio
- Duracion en segundos
- Estado
- Total de vehiculos
- Total de vehiculos finalizados

### GET /api/simulaciones/{id}

Consulta una simulacion especifica.

Respuestas:

- HTTP 200: simulacion encontrada.
- HTTP 400: identificador invalido.
- HTTP 404: simulacion inexistente.
- HTTP 503: conexion no disponible.
- HTTP 500: error interno.

### POST /api/simulaciones/{id}/vehiculos

Incorpora un vehiculo existente a una simulacion.

Ejemplo de solicitud:

```json
{
  "idVehiculo": 1,
  "idCalle": 1,
  "velocidad": 65,
  "segundoLlegada": 10
}
```

Validaciones:

- La simulacion debe existir.
- La simulacion debe estar en estado Configurada.
- El vehiculo debe existir y estar activo.
- La calle debe existir y estar activa.
- La velocidad no puede ser negativa.
- El segundo de llegada debe estar dentro de la duracion.
- Un vehiculo no puede registrarse dos veces en la misma simulacion.

El servidor calcula HoraLlegada utilizando la hora simulada de inicio y SegundoLlegada.

El registro del vehiculo y la actualizacion de TotalVehiculos se realizan mediante una transaccion MySQL.

Respuestas:

- HTTP 201: vehiculo incorporado.
- HTTP 400: datos invalidos.
- HTTP 404: simulacion, vehiculo o calle inexistente.
- HTTP 409: vehiculo duplicado o simulacion no configurable.
- HTTP 503: conexion no disponible.
- HTTP 500: error interno.

### GET /api/simulaciones/{id}/vehiculos

Consulta los vehiculos incorporados a una simulacion.

La respuesta incluye:

- Placa y tipo de vehiculo.
- Calle de ingreso.
- Velocidad.
- Hora y segundo de llegada.
- Hora y segundo de inicio de cruce.
- Hora y segundo de salida.
- Tiempo de espera.
- Estado del vehiculo.

Los vehiculos se ordenan por SegundoLlegada.

## 10. Estados de los vehiculos

Los vehiculos de una simulacion pueden encontrarse en:

- Programado: todavia no llega a la interseccion.
- EnCola: llego y espera su turno.
- Cruzando: esta atravesando la interseccion.
- Finalizado: completo su recorrido.

La transicion entre estos estados se implementara en la fase 5.

## 11. Estructura de archivos

```text
backend/
├── include/
│   ├── database/
│   │   └── DatabaseConnection.h
│   └── routes/
│       ├── ApiRoutes.h
│       ├── VehiculoRoutes.h
│       └── SimulacionRoutes.h
├── src/
│   ├── database/
│   │   └── DatabaseConnection.cpp
│   ├── routes/
│   │   ├── ApiRoutes.cpp
│   │   ├── VehiculoRoutes.cpp
│   │   └── SimulacionRoutes.cpp
│   └── main.cpp
└── CMakeLists.txt
```

## 12. Seguridad e integridad

- Consultas parametrizadas para operaciones con datos externos.
- Validacion de datos recibidos en JSON.
- Restricciones de claves foraneas en MySQL.
- Restriccion UNIQUE para placas y vehiculos por simulacion.
- Transacciones para mantener consistentes los registros.
- Credenciales de MySQL configuradas mediante variables de entorno.
