/* =====================================================
   SISTRA - FRONTEND
   FASE 8: API REST E INTERSECCION 2D
   ===================================================== */

const API_BASE = "http://localhost:18080/api";

const elementos = {
    serverIndicator: document.getElementById("serverIndicator"),
    serverStatus: document.getElementById("serverStatus"),
    simulationSelect: document.getElementById("simulationSelect"),
    refreshButton: document.getElementById("refreshButton"),
    startButton: document.getElementById("startButton"),
    advanceButton: document.getElementById("advanceButton"),
    advanceMultipleButton: document.getElementById("advanceMultipleButton"),
    secondsInput: document.getElementById("secondsInput"),
    simulationMessage: document.getElementById("simulationMessage"),
    currentSecond: document.getElementById("currentSecond"),
    totalVehicles: document.getElementById("totalVehicles"),
    queuedVehicles: document.getElementById("queuedVehicles"),
    crossingVehicles: document.getElementById("crossingVehicles"),
    finishedVehicles: document.getElementById("finishedVehicles"),
    trafficLights: document.getElementById("trafficLights"),
    vehiclesTable: document.getElementById("vehiclesTable")
};

const calles = [
    { idCalle: 1, nombre: "Norte" },
    { idCalle: 2, nombre: "Sur" },
    { idCalle: 3, nombre: "Este" },
    { idCalle: 4, nombre: "Oeste" }
];

let simulacionSeleccionada = null;
let estadoSimulacion = null;
let procesandoSolicitud = false;
let versionSeleccion = 0;

// Datos utilizados por el Canvas 2D.
let ultimoEstadoGrafico = null;
let vehiculosGraficos = [];

/* =====================================================
   PETICIONES HTTP
   ===================================================== */

async function solicitarAPI(ruta, opciones = {}) {
    let respuesta;

    try {
        respuesta = await fetch(`${API_BASE}${ruta}`, {
            ...opciones,
            headers: {
                "Content-Type": "application/json",
                ...(opciones.headers || {})
            }
        });
    } catch (error) {
        throw new Error(
            "No se pudo conectar con el servidor SISTRA."
        );
    }

    let datos;

    try {
        datos = await respuesta.json();
    } catch (error) {
        throw new Error(
            "El servidor devolvio una respuesta no valida."
        );
    }

    if (!respuesta.ok || datos.success === false) {
        throw new Error(
            datos.message ||
            `Error HTTP ${respuesta.status}`
        );
    }

    return datos;
}

/* =====================================================
   MENSAJES
   ===================================================== */

function mostrarMensaje(mensaje, tipo = "") {
    elementos.simulationMessage.textContent = mensaje;

    elementos.simulationMessage.className =
        `message ${tipo}`.trim();
}

/* =====================================================
   CONTROLES
   ===================================================== */

function actualizarBotones() {
    const haySeleccion = simulacionSeleccionada !== null;

    const configurada =
        estadoSimulacion === "Configurada";

    const ejecutando =
        estadoSimulacion === "Ejecutando";

    elementos.simulationSelect.disabled =
        procesandoSolicitud;

    elementos.refreshButton.disabled =
        procesandoSolicitud;

    elementos.startButton.disabled =
        procesandoSolicitud ||
        !haySeleccion ||
        !configurada;

    elementos.advanceButton.disabled =
        procesandoSolicitud ||
        !haySeleccion ||
        !ejecutando;

    elementos.advanceMultipleButton.disabled =
        procesandoSolicitud ||
        !haySeleccion ||
        !ejecutando;

    elementos.secondsInput.disabled =
        procesandoSolicitud ||
        !haySeleccion ||
        !ejecutando;
}

async function ejecutarOperacion(operacion) {
    if (procesandoSolicitud) {
        return;
    }

    procesandoSolicitud = true;
    actualizarBotones();

    try {
        await operacion();
    } catch (error) {
        mostrarMensaje(error.message, "error");
    } finally {
        procesandoSolicitud = false;
        actualizarBotones();
    }
}

/* =====================================================
   ESTADO DEL SERVIDOR
   ===================================================== */

async function verificarServidor() {
    try {
        const respuesta = await fetch(
            `${API_BASE}/health`
        );

        if (!respuesta.ok) {
            throw new Error("Servidor no disponible");
        }

        elementos.serverIndicator.className =
            "status-dot online";

        elementos.serverStatus.textContent =
            "Servidor conectado";

        return true;
    } catch (error) {
        elementos.serverIndicator.className =
            "status-dot offline";

        elementos.serverStatus.textContent =
            "Servidor desconectado";

        return false;
    }
}

/* =====================================================
   ESTADISTICAS
   ===================================================== */

