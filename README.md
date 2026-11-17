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