
# Guía de instalación — SISTRA

## Proyecto 18 — Intersección Semaforizada

Esta guía explica cómo configurar, compilar y ejecutar
el proyecto desde una computadora diferente.

## 1. Requisitos iniciales

- Windows 10 u 11 de 64 bits.
- Git.
- CLion.
- CMake 3.21 o superior.
- Compilador compatible con C++20.

## 2. Clonar el repositorio

Abrir una terminal y ejecutar:

```bash
git clone https://github.com/USUARIO/SISTRA.git
```

Reemplazar USUARIO por el propietario del repositorio.

También se puede utilizar la opción Clone Repository de CLion.

## 3. Abrir el proyecto

1. Abrir CLion.
2. Seleccionar Open.
3. Seleccionar la carpeta SISTRA.
4. Esperar a que CMake configure el proyecto.

## 4. Configurar el compilador

En CLion, ingresar a:

Settings > Build, Execution, Deployment > Toolchains

Verificar que se detecten correctamente:

- Compilador C++ compatible con C++20.
- CMake.
- Herramienta de compilación.
- Depurador.

## 5. Compilar y ejecutar

1. Seleccionar el ejecutable SISTRA.
2. Presionar Build.
3. Ejecutar el programa.

El programa mostrará un mensaje indicando
que la configuración inicial fue exitosa.

## 6. Dependencias futuras

En las siguientes fases se incorporarán:

- Crow.
- vcpkg.
- MySQL 8.
- MySQL Connector/C++.
- Frontend HTML, CSS y JavaScript.

Se agregarán instrucciones para instalar y configurar
estas dependencias de manera reproducible.

## 7. Configuración de MySQL

Pendiente de implementación en la fase 2.

## 8. Ejecución de la aplicación web

Pendiente de implementación en la fase 3.

## 9. Problemas frecuentes

Si CMake no detecta el compilador, revisar
la configuración de Toolchains en CLion.

Si el proyecto no compila, comprobar que
el compilador soporte C++20.
