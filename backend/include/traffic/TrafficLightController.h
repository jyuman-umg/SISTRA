#ifndef SISTRA_TRAFFIC_LIGHT_CONTROLLER_H
#define SISTRA_TRAFFIC_LIGHT_CONTROLLER_H
#include <array>
#include <string>

enum class EstadoSemaforo {
    Rojo,
    Amarillo,
    Verde
};

struct ResumenSemaforos {
    int segundoActual = 0;
    int calleConPrioridad = 1;
    int segundosRestantes = 0;

    std::array<EstadoSemaforo, 4> estados{};
    std::array<int, 4> vehiculosEnCola{};
};

class TrafficLightController {
private:
    int segundoActual;
    int calleActual;

    int duracionVerde;
    int duracionAmarillo;
    int duracionMinimaVerde;
    int duracionMaximaVerde;

    int segundosTranscurridos;
    int duracionVerdeActual;

    bool faseAmarilla;

    std::array<int, 4> colas;

    int seleccionarCallePrioritaria() const;
    int calcularDuracionVerde(int idCalle) const;

public:
    TrafficLightController(
        int duracionVerde = 15,
        int duracionAmarillo = 3,
        int duracionMinimaVerde = 5,
        int duracionMaximaVerde = 30
    );

    void actualizarColas(
        const std::array<int, 4>& nuevasColas
    );

    void avanzarSegundo();

    void reiniciar();

    bool puedeCruzar(int idCalle) const;

    EstadoSemaforo obtenerEstado(
        int idCalle
    ) const;

    ResumenSemaforos obtenerResumen() const;

    static std::string estadoComoTexto(
        EstadoSemaforo estado
    );
};

#endif
