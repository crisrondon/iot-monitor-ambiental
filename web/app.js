const API_URL =
    "https://func-iot-monitor-ambiental.azurewebsites.net/api/ultimas_lecturas?limite=20";

let graficaTemperatura = null;
let graficaHumedad = null;


// ============================================================
// CONSULTAR API
// ============================================================

async function cargarDatos() {

    const estadoConexion =
        document.getElementById("estadoConexion");

    try {

        estadoConexion.textContent =
            "Conectando...";

        estadoConexion.className =
            "conexion";


        const respuesta =
            await fetch(API_URL);


        if (!respuesta.ok) {
            throw new Error(
                `Error HTTP: ${respuesta.status}`
            );
        }


        const datos =
            await respuesta.json();


        if (
            !datos.ok ||
            !datos.lecturas ||
            datos.lecturas.length === 0
        ) {
            throw new Error(
                "No hay lecturas disponibles."
            );
        }


        estadoConexion.textContent =
            "Conectado";

        estadoConexion.className =
            "conexion conexion-ok";


        actualizarDashboard(
            datos.lecturas
        );

    }
    catch (error) {

        console.error(
            "Error consultando la API:",
            error
        );

        estadoConexion.textContent =
            "Sin conexión";

        estadoConexion.className =
            "conexion conexion-error";
    }
}


// ============================================================
// ACTUALIZAR DASHBOARD
// ============================================================

function actualizarDashboard(lecturas) {

    const lecturaActual =
        lecturas[0];


    document.getElementById(
        "temperaturaActual"
    ).textContent =
        Number(
            lecturaActual.temperatura
        ).toFixed(1);


    document.getElementById(
        "humedadActual"
    ).textContent =
        Number(
            lecturaActual.humedad
        ).toFixed(1);


    document.getElementById(
        "dispositivoActual"
    ).textContent =
        lecturaActual.dispositivo || "--";


    document.getElementById(
        "ubicacionActual"
    ).textContent =
        lecturaActual.ubicacion || "--";


    document.getElementById(
        "fechaActualizacion"
    ).textContent =
        formatearFecha(
            lecturaActual.fechaHora
        );


    actualizarEstado(
        lecturaActual.estado
    );


    actualizarTabla(
        lecturas
    );


    actualizarGraficas(
        lecturas
    );
}


// ============================================================
// ACTUALIZAR ESTADO
// ============================================================

function actualizarEstado(estado) {

    const elemento =
        document.getElementById(
            "estadoActual"
        );


    const valor =
        estado
            ? estado.toUpperCase()
            : "SIN DATOS";


    elemento.textContent =
        valor;


    elemento.className =
        "estado";


    if (valor === "NORMAL") {

        elemento.classList.add(
            "estado-normal"
        );

    }
    else if (
        valor === "ADVERTENCIA"
    ) {

        elemento.classList.add(
            "estado-advertencia"
        );

    }
    else {

        elemento.classList.add(
            "estado-critico"
        );
    }
}


// ============================================================
// ACTUALIZAR TABLA
// ============================================================

function actualizarTabla(lecturas) {

    const tbody =
        document.getElementById(
            "tablaLecturas"
        );


    tbody.innerHTML = "";


    lecturas.forEach(
        lectura => {

            const fila =
                document.createElement(
                    "tr"
                );


            fila.innerHTML = `
                <td>
                    ${formatearFecha(
                        lectura.fechaHora
                    )}
                </td>

                <td>
                    ${lectura.dispositivo ?? "--"}
                </td>

                <td>
                    ${lectura.ubicacion ?? "--"}
                </td>

                <td>
                    ${Number(
                        lectura.temperatura
                    ).toFixed(1)} °C
                </td>

                <td>
                    ${Number(
                        lectura.humedad
                    ).toFixed(1)} %
                </td>

                <td>
                    ${lectura.estado ?? "--"}
                </td>
            `;


            tbody.appendChild(
                fila
            );
        }
    );
}


// ============================================================
// ACTUALIZAR GRAFICAS
// ============================================================

function actualizarGraficas(lecturas) {

    // La API entrega primero las más recientes.
    // Para la gráfica queremos orden cronológico.
    const datosGrafica =
        [...lecturas].reverse();


    const etiquetas =
        datosGrafica.map(
            lectura =>
                formatearHora(
                    lectura.fechaHora
                )
        );


    const temperaturas =
        datosGrafica.map(
            lectura =>
                Number(
                    lectura.temperatura
                )
        );


    const humedades =
        datosGrafica.map(
            lectura =>
                Number(
                    lectura.humedad
                )
        );


    if (graficaTemperatura) {
        graficaTemperatura.destroy();
    }


    if (graficaHumedad) {
        graficaHumedad.destroy();
    }


    const contextoTemperatura =
        document.getElementById(
            "graficaTemperatura"
        );


    graficaTemperatura =
        new Chart(
            contextoTemperatura,
            {
                type: "line",

                data: {
                    labels:
                        etiquetas,

                    datasets: [
                        {
                            label:
                                "Temperatura °C",

                            data:
                                temperaturas,

                            tension:
                                0.3,

                            fill:
                                false
                        }
                    ]
                },

                options: {
                    responsive:
                        true,

                    maintainAspectRatio:
                        true,

                    scales: {
                        y: {
                            beginAtZero:
                                false
                        }
                    }
                }
            }
        );


    const contextoHumedad =
        document.getElementById(
            "graficaHumedad"
        );


    graficaHumedad =
        new Chart(
            contextoHumedad,
            {
                type: "line",

                data: {
                    labels:
                        etiquetas,

                    datasets: [
                        {
                            label:
                                "Humedad %",

                            data:
                                humedades,

                            tension:
                                0.3,

                            fill:
                                false
                        }
                    ]
                },

                options: {
                    responsive:
                        true,

                    maintainAspectRatio:
                        true,

                    scales: {
                        y: {
                            beginAtZero:
                                false
                        }
                    }
                }
            }
        );
}


// ============================================================
// FORMATEAR FECHA
// ============================================================

function formatearFecha(fecha) {

    if (!fecha) {
        return "--";
    }


    const fechaObjeto =
        new Date(fecha);


    return fechaObjeto
        .toLocaleString(
            "es-CO",
            {
                dateStyle:
                    "short",

                timeStyle:
                    "medium"
            }
        );
}


// ============================================================
// FORMATEAR HORA
// ============================================================

function formatearHora(fecha) {

    if (!fecha) {
        return "--";
    }


    const fechaObjeto =
        new Date(fecha);


    return fechaObjeto
        .toLocaleTimeString(
            "es-CO",
            {
                hour:
                    "2-digit",

                minute:
                    "2-digit",

                second:
                    "2-digit"
            }
        );
}


// ============================================================
// BOTON ACTUALIZAR
// ============================================================

document.getElementById(
    "btnActualizar"
).addEventListener(
    "click",
    cargarDatos
);


// ============================================================
// CARGA INICIAL
// ============================================================

cargarDatos();


// ============================================================
// ACTUALIZACION AUTOMATICA
// ============================================================

// Actualizar el dashboard cada 10 segundos.

setInterval(
    cargarDatos,
    10000
);