function reiniciarEstadisticas() {
    elementos.currentSecond.textContent = "0 s";
    elementos.totalVehicles.textContent = "0";
    elementos.queuedVehicles.textContent = "0";
    elementos.crossingVehicles.textContent = "0";
    elementos.finishedVehicles.textContent = "0";
}

function actualizarEstadisticas(datos) {
    elementos.currentSecond.textContent =
        `${datos.segundoActual ?? 0} s`;

    elementos.totalVehicles.textContent =
        datos.totalVehiculos ?? 0;

    elementos.queuedVehicles.textContent =
        datos.enCola ?? 0;

    elementos.crossingVehicles.textContent =
        datos.cruzando ?? 0;

    elementos.finishedVehicles.textContent =
        datos.finalizados ?? 0;
}

/* =====================================================
   SEMAFOROS
   ===================================================== */

function crearLuz(color, estadoActual) {
    const luz = document.createElement("div");

    luz.className = `light ${color}`;

    const estados = {
        red: "Rojo",
        yellow: "Amarillo",
        green: "Verde"
    };

    if (estados[color] === estadoActual) {
        luz.classList.add("active");
    }

    return luz;
}

function crearTarjetaSemaforo(calle) {
    const tarjeta = document.createElement("article");
    tarjeta.className = "traffic-card";

    const titulo = document.createElement("h3");
    titulo.textContent = calle.nombre;

    const semaforo = document.createElement("div");
    semaforo.className = "traffic-light";

    semaforo.appendChild(
        crearLuz("red", calle.estado)
    );

    semaforo.appendChild(
        crearLuz("yellow", calle.estado)
    );

    semaforo.appendChild(
        crearLuz("green", calle.estado)
    );

    const estado = document.createElement("p");
    estado.className = "traffic-state";
    estado.textContent = calle.estado;

    const cola = document.createElement("p");
    cola.className = "traffic-details";

    cola.textContent =
        `Vehiculos en espera: ${calle.vehiculosEnCola ?? 0}`;

    tarjeta.appendChild(titulo);
    tarjeta.appendChild(semaforo);
    tarjeta.appendChild(estado);
    tarjeta.appendChild(cola);

    return tarjeta;
}

function mostrarSemaforos(datos) {
    elementos.trafficLights.replaceChildren();

    const lista = datos?.semaforos?.calles;

    if (!Array.isArray(lista)) {
        mostrarSemaforosIniciales();
        return;
    }

    for (const calle of lista) {
        elementos.trafficLights.appendChild(
            crearTarjetaSemaforo(calle)
        );
    }

    const segundosRestantes =
        datos.semaforos.segundosRestantes ?? 0;

    const detalle = document.createElement("p");
    detalle.className = "traffic-details";

    detalle.textContent =
        `Tiempo restante de la fase: ${segundosRestantes} segundos`;

    detalle.style.gridColumn = "1 / -1";
    detalle.style.textAlign = "center";

    elementos.trafficLights.appendChild(detalle);
}

function mostrarSemaforosIniciales() {
    elementos.trafficLights.replaceChildren();

    for (const calle of calles) {
        elementos.trafficLights.appendChild(
            crearTarjetaSemaforo({
                ...calle,
                estado: "Sin datos",
                vehiculosEnCola: 0
            })
        );
    }
}

/* =====================================================
   INTERSECCION 2D - FASE 8
   ===================================================== */

function obtenerRenderizador() {
    return window.sistraIntersection ?? null;
}

function reiniciarInterseccion() {
    ultimoEstadoGrafico = null;
    vehiculosGraficos = [];

    const renderizador = obtenerRenderizador();

    if (renderizador) {
        renderizador.reset();
    }
}

function actualizarInterseccion() {
    const renderizador = obtenerRenderizador();

    if (!renderizador || !ultimoEstadoGrafico) {
        return;
    }

    renderizador.update(
        ultimoEstadoGrafico,
        vehiculosGraficos
    );
}

function guardarEstadoGrafico(datos) {
    ultimoEstadoGrafico = datos;
}

function guardarVehiculosGraficos(vehiculos) {
    vehiculosGraficos = Array.isArray(vehiculos)
        ? vehiculos
        : [];
}

function mostrarEstadoInterseccion(texto, tipo = "") {
    const renderizador = obtenerRenderizador();

    if (renderizador) {
        renderizador.setStatus(texto, tipo);
    }
}

/* =====================================================
   TABLA DE VEHICULOS
   ===================================================== */

function crearCelda(texto) {
    const celda = document.createElement("td");
    celda.textContent = texto ?? "-";
    return celda;
}

