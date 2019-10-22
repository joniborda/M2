package com.example.smartgarden;

import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Vibrator;
import android.widget.Toast;

import com.example.smartgarden.logic.BTHandler;
import com.google.android.material.tabs.TabLayout;
import androidx.viewpager.widget.ViewPager;
import androidx.appcompat.app.AppCompatActivity;
import com.example.smartgarden.ui.main.SectionsPagerAdapter;

import java.util.Objects;

public class MainActivity extends AppCompatActivity implements SensorEventListener {

    private TabLayout tabs;

    // Sensores
    private SensorManager sensorManager;
    private Sensor sensorShake;
    private Sensor sensorProx;

    ///Variables para Shake
    private float acelVal; // valor actual de la aceleracion y gravedad
    private float acelLast; // ultimo valor de la aceleracion y gravedad
    private float shake; // diferencia de valor entre aceleracion y gravedad
    private Vibrator v;

    public static BTHandler btHandler = null;
    public static boolean isShacking;

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

        ///Vibrador para el sensor shake
        v = (Vibrator) getSystemService(VIBRATOR_SERVICE);

        ///Asigno sensores
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensorShake = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        sensorProx = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);

        //Asigno listeners a sensores
        sensorManager.registerListener(this, sensorShake, SensorManager.SENSOR_DELAY_GAME);
        sensorManager.registerListener(this, sensorProx, SensorManager.SENSOR_DELAY_GAME);

        acelVal = SensorManager.GRAVITY_EARTH;
        acelLast = SensorManager.GRAVITY_EARTH;
        shake = 0.00f;

    }

    private void populateviewPager() {

        Objects.requireNonNull(tabs.getTabAt(0)).setIcon(R.drawable.home_icon);
        Objects.requireNonNull(tabs.getTabAt(1)).setIcon(R.drawable.settings_icon);
        Objects.requireNonNull(tabs.getTabAt(2)).setIcon(R.drawable.maintenance_icon);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        switch(event.sensor.getType()){
            case Sensor.TYPE_PROXIMITY:
                eventAProx(event);
                break;
            case Sensor.TYPE_ACCELEROMETER:
                eventShake(event)
                ;break;
        }
    }

    private void eventShake(SensorEvent event) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];

        acelLast=acelVal;
        acelVal= (float) Math.sqrt((double) (x*x + y*y + z*z));
        float delta = acelVal-acelLast;
        shake = shake * 0.9f + delta;

        if(shake>12) {
            Toast.makeText(this, "Shake event", Toast.LENGTH_LONG).show();
        }
    }

    private void eventAProx(SensorEvent event) {
        if(event.values[0] == event.sensor.getMaximumRange())
            Toast.makeText(this, "Proximity sensor changed", Toast.LENGTH_LONG).show();
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        sensorManager.unregisterListener(this);
    }
}