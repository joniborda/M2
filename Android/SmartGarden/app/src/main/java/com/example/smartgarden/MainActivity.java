package com.example.smartgarden;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.example.smartgarden.logic.ArduinoStatus;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;
import com.example.smartgarden.logic.DBHelper;
import com.example.smartgarden.logic.HandlerMessage;
import com.example.smartgarden.logic.Message;
import com.example.smartgarden.logic.RiegoStandard;
import com.example.smartgarden.logic.SensorEventHandler;
import com.example.smartgarden.ui.main.TabConfiguracionFragment;
import com.example.smartgarden.ui.main.TabMantenimientoFragment;
import com.google.android.material.tabs.TabLayout;

import androidx.annotation.Nullable;
import androidx.viewpager.widget.ViewPager;
import androidx.appcompat.app.AppCompatActivity;

import com.example.smartgarden.ui.main.SectionsPagerAdapter;

import java.io.IOException;
import java.util.Objects;

@SuppressLint("Registered")
public class MainActivity extends AppCompatActivity {

    private int conectionAttempts = 3;
    private TabLayout tabs;
    private Button btnConnect;
    public static HandlerMessage bluetoothIN;
    public static DBHelper dbHelper;

    // Sensores
    private SensorManager sensorManager;

    ///Variables para Shake
    private float acelVal; // valor actual de la aceleracion y gravedad
    private float acelLast; // ultimo valor de la aceleracion y gravedad
    private float shake; // diferencia de valor entre aceleracion y gravedad

    public static ArduinoStatus arduinoStatus = ArduinoStatus.Desconnected; // 0 no, 1 si, 2 estableciendo conexion


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.activity_main);

        SectionsPagerAdapter sectionsPagerAdapter = new SectionsPagerAdapter(this, getSupportFragmentManager());
        tabs = findViewById(R.id.tabs);
        ViewPager viewPager = findViewById(R.id.view_pager);

        viewPager.setAdapter(sectionsPagerAdapter);
        tabs.setupWithViewPager(viewPager);
        populateviewPager();

        bluetoothIN = new HandlerMessage(sectionsPagerAdapter.getItems());
        dbHelper = new DBHelper(this);

        ///Asigno sensores
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        Sensor sensorShake = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        Sensor sensorProx = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);

        //Asigno listeners a sensores
        sensorManager.registerListener(sensorChangedEventListener, sensorShake, SensorManager.SENSOR_DELAY_GAME);
        sensorManager.registerListener(sensorChangedEventListener, sensorProx, SensorManager.SENSOR_DELAY_GAME);

        acelVal = SensorManager.GRAVITY_EARTH;
        acelLast = SensorManager.GRAVITY_EARTH;
        shake = 0.00f;

        btnConnect = findViewById(R.id.btn_connect);
        btnConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(getArduinoStatus() == ArduinoStatus.Desconnected) {
                    showDialogConnect();
                }
                else {
                    desconnect();
                }
            }
        });
    }

    private void populateviewPager() {
        Objects.requireNonNull(tabs.getTabAt(0)).setIcon(R.drawable.home_icon);
        Objects.requireNonNull(tabs.getTabAt(1)).setIcon(R.drawable.settings_icon);
        Objects.requireNonNull(tabs.getTabAt(2)).setIcon(R.drawable.maintenance_icon);
    }

    private void desconnect() {
        try {
            BTHandler.getInstance().desconnect();
        } catch (IOException ignored) {
        }
        setArduinoStatus(ArduinoStatus.Desconnected);
    }

    SensorEventListener sensorChangedEventListener
            = new SensorEventListener() {
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // TODO Auto-generated method stub
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            switch (event.sensor.getType()) {
                case Sensor.TYPE_PROXIMITY:
                    if(SensorEventHandler.eventProx(event)) {
                        // Detected something nearby
                        if (getArduinoStatus() == ArduinoStatus.Connected) {
                            BTHandler.getInstance().sendMsg(new Message(Command.DETENER_RIEGO));
                        } else if(getArduinoStatus() == ArduinoStatus.Desconnected){
                            showToast("Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG);
                        }
                    }
                    break;
                case Sensor.TYPE_ACCELEROMETER:
                    if(SensorEventHandler.eventAcceletometer(event, acelLast, acelVal, shake)) {
                        if (getArduinoStatus() == ArduinoStatus.Connected) {
                            RiegoStandard riego = MainActivity.dbHelper.getRiegoStandard();
                            BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_RIEGO, riego));
                        } else if (getArduinoStatus() == ArduinoStatus.Desconnected) {
                            showToast("Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG);
                        }
                    }
                    break;
            }
        }
    };

    public void showToast(String message, int length) {
        Toast.makeText(this, message, length).show();
    }

    public void showDialogConnect() {
        //primero verifico si el bluetooh esta habilitado, si no lo estoy pide que lo habilites
        if (!BTHandler.getInstance().isBluetoothEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, 1);
        } else
        {
            tryConnect();
        }
    }

    public void tryConnect() {
        setArduinoStatus(ArduinoStatus.AttemptingToConnect);
        btnConnect.setEnabled(false);
        Toast.makeText(this, "Intentando establecer conexión...", Toast.LENGTH_SHORT).show();

        Thread thread = new Thread() {
            @Override
            public void run() {
                //llama al metodo conectar de la clase bluetooh, si se conecta setea el flag de modo de trabajo arduino
                if (BTHandler.getInstance().connect()) {
                    conectionAttempts = 3;
                    setArduinoStatus(ArduinoStatus.Connected);
                    runOnUiThread(new Runnable(){
                        public void run() {
                            btnConnect.setEnabled(true);
                            showToast("Conexión con arduino exitosa", Toast.LENGTH_SHORT);
                        }
                    });
                } else {
                    conectionAttempts--;
                    setArduinoStatus(ArduinoStatus.Desconnected);
                    runOnUiThread(new Runnable(){
                        public void run() {
                            btnConnect.setEnabled(true);
                            if(conectionAttempts == 0) {
                                showToast("Problemas de conexión con SmartGarden. Se cerrará la aplicación...", Toast.LENGTH_LONG);
                                new Handler().postDelayed(new Runnable(){
                                    @Override
                                    public void run(){
                                        finish();
                                    };
                                }, 3000);
                            } else {
                                showToast("Conexión con arduino fallida", Toast.LENGTH_SHORT);
                            }
                        }
                    });
                }
            }
        };
        thread.start();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        //metodo que cuando recibe que me conecte al bluetooh, trata de conectarse al arduino
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 1 && resultCode == RESULT_OK) {
            tryConnect();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        sensorManager.unregisterListener(sensorChangedEventListener);
        if (getArduinoStatus() == ArduinoStatus.Connected) {
            desconnect();
        }
    }

    public void setArduinoStatus(ArduinoStatus arduinoStatus) {
        if(arduinoStatus == ArduinoStatus.Desconnected) {
            runOnUiThread(() -> btnConnect.setText(R.string.btn_connect));
        }
        if (arduinoStatus == ArduinoStatus.Connected) {
            runOnUiThread(() -> btnConnect.setText(R.string.btn_desconnect));
        }
        MainActivity.arduinoStatus = arduinoStatus;
    }

    public ArduinoStatus getArduinoStatus() {
        return arduinoStatus;
    }

}