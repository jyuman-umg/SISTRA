
# SISTRA - Manual de Usuario

## Proyecto 18 - Intersección Semaforizada

### Variante A

**Curso:** Algoritmos  
**Estudiante:** Jefferson Gadiel Yuman Pérez  
**Carné:** 2294-24-10258

---

## 1. Introducción

SISTRA es un sistema web de simulación de tráfico vehicular que permite representar el funcionamiento de una intersección semaforizada de cuatro calles: Norte, Sur, Este y Oeste.

Su objetivo es mostrar cómo se comportan los vehículos al llegar a una intersección, esperar su turno y atravesarla de acuerdo con el funcionamiento de los semáforos.

El sistema también incorpora mecanismos de control inteligente de tráfico, cámaras virtuales para detectar excesos de velocidad, registro de multas y estadísticas sobre el comportamiento de los vehículos.

Este manual explica las principales funcionalidades de SISTRA y cómo interpretar los resultados de una simulación.

## 2. Requisitos para utilizar el sistema

Para utilizar SISTRA se necesita:

- Una computadora con un navegador web actualizado.
- El backend de SISTRA ejecutándose.
- Una conexión disponible con la base de datos MySQL.
- La interfaz web del proyecto.

El servidor HTTP utilizado durante el desarrollo funciona en:

`http://localhost:18080`

La configuración técnica del proyecto se encuentra en el documento [INSTALACION.md](INSTALACION.md).

## 3. Funcionamiento general

SISTRA permite representar el tráfico de una intersección mediante simulaciones.

Cada simulación contiene una duración determinada y puede incluir diferentes vehículos con velocidades y momentos de llegada específicos.

Durante la ejecución, el sistema administra el avance del tiempo y actualiza el comportamiento de los vehículos.

El funcionamiento general comprende:

1. Preparar una simulación.
2. Incorporar los vehículos correspondientes.
3. Iniciar la simulación.
4. Avanzar el tiempo simulado.
5. Observar el movimiento de los vehículos.
6. Consultar los resultados obtenidos.

Los vehículos pueden permanecer en espera, cruzar la intersección o finalizar su recorrido.

## 4. Intersección semaforizada

La intersección utilizada por SISTRA está formada por cuatro calles.

| Calle | Dirección |
|---|---|
| Norte | Norte |
| Sur | Sur |
| Este | Este |
| Oeste | Oeste |

La representación gráfica muestra la intersección desde una vista superior en dos dimensiones.

Los vehículos circulan por las calles correspondientes y deben respetar el control de los semáforos.

## 5. Vehículos

SISTRA permite trabajar con vehículos registrados en el sistema.

### 5.1. Información de los vehículos

Cada vehículo dispone de información como:

- Identificador.
- Placa.
- Tipo de vehículo.

Cuando un vehículo se incorpora a una simulación, también se especifican:

- Calle de procedencia.
- Velocidad.
- Segundo de llegada.

### 5.2. Tipos de vehículos

Los tipos de vehículos contemplados por el sistema son:

- Automóvil.
- Motocicleta.
- Camión.
- Bus.

### 5.3. Registro de vehículos

El registro de un vehículo requiere proporcionar una placa y seleccionar un tipo válido.

La placa debe contener entre 5 y 10 caracteres alfanuméricos.

El sistema no permite registrar dos vehículos con la misma placa.

## 6. Simulaciones

Una simulación representa un período de funcionamiento de la intersección.

Cada simulación contiene información como:

- Nombre.
- Descripción.
- Hora simulada de inicio.
- Duración en segundos.
- Estado.
- Segundo actual.
- Cantidad de vehículos.
- Cantidad de vehículos finalizados.

### 6.1. Estados de las simulaciones

| Estado | Descripción |
|---|---|
| Configurada | La simulación está preparada para recibir vehículos |
| Ejecutando | La simulación se encuentra en funcionamiento |
| Finalizada | La simulación terminó y sus resultados están disponibles |

### 6.2. Simulación configurada

En este estado pueden incorporarse vehículos.

Cada vehículo debe tener asignada una calle, una velocidad y un segundo de llegada.

Una vez iniciada la simulación, ya no se permite agregar nuevos vehículos mediante la operación de incorporación.

### 6.3. Simulación en ejecución

Cuando se inicia una simulación, su estado cambia a `Ejecutando`.

A partir de ese momento, el motor puede avanzar el tiempo simulado y actualizar el comportamiento de los vehículos.

### 6.4. Simulación finalizada

Una simulación termina cuando alcanza su duración configurada.

Por ejemplo, si la duración es de 120 segundos, la simulación finaliza al llegar al segundo 120.

Los resultados quedan almacenados para su consulta posterior.

Una simulación finalizada no puede iniciarse nuevamente mediante la operación normal de inicio.

## 7. Incorporación de vehículos a una simulación

Para incorporar un vehículo deben proporcionarse los siguientes datos:

