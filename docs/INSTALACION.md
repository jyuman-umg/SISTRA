
# Instalación de SISTRA

## Proyecto 18 - Intersección Semaforizada
### Variante A

SISTRA es un sistema de simulación y control de tráfico vehicular desarrollado en C++20, con MySQL como base de datos y una interfaz web que se implementará en las siguientes fases.

## 1. Requisitos

Para compilar y ejecutar el proyecto se necesitan:

- Git
- CLion con compilador C++ compatible con C++20
- CMake
- Ninja
- MySQL Server 8
- MySQL Workbench (opcional)
- vcpkg
- Biblioteca libmariadb instalada mediante vcpkg

En Windows se utiliza el triplet `x64-mingw-dynamic` cuando el compilador es MinGW.

## 2. Clonar el repositorio

Ejecutar:

```bash
git clone https://github.com/jyuman-umg/SISTRA.git
```

Abrir la carpeta SISTRA en CLion.

## 3. Instalar vcpkg

En Windows, abrir PowerShell:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

Si vcpkg ya está instalado, no es necesario repetir estos pasos.

## 4. Configurar dependencias

El archivo `vcpkg.json` declara las bibliotecas requeridas.

Para instalar las dependencias con MinGW:

```powershell
C:\vcpkg\vcpkg.exe install --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic
```

El compilador MinGW debe estar disponible en PATH.

En algunos entornos de Windows puede ser necesario utilizar una ruta de MinGW sin espacios para compilar libmariadb correctamente.

## 5. Crear la base de datos

Iniciar MySQL Server 8.

Desde MySQL Workbench ejecutar los archivos en este orden:

1. `database/schema.sql`
2. `database/seed.sql`

Esto creará la base de datos `sistra_db`, sus 11 tablas y los registros iniciales.

## 6. Configurar CMake en CLion

Abrir:

File > Settings > Build, Execution, Deployment > CMake

En el perfil Debug agregar las siguientes opciones:

```text
-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic -DVCPKG_HOST_TRIPLET=x64-mingw-dynamic
```

Si vcpkg está instalado en otra ubicación, ajustar la ruta.

Recargar el proyecto CMake.

## 7. Configurar la conexión MySQL

SISTRA utiliza estas variables de entorno:

- SISTRA_DB_HOST
- SISTRA_DB_PORT
- SISTRA_DB_USER
- SISTRA_DB_PASSWORD
- SISTRA_DB_NAME
- SISTRA_DB_PLUGIN_DIR

Ejemplo para PowerShell:

```powershell
$env:SISTRA_DB_HOST = "127.0.0.1"
$env:SISTRA_DB_PORT = "3306"
$env:SISTRA_DB_USER = "root"
$env:SISTRA_DB_NAME = "sistra_db"
$env:SISTRA_DB_PLUGIN_DIR = "$PWD\vcpkg_installed\x64-mingw-dynamic\plugins\libmariadb"
```

Solicitar la contraseña sin escribirla directamente en el comando:

```powershell
$securePassword = Read-Host "Contraseña MySQL" -AsSecureString
$env:SISTRA_DB_PASSWORD = [System.Net.NetworkCredential]::new("", $securePassword).Password
```

## 8. Ejecutar SISTRA

Compilar desde CLion:

Build > Build Project

Después, desde la carpeta raíz del proyecto:

```powershell
$env:PATH = "$PWD\vcpkg_installed\x64-mingw-dynamic\bin;$env:PATH"
.\cmake-build-debug\backend\SISTRA.exe
```

La aplicación debe conectarse a MySQL y mostrar las calles registradas.

## 9. Estado del proyecto

Fase 1: Configuración inicial completada.

Fase 2: Base de datos y conexión C++ con MySQL implementadas.

Las siguientes fases incorporarán la API REST, los vehículos, la simulación de tráfico, los semáforos inteligentes, las cámaras y la interfaz gráfica.
