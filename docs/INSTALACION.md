
# Instalación y configuración de SISTRA

## Proyecto 18 - Intersección Semaforizada

### Variante A

SISTRA es un sistema web de simulación y control de tráfico vehicular desarrollado con C++20, Crow, MySQL y tecnologías web.

Este documento explica cómo preparar el entorno de desarrollo, instalar las dependencias, configurar la base de datos, compilar el backend y ejecutar el sistema.

## 1. Requisitos del sistema

Para compilar y ejecutar SISTRA en Windows se necesitan las siguientes herramientas:

- Git.
- CLion.
- Compilador MinGW compatible con C++20.
- CMake.
- Ninja.
- MySQL Server 8.
- MySQL Workbench (opcional).
- vcpkg.
- Dependencias declaradas en `vcpkg.json`.
- Biblioteca `libmariadb`.
- Navegador web actualizado.

### Configuración utilizada

Durante el desarrollo se utilizó el triplet:

`x64-mingw-dynamic`

Este triplet permite utilizar las dependencias compiladas para MinGW en Windows de 64 bits.

Es importante mantener la compatibilidad entre el compilador configurado en CLion y las bibliotecas instaladas mediante vcpkg.

## 2. Clonar el repositorio

Abrir una terminal y ejecutar:

```bash
git clone https://github.com/jyuman-umg/SISTRA.git
```

Ingresar a la carpeta del proyecto:

```bash
cd SISTRA
```

Después, abrir la carpeta SISTRA desde CLion.

El repositorio contiene el código fuente del backend, la interfaz web, los scripts de base de datos y la documentación.

## 3. Instalar vcpkg

Si vcpkg todavía no está instalado, abrir PowerShell y ejecutar:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

Si vcpkg ya se encuentra instalado, no es necesario repetir este procedimiento.

La ubicación `C:\vcpkg` corresponde a la ruta utilizada en los ejemplos de configuración.

Si se instala en otra carpeta, deberán ajustarse las rutas correspondientes.

## 4. Configurar las dependencias

SISTRA utiliza el archivo `vcpkg.json` para declarar las dependencias del proyecto.

Entre las tecnologías utilizadas en el backend se encuentran:

- Crow, para el servidor HTTP y la API REST.
- libmariadb, para la comunicación con MySQL.
- Bibliotecas auxiliares requeridas por el proyecto.

### Instalar dependencias con MinGW

Desde la carpeta raíz de SISTRA, ejecutar en PowerShell:

```powershell
C:\vcpkg\vcpkg.exe install --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic
```

El compilador MinGW debe encontrarse disponible en la variable de entorno `PATH`.

En algunos entornos de Windows puede ser necesario utilizar una instalación de MinGW ubicada en una ruta sin espacios para evitar inconvenientes durante la compilación de libmariadb.

### Carpeta de dependencias

Las bibliotecas instaladas para el proyecto se encuentran en una estructura similar a:

```text
vcpkg_installed/
└── x64-mingw-dynamic/
    ├── bin/
    ├── include/
    ├── lib/
    └── plugins/
```

Estas carpetas contienen archivos necesarios para compilar y ejecutar el backend.

## 5. Preparar MySQL

SISTRA utiliza MySQL para almacenar la información de vehículos, calles, simulaciones, multas y otros registros del sistema.

### Iniciar MySQL

Verificar que MySQL Server 8 se encuentre instalado y que el servicio esté iniciado.

La conexión utilizada en el entorno local normalmente corresponde a:

| Parámetro | Valor de ejemplo |
|---|---|
| Servidor | `127.0.0.1` |
| Puerto | `3306` |
| Usuario | `root` |
| Base de datos | `sistra_db` |

La contraseña dependerá de la configuración local de MySQL.

### Crear la base de datos

Abrir MySQL Workbench o una herramienta compatible.

Ejecutar los scripts del proyecto en el siguiente orden:

1. `database/schema.sql`
2. `database/seed.sql`

El primer archivo crea la estructura de la base de datos y el segundo incorpora los registros iniciales necesarios para el funcionamiento de SISTRA.

**Importante:** Estos scripts deben ejecutarse sobre una instalación nueva. Si la base de datos ya contiene información, revisar su contenido antes de volver a ejecutar instrucciones que puedan modificar o reinicializar los datos.

## 6. Configurar CMake en CLion

Abrir el proyecto SISTRA en CLion.

Ingresar a:

`File > Settings > Build, Execution, Deployment > CMake`

Seleccionar el perfil `Debug`.

En el campo de opciones de CMake agregar:

```text
-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic
```

Si vcpkg está instalado en una ubicación diferente, modificar la ruta de `CMAKE_TOOLCHAIN_FILE`.

Después de guardar la configuración, recargar el proyecto CMake.

### Compilador

Verificar que CLion esté utilizando un compilador MinGW compatible con C++20.

La configuración del compilador debe ser compatible con el triplet utilizado para instalar las dependencias.

## 7. Configurar la conexión con MySQL

SISTRA utiliza variables de entorno para obtener los datos de conexión con MySQL.

Las variables son:

