package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentTransaction;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.ArduinoStatus;
import com.example.smartgarden.logic.MantenimientoStatus;

import java.util.Objects;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabMantenimientoFragment extends Fragment implements IFragment {

    private LinearLayout layoutInProgress;
    private LinearLayout layoutOk;
    private LinearLayout layoutError;
    private TextView txtNoMantenimiento;
    private TextView txtTemp;
    private TextView txtHumAmb;
    private TextView txtLdr1;
    private TextView txtLdr2;

    private int errorTemp;
    private int errorHumAmb;
    private int errorLdr1;
    private int errorLdr2;

    public TabMantenimientoFragment() {
        MainActivity.mantenimientoStatus = MantenimientoStatus.NoMantenimiento;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
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
        txtLdr1 = root.findViewById(R.id.txt_error_ldr1);
        txtLdr2 = root.findViewById(R.id.txt_error_ldr2);

        return root;
    }

    public void iniciarMantenimiento() {
        MainActivity.mantenimientoStatus = MantenimientoStatus.InProgress;
        layoutInProgress.setVisibility(View.VISIBLE);
        txtNoMantenimiento.setVisibility(View.GONE);
        layoutOk.setVisibility(View.GONE);
        layoutError.setVisibility(View.GONE);
        txtTemp.setVisibility(View.GONE);
        txtHumAmb.setVisibility(View.GONE);
        txtLdr1.setVisibility(View.GONE);
        txtLdr2.setVisibility(View.GONE);
    }

    @Override
    public void onResume() {
        super.onResume();
        if(MainActivity.arduinoStatus == ArduinoStatus.Desconnected) {
            layoutError.setVisibility(View.GONE);
            layoutInProgress.setVisibility(View.GONE);
            layoutOk.setVisibility(View.GONE);
            txtNoMantenimiento.setVisibility(View.VISIBLE);
        } else {
            switch (MainActivity.mantenimientoStatus) {
                case OK:
                    layoutOk.setVisibility(View.VISIBLE);
                    layoutInProgress.setVisibility(View.GONE);
                    txtNoMantenimiento.setVisibility(View.GONE);
                    layoutError.setVisibility(View.GONE);
                    break;
                case InProgress:
                    layoutInProgress.setVisibility(View.VISIBLE);
                    txtNoMantenimiento.setVisibility(View.GONE);
                    layoutOk.setVisibility(View.GONE);
                    layoutError.setVisibility(View.GONE);
                    break;
                case Error:
                    layoutOk.setVisibility(View.GONE);
                    layoutInProgress.setVisibility(View.GONE);
                    txtNoMantenimiento.setVisibility(View.GONE);
                    if (errorTemp == 0) {
                        txtTemp.setVisibility(View.VISIBLE);
                    }
                    if(errorHumAmb == 0) {
                        txtHumAmb.setVisibility(View.VISIBLE);
                    }
                    if(errorLdr1 == 0) {
                        txtLdr1.setVisibility(View.VISIBLE);
                    }
                    if(errorLdr2 == 0) {
                        txtLdr2.setVisibility(View.VISIBLE);
                    }
                    layoutError.setVisibility(View.VISIBLE);
                    break;
            }
        }
    }

    @Override
    public void desconexion() {
        if(isVisible()) {
            Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
                layoutError.setVisibility(View.GONE);
                layoutInProgress.setVisibility(View.GONE);
                layoutOk.setVisibility(View.GONE);
                txtNoMantenimiento.setVisibility(View.VISIBLE);
            });
        }
        MainActivity.mantenimientoStatus = MantenimientoStatus.NoMantenimiento;
    }

    @Override
    public void showErrorMantenimiento() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            layoutError.setVisibility(View.GONE);
            layoutInProgress.setVisibility(View.GONE);
            layoutOk.setVisibility(View.GONE);
            txtNoMantenimiento.setVisibility(View.VISIBLE);
        });
        MainActivity.mantenimientoStatus = MantenimientoStatus.NoMantenimiento;
    }

    @Override
    public void resultadoMantenimiento(String[] values) {

        int indexData = 0;
        int noHacerNada;

        // leer datos
        errorTemp = Integer.parseInt(values[indexData++]);
        errorHumAmb = Integer.parseInt(values[indexData++]);
        noHacerNada = Integer.parseInt(values[indexData++]);
        errorLdr1 = Integer.parseInt(values[indexData++]);
        errorLdr2 = Integer.parseInt(values[indexData]);

        boolean noHayError = true;

        if(errorTemp == 0) {
            noHayError = false;
        }
        if(errorHumAmb == 0) {
            noHayError = false;
        }
        if(errorLdr1 == 0) {
            noHayError = false;
        }
        if(errorLdr2 == 0) {
            noHayError = false;
        }
        if(noHayError) {
            MainActivity.mantenimientoStatus = MantenimientoStatus.OK;
        } else {
            MainActivity.mantenimientoStatus = MantenimientoStatus.Error;
        }

        boolean finalNoHayError = noHayError;
        if(isVisible()) {
            Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
                if (errorTemp == 0) {
                    txtTemp.setVisibility(View.VISIBLE);
                }
                if (errorHumAmb == 0) {
                    txtHumAmb.setVisibility(View.VISIBLE);
                }
                if (errorLdr1 == 0) {
                    txtLdr1.setVisibility(View.VISIBLE);
                }
                if (errorLdr2 == 0) {
                    txtLdr2.setVisibility(View.VISIBLE);
                }
                if (finalNoHayError) {
                    layoutOk.setVisibility(View.VISIBLE);
                    layoutError.setVisibility(View.GONE);
                } else {
                    layoutOk.setVisibility(View.GONE);
                    layoutError.setVisibility(View.VISIBLE);
                }
                layoutInProgress.setVisibility(View.GONE);
                txtNoMantenimiento.setVisibility(View.GONE);
            });
        }

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
    public void resultadoCensoAutomatico(String[] values) {

    }

    @Override
    public void resultadoDetenerRiego(boolean ok) {

    }

    @Override
    public void showErrorRiegoManual() {

    }

}
