package com.example.smartgarden.logic;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DBHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "data.db";
    private static final int ID = 0;

    public DBHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        // Create table...
        db.execSQL("CREATE TABLE " + DataBaseContract.RiegoEntry.TABLE_NAME + " ("
                +  DataBaseContract.RiegoEntry.ZONA_ID + " INTEGER PRIMARY KEY,"
                +  DataBaseContract.RiegoEntry.INTENSITY + " INTEGER NOT NULL,"
                +  DataBaseContract.RiegoEntry.DURATION + " INTEGER NOT NULL,"
                +  DataBaseContract.RiegoEntry.HUM_SUELO + " REAL NOT NULL,"
                + "UNIQUE (" +  DataBaseContract.RiegoEntry.ZONA_ID + "))");

        db.execSQL("CREATE TABLE " + DataBaseContract.RiegoStandardEntry.TABLE_NAME + " ("
                +  DataBaseContract.RiegoStandardEntry.ID + " TEXT PRIMARY KEY,"
                +  DataBaseContract.RiegoStandardEntry.DURATION_STANDARD + " TEXT NOT NULL,"
                +  DataBaseContract.RiegoStandardEntry.INTENSITY_STANDARD + " TEXT NOT NULL,"
                + "UNIQUE (" +  DataBaseContract.RiegoStandardEntry.ID + "))");

        Riego riego1 = new Riego(1);
        Riego riego2 = new Riego(2);

        db.insert(DataBaseContract.RiegoEntry.TABLE_NAME, null, riego1.alltoContentValues());
        db.insert(DataBaseContract.RiegoEntry.TABLE_NAME, null, riego2.alltoContentValues());

        ContentValues values = new ContentValues();

        // Pares clave-valor
        values.put(DataBaseContract.RiegoStandardEntry.ID, ID);
        values.put(DataBaseContract.RiegoStandardEntry.DURATION_STANDARD, "10");
        values.put(DataBaseContract.RiegoStandardEntry.INTENSITY_STANDARD, "50");

        // Insertar...
        db.insert(DataBaseContract.RiegoStandardEntry.TABLE_NAME, null, values);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }

    public void setDuracion(int duracion) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoStandardEntry.ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(ID)};

        RiegoStandard riegoStandard = new RiegoStandard("", String.valueOf(duracion));

        // Actualizar
        db.update(
                DataBaseContract.RiegoStandardEntry.TABLE_NAME,
                riegoStandard.duraciontoContenValues(),
                selection,
                selectionArgs);

        db.close();
    }

    public void setIntensidad(int intensidad) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoStandardEntry.ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(ID)};

        RiegoStandard riegoStandard = new RiegoStandard(String.valueOf(intensidad), "");

        // Actualizar
        db.update(
                DataBaseContract.RiegoStandardEntry.TABLE_NAME,
                riegoStandard.intensidadtoContenValues(),
                selection,
                selectionArgs);

        db.close();
    }


    public RiegoStandard getRiegoStandard() {
        RiegoStandard riegoStandard = null;

        SQLiteDatabase db = getReadableDatabase();
        String[] columns = new String[]{DataBaseContract.RiegoStandardEntry.DURATION_STANDARD,
                                        DataBaseContract.RiegoStandardEntry.INTENSITY_STANDARD};
        String selection = DataBaseContract.RiegoStandardEntry.ID + " LIKE ?"; // WHERE id LIKE ?
        String[] selectionArgs = new String[]{String.valueOf(ID)};

        Cursor c = db.query(
                DataBaseContract.RiegoStandardEntry.TABLE_NAME,
                columns,
                selection,
                selectionArgs,
                null,
                null,
                null
        );

        if(c.moveToNext()){
            String duracion = c.getString(c.getColumnIndex(DataBaseContract.RiegoStandardEntry.DURATION_STANDARD));
            String intensidad = c.getString(c.getColumnIndex(DataBaseContract.RiegoStandardEntry.INTENSITY_STANDARD));
            riegoStandard = new RiegoStandard(intensidad, duracion);
        }

        c.close();
        db.close();

        return riegoStandard;
    }

    public void setComienzoRiego(Riego riego) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoEntry.ZONA_ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(riego.getNroZona())};

        // Actualizar
        db.update(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                riego.datosInicialestoContentValues(),
                selection,
                selectionArgs);

        db.close();
    }

    public void setResultadoRiego(Riego riego) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoEntry.ZONA_ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(riego.getNroZona())};

        // Actualizar
        db.update(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                riego.humedadSuelotoContenValues(),
                selection,
                selectionArgs);

        db.close();
    }

    public Riego getUltimoRiego(int zonaID) {
        Riego riego = null;

        SQLiteDatabase db = getReadableDatabase();
        String[] columns = new String[]{DataBaseContract.RiegoEntry.INTENSITY,
                                        DataBaseContract.RiegoEntry.DURATION,
                                        DataBaseContract.RiegoEntry.HUM_SUELO};
        String selection = DataBaseContract.RiegoEntry.ZONA_ID + " LIKE ?"; // WHERE id LIKE ?
        String[] selectionArgs = new String[]{String.valueOf(zonaID)};

        Cursor c = db.query(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                columns,
                selection,
                selectionArgs,
                null,
                null,
                null
        );

        if(c.moveToNext()) {
            String intensity = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.INTENSITY));
            String duration = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.DURATION));
            String humSuelo = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.HUM_SUELO));
            riego = new Riego(zonaID);
            riego.setIntensidad(Integer.parseInt(intensity));
            riego.setDuracion(Integer.parseInt(duration));
            riego.setHumSueloResultado(Float.parseFloat(humSuelo));
        }

        c.close();
        db.close();

        return riego;
    }

}