function mostrarVehiculos(vehiculos) {
    elementos.vehiclesTable.replaceChildren();

    if (!Array.isArray(vehiculos) ||
        vehiculos.length === 0) {

        const fila = document.createElement("tr");
        const celda = document.createElement("td");

        celda.colSpan = 6;
        celda.textContent =
            "No hay vehiculos registrados.";

        fila.appendChild(celda);
        elementos.vehiclesTable.appendChild(fila);

        return;
    }

    for (const vehiculo of vehiculos) {
        const fila = document.createElement("tr");

        fila.appendChild(
            crearCelda(vehiculo.placa)
        );

        fila.appendChild(
            crearCelda(vehiculo.tipoVehiculo)
        );

        fila.appendChild(
            crearCelda(vehiculo.calle)
        );

        fila.appendChild(
            crearCelda(`${vehiculo.velocidad} km/h`)
        );

        const celdaEstado = document.createElement("td");
        const etiqueta = document.createElement("span");

        etiqueta.className =
            `vehicle-status ${String(
                vehiculo.estado || ""
            ).toLowerCase()}`;

        etiqueta.textContent =
            vehiculo.estado || "-";

        celdaEstado.appendChild(etiqueta);
        fila.appendChild(celdaEstado);

        fila.appendChild(
            crearCelda(
                `${vehiculo.tiempoEsperaSegundos ?? 0} s`
            )
        );

        elementos.vehiclesTable.appendChild(fila);
    }
}

/* =====================================================
   CONSULTAR VEHICULOS
   ===================================================== */

async function consultarVehiculos(idSimulacion) {
    const datos = await solicitarAPI(
        `/simulaciones/${idSimulacion}/vehiculos`
    );

    if (simulacionSeleccionada !== idSimulacion) {
        return;
    }

    const vehiculos = datos.data || [];

    mostrarVehiculos(vehiculos);
    guardarVehiculosGraficos(vehiculos);
    actualizarInterseccion();
}

/* =====================================================
   CONSULTAR ESTADO DE SIMULACION
   ===================================================== */

async function consultarEstado(idSimulacion) {
    try {
        const datos = await solicitarAPI(
            `/simulaciones/${idSimulacion}/estado`
        );

        if (simulacionSeleccionada === idSimulacion) {
            actualizarEstadisticas(datos);
            mostrarSemaforos(datos);

            guardarEstadoGrafico(datos);
            actualizarInterseccion();
        }

        return datos;
    } catch (error) {
        if (simulacionSeleccionada === idSimulacion) {
            mostrarSemaforosIniciales();

            mostrarEstadoInterseccion(
                "Simulacion no activa"
            );
        }

        return null;
    }
}

/* =====================================================
   SELECCIONAR SIMULACION
   ===================================================== */

async function seleccionarSimulacion() {
    const valor = elementos.simulationSelect.value;
    const versionActual = ++versionSeleccion;

    if (!valor) {
        simulacionSeleccionada = null;
        estadoSimulacion = null;

        reiniciarEstadisticas();
        mostrarSemaforosIniciales();
        mostrarVehiculos([]);
        reiniciarInterseccion();

        mostrarMensaje("");
        actualizarBotones();

        return;
    }

    const id = Number(valor);

    simulacionSeleccionada = id;
    estadoSimulacion = null;

    reiniciarEstadisticas();
    mostrarSemaforosIniciales();
    mostrarVehiculos([]);
    reiniciarInterseccion();
    actualizarBotones();

    try {
        const datos = await solicitarAPI(
            `/simulaciones/${id}`
        );

        if (versionActual !== versionSeleccion) {
            return;
        }

        estadoSimulacion = datos.data.estado;

        elementos.totalVehicles.textContent =
            datos.data.totalVehiculos ?? 0;

        elementos.finishedVehicles.textContent =
            datos.data.totalVehiculosFinalizados ?? 0;

        await consultarVehiculos(id);

        if (versionActual !== versionSeleccion) {
            return;
        }

        if (estadoSimulacion === "Ejecutando") {
            const estado = await consultarEstado(id);

            if (!estado) {
                mostrarMensaje(
                    "La simulacion figura como Ejecutando, " +
                    "pero no esta activa en la memoria del servidor.",
                    "error"
                );
            } else {
                mostrarMensaje(
                    `Simulacion seleccionada: ${datos.data.nombre}`
                );
            }

        } else {
            mostrarEstadoInterseccion(
                estadoSimulacion === "Finalizada"
                    ? "Simulacion finalizada"
                    : "Simulacion configurada",
                estadoSimulacion === "Finalizada"
                    ? "finished"
                    : ""
            );

            mostrarMensaje(
                `Simulacion seleccionada: ${datos.data.nombre}`
            );
        }

    } catch (error) {
        if (versionActual === versionSeleccion) {
            mostrarMensaje(error.message, "error");
        }
    } finally {
        if (versionActual === versionSeleccion) {
            actualizarBotones();
        }
    }
}

