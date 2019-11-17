package com.example.smartgarden.ui.main;


import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.example.smartgarden.MainActivity;
import com.example.smartgarden.R;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;
import com.example.smartgarden.logic.Message;
import com.example.smartgarden.logic.RiegoStandard;

import java.util.Objects;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabConfiguracionFragment extends Fragment implements IFragment {

    public String tag = "configuracion";

    private boolean bandera;

    private TextView tiposDeRiego;
    private RadioGroup rg;
    private TextView btnIniciarRiego;
    private TextView btnIniciarCenso;
    private TextView btnIniciarMantenimiento;
    private TextView btnDetenerRiego;
    private EditText txtDuracion;
    private Button editButtonDuracion;
    private EditText txtIntensidad;
    private Button editButtonIntensidad;
    private ProgressBar progressBarCenso;
    private ProgressBar progressBarMant;

    SendCommand SC;

    public TabConfiguracionFragment() {

    }

    interface SendCommand {
        void iniciarMantenimiento();
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);

        try {
            SC = (SendCommand) getActivity();
        } catch (ClassCastException e) {
            throw new ClassCastException("Error in retrieving data. Please try again");
        }
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
        editButtonDuracion = root.findViewById(R.id.edit_btn_duracion);
        txtIntensidad = root.findViewById(R.id.txt_intensidad_riego);
        editButtonIntensidad = root.findViewById(R.id.edit_btn_intensidad);
        progressBarCenso = root.findViewById(R.id.progress_bar_censo);
        progressBarMant = root.findViewById(R.id.progress_bar_mant);

        mostrarComoDesconectado();

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
                SC.iniciarMantenimiento();
            }
        });

        btnDetenerRiego.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                detenerRiego();
            }
        });

        rg.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener()
        {
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(bandera) {
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

            boolean isToSave = false;

            @Override
            public void onClick(View v) {
                if(isToSave) {
                    // ahora se convierte en boton de editar
                    txtDuracion.setFocusable(false);
                    editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_edit);
                    isToSave = false;
                } else {
                    // ahora se convierte en boton de guardar cambios
                    txtDuracion.setFocusable(true);
                    editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_save);
                    isToSave = true;
                }
            }
        });

        editButtonIntensidad.setOnClickListener(new View.OnClickListener() {
            boolean isToSave = true;

            @Override
            public void onClick(View v) {
                if(isToSave) {
                    // ahora se convierte en boton de editar
                    txtIntensidad.setFocusable(false);
                    editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_edit);
                    isToSave = true;
                } else {
                    // ahora se convierte en boton de guardar cambios
                    txtIntensidad.setFocusable(true);
                    editButtonDuracion.setBackgroundResource(android.R.drawable.ic_menu_save);
                    isToSave = false;
                }
            }
        });

        txtDuracion.addTextChangedListener(new TextWatcher() {

            public void onTextChanged(CharSequence s, int start, int before,
                                      int count) {
                if(!s.equals("") ) {
                    //do your work here
                }
            }



            public void beforeTextChanged(CharSequence s, int start, int count,
                                          int after) {

            }

            public void afterTextChanged(Editable s) {

            }
        });

        txtIntensidad.addTextChangedListener(new TextWatcher() {

            public void onTextChanged(CharSequence s, int start, int before,
                                      int count) {
                if(!s.equals("") ) {
                    //do your work here
                }
            }



            public void beforeTextChanged(CharSequence s, int start, int count,
                                          int after) {

            }

            public void afterTextChanged(Editable s) {

            }
        });

        return root;
    }

    private void showToast(String msg) {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            Toast.makeText(getContext(), msg, Toast.LENGTH_LONG).show();
        });
    }

    private void mostrarComoDesconectado() {
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            // Inicialmente no se muestra
            tiposDeRiego.setVisibility(View.GONE);
            rg.setVisibility(View.GONE);
            // Inicialmente no puede accionar los botones
            btnIniciarRiego.setClickable(false);
            btnIniciarCenso.setClickable(false);
            btnIniciarMantenimiento.setClickable(false);
            btnDetenerRiego.setClickable(false);
        });
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
        bandera = false;
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            // set clickable to all buttons
            btnIniciarRiego.setClickable(true);
            btnIniciarCenso.setClickable(true);
            btnIniciarMantenimiento.setClickable(true);
            btnDetenerRiego.setClickable(true);
            tiposDeRiego.setVisibility(View.VISIBLE);
            rg.setVisibility(View.VISIBLE);

            Command tipoRiego = Command.valueOf(Integer.parseInt(values[0]));
            switch (tipoRiego) {
                case CAMBIAR_RIEGO_CONTINUO:
                    rg.check(R.id.radiobtn_continuo);
                    break;
                case CAMBIAR_RIEGO_INTERMITENTE:
                    rg.check(R.id.radiobtn_intermitente);
                    break;
            }
        });
    }

    @Override
    public void desconexion() {
        mostrarComoDesconectado();
    }

    @Override
    public void resultadoDetenerRiego(boolean ok) {
        if(ok) {
            showToast("Se han detenido correctamente los riegos en curso");
        } else {
            showToast("No se ha encontrado ningún riego en curso");
        }
    }

    @Override
    public void showErrorRiegoManual() {
        showToast("No se ha podido iniciar el riego manual. Intente más tarde.");
    }

    @Override
    public void showErrorMantenimiento() {
        showToast("No se ha podido iniciar el mantenimiento. Intente más tarde.");
    }

    @Override
    public void showErrorCensoManual() {
        showToast("No se ha podido iniciar el censo manual. Intente más tarde.");
    }

    @Override
    public void terminoRiegoManual() {
        showToast("El riego manual ha finalizado correctamente. Deslice a la derecha para ver los resultados");
    }

    @Override
    public void resultadoMantenimiento(String[] values) {
        showToast("El mantenimiento ha finalizado correctamente. Deslice a la izquierda para ver los resultados");
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            progressBarMant.setVisibility(View.GONE);
        });
    }

    @Override
    public void resultadoCenso(String[] values) {
        showToast("El censo manual ha finalizado correctamente. Deslice a la derecha para ver los resultados");
        Objects.requireNonNull(getActivity()).runOnUiThread(() -> {
            progressBarCenso.setVisibility(View.GONE);
        });
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
