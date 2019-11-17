package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;

import java.util.Objects;
import java.util.Observable;
import java.util.Observer;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabMantenimientoFragment extends Fragment implements IFragment {

    public String tag = "mantenimiento";

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

        return root;
    }

    protected void iniciarMantenimientoReceived()
    {
        layoutInProgress.setVisibility(View.VISIBLE);
        txtNoMantenimiento.setVisibility(View.GONE);
    }

    @Override
    public void desconexion() {

    }

    @Override
    public void showErrorMantenimiento() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            txtNoMantenimiento.setVisibility(View.VISIBLE);
        });
    }

    @Override
    public void resultadoMantenimiento(String[] values) {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {

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