/* =====================================================
   CARGAR SIMULACIONES
   ===================================================== */

async function cargarSimulaciones() {
    const datos = await solicitarAPI("/simulaciones");

    const seleccionAnterior =
        simulacionSeleccionada;

    elementos.simulationSelect.replaceChildren();

    const opcionInicial =
        document.createElement("option");

    opcionInicial.value = "";
    opcionInicial.textContent =
        "Selecciona una simulacion";

    elementos.simulationSelect.appendChild(
        opcionInicial
    );

    for (const simulacion of datos.data || []) {
        const opcion = document.createElement("option");

        opcion.value = simulacion.idSimulacion;

        opcion.textContent =
            `#${simulacion.idSimulacion} - ` +
            `${simulacion.nombre} (${simulacion.estado})`;

        elementos.simulationSelect.appendChild(
            opcion
        );
    }

    if (seleccionAnterior !== null) {
        const existe = Array.from(
            elementos.simulationSelect.options
        ).some(
            opcion =>
                Number(opcion.value) === seleccionAnterior
        );

        if (existe) {
            elementos.simulationSelect.value =
                String(seleccionAnterior);
        }
    }

    await seleccionarSimulacion();
}

/* =====================================================
   INICIAR SIMULACION
   ===================================================== */

async function iniciarSimulacion() {
    const id = simulacionSeleccionada;

    if (id === null) {
        mostrarMensaje(
            "Selecciona una simulacion.",
            "error"
        );
        return;
    }

    await ejecutarOperacion(async () => {
        const datos = await solicitarAPI(
            `/simulaciones/${id}/iniciar`,
            { method: "POST" }
        );

        estadoSimulacion =
            datos.estadoSimulacion || "Ejecutando";

        actualizarEstadisticas(datos);
        mostrarSemaforos(datos);

        guardarEstadoGrafico(datos);

        await consultarVehiculos(id);

        mostrarMensaje(
            datos.message ||
            "Simulacion iniciada correctamente.",
            "success"
        );
    });
}

/* =====================================================
   AVANZAR SIMULACION
   ===================================================== */

async function avanzarSimulacion(segundos) {
    const id = simulacionSeleccionada;

    if (id === null) {
        mostrarMensaje(
            "Selecciona una simulacion.",
            "error"
        );
        return;
    }

    await ejecutarOperacion(async () => {
        const ruta = segundos === 1
            ? `/simulaciones/${id}/avanzar`
            : `/simulaciones/${id}/avanzar-multiples`;

        const opciones = {
            method: "POST"
        };

        if (segundos !== 1) {
            opciones.body = JSON.stringify({
                segundos
            });
        }

        const datos = await solicitarAPI(
            ruta,
            opciones
        );

        estadoSimulacion =
            datos.estadoSimulacion || "Ejecutando";

        actualizarEstadisticas(datos);
        mostrarSemaforos(datos);

        guardarEstadoGrafico(datos);

        await consultarVehiculos(id);

        mostrarMensaje(
            datos.message ||
            "Simulacion avanzada correctamente.",
            "success"
        );

        if (estadoSimulacion === "Finalizada") {
            mostrarMensaje(
                "La simulacion ha finalizado correctamente.",
                "success"
            );
        }
    });
}

/* =====================================================
   EVENTOS
   ===================================================== */

elementos.simulationSelect.addEventListener(
    "change",
    seleccionarSimulacion
);

elementos.refreshButton.addEventListener(
    "click",
    () => ejecutarOperacion(cargarSimulaciones)
);

elementos.startButton.addEventListener(
    "click",
    iniciarSimulacion
);

elementos.advanceButton.addEventListener(
    "click",
    () => avanzarSimulacion(1)
);

elementos.advanceMultipleButton.addEventListener(
    "click",
    () => {
        const segundos =
            Number(elementos.secondsInput.value);

        if (!Number.isInteger(segundos) ||
            segundos < 1 ||
            segundos > 3600) {

            mostrarMensaje(
                "Ingresa una cantidad entre 1 y 3600.",
                "error"
            );

            return;
        }

        avanzarSimulacion(segundos);
    }
);

/* =====================================================
   INICIALIZACION
   ===================================================== */

async function inicializar() {
    reiniciarEstadisticas();
    mostrarSemaforosIniciales();
    mostrarVehiculos([]);
    reiniciarInterseccion();
    actualizarBotones();

    const conectado = await verificarServidor();

    if (!conectado) {
        mostrarMensaje(
            "Inicia el backend de SISTRA en el puerto 18080.",
            "error"
        );
        return;
    }

    await ejecutarOperacion(cargarSimulaciones);
}

document.addEventListener(
    "DOMContentLoaded",
    inicializar
);