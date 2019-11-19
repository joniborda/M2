package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;
import com.example.smartgarden.R;

import java.util.Objects;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabMantenimientoFragment extends Fragment implements IFragment {

    private static final String TAG = "Mantenimiento";

    private LinearLayout layoutActual;
    private LinearLayout layoutInProgress;
    private LinearLayout layoutOk;
    private LinearLayout layoutError;
    private TextView txtNoMantenimiento;
    private TextView txtTemp;
    private TextView txtHumAmb;
    private TextView txtHumSuelo;
    private TextView txtLdr1;
    private TextView txtLdr2;

    public TabMantenimientoFragment() {

    }

    @SuppressLint("HandlerLeak")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.tab3_mantenimiento, container, false);

        layoutInProgress = root.findViewById(R.id.layout_in_progress);
        layoutError = root.findViewById(R.id.layout_error);
        layoutOk = root.findViewById(R.id.layout_ok);
        txtNoMantenimiento  = root.findViewById(R.id.txt_no_mantenimiento);
        txtTemp = root.findViewById(R.id.txt_error_temp);
        txtHumAmb = root.findViewById(R.id.txt_error_hum_amb);
        txtHumSuelo = root.findViewById(R.id.txt_error_hum_suelo);
        txtLdr1 = root.findViewById(R.id.txt_error_ldr1);
        txtLdr2 = root.findViewById(R.id.txt_error_ldr2);

        if(layoutActual != null) {
            layoutActual.setVisibility(View.VISIBLE);
            txtNoMantenimiento.setVisibility(View.GONE);
        }

        return root;
    }

    public void iniciarMantenimientoReceived() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            if(layoutActual!= null) {
                layoutActual.setVisibility(View.GONE);
            } else {
                txtNoMantenimiento.setVisibility(View.GONE);
            }
            layoutActual = layoutInProgress;
            layoutActual.setVisibility(View.VISIBLE);
        });
    }

    @Override
    public void desconexion() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            if(layoutActual != null) {
                layoutActual.setVisibility(View.GONE);
            }
            layoutActual = null;
            txtNoMantenimiento.setVisibility(View.VISIBLE);
        });
    }

    @Override
    public void showErrorMantenimiento() {
        getActivity().runOnUiThread(() -> {
            if(layoutActual != null) {
                layoutActual.setVisibility(View.GONE);
            }
            layoutActual = null;
            txtNoMantenimiento.setVisibility(View.VISIBLE);
        });
    }

    @Override
    public void resultadoMantenimiento(String[] values) {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            int indexData = 0;
            boolean noHayError = true;
            if(layoutActual != null) {
                layoutActual.setVisibility(View.GONE);
            }
            // leer datos
            int errorTemp = Integer.parseInt(values[indexData++]);
            int errorHumAmb = Integer.parseInt(values[indexData++]);
            int errorHumSuelo = Integer.parseInt(values[indexData++]);
            int errorLdr1 = Integer.parseInt(values[indexData++]);
            int errorLdr2 = Integer.parseInt(values[indexData]);

            if(errorTemp == 0) {
                noHayError = false;
                txtTemp.setVisibility(View.VISIBLE);
            }
            if(errorHumAmb == 0) {
                noHayError = false;
                txtHumAmb.setVisibility(View.VISIBLE);
            }
            if(errorHumSuelo == 0) {
                noHayError = false;
                txtHumSuelo.setVisibility(View.VISIBLE);
            }
            if(errorLdr1 == 0) {
                noHayError = false;
                txtLdr1.setVisibility(View.VISIBLE);
            }
            if(errorLdr2 == 0) {
                noHayError = false;
                txtLdr2.setVisibility(View.VISIBLE);
            }
            if(noHayError) {
                layoutActual = layoutOk;
            } else {
                layoutActual = layoutError;
            }
            layoutActual.setVisibility(View.VISIBLE);
        });
    }

    // ACCIONES EN OTROS FRAGMENTS

    @Override
    public void conexion(String[] values) {

    }

    @Override
    public void showErrorCensoManual() {

    }

    @Override
    public void comenzoRiegoAutomatico(int nroZona, String[] values) {

    }

    @Override
    public void terminoRiegoManual() {

    }

    @Override
    public void terminoRiegoAutomatico(int nroZona) {

    }

    @Override
    public void resultadoRiegoAutomatico(int nroZona, String[] values) {

    }

    @Override
    public void resultadoCenso(String[] values) {

    }

    @Override
    public void resultadoDetenerRiego(boolean ok) {

    }

    @Override
    public void showErrorRiegoManual() {

    }
}
