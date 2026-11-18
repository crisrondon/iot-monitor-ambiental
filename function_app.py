import sys
from pathlib import Path

# Fuerza a Azure Functions a buscar primero las dependencias
# instaladas localmente en .python_packages
packages_path = (
    Path(__file__).parent
    / ".python_packages"
    / "lib"
    / "site-packages"
)

sys.path.insert(0, str(packages_path))

import azure.functions as func
import logging
import json
import os
import uuid

from datetime import datetime, timezone
from azure.data.tables import TableServiceClient


app = func.FunctionApp(
    http_auth_level=func.AuthLevel.ANONYMOUS
)

TABLE_NAME = "LecturasAmbientales"


# ============================================================
# FUNCION 1
# RECIBIR DATOS DEL ESP32
# ============================================================

@app.route(
    route="recibir_datos_esp32",
    methods=["POST"]
)
def recibir_datos_esp32(
    req: func.HttpRequest
) -> func.HttpResponse:

    logging.info(
        "Solicitud recibida desde ESP32."
    )

    # --------------------------------------------------------
    # Leer JSON recibido
    # --------------------------------------------------------

    try:
        datos = req.get_json()

    except ValueError:

        return func.HttpResponse(
            json.dumps({
                "ok": False,
                "mensaje":
                    "El cuerpo de la solicitud no contiene JSON valido."
            }),
            status_code=400,
            mimetype="application/json"
        )

    # --------------------------------------------------------
    # Obtener datos enviados por el ESP32
    # --------------------------------------------------------

    dispositivo = datos.get(
        "dispositivo"
    )

    ubicacion = datos.get(
        "ubicacion",
        "Sin ubicacion"
    )

    temperatura = datos.get(
        "temperatura"
    )

    humedad = datos.get(
        "humedad"
    )

    estado = datos.get(
        "estado"
    )

    # --------------------------------------------------------
    # Validar datos obligatorios
    # --------------------------------------------------------

    if (
        dispositivo is None
        or temperatura is None
        or humedad is None
    ):

        return func.HttpResponse(
            json.dumps({
                "ok": False,
                "mensaje":
                    "Faltan datos obligatorios."
            }),
            status_code=400,
            mimetype="application/json"
        )

    # --------------------------------------------------------
    # Fecha y hora UTC
    # --------------------------------------------------------

    fecha_hora = datetime.now(
        timezone.utc
    )

    try:

        # ----------------------------------------------------
        # Obtener conexión de Azure Storage
        # ----------------------------------------------------

        connection_string = os.environ[
            "AzureWebJobsStorage"
        ]

        # ----------------------------------------------------
        # Crear cliente de Table Storage
        # ----------------------------------------------------

        table_service = (
            TableServiceClient
            .from_connection_string(
                connection_string
            )
        )

        # ----------------------------------------------------
        # Crear tabla si todavía no existe
        # ----------------------------------------------------

        table_service.create_table_if_not_exists(
            table_name=TABLE_NAME
        )

        # ----------------------------------------------------
        # Obtener cliente de la tabla
        # ----------------------------------------------------

        table_client = (
            table_service
            .get_table_client(
                table_name=TABLE_NAME
            )
        )

        # ----------------------------------------------------
        # Crear entidad
        # ----------------------------------------------------

        entidad = {
            "PartitionKey":
                dispositivo,

            "RowKey":
                str(uuid.uuid4()),

            "dispositivo":
                dispositivo,

            "ubicacion":
                ubicacion,

            "temperatura":
                float(temperatura),

            "humedad":
                float(humedad),

            "estado":
                estado,

            "fechaHora":
                fecha_hora.isoformat()
        }

        # ----------------------------------------------------
        # Guardar registro
        # ----------------------------------------------------

        table_client.create_entity(
            entity=entidad
        )

        logging.info(
            f"Lectura almacenada: "
            f"{dispositivo}, "
            f"{temperatura} C, "
            f"{humedad} %, "
            f"{estado}"
        )

    except Exception as e:

        logging.error(
            f"Error almacenando lectura: "
            f"{str(e)}"
        )

        return func.HttpResponse(
            json.dumps({
                "ok": False,
                "mensaje":
                    "Los datos fueron recibidos "
                    "pero no pudieron almacenarse.",
                "error":
                    str(e)
            }),
            status_code=500,
            mimetype="application/json"
        )

    # --------------------------------------------------------
    # Respuesta exitosa
    # --------------------------------------------------------

    respuesta = {
        "ok": True,

        "mensaje":
            "Datos recibidos y almacenados correctamente",

        "datos": {

            "dispositivo":
                dispositivo,

            "ubicacion":
                ubicacion,

            "temperatura":
                temperatura,

            "humedad":
                humedad,

            "estado":
                estado,

            "fechaHora":
                fecha_hora.isoformat()
        }
    }

    return func.HttpResponse(
        json.dumps(respuesta),
        status_code=200,
        mimetype="application/json"
    )