| Variable | Descripción |
|---|---|
| `SISTRA_DB_HOST` | Dirección del servidor MySQL |
| `SISTRA_DB_PORT` | Puerto de conexión |
| `SISTRA_DB_USER` | Usuario de MySQL |
| `SISTRA_DB_PASSWORD` | Contraseña de MySQL |
| `SISTRA_DB_NAME` | Nombre de la base de datos |
| `SISTRA_DB_PLUGIN_DIR` | Ruta de plugins de libmariadb |

### Configuración en PowerShell

Desde la carpeta raíz del proyecto:

```powershell
$env:SISTRA_DB_HOST = "127.0.0.1"
$env:SISTRA_DB_PORT = "3306"
$env:SISTRA_DB_USER = "root"
$env:SISTRA_DB_NAME = "sistra_db"
$env:SISTRA_DB_PLUGIN_DIR = "$PWD\vcpkg_installed\x64-mingw-dynamic\plugins\libmariadb"
```

Para solicitar la contraseña sin escribirla directamente en el comando:

```powershell
$securePassword = Read-Host "Contraseña MySQL" -AsSecureString
$env:SISTRA_DB_PASSWORD = [System.Net.NetworkCredential]::new("", $securePassword).Password
```

Estas variables se establecen en la sesión actual de PowerShell.

Si el backend se ejecuta directamente desde CLion, las variables también deben estar disponibles en el entorno del proceso iniciado por el IDE.

## 8. Compilar el backend

Abrir SISTRA en CLion.

Seleccionar:

`Build > Build Project`

Esperar a que CMake y el compilador terminen el proceso.

La compilación genera el ejecutable del backend.

En la configuración utilizada durante el desarrollo, la ruta del ejecutable es:

```text
cmake-build-debug/backend/SISTRA.exe
```

La ubicación puede variar si se utiliza otro perfil de compilación.

## 9. Ejecutar el backend

Desde PowerShell, ubicarse en la carpeta raíz del proyecto.

Primero agregar las bibliotecas dinámicas al `PATH`:

```powershell
$env:PATH = "$PWD\vcpkg_installed\x64-mingw-dynamic\bin;$env:PATH"
```

Después ejecutar:

```powershell
.\cmake-build-debug\backend\SISTRA.exe
```

El backend debe iniciar el servidor HTTP y establecer la conexión con MySQL utilizando la configuración disponible.

### Dirección del servidor

Durante el desarrollo, SISTRA utiliza:

`http://localhost:18080`

Esta dirección corresponde al servidor HTTP del backend.

## 10. Interfaz web

SISTRA dispone de una interfaz web desarrollada con:

- HTML5.
- CSS3.
- JavaScript.
- Canvas 2D.

La interfaz permite interactuar con las funcionalidades del sistema y visualizar la intersección semaforizada.

### Funcionalidades de la interfaz

La aplicación permite representar:

- Las calles Norte, Sur, Este y Oeste.
- Los vehículos incorporados a las simulaciones.
- El movimiento vehicular.
- Los semáforos.
- El desarrollo de la simulación.

La interfaz utiliza solicitudes HTTP para comunicarse con el backend.

Para utilizarla, el servidor de SISTRA debe encontrarse ejecutándose y accesible desde el navegador.

## 11. API REST

SISTRA utiliza Crow para exponer los servicios del backend.

La dirección base es:

`http://localhost:18080`

### Servicios generales

| Método | Endpoint | Descripción |
|---|---|---|
| GET | `/api/health` | Consulta el estado del servidor |
| GET | `/api/sistema` | Consulta información del sistema |
| GET | `/api/calles` | Consulta las calles |
| GET | `/api/database/status` | Consulta el estado de la conexión MySQL |

### Vehículos

| Método | Endpoint | Descripción |
|---|---|---|
| GET | `/api/vehiculos` | Consulta vehículos |
| GET | `/api/vehiculos/{id}` | Consulta un vehículo |
| POST | `/api/vehiculos` | Registra un vehículo |

### Simulaciones

| Método | Endpoint | Descripción |
|---|---|---|
| GET | `/api/simulaciones` | Lista las simulaciones |
| GET | `/api/simulaciones/{id}` | Consulta una simulación |
| POST | `/api/simulaciones/{id}/vehiculos` | Agrega un vehículo |
| GET | `/api/simulaciones/{id}/vehiculos` | Consulta vehículos de una simulación |
| POST | `/api/simulaciones/{id}/iniciar` | Inicia una simulación |
| POST | `/api/simulaciones/{id}/avanzar` | Avanza un segundo |
| POST | `/api/simulaciones/{id}/avanzar-multiples` | Avanza varios segundos |
| GET | `/api/simulaciones/{id}/estado` | Consulta el estado del motor |
| GET | `/api/simulaciones/{id}/estadisticas` | Consulta estadísticas |

La documentación detallada de los servicios se encuentra en:

[Documentación de la API](API.md)

## 12. Funcionamiento de las simulaciones

Las simulaciones utilizan tres estados principales:

| Estado | Descripción |
|---|---|
| `Configurada` | Permite preparar la simulación e incorporar vehículos |
| `Ejecutando` | La simulación se encuentra iniciada y puede avanzar |
| `Finalizada` | La simulación terminó y conserva sus resultados |

