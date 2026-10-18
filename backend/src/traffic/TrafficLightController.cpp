#include "traffic/TrafficLightController.h"
#include <algorithm>
#include <stdexcept>

TrafficLightController::TrafficLightController(
    int duracionVerde,
    int duracionAmarillo,
    int duracionMinimaVerde,
    int duracionMaximaVerde
) :
    segundoActual(0),
    calleActual(1),
    duracionVerde(duracionVerde),
    duracionAmarillo(duracionAmarillo),
    duracionMinimaVerde(duracionMinimaVerde),
    duracionMaximaVerde(duracionMaximaVerde),
    segundosTranscurridos(0),
    duracionVerdeActual(duracionVerde),
    faseAmarilla(false),
    colas{} {

    if (duracionVerde <= 0 ||
        duracionAmarillo <= 0 ||
        duracionMinimaVerde <= 0 ||
        duracionMaximaVerde < duracionMinimaVerde) {

        throw std::invalid_argument(
            "Configuracion de semaforos invalida."
        );
    }

    duracionVerdeActual = std::clamp(
        duracionVerde,
        duracionMinimaVerde,
        duracionMaximaVerde
    );
}

void TrafficLightController::actualizarColas(
    const std::array<int, 4>& nuevasColas
) {
    colas = nuevasColas;
}

int TrafficLightController::seleccionarCallePrioritaria()
const {
    int mejorCalle = 0;
    int mayorCola = 0;

    // Seleccionamos la calle con mas vehiculos.
    // En empate, damos preferencia a una calle
    // diferente de la que acaba de recibir verde.

    for (int i = 0; i < 4; i++) {
        const int idCalle = i + 1;

        if (colas[i] > mayorCola) {
            mayorCola = colas[i];
            mejorCalle = idCalle;
        } else if (
            colas[i] == mayorCola &&
            mayorCola > 0 &&
            mejorCalle == calleActual &&
            idCalle != calleActual
        ) {
            mejorCalle = idCalle;
        }
    }

    if (mejorCalle == 0) {
        return calleActual % 4 + 1;
    }

    return mejorCalle;
}

int TrafficLightController::calcularDuracionVerde(
    int idCalle
) const {
    if (idCalle < 1 || idCalle > 4) {
        return duracionMinimaVerde;
    }

    const int cantidad =
        colas[idCalle - 1];

    const int duracionCalculada =
        duracionMinimaVerde + cantidad * 2;

    return std::clamp(
        duracionCalculada,
        duracionMinimaVerde,
        duracionMaximaVerde
    );
}

void TrafficLightController::avanzarSegundo() {
    segundoActual++;
    segundosTranscurridos++;

    if (!faseAmarilla) {
        if (segundosTranscurridos >=
            duracionVerdeActual) {

            faseAmarilla = true;
            segundosTranscurridos = 0;
        }

        return;
    }

    if (segundosTranscurridos >=
        duracionAmarillo) {

        calleActual =
            seleccionarCallePrioritaria();

        duracionVerdeActual =
            calcularDuracionVerde(calleActual);

        faseAmarilla = false;
        segundosTranscurridos = 0;
    }
}

void TrafficLightController::reiniciar() {
    segundoActual = 0;
    calleActual = 1;
    segundosTranscurridos = 0;
    faseAmarilla = false;
    colas.fill(0);

    duracionVerdeActual = std::clamp(
        duracionVerde,
        duracionMinimaVerde,
        duracionMaximaVerde
    );
}

bool TrafficLightController::puedeCruzar(
    int idCalle
) const {
    return idCalle >= 1 &&
           idCalle <= 4 &&
           idCalle == calleActual &&
           !faseAmarilla;
}

EstadoSemaforo TrafficLightController::obtenerEstado(
    int idCalle
) const {
    if (idCalle < 1 || idCalle > 4) {
        return EstadoSemaforo::Rojo;
    }

    if (idCalle != calleActual) {
        return EstadoSemaforo::Rojo;
    }

    return faseAmarilla
        ? EstadoSemaforo::Amarillo
        : EstadoSemaforo::Verde;
}

ResumenSemaforos
TrafficLightController::obtenerResumen() const {
    ResumenSemaforos resumen;

    resumen.segundoActual = segundoActual;
    resumen.calleConPrioridad = calleActual;
    resumen.vehiculosEnCola = colas;

    resumen.segundosRestantes =
        (faseAmarilla
            ? duracionAmarillo
            : duracionVerdeActual)
        - segundosTranscurridos;

    for (int i = 0; i < 4; i++) {
        resumen.estados[i] =
            obtenerEstado(i + 1);
    }

    return resumen;
}

std::string TrafficLightController::estadoComoTexto(
    EstadoSemaforo estado
) {
    switch (estado) {
        case EstadoSemaforo::Rojo:
            return "Rojo";

        case EstadoSemaforo::Amarillo:
            return "Amarillo";

        case EstadoSemaforo::Verde:
            return "Verde";
    }

    return "Desconocido";
}