| Dato | Descripción |
|---|---|
| Vehículo | Vehículo previamente registrado |
| Calle | Calle por la que ingresará |
| Velocidad | Velocidad asignada |
| Segundo de llegada | Momento en que llegará a la intersección |

### Ejemplo

Se incorpora un automóvil con los siguientes datos:

- Placa: `P201ABC`.
- Calle: Norte.
- Velocidad: 95 km/h.
- Segundo de llegada: 0.

Esto significa que el vehículo llegará a la intersección desde el inicio de la simulación.

El sistema calcula su hora de llegada tomando como referencia la hora simulada de inicio.

## 8. Inicio de una simulación

Para iniciar una simulación, esta debe encontrarse en estado `Configurada`.

Al iniciarse:

1. El sistema prepara el motor de simulación.
2. Se establece el estado `Ejecutando`.
3. Se habilita el avance del tiempo simulado.
4. Los vehículos se procesan según sus segundos de llegada.

Si la simulación ya está finalizada, no se permite volver a iniciarla.

## 9. Avance del tiempo simulado

SISTRA permite avanzar el tiempo de dos maneras.

### 9.1. Avance individual

Permite avanzar un segundo de simulación.

El motor actualiza los vehículos y los elementos de tráfico correspondientes a ese segundo.

### 9.2. Avance múltiple

Permite avanzar varios segundos mediante una sola operación.

La cantidad solicitada debe ser un número entero entre 1 y 3600.

Por ejemplo, al solicitar un avance de 10 segundos, el motor procesa el período correspondiente.

Si durante ese avance se alcanza la duración de la simulación, esta pasa al estado `Finalizada`.

### 9.3. Restricciones

No se permite avanzar una simulación que no se encuentre en estado `Ejecutando`.

Tampoco se permiten cantidades de segundos inválidas, como valores negativos, decimales o superiores al límite establecido.

## 10. Estados de los vehículos

Durante una simulación, cada vehículo puede pasar por diferentes estados.

| Estado | Significado |
|---|---|
| Programado | El vehículo todavía no ha llegado |
| EnCola | El vehículo llegó y está esperando |
| Cruzando | El vehículo está atravesando la intersección |
| Finalizado | El vehículo completó su recorrido |

### Ejemplo de recorrido

Un vehículo puede seguir esta secuencia:

`Programado → EnCola → Cruzando → Finalizado`

Dependiendo de las condiciones de tráfico, un vehículo puede iniciar su cruce sin acumular tiempo de espera.

## 11. Semáforos inteligentes

SISTRA incorpora un sistema de control de semáforos para administrar el paso de vehículos por la intersección.

Los semáforos regulan qué calles pueden avanzar y cuáles deben esperar.

El control inteligente considera las condiciones de tráfico para administrar los tiempos de paso y favorecer el flujo vehicular.

Durante la simulación, el comportamiento de los semáforos influye en la formación de colas y los tiempos de espera.

## 12. Visualización gráfica 2D

SISTRA dispone de una representación gráfica de la intersección mediante Canvas 2D.

Esta visualización permite observar el comportamiento del tráfico desde una vista superior.

### Elementos representados

- Calles Norte, Sur, Este y Oeste.
- Vehículos.
- Semáforos.
- Movimiento de los vehículos.
- Cruce de la intersección.

### Interpretación

Cuando un vehículo llega a la intersección, el motor determina cómo debe continuar su recorrido.

Si debe esperar, permanece en cola.

Cuando puede atravesar la intersección, cambia al estado `Cruzando`.

Después de completar el recorrido, cambia a `Finalizado`.

La representación gráfica permite observar estos cambios durante el desarrollo de la simulación.

## 13. Control de velocidad

SISTRA incorpora cámaras virtuales para detectar vehículos que superan los límites de velocidad configurados.

Durante la simulación, el sistema verifica las velocidades y determina si corresponde registrar una infracción.

### Ejemplo

Durante las pruebas se utilizó un vehículo con estas características:

- Placa: `P201ABC`.
- Calle: Norte.
- Velocidad: 95 km/h.

El sistema detectó una infracción y registró una multa de Q600.

Los montos dependen de las reglas de control de velocidad configuradas en el sistema.

## 14. Multas

Cuando se detecta una infracción, SISTRA registra la multa correspondiente en la base de datos.

Las estadísticas permiten consultar:

- Cantidad total de multas.
- Monto total acumulado.

### Prevención de multas duplicadas

El sistema contempla mecanismos para evitar que una misma multa se registre nuevamente cuando se recupera y continúa una simulación.

Esto permite conservar la consistencia de los resultados.

## 15. Estadísticas de tráfico

SISTRA genera estadísticas para conocer el comportamiento de los vehículos durante una simulación.

### 15.1. Estadísticas generales

| Estadística | Descripción |
|---|---|
| Total de vehículos | Cantidad de vehículos incorporados |
| Programados | Vehículos pendientes de llegar |
| En cola | Vehículos esperando |
| Cruzando | Vehículos atravesando la intersección |
| Finalizados | Vehículos que completaron su recorrido |
| Tiempo promedio de espera | Promedio de tiempo de espera registrado |

