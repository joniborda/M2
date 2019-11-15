package com.example.smartgarden.ui.main;

public interface IFragment {
    public void conexion(String[] values);
    public void desconexion();
    public void resultadoDetenerRiego(boolean ok);
    public void showErrorRiegoManual();
    public void showErrorMantenimiento();
    public void showErrorCenso();
    public void comenzoRiegoAutomatico(int nroZona, String[] values);
    public void terminoRiegoManual();
    public void terminoRiegoAutomatico(int nroZona);
    public void resultadoRiegoAutomatico(int nroZona, String[] values);
    public void resultadoCenso(String[] values);
    public void resultadoMantenimiento(String[] values);

}
