package com.example.smartgarden.logic;

import com.example.smartgarden.R;

public enum ZonaStatus {

    Censando(R.string.censando),
    Regando(R.string.regando),
    CalculandoResultados(R.string.calculando_resultados);

    private int value;

    ZonaStatus(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
