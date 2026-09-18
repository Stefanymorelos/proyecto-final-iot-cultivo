// ============================================================
// SONDA EN "Y" PARA MONITOREO DE CULTIVO
// Proyecto final IoT - Tefa
//
// COMO USAR:
// 1. Instala OpenSCAD (gratis): https://openscad.org/downloads.html
// 2. Abre este archivo (.scad) con OpenSCAD
// 3. El lunes, cuando tengas las medidas reales, solo cambia los
//    numeros en la seccion "MEDIDAS" de aqui abajo. Todo lo demas
//    se recalcula solo.
// 4. Para exportar a STL: menu File > Export > Export as STL
//
// Todas las medidas estan en milimetros (mm).
// ============================================================

// ============================================================
// MEDIDAS A CONFIRMAR EL LUNES (valores de referencia por ahora)
// ============================================================

// --- Placa principal (modulo con antena, tipo LoRa/GSM) ---
placa_largo   = 55;  // mm, el lado mas largo de la placa
placa_ancho   = 28;  // mm
placa_alto    = 10;  // mm, grosor de la placa con componentes soldados
antena_alto   = 35;  // mm, cuanto sobresale la antena hacia arriba

// --- Bateria ---
bateria_largo = 50;  // mm
bateria_ancho = 34;  // mm
bateria_alto  = 8;   // mm

// --- Sensor de humedad de suelo (el de dos varillas) ---
sensor_hum_placa_largo = 30;  // mm, la parte con el chip
sensor_hum_placa_ancho = 15;  // mm
sensor_hum_varilla_largo = 60; // mm, largo de las varillas metalicas

// --- Sensor de temperatura/humedad ambiente (modulo chiquito) ---
sensor_temp_largo = 15;  // mm
sensor_temp_ancho = 12;  // mm
sensor_temp_alto  = 5;   // mm

// --- Sensor de luz (LDR) ---
sensor_luz_largo = 15;  // mm
sensor_luz_ancho = 12;  // mm
sensor_luz_alto  = 5;   // mm

// ============================================================
// PARAMETROS DE IMPRESION (normalmente no hay que tocarlos)
// ============================================================

grosor_pared   = 2.5;  // grosor de pared, ideal para FDM (no muy delgado)
holgura        = 1.0;  // espacio extra alrededor de cada sensor para que entre sin forzar
diametro_cable = 4;    // diametro del canal por donde pasan los cables

// ============================================================
// DIMENSIONES CALCULADAS DEL TRONCO (a partir de lo de arriba)
// ============================================================

// El tronco debe ser suficientemente ancho/profundo para la placa Y la bateria,
// puestas una sobre otra. Aqui asumimos una sobre otra.
tronco_ancho      = max(placa_ancho, bateria_ancho) + 2*grosor_pared + 2*holgura;
tronco_profundo   = max(placa_largo, bateria_largo) + 2*grosor_pared + 2*holgura;
tronco_alto       = placa_alto + bateria_alto + 20 + 2*grosor_pared; // +20mm de aire/separacion interna

// --- Patas (van enterradas) ---
pata_diametro     = 22;  // debe ser mayor al ancho de la placa del sensor + pared
pata_largo        = sensor_hum_varilla_largo + 40; // varillas + un poco de manguito plastico
angulo_patas      = 25;  // grados de apertura respecto a la vertical (la "Y")
separacion_patas  = 20;  // separacion horizontal en la base entre las dos patas

// --- Soporte del sensor de luz (arriba del tronco) ---
soporte_luz_alto  = sensor_luz_alto + 2*grosor_pared + 8;
soporte_luz_ancho = sensor_luz_ancho + 2*grosor_pared + 2*holgura;
soporte_luz_largo = sensor_luz_largo + 2*grosor_pared + 2*holgura;

$fn = 48; // suavidad de curvas/cilindros

// ============================================================
// MODULO: TRONCO (electronica + bateria)
// ============================================================
module tronco() {
    difference() {
        // cuerpo solido
        cube([tronco_ancho, tronco_profundo, tronco_alto], center = false);

        // cavidad interior hueca (deja pared de "grosor_pared" en todos lados)
        translate([grosor_pared, grosor_pared, grosor_pared])
            cube([
                tronco_ancho - 2*grosor_pared,
                tronco_profundo - 2*grosor_pared,
                tronco_alto - grosor_pared // abierto por arriba para poder meter la placa
            ]);

        // canal para la antena, saliendo por la tapa superior
        translate([tronco_ancho/2, tronco_profundo/2, tronco_alto - 5])
            cylinder(d = 8, h = 10);

        // ventana lateral para acceder a la bateria (cambiarla sin desarmar todo)
        translate([tronco_ancho - grosor_pared - 1, tronco_profundo*0.2, tronco_alto*0.15])
            cube([grosor_pared + 2, tronco_profundo*0.6, bateria_alto + 5]);
    }
}

// ============================================================
// MODULO: UNA PATA (housing tipo tubo para un sensor enterrado)
// ============================================================
module pata(largo, diametro) {
    difference() {
        cylinder(d = diametro, h = largo);
        // hueco interior para el sensor + su varilla
        translate([0, 0, grosor_pared])
            cylinder(d = diametro - 2*grosor_pared, h = largo);
    }
}

// ============================================================
// MODULO: SOPORTE DEL SENSOR DE LUZ (arriba, mirando al sol)
// ============================================================
module soporte_luz() {
    difference() {
        cube([soporte_luz_ancho, soporte_luz_largo, soporte_luz_alto]);
        translate([grosor_pared, grosor_pared, grosor_pared])
            cube([
                soporte_luz_ancho - 2*grosor_pared,
                soporte_luz_largo - 2*grosor_pared,
                soporte_luz_alto
            ]);
        // ventana para que la luz llegue al sensor
        translate([soporte_luz_ancho/2, soporte_luz_largo/2, -1])
            cylinder(d = sensor_luz_largo * 0.7, h = grosor_pared + 2);
    }
}

// ============================================================
// ENSAMBLE COMPLETO
// ============================================================
module sonda_completa() {
    // tronco (queda por encima del nivel del suelo)
    tronco();

    // soporte de luz, encima del tronco
    translate([
        (tronco_ancho - soporte_luz_ancho)/2,
        (tronco_profundo - soporte_luz_largo)/2,
        tronco_alto
    ])
        soporte_luz();

    // pata izquierda (sensor de humedad), abriendo hacia -X
    translate([tronco_ancho/2 - separacion_patas/2, tronco_profundo/2, 0])
        rotate([0, -angulo_patas, 0])
            translate([0, 0, -pata_largo])
                pata(pata_largo, pata_diametro);

    // pata derecha (sensor de temperatura), abriendo hacia +X
    translate([tronco_ancho/2 + separacion_patas/2, tronco_profundo/2, 0])
        rotate([0, angulo_patas, 0])
            translate([0, 0, -pata_largo])
                pata(pata_largo, pata_diametro);
}

sonda_completa();
