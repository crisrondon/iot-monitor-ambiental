# Monitor Ambiental IoT con ESP32 y Azure

Proyecto académico de monitoreo ambiental desarrollado con un ESP32, un sensor DHT11, Azure Functions y Azure Table Storage.

## Objetivo

Diseñar e implementar una solución IoT capaz de medir temperatura y humedad, enviar los datos a la nube y almacenarlos para su consulta y análisis posterior.

## Tecnologías utilizadas

- ESP32
- Sensor DHT11
- Arduino IDE
- Python
- Visual Studio Code
- Azure Functions
- Azure Table Storage
- Azurite
- Git
- GitHub

## Arquitectura del proyecto

```text
ESP32 + DHT11
      |
      | HTTPS / JSON
      v
Azure Function
      |
      v
Azure Table Storage
      |
      v
LecturasAmbientales
## Código del ESP32

El código fuente correspondiente al ESP32 se encuentra en la carpeta:

`esp32/esp32_monitor_ambiental.ino`

Este código se encarga de:

- Conectarse a la red WiFi.
- Leer temperatura y humedad desde el sensor DHT11.
- Clasificar el estado de la temperatura.
- Mostrar información mediante un servidor web local.
- Enviar periódicamente los datos a Azure Functions mediante HTTPS y JSON.

## Evidencias de funcionamiento

Durante las pruebas se verificó correctamente:

- Lectura de temperatura y humedad desde el sensor DHT11.
- Conexión del ESP32 a la red WiFi.
- Envío de datos mediante HTTP POST.
- Recepción de información en Azure Functions.
- Almacenamiento de las lecturas en Azure Table Storage.
- Registro continuo de datos en la tabla `LecturasAmbientales`.

El flujo validado fue:

```text
DHT11
  ↓
ESP32
  ↓
HTTPS / JSON
  ↓
Azure Function
  ↓
Azure Table Storage
  ↓
LecturasAmbientales