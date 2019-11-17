package com.example.smartgarden.logic;

import android.content.ContentValues;

import androidx.annotation.NonNull;

public class Riego {

    private int nroZona;
    private float humSueloResultado;
    //-------- datos iniciales ------
    private int intensidad;
    private int duracion;

    public Riego(int nroZona) {
        this.nroZona = nroZona;
        this.duracion = 0;
        this.intensidad = 0;
        this.humSueloResultado = 0;
    }

    public ContentValues alltoContentValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoEntry.INTENSITY, intensidad);
        values.put(DataBaseContract.RiegoEntry.DURATION, duracion);
        values.put(DataBaseContract.RiegoEntry.HUM_SUELO, humSueloResultado);
        return values;
    }

    public ContentValues datosInicialestoContentValues() {
        ContentValues values = new ContentValues();
        values.put(DataBaseContract.RiegoEntry.INTENSITY, intensidad);
        values.put(DataBaseContract.RiegoEntry.DURATION, duracion);
        return values;
    }

    public ContentValues humedadSuelotoContenValues() {
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
