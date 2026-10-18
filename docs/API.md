
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
