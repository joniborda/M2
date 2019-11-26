package com.example.smartgarden.logic;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import com.example.smartgarden.MainActivity;

import android.os.Handler;
import android.widget.Toast;

public class MySensorEventListener implements SensorEventListener {

    private MainActivity main;
    private Handler handler;
    private int intentosToStop;
    
    ///Variables para Shake
    private float acelVal; // valor actual de la aceleracion y gravedad
    private float acelLast; // ultimo valor de la aceleracion y gravedad
    private float shake; // diferencia de valor entre aceleracion y gravedad

    public MySensorEventListener(MainActivity main, Handler handler) {
        this.main = main;
        intentosToStop = 0;
        acelVal = SensorManager.GRAVITY_EARTH;
        acelLast = SensorManager.GRAVITY_EARTH;
        shake = 0.00f;
        this.handler = handler;
    }

    private boolean eventAcceletometer(SensorEvent event) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];

        acelLast = acelVal;
        acelVal = (float) Math.sqrt((double) (x * x + y * y + z * z));
        float delta = acelVal - acelLast;
        shake = shake * 0.9f + delta;

        return shake > 12;
    }

    private boolean eventProx(SensorEvent event) {
        return event.values[0] < event.sensor.getMaximumRange();
    }

    private boolean eventLuz(SensorEvent event) {
        // No hay luz
        return event.values[0] < 5;
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        switch (event.sensor.getType()) {
            case Sensor.TYPE_PROXIMITY:
                if(eventProx(event)) {
                    // Detected something nearby
                    if (main.getArduinoStatus() == ArduinoStatus.Connected) {
                        intentosToStop++;
                        if(intentosToStop == 1) {
                            BTHandler.getInstance().sendMsg(new Message(Command.DETENER_RIEGO));
                        } else if(intentosToStop > 10) {
                            intentosToStop = 0;
                        }
                    }
                }
                break;
            case Sensor.TYPE_ACCELEROMETER:
                if(eventAcceletometer(event)) {
                    if (main.getArduinoStatus() == ArduinoStatus.Connected) {
                        RiegoStandard riego = MainActivity.dbHelper.getRiegoStandard();
                        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_RIEGO, riego));
                        main.showToast("Riego iniciado", Toast.LENGTH_LONG);
                    }
                }
                break;
            case Sensor.TYPE_LIGHT:
                if(eventLuz(event)) {
                    if (main.getArduinoStatus() == ArduinoStatus.Connected &&
                            MainActivity.mantenimientoStatus != MantenimientoStatus.InProgress) {
                        handler.obtainMessage(0, -1, -1, null).sendToTarget();
                        BTHandler.getInstance().sendMsg(new Message(Command.INICIAR_MANTENIMIENTO));
                        main.showToast("Mantenimiento iniciado", Toast.LENGTH_LONG);
                    }

                }
        }
    }
}
