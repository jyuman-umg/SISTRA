#ifndef SISTRA_SIMULATION_ENGINE_H
#define SISTRA_SIMULATION_ENGINE_H
#include <array>
#include <string>
#include <vector>

#include "traffic/TrafficLightController.h"

enum class EstadoVehiculo {
    Programado,
    EnCola,
    Cruzando,
    Finalizado
};

struct VehiculoSimulado {
    int idSimulacionVehiculo = 0;
    int idVehiculo = 0;
    int idCalle = 0;

    std::string placa;
    double velocidad = 0.0;

    int segundoLlegada = 0;
    int segundoInicioCruce = -1;
    int segundoSalida = -1;
    int tiempoEsperaSegundos = 0;

    EstadoVehiculo estado = EstadoVehiculo::Programado;
};

struct ResumenSimulacion {
    int segundoActual = 0;
    int totalVehiculos = 0;

    int programados = 0;
    int enCola = 0;
    int cruzando = 0;
    int finalizados = 0;

    std::array<int, 4> vehiculosEnCola{};

    ResumenSemaforos semaforos;
};

class SimulationEngine {
private:
    int segundoActual;
    int duracionSegundos;
    int duracionCruceSegundos;

    std::vector<VehiculoSimulado> vehiculos;

    TrafficLightController semaforos;

    bool calleValida(int idCalle) const;
    bool vehiculoPuedeCruzar(int idCalle) const;

    void procesarLlegadas();
    void procesarSalidas();
    void procesarCruces();

    void actualizarColasSemaforos();

public:
    explicit SimulationEngine(
        int duracionSegundos,
        int duracionCruceSegundos = 5
    );

    void agregarVehiculo(
        const VehiculoSimulado& vehiculo
    );

    bool avanzarSegundo();

    void reiniciar();

    int obtenerSegundoActual() const;

    int obtenerDuracionSegundos() const;

    bool haFinalizado() const;

    ResumenSimulacion obtenerResumen() const;

    ResumenSemaforos obtenerResumenSemaforos() const;

    const std::vector<VehiculoSimulado>&
    obtenerVehiculos() const;

    static std::string estadoComoTexto(
        EstadoVehiculo estado
    );
};

#endif
