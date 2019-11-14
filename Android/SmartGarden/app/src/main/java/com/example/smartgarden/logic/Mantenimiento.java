package com.example.smartgarden.logic;

import androidx.annotation.NonNull;

public class Mantenimiento {

    private int errorTemp,
                errorHumAmb,
                errorHumSuelo,
                errorLDR1,
                errorLDR2;

    public Mantenimiento(int errorTemp, int errorHumAmb, int errorHumSuelo, int errorLDR1, int errorLDR2) {

        this.errorTemp = errorTemp;
        this.errorHumAmb = errorHumAmb;
        this.errorHumSuelo = errorHumSuelo;
        this.errorLDR1 = errorLDR1;
        this.errorLDR2 = errorLDR2;
    }

}
