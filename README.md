# SISTRA — Proyecto 18: Intersección Semaforizada

**Variante A — Vehículos en línea recta**  
**Curso:** Algoritmos  
**Estudiante:** Jefferson Gadiel Yuman Pérez  
**Carné:** 2294-24-10258

## Descripción

SISTRA es una aplicación web que simula una intersección de cuatro calles (Norte, Sur, Este y Oeste). Su motor está desarrollado en **C++20**, utiliza **Crow** para exponer una API HTTP, **MySQL 8** para persistencia y **HTML, CSS y JavaScript** para la interfaz gráfica 2D.

Permite crear simulaciones, registrar vehículos por placa, tipo, calle, velocidad y segundo de llegada, iniciar y avanzar el tiempo, visualizar los estados de los vehículos y semáforos, consultar tiempos de espera, estadísticas y multas por exceso de velocidad.

## Tecnologías

C++20, Crow, MySQL 8, libmariadb, CMake, vcpkg, CLion, HTML5, CSS3 y JavaScript.

## Estructura

```text
SISTRA/
├── backend/
│   ├── include/
│   └── src/
├── frontend/
│   ├── index.html
│   ├── css/
│   └── js/
├── database/
│   ├── schema.sql
│   └── seed.sql
├── docs/
│   ├── INSTALACION.md
│   ├── API.md
│   └── MANUAL_USUARIO.md
├── CMakeLists.txt
└── vcpkg.json
```

## Instalación rápida

1. Clonar el repositorio y abrir la carpeta raíz en CLion.
2. Instalar MySQL 8 y ejecutar `database/schema.sql` y `database/seed.sql`.
3. Configurar vcpkg y CMake conforme a [INSTALACION.md](docs/INSTALACION.md).
4. Configurar las variables `SISTRA_DB_HOST`, `SISTRA_DB_PORT`, `SISTRA_DB_USER`, `SISTRA_DB_PASSWORD`, `SISTRA_DB_NAME` y, si corresponde, `SISTRA_DB_PLUGIN_DIR` en la configuración de ejecución de CLion.
5. Compilar y ejecutar el target `SISTRA`.
6. Abrir **http://localhost:18080/**. El backend y el frontend se sirven desde el mismo servidor.

> No subir contraseñas ni archivos de configuración que contengan credenciales al repositorio.

## Uso

Crear una simulación → agregar vehículos → iniciar → avanzar uno o varios segundos → observar la intersección → revisar la tabla de vehículos, multas y estadísticas.

## Documentación

- [Instalación y configuración](docs/INSTALACION.md)
- [API REST](docs/API.md)
- [Manual de usuario](docs/MANUAL_USUARIO.md)
