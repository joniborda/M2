package com.example.smartgarden.logic;

import com.example.smartgarden.R;

import java.util.ArrayList;

public class Zona {

    // zona: 1 o 2
    private int nroZona;
    private float tempAmb, humAmb, humSuelo, luzAmb;
    private boolean luzIluminacion, luzAutomatica;
    private Riego riego;
    private ZonaStatus estado;

    public Zona(int nroZona) {
        this.nroZona = nroZona;
        this.estado = ZonaStatus.Censando;
        this.luzIluminacion = false;
        this.tempAmb = 0;
        this.humAmb = 0;
        this.humSuelo = 0;
        this.luzAmb = 0;
        this.riego = new Riego(nroZona);
        this.luzAutomatica = true;
    }

    public int getEstado() {
        return estado.getValue();
    }

    public void setEstado(ZonaStatus estado) {
        this.estado = estado;
    }

    public int getStringFromNroZona() {
        return this.nroZona == 1 ? R.string.zona_1 : R.string.zona_2;
    }

    public boolean hayLuzIluminacion() {
        return luzIluminacion;
    }

    public void setLuzIluminacion(boolean luzIluminacion) {
        this.luzIluminacion = luzIluminacion;
    }

    public float getTempAmb() {
        return tempAmb;
    }

    public void setTempAmb(float tempAmb) {
        this.tempAmb = tempAmb;
    }

    public float getHumAmb() {
        return humAmb;
    }

    public void setHumAmb(float humAmb) {
        this.humAmb = humAmb;
    }

    public float getHumSuelo() {
        return humSuelo;
    }

    public void setHumSuelo(float humSuelo) {
        this.humSuelo = humSuelo;
    }

    public float getLuzAmb() {
        return luzAmb;
    }

    public void setLuzAmb(float luzAmb) {
        this.luzAmb = luzAmb;
    }

    public static ArrayList<Zona> createZonaList() {
        ArrayList<Zona> zonas = new ArrayList<Zona>();

        zonas.add(new Zona(1)); // pos 0
        zonas.add(new Zona(2)); // pos 1

        return zonas;
    }

    public Riego getRiego() {
        return riego;
    }

    public void setRiego(Riego riego) {
        this.riego = riego;
    }

    public boolean isLuzAutomatica() {
        return luzAutomatica;
    }

    public void setLuzAutomatica(boolean modoLuz) {
        this.luzAutomatica = modoLuz;
    }

    public int getNroZona() {
        return nroZona;
    }


}
