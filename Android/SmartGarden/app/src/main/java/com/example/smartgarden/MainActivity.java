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

import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.Command;
import com.example.smartgarden.logic.Message;
import com.google.android.material.tabs.TabLayout;

import androidx.annotation.Nullable;
import androidx.viewpager.widget.ViewPager;
import androidx.appcompat.app.AppCompatActivity;

import com.example.smartgarden.ui.main.SectionsPagerAdapter;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Objects;

@SuppressLint("Registered")
public class MainActivity extends AppCompatActivity {

    private int conectionAttempts = 3;
    private TabLayout tabs;
    private Button btnConnect;

    // Sensores
    private SensorManager sensorManager;

    ///Variables para Shake
    private float acelVal; // valor actual de la aceleracion y gravedad
    private float acelLast; // ultimo valor de la aceleracion y gravedad
    private float shake; // diferencia de valor entre aceleracion y gravedad

    private static BTHandler btHandler = null;
    public int isArduinoConnected = 0; // 0 no, 1 si, 2 estableciendo conexion

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.activity_main);

        btHandler = new BTHandler();

        SectionsPagerAdapter sectionsPagerAdapter = new SectionsPagerAdapter(this, getSupportFragmentManager(), btHandler);
        tabs = findViewById(R.id.tabs);
        ViewPager viewPager = findViewById(R.id.view_pager);

        viewPager.setAdapter(sectionsPagerAdapter);
        tabs.setupWithViewPager(viewPager);
        populateviewPager();

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
                if(btnConnect.getText().equals(getString(R.string.btn_connect))) {
                    showDialogConnect();
                }
                else {
                    desconnect();
                }
            }
        });
    }

    public void populateviewPager() {

        Objects.requireNonNull(tabs.getTabAt(0)).setIcon(R.drawable.home_icon);
        Objects.requireNonNull(tabs.getTabAt(1)).setIcon(R.drawable.settings_icon);
        Objects.requireNonNull(tabs.getTabAt(2)).setIcon(R.drawable.maintenance_icon);
    }

    private void desconnect() {
        try {
            btHandler.desconnect();
        } catch (IOException ignored) {
        }
        setArduinoConnected(0);
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
                    eventAProx(event);
                    break;
                case Sensor.TYPE_ACCELEROMETER:
                    eventShake(event);
                    break;
            }
        }
    };

    public void showDialogConnect() {
        //primero verifico si el bluetooh esta habilitado, si no lo estoy pide que lo habilites
        if (!btHandler.isBluetoothEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, 1);
        } else
        {
            tryConnect();
        }
    }

    private void eventShake(SensorEvent event) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];

        acelLast = acelVal;
        acelVal = (float) Math.sqrt((double) (x * x + y * y + z * z));
        float delta = acelVal - acelLast;
        shake = shake * 0.9f + delta;

        if (shake > 12) {
            if (isArduinoConnected() == 1) {
                Toast.makeText(this, "Iniciando riego...", Toast.LENGTH_SHORT).show();
                ArrayList<String> values = new ArrayList<String>();
                String duration = "10000";
                String intensity = "50"; // %
                values.add(duration);
                values.add(intensity);
                if(!btHandler.sendMsg(new Message(Command.START, values))) {
                    setArduinoConnected(0);
                }
            } else if(isArduinoConnected() == 0){
                Toast.makeText(this, "Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void eventAProx(SensorEvent event) {
        if (event.values[0] < event.sensor.getMaximumRange()) {
            // Detected something nearby
            if (isArduinoConnected() == 1) {
                Toast.makeText(this, "Deteniendo riego...", Toast.LENGTH_SHORT).show();
                if(!btHandler.sendMsg(new Message(Command.STOP))) {
                    setArduinoConnected(0);
                }
            } else if(isArduinoConnected() == 0){
                Toast.makeText(this, "Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG).show();
            }
        }
    }

    public void tryConnect() {
        setArduinoConnected(2);
        Toast.makeText(this, "Intentando establecer conexión...", Toast.LENGTH_SHORT).show();
        Thread thread = new Thread() {
            @Override
            public void run() {
                //llama al metodo conectar de la clase bluetooh, si se conecta setea el flag de modo de trabajo arduino
                if (btHandler.connect()) {
                    setArduinoConnected(1);
                    runOnUiThread(() -> Toast.makeText(MainActivity.this, "Conexión con arduino exitosa", Toast.LENGTH_SHORT).show());
                } else {
                    setArduinoConnected(0);
                    runOnUiThread(() -> Toast.makeText(MainActivity.this, "Conexión con arduino fallida", Toast.LENGTH_SHORT).show());
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
        if (isArduinoConnected() == 1) {
            desconnect();
        }
    }

    public void setArduinoConnected(int arduinoConnected) {
        if(arduinoConnected == 0) {
            conectionAttempts--;
            runOnUiThread(() -> btnConnect.setText(R.string.btn_connect));
        }
        if (arduinoConnected == 1) {
            conectionAttempts = 3;
            runOnUiThread(() -> btnConnect.setText(R.string.btn_desconnect));
        }
        if(conectionAttempts == 0) {
            Toast.makeText(this, "Problemas de conexión con SmartGarden. Se cerrará la aplicación", Toast.LENGTH_LONG).show();
            new Handler().postDelayed(new Runnable(){
                @Override
                public void run(){
                    finish();
                };
            }, 3000);
            finish();
        }
        isArduinoConnected = arduinoConnected;
    }

    public int isArduinoConnected() {
        return isArduinoConnected;
    }
}