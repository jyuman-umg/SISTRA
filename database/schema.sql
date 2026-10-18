-- =====================================================
-- PROYECTO 18 - INTERSECCION SEMAFORIZADA
-- VARIANTE A
-- SISTRA - SISTEMA DE CONTROL DE TRAFICO VEHICULAR
-- =====================================================

CREATE DATABASE IF NOT EXISTS sistra_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE sistra_db;


-- =====================================================
-- TABLA: Calles
-- =====================================================

CREATE TABLE IF NOT EXISTS Calles (
    IdCalle INT AUTO_INCREMENT PRIMARY KEY,
    Nombre VARCHAR(20) NOT NULL,
    DireccionEntrada VARCHAR(20) NOT NULL,
    DireccionSalida VARCHAR(20) NOT NULL,
    EstadoRegistro CHAR(1) NOT NULL DEFAULT 'A',

    CONSTRAINT UQ_Calles_Nombre UNIQUE (Nombre)
    ) ENGINE = InnoDB;


-- =====================================================
-- TABLA: Vehiculos
-- =====================================================

CREATE TABLE IF NOT EXISTS Vehiculos (
    IdVehiculo INT AUTO_INCREMENT PRIMARY KEY,
    Placa VARCHAR(15) NOT NULL,
    TipoVehiculo VARCHAR(30) NOT NULL DEFAULT 'Automovil',
    FechaRegistro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    EstadoRegistro CHAR(1) NOT NULL DEFAULT 'A',

    CONSTRAINT UQ_Vehiculos_Placa UNIQUE (Placa)
    ) ENGINE = InnoDB;


-- =====================================================
-- TABLA: Simulaciones
-- =====================================================

CREATE TABLE IF NOT EXISTS Simulaciones (
    IdSimulacion INT AUTO_INCREMENT PRIMARY KEY,
    Nombre VARCHAR(100) NOT NULL,
    Descripcion VARCHAR(255) NULL,
    FechaCreacion DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FechaInicio DATETIME NULL,
    FechaFin DATETIME NULL,
    HoraSimuladaInicio TIME NOT NULL DEFAULT '06:00:00',
    DuracionSimuladaSegundos INT NOT NULL DEFAULT 3600,
    Estado VARCHAR(20) NOT NULL DEFAULT 'Configurada',
    TotalVehiculos INT NOT NULL DEFAULT 0,
    TotalVehiculosFinalizados INT NOT NULL DEFAULT 0,

    CONSTRAINT CHK_Simulaciones_Duracion
    CHECK (DuracionSimuladaSegundos > 0),

    CONSTRAINT CHK_Simulaciones_Estado
    CHECK (
              Estado IN (
              'Configurada',
              'Ejecutando',
              'Pausada',
              'Finalizada',
              'Cancelada'
                        )
    )
    ) ENGINE = InnoDB;


-- =====================================================
-- TABLA: SimulacionVehiculos
-- =====================================================

