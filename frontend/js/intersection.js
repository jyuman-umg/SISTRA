/* =====================================================
   SISTRA - INTERSECCION 2D
   FASE 8: REPRESENTACION GRAFICA CON CANVAS
   ===================================================== */

class IntersectionRenderer {

    constructor(canvasId, statusId) {
        this.canvas = document.getElementById(canvasId);
        this.status = document.getElementById(statusId);

        if (!this.canvas) {
            throw new Error("No se encontro el Canvas de SISTRA.");
        }

        this.ctx = this.canvas.getContext("2d");

        if (!this.ctx) {
            throw new Error("El navegador no soporta Canvas 2D.");
        }

        this.width = this.canvas.width;
        this.height = this.canvas.height;

        this.centerX = this.width / 2;
        this.centerY = this.height / 2;

        this.roadWidth = 200;
        this.halfRoad = this.roadWidth / 2;

        this.vehicles = [];
        this.trafficLights = [];
        this.currentSecond = 0;

        this.animationDuration = 650;
        this.animationStart = 0;
        this.animationFrame = null;

        this.previousPositions = new Map();
        this.targetPositions = new Map();

        this.colors = {
            grass: "#365b46",
            grassDetail: "#426b51",
            road: "#334155",
            roadEdge: "#94a3b8",
            white: "#f8fafc",
            yellow: "#facc15",
            red: "#ef4444",
            green: "#22c55e",
            blue: "#3b82f6",
            orange: "#f59e0b",
            gray: "#94a3b8",
            dark: "#0f172a"
        };

        this.reset();
    }

    /* =====================================================
       DATOS
       ===================================================== */

    reset() {
        this.vehicles = [];
        this.trafficLights = [];
        this.currentSecond = 0;

        this.previousPositions.clear();
        this.targetPositions.clear();

        if (this.animationFrame !== null) {
            cancelAnimationFrame(this.animationFrame);
            this.animationFrame = null;
        }

        this.setStatus("Esperando simulacion", "");
        this.draw();
    }

    update(data, vehicles = []) {
        const previous = this.getCurrentPositions();

        this.currentSecond = Number(
            data?.segundoActual ?? this.currentSecond
        );

        this.vehicles = Array.isArray(vehicles)
            ? vehicles
            : [];

        this.trafficLights = Array.isArray(data?.semaforos?.calles)
            ? data.semaforos.calles
            : [];

        this.previousPositions = previous;
        this.targetPositions = this.calculatePositions();

        if (this.animationFrame !== null) {
            cancelAnimationFrame(this.animationFrame);
        }

        this.animationStart = performance.now();

        if (data?.estadoSimulacion === "Finalizada") {
            this.setStatus("Simulacion finalizada", "finished");
        } else {
            this.setStatus("Simulacion activa", "active");
        }

        this.animate(this.animationStart);
    }

    setStatus(text, state) {
        if (this.status) {
            this.status.textContent = text;

            const container = this.status.closest(
                ".intersection-status"
            );

            if (container) {
                container.className = "intersection-status";

                if (state) {
                    container.classList.add(state);
                }
            }
        }
    }

    /* =====================================================
       POSICIONES DE VEHICULOS
       ===================================================== */

    getVehicleKey(vehicle, index) {
        return String(
            vehicle.idSimulacionVehiculo ??
            vehicle.idVehiculo ??
            vehicle.placa ??
            index
        );
    }

    getStreetId(vehicle) {
        const numericId = Number(vehicle.idCalle);

        if (numericId >= 1 && numericId <= 4) {
            return numericId;
        }

        const name = String(
            vehicle.calle ??
            vehicle.nombreCalle ??
            ""
        ).toLowerCase();

        if (name.includes("norte")) return 1;
        if (name.includes("sur")) return 2;
        if (name.includes("este")) return 3;
        if (name.includes("oeste")) return 4;

        return 0;
    }

    getVehicleState(vehicle) {
        return String(
            vehicle.estado ?? "Programado"
        );
    }

    getCurrentPositions() {
        const positions = new Map();

        const progress = this.animationFrame !== null
            ? Math.min(
                (performance.now() - this.animationStart) /
                this.animationDuration,
                1
            )
            : 1;

        for (const [key, target] of this.targetPositions) {
            const origin = this.previousPositions.get(key) ?? target;

            positions.set(key, {
                x: origin.x + (target.x - origin.x) * progress,
                y: origin.y + (target.y - origin.y) * progress,
                angle: target.angle
            });
        }

        return positions;
    }

