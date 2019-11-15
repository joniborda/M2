package com.example.smartgarden.logic;

import android.content.ContentValues;

import androidx.annotation.NonNull;

public class Riego {

    private int intensidad;
    private int duracion;
    private int nroZona;
    private float humSueloResultado;

    public Riego(int nroZona) {
        this.nroZona = nroZona;
        this.duracion = 0;
        this.intensidad = 0;
        this.humSueloResultado = 0;
    }

    @NonNull
    @Override
    public String toString() {
        String coma = ",";
        StringBuilder toString = new StringBuilder();
        toString.append(coma).append(intensidad);
        toString.append(coma).append(duracion);
        toString.append(coma).append(humSueloResultado);
        return super.toString();
    }

    public ContentValues toContentValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoEntry.INTENSITY, intensidad);
        values.put(DataBaseContract.RiegoEntry.DURATION, duracion);
        return values;
    }

    public ContentValues HumedadSuelotoContenValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoEntry.HUM_SUELO, humSueloResultado);
        return values;
    }

    public int getNroZona() {
        return nroZona;
    }

    public int getIntensidad() {
        return intensidad;
    }

    public int getDuracion() {
        return duracion;
    }

    public void setIntensidad(int intensidad) {
        this.intensidad = intensidad;
    }

    public void setDuracion(int duracion) {
        this.duracion = duracion;
    }

    public float getHumSueloResultado() {
        return humSueloResultado;
    }

    public void setHumSueloResultado(float humSueloResultado) {
        this.humSueloResultado = humSueloResultado;
    }
}