CREATE TABLE IF NOT EXISTS SimulacionVehiculos (
    IdSimulacionVehiculo INT AUTO_INCREMENT PRIMARY KEY,
    IdSimulacion INT NOT NULL,
    IdVehiculo INT NOT NULL,
    IdCalle INT NOT NULL,
    Velocidad DECIMAL(6,2) NOT NULL,
    HoraLlegada TIME NOT NULL,
    HoraInicioCruce TIME NULL,
    HoraSalida TIME NULL,

    SegundoLlegada INT NOT NULL,
    SegundoInicioCruce INT NULL,
    SegundoSalida INT NULL,

    TiempoEsperaSegundos INT NOT NULL DEFAULT 0,

    Estado VARCHAR(20) NOT NULL DEFAULT 'Programado',

    FechaRegistro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT FK_SimulacionVehiculos_Simulaciones
    FOREIGN KEY (IdSimulacion)
    REFERENCES Simulaciones(IdSimulacion),

    CONSTRAINT FK_SimulacionVehiculos_Vehiculos
    FOREIGN KEY (IdVehiculo)
    REFERENCES Vehiculos(IdVehiculo),

    CONSTRAINT FK_SimulacionVehiculos_Calles
    FOREIGN KEY (IdCalle)
    REFERENCES Calles(IdCalle),

    CONSTRAINT UQ_SimulacionVehiculos
    UNIQUE (IdSimulacion, IdVehiculo),

    CONSTRAINT CHK_SimulacionVehiculos_Velocidad
    CHECK (Velocidad >= 0),

    CONSTRAINT CHK_SimulacionVehiculos_Estado
    CHECK (
              Estado IN (
              'Programado',
              'EnCola',
              'Cruzando',
              'Finalizado'
                        )
    ),

    CONSTRAINT CHK_SimulacionVehiculos_Segundos
    CHECK (
              SegundoLlegada >= 0
              AND (SegundoInicioCruce IS NULL OR SegundoInicioCruce >= SegundoLlegada)
    AND (SegundoSalida IS NULL OR SegundoSalida >= SegundoInicioCruce)
    ),

    INDEX IX_SimulacionVehiculos_SimulacionEstado (
          IdSimulacion,
          Estado
    ),

    INDEX IX_SimulacionVehiculos_CalleLlegada (
          IdSimulacion,
          IdCalle,
          SegundoLlegada
          )
    ) ENGINE = InnoDB;


-- =====================================================
-- TABLA: Semaforos
-- =====================================================

