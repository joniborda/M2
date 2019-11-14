package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.Riego;
import com.example.smartgarden.logic.Zona;
import com.example.smartgarden.logic.ZonaStatus;

import java.util.ArrayList;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabHomeFragment extends Fragment implements IFragment{

    private ImageView imageDesconnected;
    private RecyclerView rvZonas;

    private ArrayList<Zona> zonas;
    private int indexZona1 = 0;
    private int indexZona2 = 1;
    private ZonaAdapter adapter;

    public TabHomeFragment() {

    }

    @SuppressLint("HandlerLeak")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.tab1_home, container, false);

        // find views
        findViews(v);

        return v;
    }

    private void findViews(View v) {
        imageDesconnected = v.findViewById(R.id.image_state);

        // Lookup the recyclerview in activity layout
        rvZonas = v.findViewById(R.id.rv_zonas);

        // Initialize contacts
        zonas = Zona.createZonaList();

        // Create adapter passing in the sample user data
        adapter = new ZonaAdapter(zonas);
        // Attach the adapter to the recyclerview to populate items

        // Buscar los ultimos resultados a la bd

        rvZonas.setAdapter(adapter);
        // Set layout manager to position the items
        rvZonas.setLayoutManager(new LinearLayoutManager(getContext()));
    }

    @SuppressLint("SetTextI18n")
    public void conexion(String [] values) {
        getActivity().runOnUiThread(() -> {
            int indexData = 0;

            Zona newZona1 = zonas.get(indexZona1);
            Zona newZona2 = zonas.get(indexZona2);

            // data
            // tipo de riego -> no darle bola, comienzo index = 1
            // tempAmb1, humAmb1, humSuelo1, intLuz1
            // tempAmb2, humAmb2, humSuelo2, intLuz2

            // ---------- zona 1 ------------
            newZona1.setEstado(ZonaStatus.Censando);
            newZona1.setTempAmb(Float.parseFloat(values[++indexData]));
            newZona1.setHumAmb(Float.parseFloat(values[++indexData]));
            newZona1.setHumSuelo(Float.parseFloat(values[++indexData]));
            newZona1.setLuzAmb(Float.parseFloat(values[++indexData]));
            newZona1.setLuzIluminacion(false);
            // ---------- zona 2 ------------
            newZona2.setEstado(ZonaStatus.Censando);
            newZona2.setTempAmb(Float.parseFloat(values[++indexData]));
            newZona2.setHumAmb(Float.parseFloat(values[++indexData]));
            newZona2.setHumSuelo(Float.parseFloat(values[++indexData]));
            newZona2.setLuzAmb(Float.parseFloat(values[++indexData]));
            newZona2.setLuzIluminacion(false);

            zonas.set(indexZona1, newZona1);
            adapter.notifyItemChanged(indexZona1);
            zonas.set(indexZona2, newZona2);
            adapter.notifyItemChanged(indexZona2);

            // give visibility
            imageDesconnected.setVisibility(View.GONE);
            rvZonas.setVisibility(View.VISIBLE);

        });
    }

    public void desconexion() {
        imageDesconnected.setVisibility(View.VISIBLE);
        rvZonas.setVisibility(View.GONE);
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
        // Guardar datos en bd
        Riego riego = new Riego(nroZona);
        riego.setIntensidad(Integer.parseInt(values[0]));
        riego.setDuracion(Integer.parseInt(values[1]));
        MainActivity.dbHelper.setComienzoRiego(riego);
        // set estado a la zona
        Zona newZona = zonas.get(nroZona);
        newZona.setEstado(ZonaStatus.Regando);
        zonas.set(nroZona - 1, newZona);
        adapter.notifyItemChanged(nroZona - 1);
    }

    @Override
    public void terminoRiegoManual() {
        // Do nothing
    }

    @Override
    public void terminoRiegoAutomatico(int nroZona) {
        // set estado a la zona
        Zona newZona = zonas.get(nroZona);
        newZona.setEstado(ZonaStatus.CalculandoResultados);
        zonas.set(nroZona - 1, newZona);
        adapter.notifyItemChanged(nroZona - 1);
    }

    @Override
    public void resultadoRiegoAutomatico(int nroZona, String[] values) {
        // Guardar datos en bd
        Riego riego = new Riego(nroZona);
        riego.setHumSueloResultado(Float.parseFloat(values[0]));
        MainActivity.dbHelper.setResultadoRiego(riego);
        // Juntar todos los datos de la bd
        riego = MainActivity.dbHelper.getUltimoRiego(nroZona);
        // set estado a la zona
        Zona newZona = zonas.get(nroZona);
        newZona.setEstado(ZonaStatus.Censando);
        newZona.setRiego(riego);
        zonas.set(nroZona - 1, newZona);
        adapter.notifyItemChanged(nroZona - 1);
    }

    @Override
    public void resultadoCenso(String[] values) {

    }

    @Override
    public void resultadoMantenimiento(String[] values) {

    }


    private void regandoZona(int nroZona) {
        Zona newZona = zonas.get(nroZona);
        newZona.setEstaRegando(true);
        zonas.set(nroZona - 1, newZona);
        adapter.notifyItemChanged(nroZona - 1);
    }

    private void regueZona(int nroZona, String [] values) {
        // data
        //  intensidad(int %), duracion(ms)
        int indexData = 0;

        Zona newZona = zonas.get(nroZona);
        Riego riego = newZona.getRiego();
        riego.setDuracion(Integer.parseInt(values[++indexData]));
        riego.setIntensidad(Integer.parseInt(values[++indexData]));
        newZona.setRiego(riego);
        zonas.set(nroZona - 1, newZona);
        adapter.notifyItemChanged(nroZona - 1);
    }

    private void regueManual(String [] values) {

    }

}