### 15.2. Estadísticas por calle

Para cada calle se pueden consultar:

- Nombre.
- Total de vehículos.
- Vehículos en cola.
- Vehículos finalizados.
- Tiempo promedio de espera.

Estas estadísticas permiten comparar el comportamiento del tráfico entre las cuatro direcciones.

### 15.3. Estadísticas de multas

También se muestran:

- Total de multas registradas.
- Monto total de las multas.

Los montos se expresan en quetzales.

## 16. Consulta del historial de vehículos

SISTRA permite consultar los vehículos asociados a una simulación.

La información puede incluir:

- Placa.
- Tipo de vehículo.
- Calle.
- Velocidad.
- Segundo y hora de llegada.
- Segundo y hora de inicio de cruce.
- Segundo y hora de salida.
- Tiempo de espera.
- Estado final.

### Ejemplo de historial

Durante las pruebas se registró el siguiente recorrido:

| Dato | Resultado |
|---|---|
| Placa | P201ABC |
| Calle | Norte |
| Velocidad | 95 km/h |
| Hora de llegada | 08:00:00 |
| Hora de inicio de cruce | 08:00:00 |
| Hora de salida | 08:00:05 |
| Tiempo de espera | 0 segundos |
| Estado | Finalizado |

Este historial permite conocer cómo se comportó el vehículo durante la simulación.

## 17. Consulta de simulaciones anteriores

SISTRA almacena las simulaciones y sus resultados en MySQL.

Esto permite consultar información de ejecuciones anteriores, incluyendo simulaciones finalizadas.

Entre los datos disponibles se encuentran:

- Nombre de la simulación.
- Estado.
- Duración.
- Segundo actual.
- Vehículos incorporados.
- Vehículos finalizados.
- Estadísticas.
- Multas.

Los resultados históricos permanecen disponibles después de reiniciar el backend.

## 18. Recuperación de simulaciones

Si el servidor se reinicia mientras una simulación se encuentra en estado `Ejecutando`, SISTRA puede reconstruir el estado del motor utilizando la información almacenada en MySQL.

Esto permite continuar la simulación desde el progreso persistido.

### Consideraciones

- La simulación debe encontrarse en estado `Ejecutando`.
- El progreso se recupera desde la base de datos.
- Los resultados previamente registrados se conservan.
- Las multas no deben duplicarse.
- Las simulaciones finalizadas no pueden recuperarse como activas.

## 19. Mensajes y errores comunes

El sistema utiliza mensajes y códigos HTTP para indicar el resultado de las operaciones.

### Solicitud inválida

Ocurre cuando se envían datos incorrectos o incompletos.

**Código HTTP:** 400.

### Recurso inexistente

Ocurre cuando se intenta consultar una simulación que no existe.

**Código HTTP:** 404.

### Operación no permitida

Ocurre cuando se intenta realizar una operación incompatible con el estado de la simulación.

**Código HTTP:** 409.

Por ejemplo, intentar avanzar una simulación finalizada produce el mensaje:

`La simulacion no esta en estado Ejecutando`

### Problemas de conexión

Si la base de datos no se encuentra disponible, las operaciones que dependen de MySQL pueden fallar.

En ese caso, debe verificarse que el servidor MySQL esté iniciado y que la configuración de conexión sea correcta.

## 20. Recomendaciones de uso

Para utilizar SISTRA correctamente se recomienda:

1. Verificar que el backend esté ejecutándose.
2. Confirmar que MySQL esté disponible.
3. Utilizar vehículos previamente registrados.
4. Incorporar los vehículos antes de iniciar una simulación.
5. Revisar la duración configurada.
6. Iniciar la simulación.
7. Avanzar el tiempo según el comportamiento que se desee observar.
8. Consultar las estadísticas y resultados.
9. Utilizar el historial para revisar las simulaciones finalizadas.

## 21. Documentación adicional

Para conocer la instalación y configuración del proyecto:

[Guía de instalación](INSTALACION.md)

Para consultar los endpoints disponibles:

[Documentación de la API REST](API.md)

Para obtener información general:

[README principal](../README.md)

## 22. Estado del proyecto

**Fase 12 de 13 - Documentación.**

El sistema cuenta con funcionalidades de registro de vehículos, simulación de tráfico, semáforos inteligentes, visualización 2D, control de velocidad, multas, estadísticas y persistencia en MySQL.

Las principales pruebas de integración fueron realizadas durante la Fase 11.

La Fase 12 está destinada a documentar el funcionamiento del sistema.

La Fase 13 corresponde a la preparación de la entrega final.

## 23. Repositorio

El código fuente de SISTRA se encuentra disponible en:

https://github.com/jyuman-umg/SISTRA

## 24. Autor

**Jefferson Gadiel Yuman Pérez**  
**Curso:** Algoritmos  
**Proyecto:** 18 - Intersección Semaforizada  
**Variante:** A
