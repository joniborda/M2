package com.example.smartgarden.logic;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DBHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "data.db";
    private static final int ID_DURATION = 0;

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

        db.execSQL("CREATE TABLE " + DataBaseContract.ParametrosEntry.TABLE_NAME + " ("
                +  DataBaseContract.ParametrosEntry.ID + " INTEGER PRIMARY KEY,"
                +  DataBaseContract.ParametrosEntry.DURATION_STANDARD + " INTEGER NOT NULL,"
                + "UNIQUE (" +  DataBaseContract.ParametrosEntry.ID + "))");

        Riego riego1 = new Riego(1);
        Riego riego2 = new Riego(2);

        db.insert(DataBaseContract.RiegoEntry.TABLE_NAME, null, riego1.toContentValues());
        db.insert(DataBaseContract.RiegoEntry.TABLE_NAME, null, riego2.toContentValues());

        ContentValues values = new ContentValues();

        // Pares clave-valor
        values.put(DataBaseContract.ParametrosEntry.ID, ID_DURATION);
        values.put(DataBaseContract.ParametrosEntry.DURATION_STANDARD, 10);

        // Insertar...
        db.insert(DataBaseContract.ParametrosEntry.TABLE_NAME, null, values);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // No hay operaciones
    }

    public long setDuracion(int duracion) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.ParametrosEntry.ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(ID_DURATION)};

        ContentValues values = new ContentValues();
        values.put(DataBaseContract.ParametrosEntry.DURATION_STANDARD, duracion);

        // Actualizar
        long vRet = db.update(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                values,
                selection,
                selectionArgs);

        db.close();

        return vRet;
    }

    public int getDuracion() {
        int duracion = -1;

        SQLiteDatabase db = getReadableDatabase();
        String[] columns = new String[]{DataBaseContract.ParametrosEntry.DURATION_STANDARD};
        String selection = DataBaseContract.ParametrosEntry.ID + " LIKE ?"; // WHERE id LIKE ?
        String[] selectionArgs = new String[]{String.valueOf(ID_DURATION)};

        Cursor c = db.query(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                columns,
                selection,
                selectionArgs,
                null,
                null,
                null
        );

        if(c.moveToNext()){
            duracion = Integer.parseInt(c.getString(c.getColumnIndex(DataBaseContract.ParametrosEntry.DURATION_STANDARD)));
        }

        c.close();
        db.close();

        return duracion;
    }

    public long setComienzoRiego(Riego riego) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoEntry.ZONA_ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(riego.getNroZona())};

        // Actualizar
        long vRet = db.update(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                riego.toContentValues(),
                selection,
                selectionArgs);

        db.close();

        return vRet;
    }

    public long setResultadoRiego(Riego riego) {
        SQLiteDatabase db = getWritableDatabase();

        // WHERE
        String selection = DataBaseContract.RiegoEntry.ZONA_ID + " LIKE ?";
        String[] selectionArgs = {String.valueOf(riego.getNroZona())};

        // Actualizar
        long vRet = db.update(
                DataBaseContract.RiegoEntry.TABLE_NAME,
                riego.HumedadSuelotoContenValues(),
                selection,
                selectionArgs);

        db.close();

        return vRet;
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

        if(c.moveToNext()){
            String intensity = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.INTENSITY));
            if(c.moveToNext()) {
                String duration = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.DURATION));
                if(c.moveToNext()) {
                    String humSuelo = c.getString(c.getColumnIndex(DataBaseContract.RiegoEntry.HUM_SUELO));
                    riego = new Riego(zonaID);
                    riego.setIntensidad(Integer.parseInt(intensity));
                    riego.setDuracion(Integer.parseInt(duration));
                    riego.setHumSueloResultado(Float.parseFloat(humSuelo));
                }
            }
        }

        c.close();
        db.close();

        return riego;
    }

}
