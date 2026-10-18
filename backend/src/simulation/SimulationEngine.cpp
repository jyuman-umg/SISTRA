#include "simulation/SimulationEngine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

SimulationEngine::SimulationEngine(
    int duracion,
    int cruce
) :
    segundoActual(0),
    duracionSegundos(duracion),
    duracionCruceSegundos(cruce),
    semaforos() {

    if (duracion <= 0 || cruce <= 0) {
        throw std::invalid_argument(
            "Duraciones invalidas"
        );
    }
}

bool SimulationEngine::calleValida(
    int id
) const {
    return id >= 1 && id <= 4;
}

void SimulationEngine::agregarVehiculo(
    const VehiculoSimulado& v
) {
    if (!calleValida(v.idCalle) ||
        v.idSimulacionVehiculo <= 0 ||
        v.segundoLlegada < 0 ||
        v.segundoLlegada >= duracionSegundos ||
        !std::isfinite(v.velocidad) ||
        v.velocidad < 0) {

        throw std::invalid_argument(
            "Datos de vehiculo invalidos"
        );
    }

    for (const auto& actual : vehiculos) {
        if (actual.idSimulacionVehiculo ==
            v.idSimulacionVehiculo) {

            throw std::invalid_argument(
                "Vehiculo duplicado"
            );
        }
    }

    VehiculoSimulado nuevo = v;

    nuevo.estado = EstadoVehiculo::Programado;
    nuevo.segundoInicioCruce = -1;
    nuevo.segundoSalida = -1;
    nuevo.tiempoEsperaSegundos = 0;

    vehiculos.push_back(nuevo);

    std::sort(
        vehiculos.begin(),
        vehiculos.end(),
        [](const auto& a, const auto& b) {
            if (a.segundoLlegada != b.segundoLlegada) {
                return a.segundoLlegada < b.segundoLlegada;
            }

            return a.idSimulacionVehiculo <
                   b.idSimulacionVehiculo;
        }
    );
}

void SimulationEngine::procesarLlegadas() {
    for (auto& v : vehiculos) {
        if (v.estado == EstadoVehiculo::Programado &&
            v.segundoLlegada <= segundoActual) {

            v.estado = EstadoVehiculo::EnCola;
        }
    }
}

void SimulationEngine::procesarSalidas() {
    for (auto& v : vehiculos) {
        if (v.estado == EstadoVehiculo::Cruzando &&
            v.segundoInicioCruce >= 0 &&
            segundoActual - v.segundoInicioCruce >=
                duracionCruceSegundos) {

            v.estado = EstadoVehiculo::Finalizado;
            v.segundoSalida = segundoActual;
        }
    }
}

bool SimulationEngine::vehiculoPuedeCruzar(
    int id
) const {
    if (!calleValida(id)) {
        return false;
    }

    // La calle debe tener luz verde.
    if (!semaforos.puedeCruzar(id)) {
        return false;
    }

    // Por seguridad, solo un vehiculo puede
    // estar cruzando la interseccion.
    for (const auto& v : vehiculos) {
        if (v.estado == EstadoVehiculo::Cruzando) {
            return false;
        }
    }

    return true;
}

void SimulationEngine::procesarCruces() {
    for (auto& v : vehiculos) {
        if (v.estado == EstadoVehiculo::EnCola &&
            vehiculoPuedeCruzar(v.idCalle)) {

            v.estado = EstadoVehiculo::Cruzando;

            v.segundoInicioCruce = segundoActual;

            v.tiempoEsperaSegundos =
                segundoActual - v.segundoLlegada;

            break;
        }
    }
}

void SimulationEngine::actualizarColasSemaforos() {
    std::array<int, 4> colas{};

    for (const auto& v : vehiculos) {
        if (v.estado == EstadoVehiculo::EnCola &&
            calleValida(v.idCalle)) {

            colas[v.idCalle - 1]++;
        }
    }

    semaforos.actualizarColas(colas);
}

