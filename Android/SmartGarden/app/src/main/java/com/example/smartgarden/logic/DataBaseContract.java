package com.example.smartgarden.logic;

import android.provider.BaseColumns;

public class DataBaseContract {

    public static abstract class RiegoEntry implements BaseColumns {

        public static final String TABLE_NAME ="datos_riego";

        public static final String ZONA_ID = "zona_id";
        public static final String INTENSITY = "intensity";
        public static final String DURATION = "duration";
        public static final String HUM_SUELO = "hum_suelo";
    }

    public static abstract class ParametrosEntry implements BaseColumns {
        public static final String TABLE_NAME ="datos_parametros";

        public static final String ID = "id";
        public static final String DURATION_STANDARD = "duration";
    }
}
