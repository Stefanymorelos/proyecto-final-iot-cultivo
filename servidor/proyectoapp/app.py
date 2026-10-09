from flask import Flask, request, jsonify
import requests
import os

app = Flask(__name__)

# URL interna de Orion dentro de la red de Docker
ORION_URL = os.environ.get("ORION_URL", "http://orion:1026")

@app.route('/api/plantas', methods=['POST'])
def recibir_datos():
    data = request.get_json()

    if not data:
        return jsonify({"error": "Payload JSON invalido"}), 400

    # Construccion de entidad NGSI v2 para FIWARE Orion
    entity_id = "Planta_01"
    orion_payload = {
        "id": entity_id,
        "type": "PlantMonitor",
        "temperatura_aire": {
            "type": "Float",
            "value": data.get("temperatura_aire", 0.0)
        },
        "humedad_aire": {
            "type": "Float",
            "value": data.get("humedad_aire", 0.0)
        },
        "luz_lux": {
            "type": "Float",
            "value": data.get("luz_lux", 0.0)
        },
        "humedad_suelo_pct": {
            "type": "Integer",
            "value": data.get("humedad_suelo_pct", 0)
        },
        "alerta_seco": {
            "type": "Boolean",
            "value": data.get("alerta_seco", False)
        },
        "location": {
            "type": "geo:json",
            "value": {
                "type": "Point",
                "coordinates": [data.get("longitud", 0.0), data.get("latitud", 0.0)]
            }
        }
    }

    # Headers requeridos por FIWARE
    headers = {
        "Content-Type": "application/json",
        "Fiware-Service": "openiot",
        "Fiware-ServicePath": "/"
    }

    try:
        response = requests.post(
            f"{ORION_URL}/v2/entities/{entity_id}/attrs",
            json={k: v for k, v in orion_payload.items() if k not in ["id", "type"]},
            headers=headers
        )

        if response.status_code == 404:
            response = requests.post(
                f"{ORION_URL}/v2/entities",
                json=orion_payload,
                headers=headers
            )

        print(f"[ORION] Estado: {response.status_code}")
        return jsonify({"status": "OK", "orion_code": response.status_code}), 200

    except Exception as e:
        print(f"[ERROR] Error conectando con Orion: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3000)