    calculatePositions() {
        const positions = new Map();

        const queues = {
            1: [],
            2: [],
            3: [],
            4: []
        };

        for (let i = 0; i < this.vehicles.length; i++) {
            const vehicle = this.vehicles[i];
            const street = this.getStreetId(vehicle);

            if (street === 0) {
                continue;
            }

            if (this.getVehicleState(vehicle) === "EnCola") {
                queues[street].push({
                    vehicle,
                    index: i
                });
            }
        }

        for (const street of [1, 2, 3, 4]) {
            queues[street].sort((a, b) => {
                const arrivalA = Number(
                    a.vehicle.segundoLlegada ?? 0
                );

                const arrivalB = Number(
                    b.vehicle.segundoLlegada ?? 0
                );

                if (arrivalA !== arrivalB) {
                    return arrivalA - arrivalB;
                }

                return Number(
                    a.vehicle.idSimulacionVehiculo ?? a.index
                ) - Number(
                    b.vehicle.idSimulacionVehiculo ?? b.index
                );
            });
        }

        const queueIndices = new Map();

        for (const street of [1, 2, 3, 4]) {
            queues[street].forEach((entry, index) => {
                queueIndices.set(
                    this.getVehicleKey(
                        entry.vehicle,
                        entry.index
                    ),
                    index
                );
            });
        }

        for (let i = 0; i < this.vehicles.length; i++) {
            const vehicle = this.vehicles[i];
            const street = this.getStreetId(vehicle);

            if (street === 0) {
                continue;
            }

            const key = this.getVehicleKey(vehicle, i);
            const state = this.getVehicleState(vehicle);

            const queueIndex = queueIndices.get(key) ?? 0;

            const position = this.positionForVehicle(
                street,
                state,
                queueIndex,
                vehicle
            );

            if (position) {
                positions.set(key, position);
            }
        }

        return positions;
    }

    positionForVehicle(street, state, queueIndex, vehicle) {
        const cx = this.centerX;
        const cy = this.centerY;
        const half = this.halfRoad;

        // Cada calle representa el ORIGEN del vehiculo.
        // Norte circula hacia el sur, Sur hacia el norte,
        // Este hacia el oeste y Oeste hacia el este.
        const stopDistance = half + 34;
        const spacing = 48;
        const directions = {
            1: { x: cx + 48, y: cy - stopDistance, dx: 0, dy: 1, angle: Math.PI },
            2: { x: cx - 48, y: cy + stopDistance, dx: 0, dy: -1, angle: 0 },
            3: { x: cx + stopDistance, y: cy + 48, dx: -1, dy: 0, angle: -Math.PI / 2 },
            4: { x: cx - stopDistance, y: cy - 48, dx: 1, dy: 0, angle: Math.PI / 2 }
        };

        const direction = directions[street];
        if (!direction || state === "Finalizado") {
            return null;
        }

        const position = {
            x: direction.x,
            y: direction.y,
            angle: direction.angle
        };

        if (state === "Programado") {
            // Aun no llega: fuera de la zona de espera.
            position.x -= direction.dx * 230;
            position.y -= direction.dy * 230;
        } else if (state === "EnCola") {
            // La fila se forma detras de la linea de detencion.
            position.x -= direction.dx * queueIndex * spacing;
            position.y -= direction.dy * queueIndex * spacing;
        } else if (state === "Cruzando") {
            const inicio = Number(vehicle.segundoInicioCruce);
            const elapsed = Number.isFinite(inicio) && inicio >= 0
                ? Math.max(0, this.currentSecond - inicio)
                : 0;

            // Recorrido visual aproximado; el motor determina
            // el momento real de finalizar el cruce.
            const progress = Math.min(elapsed / 5, 1);
            const distance = stopDistance + half * 1.8;

            position.x += direction.dx * distance * progress;
            position.y += direction.dy * distance * progress;
        }

        return position;
    }

    /* =====================================================
       ANIMACION
       ===================================================== */

    animate(timestamp) {
        const progress = Math.min(
            (timestamp - this.animationStart) /
            this.animationDuration,
            1
        );

        this.draw(progress);

        if (progress < 1) {
            this.animationFrame = requestAnimationFrame(
                time => this.animate(time)
            );
        } else {
            this.animationFrame = null;
        }
    }

    /* =====================================================
       DIBUJO PRINCIPAL
       ===================================================== */

    draw(progress = 1) {
        const ctx = this.ctx;

        ctx.clearRect(0, 0, this.width, this.height);

        this.drawGround();
        this.drawRoads();
        this.drawLaneMarkings();
        this.drawCrosswalks();
        this.drawStreetLabels();
        this.drawTrafficLights();
        this.drawVehicles(progress);
        this.drawTimeIndicator();
    }

