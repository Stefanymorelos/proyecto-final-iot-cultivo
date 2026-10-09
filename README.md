# Sistema de monitoreo de cultivo con IoT

Proyecto final del curso de IoT. Sonda con sensores de temperatura/humedad
ambiente (HDC1080), luz ambiental (AP3216), humedad de suelo (capacitivo)
y posicion GPS, montada en un ESP32 TTGO T-Beam. Los datos se envian por
WiFi a una arquitectura FIWARE (Orion Context Broker + QuantumLeap +
CrateDB) desplegada en AWS EC2, que almacena el historico y permite
monitoreo en tiempo real.

## Autora

Stefany Morelos ([@Stefanymorelos](https://github.com/Stefanymorelos))

## Estado del proyecto

En construccion — ver tablero de progreso mas abajo.

## Arquitectura

[Sensores: HDC1080, AP3216, humedad suelo, GPS]
            |
            v
     [ESP32 TTGO T-Beam] --WiFi (UPBWiFi)--> [Flask "proyectoapp" :3000]
                                                       |
                                          (traduce JSON plano -> NGSI-v2)
                                                       v
                                            [Orion Context Broker :1026]
                                                   |          |
                                               [MongoDB]   [suscripcion]
                                                               |
                                                               v
                                                [QuantumLeap :8668]
                                                               |
                                                               v
                                                   [CrateDB :4200] (historico)

Todo el backend corre con Docker Compose sobre una instancia AWS EC2.
Ver la carpeta servidor/ para la configuracion completa.

## Estructura del repositorio

.
|-- src/                    (Firmware del ESP32 - PlatformIO)
|   `-- main.cpp
|-- servidor/               (Backend desplegado en AWS EC2)
|   |-- docker-compose.yml  (Orion + Mongo + CrateDB + QuantumLeap + relay Flask)
|   `-- proyectoapp/        (Servicio Flask: traduce JSON del ESP32 a NGSI-v2)
|       |-- app.py
|       `-- Dockerfile
|-- docs/
|   |-- hardware/           (Diseno fisico: fotos, medidas, archivos .scad/.stl)
|   `-- arquitectura/       (Diagramas del sistema completo)
|-- platformio.ini          (Configuracion del proyecto PlatformIO)
`-- README.md

## Componentes

| Componente | Funcion |
|---|---|
| ESP32 TTGO T-Beam | Microcontrolador, lee sensores y envia datos por WiFi |
| HDC1080 (I2C) | Temperatura y humedad del aire |
| AP3216 (I2C) | Luz ambiental |
| Sensor de humedad de suelo (capacitivo) | Mide humedad del sustrato |
| Modulo GPS | Ubicacion de la sonda |
| Flask "proyectoapp" (Docker) | Traduce el JSON plano del ESP32 a formato NGSI-v2 |
| Orion Context Broker (FIWARE) | Almacena el estado actual de la entidad "Planta" |
| MongoDB | Base de datos interna de Orion |
| QuantumLeap (FIWARE) | Escucha cambios en Orion y los persiste como historico |
| CrateDB | Base de datos de series de tiempo para el historico |

## Como correr el firmware

1. Instala PlatformIO (como extension de VS Code, o CLI).
2. Clona este repo.
3. Ajusta en src/main.cpp el SSID de WiFi (SSID_WIFI) y la URL del
   servidor (URL_SERVIDOR, debe apuntar a http://<IP-DEL-EC2>:3000/api/plantas).
4. Conecta el ESP32 por USB.
5. Compila y sube: pio run --target upload
6. Monitor serial: pio device monitor

## Como correr el servidor (backend)

1. En una instancia EC2 (Amazon Linux 2023) con Docker y Docker Compose instalados.
2. Copia la carpeta servidor/ a la instancia.
3. Dentro de servidor/: docker compose up -d --build
4. Crea la suscripcion de Orion a QuantumLeap (una sola vez):
   curl -X POST http://localhost:1026/v2/subscriptions -H "Content-Type: application/json" -H "Fiware-Service: openiot" -H "Fiware-ServicePath: /" -d '{"description":"Notify QuantumLeap","subject":{"entities":[{"idPattern":".*","type":"PlantMonitor"}]},"notification":{"http":{"url":"http://quantumleap:8668/v2/notify"},"attrsFormat":"normalized"}}'

## Roadmap

[x] Definicion de alcance y arquitectura general
[x] Confirmacion de sensores y cultivo con el profesor
[x] Firmware: lectura de sensores (HDC1080, AP3216, humedad suelo, GPS) + envio WiFi
[x] Backend: stack FIWARE (Orion + Mongo + QuantumLeap + CrateDB) en AWS EC2
[x] Servicio Flask traductor JSON -> NGSI-v2
[x] Prueba end-to-end: dato simulado llega hasta CrateDB via QuantumLeap
[ ] Medidas de los sensores + diseno del case en 3D (OpenSCAD/Tinkercad)
[ ] Impresion 3D y montaje fisico
[ ] Carga del firmware al ESP32 real y pruebas con datos reales
[ ] Dashboard de visualizacion
[ ] Sistema de alertas (umbrales + notificacion)
[ ] Siembra y pruebas con la planta real
[ ] Documentacion final y sustentacion

## Licencia

Proyecto academico - Universidad Pontificia Bolivariana, curso de IoT.