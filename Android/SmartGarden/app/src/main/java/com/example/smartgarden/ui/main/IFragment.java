package com.example.smartgarden.ui.main;

public interface IFragment {

    void conexion(String[] values);

    void desconexion();

    void resultadoDetenerRiego(boolean ok);

    void showErrorRiegoManual();

    void showErrorMantenimiento();

    void showErrorCensoManual();

    void comenzoRiegoAutomatico(int nroZona, String[] values);

    void terminoRiegoManual();

    void terminoRiegoAutomatico(int nroZona);

    void resultadoRiegoAutomatico(int nroZona, String[] values);

    void resultadoCenso(String[] values);

    void resultadoCensoAutomatico(String[] values);

    void resultadoMantenimiento(String[] values);
}