CREATE TABLE IF NOT EXISTS Semaforos (
    IdSemaforo INT AUTO_INCREMENT PRIMARY KEY,
    IdCalle INT NOT NULL,
    GrupoSemaforico VARCHAR(10) NOT NULL,
    EstadoActual VARCHAR(10) NOT NULL DEFAULT 'Rojo',
    EstadoRegistro CHAR(1) NOT NULL DEFAULT 'A',

    CONSTRAINT FK_Semaforos_Calles
    FOREIGN KEY (IdCalle)
    REFERENCES Calles(IdCalle),

    CONSTRAINT UQ_Semaforos_Calle
    UNIQUE (IdCalle),

    CONSTRAINT CHK_Semaforos_Grupo
    CHECK (
        GrupoSemaforico IN ('NS', 'EO')
    ),

    CONSTRAINT CHK_Semaforos_Estado
    CHECK (
              EstadoActual IN (
              'Rojo',
              'Amarillo',
              'Verde'
              )
    )
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: ConfiguracionesSemaforo
-- =====================================================

CREATE TABLE IF NOT EXISTS ConfiguracionesSemaforo (
    IdConfiguracionSemaforo INT AUTO_INCREMENT PRIMARY KEY,
    IdSimulacion INT NOT NULL,
    VerdeMinimoSegundos INT NOT NULL DEFAULT 20,
    VerdeMaximoSegundos INT NOT NULL DEFAULT 60,
    AmarilloSegundos INT NOT NULL DEFAULT 3,
    TodoRojoSegundos INT NOT NULL DEFAULT 2,
    EsperaMaximaSegundos INT NOT NULL DEFAULT 120,
    ControlInteligente BOOLEAN NOT NULL DEFAULT TRUE,
    CONSTRAINT FK_ConfiguracionesSemaforo_Simulaciones
    FOREIGN KEY (IdSimulacion)
    REFERENCES Simulaciones(IdSimulacion),

    CONSTRAINT UQ_ConfiguracionesSemaforo_Simulacion
    UNIQUE (IdSimulacion),

    CONSTRAINT CHK_ConfiguracionesSemaforo_Tiempos
    CHECK (
              VerdeMinimoSegundos > 0
              AND VerdeMaximoSegundos >= VerdeMinimoSegundos
              AND AmarilloSegundos > 0
              AND TodoRojoSegundos > 0
              AND EsperaMaximaSegundos > 0
          )
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: CiclosSemaforo
-- =====================================================

CREATE TABLE IF NOT EXISTS CiclosSemaforo (
    IdCicloSemaforo INT AUTO_INCREMENT PRIMARY KEY,
    IdSimulacion INT NOT NULL,
    NumeroCiclo INT NOT NULL,
    GrupoSemaforico VARCHAR(10) NOT NULL,
    Estado VARCHAR(10) NOT NULL,
    SegundoInicio INT NOT NULL,
    SegundoFin INT NOT NULL,
    DuracionSegundos INT
    GENERATED ALWAYS AS (SegundoFin - SegundoInicio) STORED,

    VehiculosAtendidos INT NOT NULL DEFAULT 0,

    CONSTRAINT FK_CiclosSemaforo_Simulaciones
    FOREIGN KEY (IdSimulacion)
    REFERENCES Simulaciones(IdSimulacion),

    CONSTRAINT CHK_CiclosSemaforo_Grupo
    CHECK (
              GrupoSemaforico IN ('NS', 'EO', 'TODOS')
    ),

    CONSTRAINT CHK_CiclosSemaforo_Estado
    CHECK (
              Estado IN ('Verde', 'Amarillo', 'Rojo')
    ),

    CONSTRAINT CHK_CiclosSemaforo_Tiempos
    CHECK (
              SegundoInicio >= 0
              AND SegundoFin > SegundoInicio
              AND VehiculosAtendidos >= 0
          ),

    INDEX IX_CiclosSemaforo_Simulacion (
                                           IdSimulacion,
                                           NumeroCiclo
                                       )
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: Camaras
-- =====================================================

CREATE TABLE IF NOT EXISTS Camaras (
    IdCamara INT AUTO_INCREMENT PRIMARY KEY,
    IdCalle INT NOT NULL,
    Nombre VARCHAR(100) NOT NULL,
    Ubicacion VARCHAR(150) NOT NULL,
    VelocidadLimite DECIMAL(6,2) NOT NULL DEFAULT 60.00,
    EstadoRegistro CHAR(1) NOT NULL DEFAULT 'A',
    FechaInstalacion DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT FK_Camaras_Calles
    FOREIGN KEY (IdCalle)
    REFERENCES Calles(IdCalle),

    CONSTRAINT UQ_Camaras_Nombre
    UNIQUE (Nombre),

    CONSTRAINT CHK_Camaras_VelocidadLimite
    CHECK (VelocidadLimite > 0),

    CONSTRAINT CHK_Camaras_EstadoRegistro
    CHECK (EstadoRegistro IN ('A', 'I')),

    INDEX IX_Camaras_Calle (
          IdCalle
                           )
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: TarifasMulta
-- =====================================================

CREATE TABLE IF NOT EXISTS TarifasMulta (
    IdTarifaMulta INT AUTO_INCREMENT PRIMARY KEY,
    Descripcion VARCHAR(150) NOT NULL,
    VelocidadMinimaExclusiva DECIMAL(6,2) NOT NULL,
    VelocidadMaximaInclusiva DECIMAL(6,2) NULL,
    MontoMulta DECIMAL(10,2) NOT NULL,
    EstadoRegistro CHAR(1) NOT NULL DEFAULT 'A',
    FechaRegistro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT UQ_TarifasMulta_VelocidadMinima
    UNIQUE (VelocidadMinimaExclusiva),

    CONSTRAINT CHK_TarifasMulta_Rango
    CHECK (
              VelocidadMinimaExclusiva >= 0
              AND (
              VelocidadMaximaInclusiva IS NULL
              OR VelocidadMaximaInclusiva > VelocidadMinimaExclusiva
                  )
    ),

    CONSTRAINT CHK_TarifasMulta_Monto
    CHECK (MontoMulta >= 0),

    CONSTRAINT CHK_TarifasMulta_Estado
    CHECK (EstadoRegistro IN ('A', 'I'))
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: Multas
-- =====================================================

CREATE TABLE IF NOT EXISTS Multas (
    IdMulta INT AUTO_INCREMENT PRIMARY KEY,
    IdSimulacionVehiculo INT NOT NULL,
    IdCamara INT NOT NULL,
    IdTarifaMulta INT NOT NULL,
    VelocidadRegistrada DECIMAL(6,2) NOT NULL,
    VelocidadLimite DECIMAL(6,2) NOT NULL,
    MontoMulta DECIMAL(10,2) NOT NULL,
    SegundoDeteccion INT NOT NULL,
    FechaRegistro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    Estado VARCHAR(20) NOT NULL DEFAULT 'Generada',

    CONSTRAINT FK_Multas_SimulacionVehiculos
    FOREIGN KEY (IdSimulacionVehiculo)
    REFERENCES SimulacionVehiculos(IdSimulacionVehiculo),

    CONSTRAINT FK_Multas_Camaras
    FOREIGN KEY (IdCamara)
    REFERENCES Camaras(IdCamara),

    CONSTRAINT FK_Multas_TarifasMulta
    FOREIGN KEY (IdTarifaMulta)
    REFERENCES TarifasMulta(IdTarifaMulta),

    CONSTRAINT UQ_Multas_VehiculoCamara
    UNIQUE (IdSimulacionVehiculo, IdCamara),

    CONSTRAINT CHK_Multas_Velocidades
    CHECK (
              VelocidadRegistrada > VelocidadLimite
              AND VelocidadLimite > 0
          ),

    CONSTRAINT CHK_Multas_Monto
    CHECK (MontoMulta >= 0),

    CONSTRAINT CHK_Multas_Segundo
    CHECK (SegundoDeteccion >= 0),

    CONSTRAINT CHK_Multas_Estado
    CHECK (
              Estado IN (
              'Generada',
              'Anulada'
                        )
    ),

    INDEX IX_Multas_FechaRegistro (
              FechaRegistro
                                  )
    ) ENGINE = InnoDB;



-- =====================================================
-- TABLA: EstadisticasTrafico
-- =====================================================

CREATE TABLE IF NOT EXISTS EstadisticasTrafico (
    IdEstadisticaTrafico INT AUTO_INCREMENT PRIMARY KEY,
    IdSimulacion INT NOT NULL,
    IdCalle INT NOT NULL,
    SegundoInicioIntervalo INT NOT NULL,
    SegundoFinIntervalo INT NOT NULL,
    VehiculosLlegados INT NOT NULL DEFAULT 0,
    VehiculosFinalizados INT NOT NULL DEFAULT 0,
    VehiculosEnCola INT NOT NULL DEFAULT 0,
    TiempoEsperaTotalSegundos BIGINT NOT NULL DEFAULT 0,
    TiempoEsperaPromedioSegundos DECIMAL(10,2) NOT NULL DEFAULT 0,
    TiempoEsperaMaximoSegundos INT NOT NULL DEFAULT 0,

    LongitudMaximaCola INT NOT NULL DEFAULT 0,

    TotalMultas INT NOT NULL DEFAULT 0,
    MontoTotalMultas DECIMAL(12,2) NOT NULL DEFAULT 0,

    FechaRegistro DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT FK_EstadisticasTrafico_Simulaciones
    FOREIGN KEY (IdSimulacion)
    REFERENCES Simulaciones(IdSimulacion),

    CONSTRAINT FK_EstadisticasTrafico_Calles
    FOREIGN KEY (IdCalle)
    REFERENCES Calles(IdCalle),

    CONSTRAINT UQ_EstadisticasTrafico_Intervalo
    UNIQUE (
               IdSimulacion,
               IdCalle,
               SegundoInicioIntervalo
           ),

    CONSTRAINT CHK_EstadisticasTrafico_Intervalo
    CHECK (
              SegundoInicioIntervalo >= 0
              AND SegundoFinIntervalo > SegundoInicioIntervalo
          ),

    CONSTRAINT CHK_EstadisticasTrafico_Cantidades
    CHECK (
              VehiculosLlegados >= 0
              AND VehiculosFinalizados >= 0
              AND VehiculosEnCola >= 0
              AND TiempoEsperaTotalSegundos >= 0
              AND TiempoEsperaPromedioSegundos >= 0
              AND TiempoEsperaMaximoSegundos >= 0
              AND LongitudMaximaCola >= 0
              AND TotalMultas >= 0
              AND MontoTotalMultas >= 0
          )
    ) ENGINE = InnoDB;


