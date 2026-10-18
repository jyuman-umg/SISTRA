
-- =====================================================
-- DATOS INICIALES - SISTRA
-- =====================================================

USE sistra_db;

-- =====================================================
-- CALLES DE LA INTERSECCION
-- =====================================================

INSERT INTO Calles (
    Nombre,
    DireccionEntrada,
    DireccionSalida
)
VALUES
    ('Norte', 'Norte', 'Sur'),
    ('Sur', 'Sur', 'Norte'),
    ('Este', 'Este', 'Oeste'),
    ('Oeste', 'Oeste', 'Este')
    ON DUPLICATE KEY UPDATE
                         DireccionEntrada = VALUES(DireccionEntrada),
                         DireccionSalida = VALUES(DireccionSalida);



-- =====================================================
-- SEMAFOROS INICIALES
-- =====================================================

INSERT INTO Semaforos (
    IdCalle,
    GrupoSemaforico,
    EstadoActual
)
SELECT
    IdCalle,
    CASE
        WHEN Nombre IN ('Norte', 'Sur') THEN 'NS'
        WHEN Nombre IN ('Este', 'Oeste') THEN 'EO'
        END,
    'Rojo'
FROM Calles
WHERE Nombre IN (
                 'Norte',
                 'Sur',
                 'Este',
                 'Oeste'
    )
    ON DUPLICATE KEY UPDATE
                 GrupoSemaforico = VALUES(GrupoSemaforico);



-- =====================================================
-- CAMARAS DE SEGURIDAD INICIALES
-- =====================================================

INSERT INTO Camaras (
    IdCalle,
    Nombre,
    Ubicacion,
    VelocidadLimite
)
SELECT
    IdCalle,
    CONCAT('Camara ', Nombre),
    CONCAT('Entrada ', Nombre),
    60.00
FROM Calles
WHERE Nombre IN (
                 'Norte',
                 'Sur',
                 'Este',
                 'Oeste'
    )
    ON DUPLICATE KEY UPDATE
                         IdCalle = VALUES(IdCalle),
                         Ubicacion = VALUES(Ubicacion),
                         VelocidadLimite = VALUES(VelocidadLimite);



-- =====================================================
-- TARIFAS DE MULTAS
-- =====================================================

INSERT INTO TarifasMulta (
    Descripcion,
    VelocidadMinimaExclusiva,
    VelocidadMaximaInclusiva,
    MontoMulta
)
VALUES
    (
        'Exceso de velocidad mayor a 60 hasta 80 km/h',
        60.00,
        80.00,
        200.00
    ),
    (
        'Exceso de velocidad mayor a 80 hasta 90 km/h',
        80.00,
        90.00,
        400.00
    ),
    (
        'Exceso de velocidad mayor a 90 km/h',
        90.00,
        NULL,
        600.00
    )
    ON DUPLICATE KEY UPDATE
                         Descripcion = VALUES(Descripcion),
                         VelocidadMaximaInclusiva = VALUES(VelocidadMaximaInclusiva),
                         MontoMulta = VALUES(MontoMulta);
