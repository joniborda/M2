package com.example.smartgarden.logic;

import android.hardware.SensorEvent;
import android.widget.Toast;

import java.util.ArrayList;

public class SensorEventHandler {

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
}
