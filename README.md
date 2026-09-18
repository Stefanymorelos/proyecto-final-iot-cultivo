# Sistema de monitoreo de cultivo con IoT

Proyecto final del curso de IoT. Sonda con sensores de humedad de suelo,
temperatura/humedad ambiente y luz, montada en un case impreso en 3D,
que envía datos por WiFi a un backend en la nube. El sistema visualiza
las lecturas en tiempo real y genera alertas automáticas cuando algún
valor sale de rango saludable para el cultivo.

## Autora

Stefany Morelos ([@Stefanymorelos](https://github.com/Stefanymorelos))

## Estado del proyecto

🚧 En construcción — ver [tablero de progreso](#roadmap) más abajo.

## Arquitectura

```
[Sensores] -> [ESP32] --WiFi--> [Backend Flask / AWS EC2] -> [Base de datos]
                                                                     |
                                                                     v
                                                          [Grafana: dashboard + alertas]
                                                                     |
                                                                     v
                                                    [Notificación: Telegram / email]
```

Ver diagrama detallado en [`docs/arquitectura/`](docs/arquitectura/).

## Estructura del repositorio

```
.
├── src/                  # Firmware del ESP32 (PlatformIO)
│   └── main.cpp
├── include/
│   ├── config.h.example  # Plantilla de configuración (WiFi, pines, umbrales)
│   └── config.h          # Tu configuración real (NO se sube al repo)
├── docs/
│   ├── hardware/         # Diseño físico: fotos, medidas, archivos .scad/.stl
│   ├── firmware/         # Documentación del código del ESP32
│   ├── backend/          # ETL, base de datos, servidor
│   └── arquitectura/     # Diagramas del sistema completo
├── platformio.ini        # Configuración del proyecto PlatformIO
└── README.md
```

## Componentes

| Componente | Función |
|---|---|
| ESP32 | Microcontrolador, lee sensores y envía datos por WiFi |
| Sensor de humedad de suelo (capacitivo) | Mide humedad del sustrato |
| DHT22 | Temperatura y humedad ambiente |
| LDR | Nivel de luz / radiación |
| Case impreso en 3D (forma de Y) | Aloja electrónica y sensores en la matera |
| Flask (AWS EC2) | Recibe y almacena los datos |
| Base de datos | Persistencia de las lecturas históricas |
| Grafana | Dashboard y sistema de alertas |

## Cómo correr el firmware

1. Instala [PlatformIO](https://platformio.org/) (como extensión de VS Code, o CLI).
2. Clona este repo.
3. Copia `include/config.h.example` a `include/config.h` y llena tus datos
   (red WiFi, URL del servidor, pines según tu cableado real).
4. Conecta el ESP32 por USB.
5. Compila y sube: `pio run --target upload`
6. Monitor serial: `pio device monitor`

## Roadmap

- [x] Definición de alcance y arquitectura general
- [x] Confirmación de sensores y cultivo con el profesor
- [ ] Medidas de los sensores + diseño del case en 3D (OpenSCAD)
- [ ] Impresión 3D y montaje físico
- [ ] Firmware: lectura de sensores + envío WiFi
- [ ] Backend: servidor Flask recibiendo y almacenando datos
- [ ] Base de datos para históricos
- [ ] Dashboard en Grafana
- [ ] Sistema de alertas (umbrales + notificación)
- [ ] Siembra y pruebas con la planta real
- [ ] Documentación final y sustentación

## Licencia

Proyecto académico — Universidad Pontificia Bolivariana, curso de IoT.
