package com.example.smartgarden.logic;

public enum Command {
    // ENVIO Y RECIBO
    CONEXION(19),
    DESCONEXION(1),

    // SOLO ENVIO
    DETENER_RIEGO(26),
    INICIAR_RIEGO(21),
    INICIAR_MANTENIMIENTO(7),
    INICIAR_CENSO(20),
    CAMBIAR_RIEGO_INTERMITENTE(23),
    CAMBIAR_RIEGO_CONTINUO(22),
    ENCENDER_LUZ_1(11),
    ENCENDER_LUZ_2(12),
    APAGAR_LUZ_1(13),
    APAGAR_LUZ_2(14),
    AUTO_LUZ_1(15),
    AUTO_LUZ_2(16),

    // RECIBO
    FIN_RIEGO_ZONA_1(5),
    FIN_RIEGO_ZONA_2(6),
    R_DETENER_RIEGO_OK(61), // Se detuvieron los riegos correctamente
    R_DETENER_RIEGO_ERROR(62), // No se encontró ningún riego
    R_INICIAR_RIEGO_ERROR(58), // No se pudo iniciar riego manual porque el arduino se encuentra ocupado
    R_MANTENIMIENTO_ERROR(60), // No se pudo iniciar mantenimiento porque el arduino se encuentra ocupado
    R_CENSO_ERROR(59), // No se pudo iniciar censo porque el arduino se encuentra ocupado
    // Solo me avisan cuando comienzan un riego automatico por zona,
    // la de manual yo la ordeno y debo accionar
    // Data acompañada: intensidad y tiempo
    REGANDO_ZONA_1(52),
    REGANDO_ZONA_2(53),
    // Me avisan cuando se termino el riego manual, y me mandan unicamente los
    // resultados(humedad del suelo) de los riegos automaticos
    REGUE_MANUAL(24),
    RESULTADO_RIEGO_1(17),
    RESULTADO_RIEGO_2(18),
    // Data: 8 valores representativos de cada sensor/actuador
    RESULTADO_CENSO(2),
    RESULTADO_MANTENIMIENTO(8);

    private int value;

    Command(int value) {
            this.value = value;
    }

    @Override
    public String toString() {
        return String.valueOf(value);
    }
}
