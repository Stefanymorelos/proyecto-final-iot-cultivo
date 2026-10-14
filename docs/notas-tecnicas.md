# Notas tecnicas - Proyecto IoT Cultivo

Registro de decisiones, descubrimientos y problemas encontrados durante el desarrollo.

## 1. Alcance actual
- Una sola planta (entidad `Planta_01`, tipo `PlantMonitor`).
- Intervalo de muestreo y envio: **5 minutos** (requisito del profesor). La primera lectura se hace al encender; para forzar una lectura inmediata se presiona RST.
- `alerta_seco` se calcula **en el codigo** a partir del porcentaje de humedad del suelo (no se usa la salida digital del sensor).

## 2. Cableado (TTGO T-Beam ESP32)
| Componente | Pines |
|---|---|
| HDC1080 (temp/humedad aire) | I2C SDA 21, SCL 22 (dir. 0x40) |
| AP3216 (luz) | I2C SDA 21, SCL 22 |
| Humedad de suelo, salida analogica (AO) | **GPIO39 (VN)**, ADC1 |
| GPS | RX 34, TX 12, 9600 baud |
| Humedad de suelo, salida digital (DO) | GPIO4, **ya no se usa** |

## 3. Red y servidor
- Flujo: ESP32 -> relay Flask (puerto 80) -> Orion -> QuantumLeap -> CrateDB.
- Servidor en AWS Academy EC2 con Elastic IP `34.227.231.151` (no cambia al detener/iniciar el Lab).
- El relay se publica en el puerto **80** (mapeo Docker `80:3000`) porque la red del campus (UPBWiFi) bloquea los puertos 1026 y 3000.
- Security Group: de forma permanente solo hacen falta 22 (SSH) y 80 (relay). Los puertos 1026, 3000 y 4200 fueron para depuracion y se pueden cerrar.
- Los contenedores no arrancan solos tras detener/iniciar el Lab: ejecutar `docker compose up -d` en `~/proyecto`.
- Orion 4.5.0 usa `-dbURI mongodb://mongo:27017` (ya no `-dbhost`).
- La suscripcion Orion -> QuantumLeap usa `attrsFormat: normalized` (`legacy` no es valido) y persiste en el volumen de Mongo.
- Tabla en CrateDB: `"mtopeniot"."etplantmonitor"` (headers `Fiware-Service: openiot`, `Fiware-ServicePath: /`).

## 4. Sensor de humedad de suelo
- El ADC2 del ESP32 **no funciona con WiFi activo**; por eso la senal analogica va a un pin ADC1 (GPIO39).
- Conversion: `map(raw, 4095, 2300, 0, 100)` limitado a 0-100.
- **Calibracion provisional**: seco = 4095 (al aire), mojado = 2300. Falta medir el extremo mojado con la punta sumergida hasta la linea del sensor.
- Umbral de alerta provisional: **30 %** (`alerta_seco = true` si el porcentaje es menor a 30). Por definir el valor real con el profesor/cultivo.

## 5. Problemas conocidos y soluciones
- **RAW 0 / error de ADC2**: la senal estaba en GPIO0 (ADC2). Solucion: moverla a GPIO39.
- **Placa en modo descarga (`boot:0x6 DOWNLOAD_BOOT`)**: GPIO0 es pin de arranque; si queda en bajo al reiniciar entra a modo descarga. No conectar nada a GPIO0.
- **Connection refused a :3000/:1026 desde UPBWiFi**: puertos bloqueados por la red; se usa el puerto 80.
- **COM4 ocupado / permission denied al subir**: hay un monitor serie abierto en otra pestana; cerrarlo con Ctrl+C.
- **`pio` no reconocido**: usar `& "$HOME\.platformio\penv\Scripts\pio.exe"`.
- **`Please specify upload_port`**: conectar el USB antes de subir.
- **`git restore` descarta cambios sin commit**: hacer commit antes de restaurar.
- **Terminal web de EC2 duplica el texto pegado**: usar ssh/scp desde PowerShell.
- **GPS sin fix en interior**: necesita al menos 4 satelites y cielo abierto.
- **`alerta_seco` de la salida digital** no cambiaba con riego: reemplazada por el calculo en codigo.
- **Aviso `AUTH_FAIL` (202) al conectar a UPBWiFi**: primer intento fallido de la red del campus; el reintento conecta bien, no requiere accion.

## 6. Pendientes
- Prueba del GPS en exterior.
- Terminar la calibracion (extremo mojado) y definir el umbral real de sequedad.
- Evaluar deep sleep para ahorrar bateria entre envios.
- Cerrar puertos de depuracion en el Security Group.
- Sincronizar `servidor/docker-compose.yml` con la version `80:3000` / `-dbURI`.
