package com.example.smartgarden.logic;

import android.os.Handler;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.ui.main.IFragment;

import java.io.IOException;
import java.util.ArrayList;

public class  HandlerMessage extends Handler {

    private StringBuilder dataStringIN;
    private ArrayList<IFragment> fragments;
    private MainActivity main;

    public HandlerMessage(ArrayList<IFragment> fragments, MainActivity main) {
        dataStringIN = new StringBuilder();
        this.fragments = fragments;
        this.main = main;
    }

    public void handleMessage(android.os.Message msg){
        if(msg.what == BTHandler.handlerState){
            String readMessage = (String) msg.obj;
            dataStringIN.append(readMessage);
            System.out.println("Mensaje: "+dataStringIN);
            if(dataStringIN.charAt(0) == '<') {
                System.out.println("Entre por <");
                int endOfLineIndex = dataStringIN.indexOf(">");
                if (endOfLineIndex > 0) {
                    System.out.println("Entre por >");
                    int firstCommaIndex = dataStringIN.indexOf(",");
                    Command command;
                    String [] values = new String[0];
                    if(firstCommaIndex == -1) {
                        command = Command.valueOf(Integer.parseInt(dataStringIN.substring(1,endOfLineIndex)));
                    } else {
                        command = Command.valueOf(Integer.parseInt(dataStringIN.substring(1,
                                firstCommaIndex)));
                        values = dataStringIN.substring(firstCommaIndex + 1, endOfLineIndex)
                                .split(",");
                    }
                    switch (command) {
                        case CONEXION:
                            // extraer los datos de tipo de riego, censo de zona1 y zona2
                            main.setArduinoStatus(ArduinoStatus.Connected);
                            for (IFragment fragment: fragments) {
                                fragment.conexion(values);
                            }
                            break;
                        case DESCONEXION:
                            try {
                                BTHandler.getInstance().desconnect();
                            } catch (IOException ignored) {
                                return;
                            }
                            main.setArduinoStatus(ArduinoStatus.Desconnected);
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
                        case INICIAR_CENSO: // Fin censo
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
                                fragment.showErrorCensoManual();
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
                        case FIN_RIEGO_ZONA_1:
                            for(IFragment fragment: fragments) {
                                fragment.terminoRiegoAutomatico(1);
                            }
                            break;
                        case FIN_RIEGO_ZONA_2:
                            for(IFragment fragment: fragments) {
                                fragment.terminoRiegoAutomatico(2);
                            }
                            break;
                        case FIN_CENSO_AUTOMATICO:
                            for (IFragment fragment: fragments) {
                                fragment.resultadoCensoAutomatico(values);
                            }
                            break;
                        default:
                            System.out.println("Instruccion recibida: " + command.toString());
                            break;
                    }
                }
            }
            dataStringIN.delete(0, dataStringIN.length());
        }
    }
}
