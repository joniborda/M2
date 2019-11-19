package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.text.method.KeyListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.ArduinoStatus;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;
import com.example.smartgarden.logic.DBHelper;
import com.example.smartgarden.logic.Message;
import com.example.smartgarden.logic.RiegoStandard;

import java.util.Objects;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabConfiguracionFragment extends Fragment implements IFragment {

    private boolean bandera;

    private TextView tiposDeRiego;
    private RadioGroup rg;
    private TextView btnIniciarRiego;
    private TextView btnIniciarCenso;
    private TextView btnIniciarMantenimiento;
    private TextView btnDetenerRiego;
    private EditText txtDuracion;
    private EditText txtIntensidad;
    private boolean isEditingDuracion;
    private boolean isEditingintensidad;
    private int tipoRiego;

    public TabConfiguracionFragment() {
        tipoRiego = 0;
    }

    @SuppressLint("HandlerLeak")
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.tab2_configuracion, container, false);

        btnIniciarRiego = root.findViewById(R.id.txt_iniciar_riego);
        btnIniciarCenso = root.findViewById(R.id.txt_iniciar_censo);
        btnIniciarMantenimiento = root.findViewById(R.id.txt_iniciar_mantenimiento);
        btnDetenerRiego = root.findViewById(R.id.txt_detener_riego);
        tiposDeRiego = root.findViewById(R.id.textView2);
        rg = root.findViewById(R.id.radiobtn_tipo_riegos);
        txtDuracion = root.findViewById(R.id.txt_duracion_riego);
        Button editButtonDuracion = root.findViewById(R.id.edit_btn_duracion);
        txtIntensidad = root.findViewById(R.id.txt_intensidad_riego);
        Button editButtonIntensidad = root.findViewById(R.id.edit_btn_intensidad);

        // No se pueden hacer cambios
        txtDuracion.setTag(txtDuracion.getKeyListener());
        txtDuracion.setKeyListener(null);

        txtIntensidad.setTag(txtIntensidad.getKeyListener());
        txtIntensidad.setKeyListener(null);

        // Los botones estan en modo edicion
        isEditingDuracion = true;
        isEditingintensidad = true;

        btnIniciarRiego.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                iniciarRiego();
            }
        });

        btnIniciarCenso.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                iniciarCenso();
            }
        });

        btnIniciarMantenimiento.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                iniciarMantenimiento();
            }
        });

        btnDetenerRiego.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                detenerRiego();
            }
        });

        bandera = false;
        rg.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener()
        {
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(bandera) { // sirve para que cada vez que se crea la view no envie el comando
                    Command cmd = null;
                    switch (checkedId) {
                        case R.id.radiobtn_continuo:
                            cmd = Command.CAMBIAR_RIEGO_CONTINUO;
                            break;
                        case R.id.radiobtn_intermitente:
                            cmd = Command.CAMBIAR_RIEGO_INTERMITENTE;
                            break;
                    }
                    BTHandler.getInstance().sendMsg(new Message(cmd));
                } else {
                    bandera = true;
                }
            }
        });

        RiegoStandard riegoStandard = MainActivity.dbHelper.getRiegoStandard();
        txtDuracion.setText(riegoStandard.getDuracion());
        txtIntensidad.setText(riegoStandard.getIntensidad());

        editButtonDuracion.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {

                if(isEditingDuracion) {
                    // ahora se convierte en boton de guardar cambios
                    txtDuracion.setKeyListener((KeyListener) txtDuracion.getTag());
                    editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_save);
                    isEditingDuracion = false;
                } else {
                    // guardar
                    String txtDuracionString = txtDuracion.getText().toString();
                    // Validate txtIntensidad
                    if(!txtDuracionString.equals("") && txtDuracionString.matches("\\d+")) {
                        MainActivity.dbHelper.setIntensidad(Integer.parseInt(txtDuracionString));
                        // ahora se convierte en boton de editar
                        editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_edit);
                        isEditingDuracion = true;
                        txtDuracion.setKeyListener(null);
                    } else {
                        showToast("Debe ingresar un numero entero");
                    }
                }
            }
        });

        editButtonIntensidad.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {

                if(isEditingintensidad) {
                    // ahora se convierte en boton de guardar cambios
                    txtIntensidad.setKeyListener((KeyListener) txtIntensidad.getTag());
                    editButtonIntensidad.setBackgroundResource(android.R.drawable.ic_menu_save);
                    isEditingintensidad = false;
                } else {
                    // guardar
                    String txtIntensidadString = txtIntensidad.getText().toString();
                    // Validate txtIntensidad
                    if(!txtIntensidadString.equals("") && txtIntensidadString.matches("\\d+")) {
                        MainActivity.dbHelper.setIntensidad(Integer.parseInt(txtIntensidadString));
                        // ahora se convierte en boton de editar
                        editButtonIntensidad.setBackgroundResource(android.R.drawable.ic_menu_edit);
                        isEditingintensidad = true;
                        txtIntensidad.setKeyListener(null);
                    } else {
                        showToast("Debe ingresar un numero entero");
                    }
                }
            }
        });

        return root;
    }

    @Override
    public void onResume() {
        super.onResume();
        if(MainActivity.arduinoStatus == ArduinoStatus.Desconnected) {
            mostrarComoDesconectado();
        } else {
            btnIniciarRiego.setEnabled(true);
            btnIniciarRiego.setTextColor(getResources().getColor(R.color.colorBoton));
            btnIniciarCenso.setEnabled(true);
            btnIniciarCenso.setTextColor(getResources().getColor(R.color.colorBoton));
            btnIniciarMantenimiento.setEnabled(true);
            btnIniciarMantenimiento.setTextColor(getResources().getColor(R.color.colorBoton));
            btnDetenerRiego.setEnabled(true);
            btnDetenerRiego.setTextColor(getResources().getColor(R.color.colorBoton));
            tiposDeRiego.setVisibility(View.VISIBLE);
            rg.setVisibility(View.VISIBLE);
            switch (tipoRiego) {
                case 0:
                    rg.check(R.id.radiobtn_continuo);
                    break;
                case 1:
                    rg.check(R.id.radiobtn_intermitente);
                    break;
            }
        }
    }

    private void showToast(String msg) {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            Toast.makeText(getContext(), msg, Toast.LENGTH_LONG).show();
        });
    }

    private void mostrarComoDesconectado() {
        // Inicialmente no se muestra
        tiposDeRiego.setVisibility(View.GONE);
        rg.setVisibility(View.GONE);
        // Inicialmente no puede accionar los botones
        btnIniciarRiego.setEnabled(false);
        btnIniciarRiego.setTextColor(getResources().getColor(R.color.colorBotonDisabled));
        btnIniciarCenso.setEnabled(false);
        btnIniciarCenso.setTextColor(getResources().getColor(R.color.colorBotonDisabled));
        btnIniciarMantenimiento.setEnabled(false);
        btnIniciarMantenimiento.setTextColor(getResources().getColor(R.color.colorBotonDisabled));
        btnDetenerRiego.setEnabled(false);
        btnDetenerRiego.setTextColor(getResources().getColor(R.color.colorBotonDisabled));
    }

    private void iniciarRiego(){
        RiegoStandard riego = MainActivity.dbHelper.getRiegoStandard();
        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_RIEGO, riego));
    }

    private void iniciarCenso(){
        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_CENSO));
    }

    private void iniciarMantenimiento(){
        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_MANTENIMIENTO));
    }

    private void detenerRiego(){
        BTHandler.getInstance().sendMsg(new Message(Command.DETENER_RIEGO));
    }

    @Override
    public void conexion(String[] values) {
        tipoRiego = Integer.parseInt(values[0]);

        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            // set clickable to all buttons
            btnIniciarRiego.setEnabled(true);
            btnIniciarRiego.setTextColor(getResources().getColor(R.color.colorBoton));
            btnIniciarCenso.setEnabled(true);
            btnIniciarCenso.setTextColor(getResources().getColor(R.color.colorBoton));
            btnIniciarMantenimiento.setEnabled(true);
            btnIniciarMantenimiento.setTextColor(getResources().getColor(R.color.colorBoton));
            btnDetenerRiego.setEnabled(true);
            btnDetenerRiego.setTextColor(getResources().getColor(R.color.colorBoton));
            tiposDeRiego.setVisibility(View.VISIBLE);
            rg.setVisibility(View.VISIBLE);

            switch (tipoRiego) {
                case 0:
                    rg.check(R.id.radiobtn_continuo);
                    break;
                case 1:
                    rg.check(R.id.radiobtn_intermitente);
                    break;
            }
        });
    }

    @Override
    public void desconexion() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            mostrarComoDesconectado();
        });
    }

    @Override
    public void resultadoDetenerRiego(boolean ok) {
        if(ok) {
            ((MainActivity) getActivity()).showToast("Se han detenido correctamente los riegos en curso", Toast.LENGTH_LONG);
        } else {
            ((MainActivity) getActivity()).showToast("No se ha encontrado ningún riego en curso", Toast.LENGTH_LONG);
        }
    }

    @Override
    public void showErrorRiegoManual() {
        ((MainActivity) getActivity()).showToast("No se ha podido iniciar el riego manual. Intente más tarde.", Toast.LENGTH_LONG);
    }

    @Override
    public void showErrorMantenimiento() {
        ((MainActivity) getActivity()).showToast("No se ha podido iniciar el mantenimiento. Intente más tarde.", Toast.LENGTH_LONG);
    }

    @Override
    public void showErrorCensoManual() {
        ((MainActivity) getActivity()).showToast("No se ha podido iniciar el censo manual. Intente más tarde.", Toast.LENGTH_LONG);
    }

    @Override
    public void terminoRiegoManual() {
        ((MainActivity) getActivity()).showToast("El riego manual ha finalizado correctamente.", Toast.LENGTH_LONG);
    }

    @Override
    public void resultadoMantenimiento(String[] values) {
        ((MainActivity) getActivity()).showToast("El mantenimiento ha finalizado correctamente.", Toast.LENGTH_LONG);
    }

    @Override
    public void resultadoCenso(String[] values) {
        ((MainActivity) getActivity()).showToast("El censo manual ha finalizado correctamente.", Toast.LENGTH_LONG);
    }

    // ACCIONES EN OTROS FRAGMENTS

    @Override
    public void comenzoRiegoAutomatico(int nroZona, String[] values) {

    }

    @Override
    public void terminoRiegoAutomatico(int nroZona) {

    }

    @Override
    public void resultadoRiegoAutomatico(int nroZona, String[] values) {

    }
}