    drawGround() {
        const ctx = this.ctx;

        ctx.fillStyle = this.colors.grass;
        ctx.fillRect(0, 0, this.width, this.height);

        ctx.fillStyle = this.colors.grassDetail;

        const areas = [
            [20, 20, 260, 180],
            [620, 20, 260, 180],
            [20, 450, 260, 180],
            [620, 450, 260, 180]
        ];

        for (const [x, y, w, h] of areas) {
            ctx.fillRect(x, y, w, h);
        }
    }

    drawRoads() {
        const ctx = this.ctx;

        ctx.fillStyle = this.colors.road;

        ctx.fillRect(
            this.centerX - this.halfRoad,
            0,
            this.roadWidth,
            this.height
        );

        ctx.fillRect(
            0,
            this.centerY - this.halfRoad,
            this.width,
            this.roadWidth
        );

        ctx.strokeStyle = this.colors.roadEdge;
        ctx.lineWidth = 3;

        ctx.beginPath();

        ctx.moveTo(this.centerX - this.halfRoad, 0);
        ctx.lineTo(this.centerX - this.halfRoad, this.centerY - this.halfRoad);

        ctx.moveTo(this.centerX + this.halfRoad, 0);
        ctx.lineTo(this.centerX + this.halfRoad, this.centerY - this.halfRoad);

        ctx.moveTo(this.centerX - this.halfRoad, this.centerY + this.halfRoad);
        ctx.lineTo(this.centerX - this.halfRoad, this.height);

        ctx.moveTo(this.centerX + this.halfRoad, this.centerY + this.halfRoad);
        ctx.lineTo(this.centerX + this.halfRoad, this.height);

        ctx.moveTo(0, this.centerY - this.halfRoad);
        ctx.lineTo(this.centerX - this.halfRoad, this.centerY - this.halfRoad);

        ctx.moveTo(0, this.centerY + this.halfRoad);
        ctx.lineTo(this.centerX - this.halfRoad, this.centerY + this.halfRoad);

        ctx.moveTo(this.centerX + this.halfRoad, this.centerY - this.halfRoad);
        ctx.lineTo(this.width, this.centerY - this.halfRoad);

        ctx.moveTo(this.centerX + this.halfRoad, this.centerY + this.halfRoad);
        ctx.lineTo(this.width, this.centerY + this.halfRoad);

        ctx.stroke();
    }

    drawLaneMarkings() {
        const ctx = this.ctx;

        ctx.strokeStyle = this.colors.yellow;
        ctx.lineWidth = 3;
        ctx.setLineDash([18, 14]);

        ctx.beginPath();

        ctx.moveTo(this.centerX, 0);
        ctx.lineTo(this.centerX, this.centerY - this.halfRoad);

        ctx.moveTo(
            this.centerX,
            this.centerY + this.halfRoad
        );
        ctx.lineTo(this.centerX, this.height);

        ctx.moveTo(0, this.centerY);
        ctx.lineTo(this.centerX - this.halfRoad, this.centerY);

        ctx.moveTo(
            this.centerX + this.halfRoad,
            this.centerY
        );
        ctx.lineTo(this.width, this.centerY);

        ctx.stroke();
        ctx.setLineDash([]);
    }

    drawCrosswalks() {
        const ctx = this.ctx;

        ctx.fillStyle = this.colors.white;

        const stripeWidth = 12;
        const gap = 10;

        for (let i = 0; i < 8; i++) {
            const offset = i * (stripeWidth + gap);

            ctx.fillRect(
                this.centerX - 85 + offset,
                this.centerY - this.halfRoad - 22,
                stripeWidth,
                14
            );

            ctx.fillRect(
                this.centerX - 85 + offset,
                this.centerY + this.halfRoad + 8,
                stripeWidth,
                14
            );

            ctx.fillRect(
                this.centerX - this.halfRoad - 22,
                this.centerY - 85 + offset,
                14,
                stripeWidth
            );

            ctx.fillRect(
                this.centerX + this.halfRoad + 8,
                this.centerY - 85 + offset,
                14,
                stripeWidth
            );
        }
    }

    drawStreetLabels() {
        const ctx = this.ctx;

        ctx.fillStyle = this.colors.white;
        ctx.font = "bold 18px Segoe UI";
        ctx.textAlign = "center";

        ctx.fillText("NORTE", this.centerX, 28);
        ctx.fillText("SUR", this.centerX, this.height - 12);

        ctx.fillText("OESTE", 70, this.centerY - 112);
        ctx.fillText("ESTE", this.width - 70, this.centerY - 112);
    }

    /* =====================================================
       SEMAFOROS
       ===================================================== */

