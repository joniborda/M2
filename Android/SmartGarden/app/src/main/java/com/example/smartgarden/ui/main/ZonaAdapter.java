package com.example.smartgarden.ui.main;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import com.example.smartgarden.R;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;
import com.example.smartgarden.logic.Message;
import com.example.smartgarden.logic.Zona;

import java.util.List;

public class ZonaAdapter extends RecyclerView.Adapter<ZonaAdapter.ViewHolder> {

    // Pass in the contact array into the constructor
    public ZonaAdapter(List<Zona> zonas) {
        mZonas = zonas;
    }

    // Provide a direct reference to each of the views within a data item
    // Used to cache the views within the item layout for fast access
    class ViewHolder extends RecyclerView.ViewHolder {
        // Your holder should contain a member variable
        // for any view that will be set as you render a row
        TextView txt_zona;
        TextView txt_state_zona;
        TextView txt_temp_amb;
        TextView txt_hum_amb;
        TextView txt_hum_suelo;
        TextView txt_int_luz;
        TextView txt_int_riego;
        TextView txt_duracion;
        TextView txt_hum_suelo_riego;
        Switch switch_luz;
        Switch switch_modo_luz;

        // We also create a constructor that accepts the entire item row
        // and does the view lookups to find each subview
        ViewHolder(View v) {
            // Stores the itemView in a public final member variable that can be used
            // to access the context from any ViewHolder instance.
            super(v);

            txt_zona = v.findViewById(R.id.txt_zona);
            txt_state_zona = v.findViewById(R.id.txt_state);
            txt_temp_amb = v.findViewById(R.id.txt_temp_amb);
            txt_hum_amb = v.findViewById(R.id.txt_hum_amb);
            txt_hum_suelo = v.findViewById(R.id.txt_hum_suelo);
            txt_int_luz = v.findViewById(R.id.txt_int_luz);
            txt_int_riego = v.findViewById(R.id.txt_int_riego);
            txt_duracion = v.findViewById(R.id.txt_duracion);
            txt_hum_suelo_riego = v.findViewById(R.id.txt_hum_suelo_riego);
            switch_luz = v.findViewById(R.id.switch_luz_on_off);
            switch_modo_luz = v.findViewById(R.id.switch_modo_luz);
        }
    }

    // Store a member variable for the contacts
    private List<Zona> mZonas;
    private boolean bandera;

    // Usually involves inflating a layout from XML and returning the holder
    @Override
    public ZonaAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        Context context = parent.getContext();
        LayoutInflater inflater = LayoutInflater.from(context);

        // Inflate the custom layout
        View contactView = inflater.inflate(R.layout.item_zona, parent, false);

        // Return a new holder instance
        ViewHolder viewHolder = new ViewHolder(contactView);
        return viewHolder;
    }

    // Involves populating data into the item through holder
    @SuppressLint("DefaultLocale")
    @Override
    public void onBindViewHolder(ZonaAdapter.ViewHolder viewHolder, int position) {
        // Get the data model based on position
        Zona zona = mZonas.get(position);

        // Set item views based on your views and data model

        TextView txt_zona = viewHolder.txt_zona;
        TextView txt_state_zona = viewHolder.txt_state_zona;
        TextView txt_hum_amb = viewHolder.txt_hum_amb;
        TextView txt_temp_amb = viewHolder.txt_temp_amb;
        TextView txt_hum_suelo = viewHolder.txt_hum_suelo;
        TextView txt_int_luz = viewHolder.txt_int_luz;
        TextView txt_int_riego = viewHolder.txt_int_riego;
        TextView txt_duracion = viewHolder.txt_duracion;
        TextView txt_hum_suelo_riego = viewHolder.txt_hum_suelo_riego;
        Switch switch_luz = viewHolder.switch_luz;
        Switch switch_modo_luz = viewHolder.switch_modo_luz;

        txt_zona.setText(zona.getStringFromNroZona());
        txt_state_zona.setText(zona.getEstado());
        txt_hum_amb.setText(String.format("%.2f",zona.getHumAmb()));
        txt_temp_amb.setText(String.format("%.2f",zona.getTempAmb()));
        txt_hum_suelo.setText(String.format("%.2f",zona.getHumSuelo()));
        txt_int_luz.setText(String.format("%.2f",zona.getLuzAmb()));
        txt_int_riego.setText(String.valueOf(zona.getRiego().getIntensidad()));
        txt_duracion.setText(String.valueOf(zona.getRiego().getDuracion()));
        txt_hum_suelo_riego.setText(String.valueOf(zona.getRiego().getHumSueloResultado()));

        switch_luz.setChecked(false);
        switch_modo_luz.setChecked(true);


        switch_modo_luz.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(bandera) {
                    // isChecked will be true if the switch is in the On position
                    switchModoLuzEventListener(isChecked, position);
                    switch_luz.setChecked(false); // si o si se manda a apagar, aunque no este prendida
                } else {
                    bandera = true; // Para que la proxima vuelva a entrar por true
                }
            }
        });

        switch_luz.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                // isChecked will be true if the switch is in the On position
                switchLuzEventListener(isChecked, position);
                if(isChecked && switch_modo_luz.isChecked()) { // Si lo pone en on y el modo automatico esta on
                    bandera = false; // Para que no entre al metodo onCheckedChanged de switch_modo_luz
                    switch_modo_luz.setChecked (false); // Activo modo manual -> false el modo automatico
                }
            }
        });

    }

    private void switchLuzEventListener(boolean isOn, int position) {
        if(isOn) {
            // mandar a encender la luz
            Command cmd = mZonas.get(position).getNroZona() == 1 ? Command.ENCENDER_LUZ_1 : Command.ENCENDER_LUZ_2;
            BTHandler.getInstance().sendMsg(new Message(cmd));
        } else {
            // mandar a apagar la luz
            Command cmd = mZonas.get(position).getNroZona() == 1 ? Command.APAGAR_LUZ_1 : Command.APAGAR_LUZ_2;
            BTHandler.getInstance().sendMsg(new Message(cmd));
        }
    }

    private void switchModoLuzEventListener(boolean isModoAutomatico, int position) {
        if(isModoAutomatico) {
            // mandar instruccion modo automatico
            Command cmd = mZonas.get(position).getNroZona() == 1 ? Command.AUTO_LUZ_1 : Command.AUTO_LUZ_2;
            BTHandler.getInstance().sendMsg(new Message(cmd));
        }
    }

    // Returns the total count of items in the list
    @Override
    public int getItemCount() {
        return mZonas.size();
    }
}