### Registro de vehículos

Los vehículos se incorporan a una simulación configurada indicando los datos requeridos por la API.

Entre ellos se encuentran:

- Identificador del vehículo.
- Identificador de la calle.
- Velocidad.
- Segundo de llegada.

### Inicio de simulación

Una simulación configurada puede iniciarse mediante:

```http
POST /api/simulaciones/{id}/iniciar
```

### Avance individual

Para avanzar un segundo:

```http
POST /api/simulaciones/{id}/avanzar
```

### Avance múltiple

Para avanzar varios segundos:

```http
POST /api/simulaciones/{id}/avanzar-multiples
Content-Type: application/json
```

Ejemplo de cuerpo JSON:

```json
{
  "segundos": 10
}
```

El campo `segundos` debe ser un número entero entre 1 y 3600.

### Consulta de resultados

Para consultar las estadísticas:

```http
GET /api/simulaciones/{id}/estadisticas
```

Para consultar los vehículos:

```http
GET /api/simulaciones/{id}/vehiculos
```

## 13. Persistencia y recuperación

SISTRA almacena información de las simulaciones en MySQL.

Esto permite conservar:

- Estado de las simulaciones.
- Segundo actual.
- Vehículos registrados.
- Estados de los vehículos.
- Segundos de llegada y salida.
- Multas.
- Estadísticas históricas.

Cuando una simulación permanece en estado `Ejecutando`, el backend puede reconstruir su estado para continuar su ejecución después de un reinicio.

Las simulaciones finalizadas conservan su información histórica, pero no pueden volver a iniciarse como simulaciones activas.

## 14. Control de velocidad

El sistema incorpora cámaras virtuales para detectar excesos de velocidad.

Cuando se identifica una infracción, SISTRA registra la multa correspondiente en MySQL.

La información registrada permite consultar el total de multas y sus montos acumulados.

El sistema también contempla mecanismos para evitar la duplicación de multas al continuar simulaciones recuperadas.

## 15. Manejo de errores

La API utiliza códigos HTTP para comunicar los resultados de las operaciones.

| Código | Significado |
|---|---|
| `200` | Solicitud procesada correctamente |
| `400` | Solicitud inválida |
| `404` | Recurso inexistente |
| `409` | Operación incompatible con el estado actual |

Por ejemplo, el backend rechaza:

- Segundos negativos o iguales a cero.
- Cantidades superiores a 3600 segundos por solicitud de avance múltiple.
- Valores decimales en el campo `segundos`.
- Valores de texto cuando se espera un número.
- JSON mal formado.
- Solicitudes sin campos obligatorios.
- Intentos de iniciar simulaciones finalizadas.
- Intentos de agregar vehículos a simulaciones que ya no están configuradas.

## 16. Consideraciones importantes

- MySQL debe estar disponible antes de ejecutar operaciones que requieren la base de datos.
- Las dependencias deben corresponder al compilador configurado.
- Las variables de entorno deben estar disponibles para el proceso del backend.
- El puerto `18080` debe estar disponible para el servidor HTTP.
- Las simulaciones finalizadas no deben modificarse mediante operaciones reservadas para simulaciones activas.
- Los scripts de inicialización de MySQL deben utilizarse con precaución cuando ya existen registros almacenados.

## 17. Estado del proyecto

**Fase 12 de 13 - Documentación.**

Las Fases 1 a 11 abarcan el desarrollo y las pruebas principales del sistema.

Entre las funcionalidades implementadas se encuentran:

- Backend en C++20.
- API REST con Crow.
- Conexión con MySQL.
- Administración de vehículos.
- Motor de simulación.
- Control de semáforos.
- Visualización gráfica 2D.
- Control de velocidad.
- Registro de multas.
- Estadísticas.
- Persistencia y recuperación.
- Pruebas de integración.


## 18. Repositorio

El código fuente se encuentra disponible en:

https://github.com/jyuman-umg/SISTRA.git
## 9. Abrir la interfaz web

Con el ejecutable `SISTRA` en funcionamiento, abrir `http://localhost:18080/`. El mismo servidor Crow sirve el frontend y la API;

Si la página no carga, verificar que el proceso se ejecute desde una ubicación desde la cual pueda encontrarse la carpeta `frontend/` del proyecto. En CLion, configurar el directorio de trabajo de la ejecución si fuera necesario.

## 10. Prueba de funcionamiento

1. Abrir `http://localhost:18080/api/health` y comprobar que el servidor responda.
2. Abrir `http://localhost:18080/`.
3. Crear una simulación, agregar vehículos con placas diferentes, iniciarla y avanzar el tiempo.
4. Verificar los semáforos, las colas, los estados de vehículos y la columna de multas.
5. Comprobar que los vehículos finalicen y que las estadísticas reflejen la ejecución.

**Nota sobre dependencias:** si aparece un error relacionado con `caching_sha2_password.dll`, revisar la instalación de libmariadb, la ruta de plugins (`SISTRA_DB_PLUGIN_DIR`) y las DLL disponibles en el entorno de ejecución. La solución exacta depende de la instalación local.

