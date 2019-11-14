package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.BTHandler;

import java.util.Observable;
import java.util.Observer;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabMantenimientoFragment extends Fragment implements IFragment{

    public static Handler bluetoothIn;
    private StringBuilder DataStringIN = new StringBuilder();

    public TabMantenimientoFragment() {

    }

    @SuppressLint("HandlerLeak")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.tab3_mantenimiento, container, false);

        bluetoothIn = new Handler(){
            public void handleMessage(android.os.Message msg){
                if(msg.what == BTHandler.handlerState){
                    String readMessage = (String) msg.obj;
                    DataStringIN.append(readMessage);
                    int endOfLineIndex = DataStringIN.indexOf("\r\n");
                    if (endOfLineIndex > 0) {
                        String dataInPrint = DataStringIN.substring(0, endOfLineIndex);
//                        String [] array = dataInPrint.split("-");
//                        if(array.length == 3){
//                            String aux = "Temperatura: " + array[0]  + "\nHumedad : " + array[1] +  "\nPresion Atmosferica: " + array[2];
//                        }
                        DataStringIN.delete(0, DataStringIN.length());
                    }
                }
            }

        };

        return root;
    }

    @Override
    public void conexion(String[] values) {

    }

    @Override
    public void desconexion() {

    }

    @Override
    public void resultadoDetenerRiego(boolean ok) {

    }

    @Override
    public void showErrorRiegoManual() {

    }

    @Override
    public void showErrorMantenimiento() {

    }

    @Override
    public void showErrorCenso() {

    }

    @Override
    public void comenzoRiegoAutomatico(int nroZona, String[] values) {

    }

    @Override
    public void terminoRiegoManual() {

    }

    @Override
    public void resultadoRiegoAutomatico(int nroZona, String[] values) {

    }

    @Override
    public void resultadoCenso(String[] values) {

    }

    @Override
    public void resultadoMantenimiento(String[] values) {

    }
}
