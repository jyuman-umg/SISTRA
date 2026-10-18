#include "simulation/SimulationEngine.h"
#include <algorithm>
#include <stdexcept>

SimulationEngine::SimulationEngine(int duracion, int cruce)
    : segundoActual(0), duracionSegundos(duracion), duracionCruceSegundos(cruce) {
    if (duracion <= 0 || cruce <= 0) throw std::invalid_argument("Duraciones invalidas");
}
bool SimulationEngine::calleValida(int id) const { return id >= 1 && id <= 4; }
void SimulationEngine::agregarVehiculo(const VehiculoSimulado& v) {
    if (!calleValida(v.idCalle) || v.idSimulacionVehiculo <= 0 ||
        v.segundoLlegada < 0 || v.segundoLlegada >= duracionSegundos || v.velocidad < 0)
        throw std::invalid_argument("Datos de vehiculo invalidos");
    for (const auto& actual : vehiculos)
        if (actual.idSimulacionVehiculo == v.idSimulacionVehiculo)
            throw std::invalid_argument("Vehiculo duplicado");
    VehiculoSimulado nuevo = v;
    nuevo.estado = EstadoVehiculo::Programado;
    nuevo.segundoInicioCruce = -1;
    nuevo.segundoSalida = -1;
    nuevo.tiempoEsperaSegundos = 0;
    vehiculos.push_back(nuevo);
    std::sort(vehiculos.begin(), vehiculos.end(), [](const auto& a, const auto& b) {
        if (a.segundoLlegada != b.segundoLlegada) return a.segundoLlegada < b.segundoLlegada;
        return a.idSimulacionVehiculo < b.idSimulacionVehiculo;
    });
}
void SimulationEngine::procesarLlegadas() {
    for (auto& v : vehiculos)
        if (v.estado == EstadoVehiculo::Programado && v.segundoLlegada <= segundoActual)
            v.estado = EstadoVehiculo::EnCola;
}
void SimulationEngine::procesarSalidas() {
    for (auto& v : vehiculos)
        if (v.estado == EstadoVehiculo::Cruzando &&
            segundoActual - v.segundoInicioCruce >= duracionCruceSegundos) {
            v.estado = EstadoVehiculo::Finalizado;
            v.segundoSalida = segundoActual;
        }
}
bool SimulationEngine::vehiculoPuedeCruzar(int id) const {
    if (!calleValida(id)) return false;
    for (const auto& v : vehiculos)
        if (v.estado == EstadoVehiculo::Cruzando) return false;
    return true;
}
void SimulationEngine::procesarCruces() {
    for (auto& v : vehiculos)
        if (v.estado == EstadoVehiculo::EnCola && vehiculoPuedeCruzar(v.idCalle)) {
            v.estado = EstadoVehiculo::Cruzando;
            v.segundoInicioCruce = segundoActual;
            v.tiempoEsperaSegundos = segundoActual - v.segundoLlegada;
            break;
        }
}
bool SimulationEngine::avanzarSegundo() {
    if (haFinalizado()) return false;
    procesarSalidas();
    procesarLlegadas();
    procesarCruces();
    ++segundoActual;
    return true;
}
void SimulationEngine::reiniciar() {
    segundoActual = 0;
    for (auto& v : vehiculos) {
        v.estado = EstadoVehiculo::Programado;
        v.segundoInicioCruce = -1;
        v.segundoSalida = -1;
        v.tiempoEsperaSegundos = 0;
    }
}
int SimulationEngine::obtenerSegundoActual() const { return segundoActual; }
int SimulationEngine::obtenerDuracionSegundos() const { return duracionSegundos; }
bool SimulationEngine::haFinalizado() const { return segundoActual >= duracionSegundos; }
ResumenSimulacion SimulationEngine::obtenerResumen() const {
    ResumenSimulacion r;
    r.segundoActual = segundoActual;
    r.totalVehiculos = static_cast<int>(vehiculos.size());
    for (const auto& v : vehiculos) {
        switch (v.estado) {
            case EstadoVehiculo::Programado: ++r.programados; break;
            case EstadoVehiculo::EnCola: ++r.enCola; ++r.vehiculosEnCola[v.idCalle - 1]; break;
            case EstadoVehiculo::Cruzando: ++r.cruzando; break;
            case EstadoVehiculo::Finalizado: ++r.finalizados; break;
        }
    }
    return r;
}
const std::vector<VehiculoSimulado>& SimulationEngine::obtenerVehiculos() const { return vehiculos; }
std::string SimulationEngine::estadoComoTexto(EstadoVehiculo estado) {
    switch (estado) {
        case EstadoVehiculo::Programado: return "Programado";
        case EstadoVehiculo::EnCola: return "EnCola";
        case EstadoVehiculo::Cruzando: return "Cruzando";
        case EstadoVehiculo::Finalizado: return "Finalizado";
    }
    return "Desconocido";
}
