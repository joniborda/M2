package com.example.smartgarden;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.example.smartgarden.logic.ArduinoStatus;
import com.example.smartgarden.logic.BTHandler;
import com.example.smartgarden.logic.DBHelper;
import com.example.smartgarden.logic.HandlerMessage;
import com.example.smartgarden.logic.MantenimientoStatus;
import com.example.smartgarden.logic.MySensorEventListener;
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
    public static Handler bluetoothIN;
    public static DBHelper dbHelper;
    public static MantenimientoStatus mantenimientoStatus;

    // Sensores
    private SensorManager sensorManager;
    private HandlerThread mSensorThread;
    private Handler mSensorHandler;
    private MySensorEventListener mySensorEventListener;

    public static ArduinoStatus arduinoStatus = ArduinoStatus.Desconnected; // 0 no, 1 si, 2 estableciendo conexion

    @SuppressLint("HandlerLeak")
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

        bluetoothIN = new HandlerMessage(sectionsPagerAdapter.getItems(), this);

        dbHelper = new DBHelper(this);

        ///Asigno sensores
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        Sensor sensorShake = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        Sensor sensorProx = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        Sensor sensorLight = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);

        mySensorEventListener = new MySensorEventListener(this, sectionsPagerAdapter.getHandler());

        mSensorThread = new HandlerThread("Sensor thread", Thread.MAX_PRIORITY);
        mSensorThread.start();
        mSensorHandler = new Handler(mSensorThread.getLooper()); //Blocks until looper is prepared, which is fairly quick

        //Asigno listeners a sensores
        sensorManager.registerListener(mySensorEventListener, sensorShake, SensorManager.SENSOR_DELAY_NORMAL, mSensorHandler);
        sensorManager.registerListener(mySensorEventListener, sensorProx, SensorManager.SENSOR_DELAY_NORMAL, mSensorHandler);
        sensorManager.registerListener(mySensorEventListener, sensorLight, SensorManager.SENSOR_DELAY_NORMAL, mSensorHandler);

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
        BTHandler.getInstance().sendDesconnect();
    }

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
                    //setArduinoStatus(ArduinoStatus.Connected);
                    runOnUiThread(new Runnable(){
                        public void run() {
                            btnConnect.setEnabled(true);
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
        sensorManager.unregisterListener(mySensorEventListener);
        mSensorThread.quitSafely();
        if (getArduinoStatus() == ArduinoStatus.Connected) {
            try {
                BTHandler.getInstance().desconnect();
            } catch (IOException ignored) {
            }
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