# ============================================================
# FUNCION 2
# CONSULTAR ULTIMAS LECTURAS
# ============================================================

@app.route(
    route="ultimas_lecturas",
    methods=["GET"]
)
def ultimas_lecturas(
    req: func.HttpRequest
) -> func.HttpResponse:

    logging.info(
        "Consulta de ultimas lecturas."
    )

    try:

        # ----------------------------------------------------
        # Cantidad de registros solicitados
        # ----------------------------------------------------

        limite_param = req.params.get(
            "limite"
        )

        try:

            limite = int(
                limite_param
            ) if limite_param else 20

        except ValueError:

            limite = 20

        # Evitar consultas demasiado grandes
        if limite < 1:
            limite = 1

        if limite > 100:
            limite = 100

        # ----------------------------------------------------
        # Filtro opcional por dispositivo
        # ----------------------------------------------------

        dispositivo = req.params.get(
            "dispositivo"
        )

        # ----------------------------------------------------
        # Obtener conexión
        # ----------------------------------------------------

        connection_string = os.environ[
            "AzureWebJobsStorage"
        ]

        table_service = (
            TableServiceClient
            .from_connection_string(
                connection_string
            )
        )

        table_client = (
            table_service
            .get_table_client(
                table_name=TABLE_NAME
            )
        )

        # ----------------------------------------------------
        # Consultar registros
        # ----------------------------------------------------

        if dispositivo:

            filtro = (
                f"PartitionKey eq "
                f"'{dispositivo}'"
            )

            entidades = list(
                table_client.query_entities(
                    query_filter=filtro
                )
            )

        else:

            entidades = list(
                table_client.list_entities()
            )

        # ----------------------------------------------------
        # Ordenar por fecha más reciente
        # ----------------------------------------------------

        entidades_ordenadas = sorted(
            entidades,
            key=lambda x:
                x.get(
                    "fechaHora",
                    ""
                ),
            reverse=True
        )

        # ----------------------------------------------------
        # Limitar cantidad
        # ----------------------------------------------------

        entidades_ordenadas = (
            entidades_ordenadas[:limite]
        )

        # ----------------------------------------------------
        # Preparar JSON
        # ----------------------------------------------------

        lecturas = []

        for entidad in entidades_ordenadas:

            lectura = {

                "dispositivo":
                    entidad.get(
                        "dispositivo"
                    ),

                "ubicacion":
                    entidad.get(
                        "ubicacion"
                    ),

                "temperatura":
                    entidad.get(
                        "temperatura"
                    ),

                "humedad":
                    entidad.get(
                        "humedad"
                    ),

                "estado":
                    entidad.get(
                        "estado"
                    ),

                "fechaHora":
                    entidad.get(
                        "fechaHora"
                    )
            }

            lecturas.append(
                lectura
            )

        # ----------------------------------------------------
        # Respuesta
        # ----------------------------------------------------

        respuesta = {

            "ok": True,

            "cantidad":
                len(lecturas),

            "lecturas":
                lecturas
        }

        return func.HttpResponse(
            json.dumps(
                respuesta
            ),
            status_code=200,
            mimetype="application/json",
            headers={
                "Access-Control-Allow-Origin": "*"
            }
        )

    except Exception as e:

        logging.error(
            f"Error consultando lecturas: "
            f"{str(e)}"
        )

        return func.HttpResponse(
            json.dumps({
                "ok": False,

                "mensaje":
                    "No fue posible consultar "
                    "las lecturas.",

                "error":
                    str(e)
            }),
            status_code=500,
            mimetype="application/json",
            headers={
                "Access-Control-Allow-Origin": "*"
            }
        )