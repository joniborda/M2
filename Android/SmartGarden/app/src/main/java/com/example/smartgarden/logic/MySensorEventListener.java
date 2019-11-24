package com.example.smartgarden.logic;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.Toast;

import com.example.smartgarden.MainActivity;

public class MySensorEventListener implements SensorEventListener {

    MainActivity main;
    private int intentosToStop;
    
    ///Variables para Shake
    private float acelVal; // valor actual de la aceleracion y gravedad
    private float acelLast; // ultimo valor de la aceleracion y gravedad
    private float shake; // diferencia de valor entre aceleracion y gravedad

    public MySensorEventListener(MainActivity main) {
        this.main = main;
        intentosToStop = 0;
        acelVal = SensorManager.GRAVITY_EARTH;
        acelLast = SensorManager.GRAVITY_EARTH;
        shake = 0.00f;
    }

    public static boolean eventAcceletometer(SensorEvent event, float acelLast, float acelVal, float shake) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];

        acelLast = acelVal;
        acelVal = (float) Math.sqrt((double) (x * x + y * y + z * z));
        float delta = acelVal - acelLast;
        shake = shake * 0.9f + delta;

        return shake > 12;
    }

    public static boolean eventProx(SensorEvent event) {
        return event.values[0] < event.sensor.getMaximumRange();
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        switch (event.sensor.getType()) {
            case Sensor.TYPE_PROXIMITY:
                if(MySensorEventListener.eventProx(event)) {
                    // Detected something nearby
                    if (main.getArduinoStatus() == ArduinoStatus.Connected) {
                        intentosToStop++;
                        if(intentosToStop == 1) {
                            BTHandler.getInstance().sendMsg(new Message(Command.DETENER_RIEGO));
                        } else if(intentosToStop > 10) {
                            intentosToStop = 0;
                        }
                    } else if(main.getArduinoStatus() == ArduinoStatus.Desconnected){
                        main.showToast("Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG);
                    }
                }
                break;
            case Sensor.TYPE_ACCELEROMETER:
                if(MySensorEventListener.eventAcceletometer(event, acelLast, acelVal, shake)) {
                    if (main.getArduinoStatus() == ArduinoStatus.Connected) {
                        RiegoStandard riego = MainActivity.dbHelper.getRiegoStandard();
                        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_RIEGO, riego));
                    } else if (main.getArduinoStatus() == ArduinoStatus.Desconnected) {
                        main.showToast("Debe iniciar una conexión con SmartGarden", Toast.LENGTH_LONG);
                    }
                }
                break;
        }
    }
}
