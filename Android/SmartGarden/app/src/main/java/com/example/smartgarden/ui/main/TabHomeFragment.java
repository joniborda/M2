package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.ArduinoStatus;
import com.example.smartgarden.logic.Riego;
import com.example.smartgarden.logic.Zona;
import com.example.smartgarden.logic.ZonaStatus;

import java.util.ArrayList;
import java.util.Objects;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabHomeFragment extends Fragment implements IFragment {

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
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Initialize contacts
        zonas = Zona.createZonaList();
        // Create adapter passing in the sample user data
        adapter = new ZonaAdapter(zonas);
    }

    @SuppressLint("HandlerLeak")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.tab1_home, container, false);

        imageDesconnected = v.findViewById(R.id.image_state);

        // Lookup the recyclerview in activity layout
        rvZonas = v.findViewById(R.id.rv_zonas);
        // Attach the adapter to the recyclerview to populate items
        rvZonas.setAdapter(adapter);
        // Set layout manager to position the items
        rvZonas.setLayoutManager(new LinearLayoutManager(getContext()));

        return v;
    }

    @Override
    public void onResume() {
        super.onResume();

        Zona newZona1 = zonas.get(indexZona1);
        Zona newZona2 = zonas.get(indexZona2);

        if(MainActivity.arduinoStatus == ArduinoStatus.Desconnected) {
            imageDesconnected.setVisibility(View.VISIBLE);
            rvZonas.setVisibility(View.GONE);
        } else {
            imageDesconnected.setVisibility(View.GONE);
            // Buscar los ultimos resultados de riego a la bd
            Riego riego1 = MainActivity.dbHelper.getUltimoRiego(1);
            Riego riego2 = MainActivity.dbHelper.getUltimoRiego(2);

            newZona1.setRiego(riego1);
            newZona2.setRiego(riego2);

            zonas.set(indexZona1, newZona1);
            zonas.set(indexZona2, newZona2);
            adapter.notifyDataSetChanged();
            rvZonas.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void conexion(String [] values) {
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
        // ---------- zona 2 ------------
        newZona2.setEstado(ZonaStatus.Censando);
        newZona2.setTempAmb(Float.parseFloat(values[++indexData]));
        newZona2.setHumAmb(Float.parseFloat(values[++indexData]));
        newZona2.setHumSuelo(Float.parseFloat(values[++indexData]));
        newZona2.setLuzAmb(Float.parseFloat(values[++indexData]));

        // Luces off y modo automatico de las luces on

        // Buscar los ultimos resultados de riego a la bd
        Riego riego1 = MainActivity.dbHelper.getUltimoRiego(1);
        Riego riego2 = MainActivity.dbHelper.getUltimoRiego(2);

        newZona1.setRiego(riego1);
        newZona2.setRiego(riego2);

        zonas.set(indexZona1, newZona1);
        zonas.set(indexZona2, newZona2);
        adapter.notifyDataSetChanged();

        if(isVisible()) {
            Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
                imageDesconnected.setVisibility(View.GONE);
                rvZonas.setVisibility(View.VISIBLE);
            });
        }
    }

    @Override
    public void desconexion() {
        if(isVisible()) {
            Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
                imageDesconnected.setVisibility(View.VISIBLE);
                rvZonas.setVisibility(View.GONE);
            });
        }
    }

    @Override
    public void comenzoRiegoAutomatico(int nroZona, String[] values) {
        // Guardar datos en bd
        Riego riego = new Riego(nroZona);
        riego.setIntensidad(Math.round(Float.parseFloat(values[0])));
        riego.setDuracion(Math.round(Float.parseFloat(values[1])));
        MainActivity.dbHelper.setComienzoRiego(riego);
        // set estado a la zona
        Zona newZona = zonas.get(nroZona == 1 ? indexZona1 : indexZona2);
        newZona.setEstado(ZonaStatus.Regando);
        zonas.set(nroZona == 1 ? indexZona1 : indexZona2, newZona);
        adapter.notifyItemChanged(nroZona == 1 ? indexZona1 : indexZona2);
    }

    @Override
    public void terminoRiegoAutomatico(int nroZona) {
        // set estado a la zona
        Zona newZona = zonas.get(nroZona == 1 ? indexZona1 : indexZona2);
        newZona.setEstado(ZonaStatus.CalculandoResultados);
        zonas.set(nroZona == 1 ? indexZona1 : indexZona2, newZona);
        adapter.notifyItemChanged(nroZona == 1 ? indexZona1 : indexZona2);
    }

    @Override
    public void resultadoRiegoAutomatico(int nroZona, String[] values) {
        // Guardar datos en bd
        Riego riego = new Riego(nroZona);
        riego.setHumSueloResultado(Float.parseFloat(values[0]));
        MainActivity.dbHelper.setResultadoRiego(riego);
        // Juntar todos los datos de la bd
        riego = MainActivity.dbHelper.getUltimoRiego(nroZona); // Piso el objeto riego con otro
        // set estado a la zona
        Zona newZona = zonas.get(nroZona == 1 ? indexZona1 : indexZona2);
        newZona.setEstado(ZonaStatus.Censando);
        newZona.setRiego(riego);
        zonas.set(nroZona == 1 ? indexZona1 : indexZona2, newZona);
        adapter.notifyItemChanged(nroZona == 1 ? indexZona1 : indexZona2);
    }

    @Override
    public void resultadoCenso(String[] values) {
        int indexData = 0;

        Zona newZona1 = zonas.get(indexZona1);
        Zona newZona2 = zonas.get(indexZona2);

        // data
        // tempAmb1, humAmb1, humSuelo1, intLuz1
        // tempAmb2, humAmb2, humSuelo2, intLuz2

        // ---------- zona 1 ------------
        newZona1.setTempAmb(Float.parseFloat(values[indexData++]));
        newZona1.setHumAmb(Float.parseFloat(values[indexData++]));
        newZona1.setHumSuelo(Float.parseFloat(values[indexData++]));
        newZona1.setLuzAmb(Float.parseFloat(values[indexData++]));
        // ---------- zona 2 ------------
        newZona2.setTempAmb(Float.parseFloat(values[indexData++]));
        newZona2.setHumAmb(Float.parseFloat(values[indexData++]));
        newZona2.setHumSuelo(Float.parseFloat(values[indexData++]));
        newZona2.setLuzAmb(Float.parseFloat(values[indexData]));

        zonas.set(indexZona1, newZona1);
        zonas.set(indexZona2, newZona2);
        adapter.notifyDataSetChanged();
    }

    @Override
    public void resultadoCensoAutomatico(String[] values) {
        int indexData = 0;

        Zona newZona1 = zonas.get(indexZona1);
        Zona newZona2 = zonas.get(indexZona2);

        // data
        // tempAmb1, humAmb1, humSuelo1, intLuz1
        // tempAmb2, humAmb2, humSuelo2, intLuz2

        // ---------- zona 1 ------------
        newZona1.setTempAmb(Float.parseFloat(values[indexData++]));
        newZona1.setHumAmb(Float.parseFloat(values[indexData++]));
        newZona1.setHumSuelo(Float.parseFloat(values[indexData++]));
        newZona1.setLuzAmb(Float.parseFloat(values[indexData++]));
        // ---------- zona 2 ------------
        newZona2.setTempAmb(Float.parseFloat(values[indexData++]));
        newZona2.setHumAmb(Float.parseFloat(values[indexData++]));
        newZona2.setHumSuelo(Float.parseFloat(values[indexData++]));
        newZona2.setLuzAmb(Float.parseFloat(values[indexData]));

        zonas.set(indexZona1, newZona1);
        zonas.set(indexZona2, newZona2);
        adapter.notifyDataSetChanged();
    }

    // ACCIONES EN OTROS FRAGMENTS

    @Override
    public void terminoRiegoManual() {
        // Do nothing
    }

    @Override
    public void resultadoMantenimiento(String[] values) {
        // Do nothing
    }


    @Override
    public void resultadoDetenerRiego(boolean ok) {
        if(ok) {
            // set estado a la zona
            Zona newZona = zonas.get(indexZona1);
            newZona.setEstado(ZonaStatus.Censando);
            zonas.set(indexZona1, newZona);
            newZona = zonas.get(indexZona2);
            newZona.setEstado(ZonaStatus.Censando);
            zonas.set(indexZona2, newZona);
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void showErrorRiegoManual() {
        // Do nothing
    }

    @Override
    public void showErrorMantenimiento() {
        // Do nothing
    }

    @Override
    public void showErrorCensoManual() {
        // Do nothing
    }
}
