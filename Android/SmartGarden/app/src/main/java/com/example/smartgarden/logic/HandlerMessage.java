package com.example.smartgarden.logic;

import android.os.Handler;

import com.example.smartgarden.ui.main.IFragment;

import java.util.ArrayList;

public class  HandlerMessage extends Handler {

    private StringBuilder dataStringIN;
    private ArrayList<IFragment> fragments;

    public HandlerMessage(ArrayList<IFragment> fragments) {
        dataStringIN = new StringBuilder();
        this.fragments = fragments;
    }

    public void handleMessage(android.os.Message msg){
        if(msg.what == BTHandler.handlerState){
            String readMessage = (String) msg.obj;
            dataStringIN.append(readMessage);
            if(dataStringIN.charAt(0) == '<') {
                int endOfLineIndex = dataStringIN.indexOf(">");
                if (endOfLineIndex > 0) {
                    int firstCommaIndex = dataStringIN.indexOf(",");
                    Command command = Command.values()[Integer.parseInt(dataStringIN.substring(1,
                                                                    firstCommaIndex))];
                    String [] values = dataStringIN.substring(firstCommaIndex + 1, endOfLineIndex)
                            .split(",");
                    switch (command) {
                        case CONEXION:
                            // extraer los datos de tipo de riego, censo de zona1 y zona2
                            for (IFragment fragment: fragments) {
                                fragment.conexion(values);
                            }
                            break;
                        case DESCONEXION:
                            // el user se desconecta y debo actualizar todas las pantallas
                            for (IFragment fragment: fragments) {
                                fragment.desconexion();
                            }
                            break;
                        // Los casos de regando son para cambiar el estado de las zonas a regando
                        // y guardar los datos para el final...
                        case REGANDO_ZONA_1:
                            for (IFragment fragment: fragments) {
                                fragment.comenzoRiegoAutomatico(1, values);
                            }
                            break;
                        case REGANDO_ZONA_2:
                            for (IFragment fragment: fragments) {
                                fragment.comenzoRiegoAutomatico(2, values);
                            }
                            break;
                        // Los casos siguientes son para cambiar el estado de las zonas a censando...
                        case REGUE_MANUAL:
                            for (IFragment fragment: fragments) {
                                fragment.terminoRiegoManual();
                            }
                            break;
                        case RESULTADO_RIEGO_1:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoRiegoAutomatico(1, values);
                            }
                            break;
                        case RESULTADO_RIEGO_2:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoRiegoAutomatico(2, values);
                            }
                            break;
                        case RESULTADO_CENSO:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoCenso(values);
                            }
                            break;
                        case RESULTADO_MANTENIMIENTO:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoMantenimiento(values);
                            }
                            break;
                        case R_DETENER_RIEGO_ERROR:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoDetenerRiego(false);
                            }
                            break;
                        case R_DETENER_RIEGO_OK:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoDetenerRiego(true);
                            }
                            break;
                        case R_CENSO_ERROR:
                            for (IFragment fragment: fragments) {
                                fragment.showErrorCenso();
                            }
                            break;
                        case R_INICIAR_RIEGO_ERROR:
                            for (IFragment fragment: fragments) {
                                fragment.showErrorRiegoManual();
                            }
                            break;
                        case R_MANTENIMIENTO_ERROR:
                            for (IFragment fragment: fragments) {
                                fragment.showErrorMantenimiento();
                            }
                            break;
                    }
                    dataStringIN.delete(0, dataStringIN.length());
                }
            }
        }
    }
}