    getTrafficLightState(street) {
        const light = this.trafficLights.find(
            item => Number(item.idCalle) === street
        );

        return light?.estado ?? "Rojo";
    }

    drawTrafficLights() {
        // Cada semaforo corresponde al carril de ENTRADA de su calle.
        // Las posiciones coinciden con las definidas en positionForVehicle().
        const locations = [
            { street: 1, name: "NORTE", x: this.centerX + 125, y: this.centerY - 125 },
            { street: 2, name: "SUR", x: this.centerX - 125, y: this.centerY + 125 },
            { street: 3, name: "ESTE", x: this.centerX + 125, y: this.centerY + 125 },
            { street: 4, name: "OESTE", x: this.centerX - 125, y: this.centerY - 125 }
        ];

        for (const location of locations) {
            this.drawTrafficLight(
                location.x,
                location.y,
                this.getTrafficLightState(location.street),
                location.name
            );
        }
    }

    drawTrafficLight(x, y, state, streetName) {
        const ctx = this.ctx;

        const colors = [
            { name: "Rojo", color: this.colors.red },
            { name: "Amarillo", color: this.colors.yellow },
            { name: "Verde", color: this.colors.green }
        ];

        ctx.fillStyle = this.colors.dark;
        ctx.beginPath();
        ctx.roundRect(x - 12, y - 35, 24, 70, 6);
        ctx.fill();

        // Identifica claramente la calle que controla el semaforo.
        ctx.save();
        ctx.font = "bold 12px Segoe UI";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const labelWidth = Math.max(62, ctx.measureText(streetName).width + 16);
        ctx.fillStyle = "rgba(15, 23, 42, 0.92)";
        ctx.beginPath();
        ctx.roundRect(x - labelWidth / 2, y - 63, labelWidth, 21, 5);
        ctx.fill();
        ctx.fillStyle = this.colors.white;
        ctx.fillText(streetName, x, y - 52);
        ctx.restore();

        colors.forEach((light, index) => {
            const cy = y - 23 + index * 23;

            ctx.beginPath();
            ctx.arc(x, cy, 8, 0, Math.PI * 2);

            ctx.fillStyle = state === light.name
                ? light.color
                : "#475569";

            ctx.fill();
        });
    }

    /* =====================================================
       VEHICULOS
       ===================================================== */

    drawVehicles(progress) {
        for (let i = 0; i < this.vehicles.length; i++) {
            const vehicle = this.vehicles[i];
            const key = this.getVehicleKey(vehicle, i);

            const target = this.targetPositions.get(key);

            if (!target) {
                continue;
            }

            const origin =
                this.previousPositions.get(key) ?? target;

            const position = {
                x: origin.x + (target.x - origin.x) * progress,
                y: origin.y + (target.y - origin.y) * progress,
                angle: target.angle
            };

            this.drawVehicle(
                position,
                vehicle
            );
        }
    }

    drawVehicle(position, vehicle) {
        const ctx = this.ctx;
        const state = this.getVehicleState(vehicle);

        const colors = {
            Programado: this.colors.gray,
            EnCola: this.colors.orange,
            Cruzando: this.colors.blue,
            Finalizado: this.colors.green
        };

        ctx.save();

        ctx.translate(position.x, position.y);
        ctx.rotate(position.angle);

        ctx.fillStyle = colors[state] ?? this.colors.gray;

        ctx.beginPath();
        ctx.roundRect(-15, -24, 30, 48, 6);
        ctx.fill();

        ctx.fillStyle = "#0f172a";
        ctx.fillRect(-10, -12, 20, 12);

        ctx.fillStyle = "#dbeafe";
        ctx.fillRect(-10, 6, 20, 8);

        ctx.restore();

        ctx.fillStyle = this.colors.white;
        ctx.font = "11px Segoe UI";
        ctx.textAlign = "center";

        ctx.fillText(
            String(vehicle.placa ?? ""),
            position.x,
            position.y - 32
        );
    }

    /* =====================================================
       INDICADOR DE TIEMPO
       ===================================================== */

    drawTimeIndicator() {
        const ctx = this.ctx;

        ctx.fillStyle = "rgba(15, 23, 42, 0.85)";
        ctx.beginPath();
        ctx.roundRect(12, 12, 145, 40, 8);
        ctx.fill();

        ctx.fillStyle = this.colors.white;
        ctx.font = "bold 15px Segoe UI";
        ctx.textAlign = "left";

        ctx.fillText(
            `Tiempo: ${this.currentSecond} s`,
            25,
            38
        );
    }
}

/* =====================================================
   RENDERIZADOR
   ===================================================== */

window.sistraIntersection = new IntersectionRenderer(
    "intersectionCanvas",
    "intersectionStatus"
);