bool SimulationEngine::avanzarSegundo() {
    if (haFinalizado()) {
        return false;
    }

    // 1. Registrar los vehiculos que ya
    // terminaron de cruzar.
    procesarSalidas();

    // 2. Incorporar nuevas llegadas.
    procesarLlegadas();

    // 3. Actualizar las colas para que el
    // semaforo conozca la demanda actual.
    actualizarColasSemaforos();

    // 4. Permitir cruces solamente con verde.
    procesarCruces();

    // 5. Actualizar nuevamente las colas
    // despues de iniciar un cruce.
    actualizarColasSemaforos();

    // 6. Avanzar el controlador semaforico.
    semaforos.avanzarSegundo();

    // 7. Avanzar el tiempo de simulacion.
    segundoActual++;

    return true;
}

void SimulationEngine::reiniciar() {
    segundoActual = 0;

    semaforos.reiniciar();

    for (auto& v : vehiculos) {
        v.estado = EstadoVehiculo::Programado;
        v.segundoInicioCruce = -1;
        v.segundoSalida = -1;
        v.tiempoEsperaSegundos = 0;
    }
}

// =====================================================
// FASE 11: RECONSTRUIR SIMULACION HASTA UN SEGUNDO
// =====================================================

void SimulationEngine::reconstruirHastaSegundo(
    int segundoObjetivo
) {
    if (segundoObjetivo < 0 ||
        segundoObjetivo > duracionSegundos) {

        throw std::invalid_argument(
            "El segundo objetivo esta fuera "
            "de la duracion de la simulacion"
        );
    }

    // Reconstruir desde el estado inicial.
    reiniciar();

    // Reproducir los segundos anteriores
    // utilizando las mismas reglas del motor.
    for (int segundo = 0;
         segundo < segundoObjetivo;
         ++segundo) {

        if (!avanzarSegundo()) {
            throw std::runtime_error(
                "No se pudo reconstruir la simulacion"
            );
        }
    }

    // Verificar que se alcanzo el segundo esperado.
    if (segundoActual != segundoObjetivo) {
        throw std::runtime_error(
            "El segundo reconstruido no coincide "
            "con el segundo solicitado"
        );
    }
}

int SimulationEngine::obtenerSegundoActual() const {
    return segundoActual;
}

int SimulationEngine::obtenerDuracionSegundos() const {
    return duracionSegundos;
}

bool SimulationEngine::haFinalizado() const {
    return segundoActual >= duracionSegundos;
}

ResumenSemaforos
SimulationEngine::obtenerResumenSemaforos() const {
    return semaforos.obtenerResumen();
}

ResumenSimulacion
SimulationEngine::obtenerResumen() const {
    ResumenSimulacion r;

    r.segundoActual = segundoActual;

    r.totalVehiculos =
        static_cast<int>(vehiculos.size());

    for (const auto& v : vehiculos) {
        switch (v.estado) {
            case EstadoVehiculo::Programado:
                r.programados++;
                break;

            case EstadoVehiculo::EnCola:
                r.enCola++;

                if (calleValida(v.idCalle)) {
                    r.vehiculosEnCola[
                        v.idCalle - 1
                    ]++;
                }
                break;

            case EstadoVehiculo::Cruzando:
                r.cruzando++;
                break;

            case EstadoVehiculo::Finalizado:
                r.finalizados++;
                break;
        }
    }

    r.semaforos = semaforos.obtenerResumen();

    return r;
}

const std::vector<VehiculoSimulado>&
SimulationEngine::obtenerVehiculos() const {
    return vehiculos;
}

std::string SimulationEngine::estadoComoTexto(
    EstadoVehiculo estado
) {
    switch (estado) {
        case EstadoVehiculo::Programado:
            return "Programado";

        case EstadoVehiculo::EnCola:
            return "EnCola";

        case EstadoVehiculo::Cruzando:
            return "Cruzando";

        case EstadoVehiculo::Finalizado:
            return "Finalizado";
    }

    return "Desconocido";
}