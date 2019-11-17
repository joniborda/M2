package com.example.smartgarden.logic;

import android.content.ContentValues;

import androidx.annotation.NonNull;

public class RiegoStandard {
    private String intensidad;
    private String duracion;

    public RiegoStandard(String intensidad, String duracion) {
        this.intensidad = intensidad;
        this.duracion = duracion;
    }

    @NonNull
    @Override
    public String toString() {
        String coma = ",";
        StringBuilder toString = new StringBuilder();
        toString.append(coma).append(intensidad);
        toString.append(coma).append(duracion);
        return super.toString();
    }

    public ContentValues intensidadtoContenValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoStandardEntry.INTENSITY_STANDARD, intensidad);
        return values;
    }

    public ContentValues duraciontoContenValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoStandardEntry.DURATION_STANDARD, duracion);
        return values;
    }

    public String getIntensidad() {
        return intensidad;
    }

    public String getDuracion() {
        return duracion;
    }
}
