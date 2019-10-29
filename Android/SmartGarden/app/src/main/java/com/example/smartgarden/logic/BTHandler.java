package com.example.smartgarden.logic;

import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.widget.Toast;

import com.example.smartgarden.MainActivity;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.UUID;

public class BTHandler{

    private BluetoothAdapter btAdapter;
    private BluetoothSocket btSocket = null;

    private ConnectedThread myConexionBT;
    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");


    public BTHandler(){
        //En el constructor solo obtengo el adapter
        btAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public boolean connect() {
        String MAC_ADRESS = "98:D3:31:F7:41:F5";
        BluetoothDevice device = btAdapter.getRemoteDevice(MAC_ADRESS);

        if(device != null)
            try {
                btSocket = device.createRfcommSocketToServiceRecord(BTMODULEUUID);
            } catch (IOException e) {
                btSocket = null;
                return false;
            }

        if(btSocket != null) {
            try {
                btSocket.connect();
            } catch (IOException e) {
                return false;
            }

            InputStream inStream = null;
            OutputStream outStream = null;

            try {
                inStream = btSocket.getInputStream();
                outStream = btSocket.getOutputStream();
            } catch (IOException e) {
                return false;
            }

            if(inStream != null && outStream != null) {

                myConexionBT = new ConnectedThread(inStream, outStream);
                myConexionBT.start();
                //Le mando un 1 al arduino para que sepa que me conecto por bluetooth
                //MyConexionBT.write("<16,1>");
                return true;
            }
        }

        return false;
    }

    public void desconnect() throws IOException {
        //le mando 0 para que sepa que no desconecto y cierro el socket
        //MyConexionBT.write("<16,0>");
        btSocket.close();
    }

    public boolean isBluetoothEnabled() {
        return btAdapter.isEnabled();
    }

    public boolean sendMsg(Message msg) {
        return myConexionBT.write(msg.toString());
    }

    //Crea la clase que permite crear el evento de conexion
    public class ConnectedThread extends Thread
    {
        private InputStream mmInStream;
        private OutputStream mmOutStream;
        private int cantMsg = 0;

        ConnectedThread(InputStream inStream, OutputStream outStream)
        {
            mmInStream = inStream;
            mmOutStream = outStream;
        }

        public void run()
        {
            byte[] buffer = new byte[256];
            int bytes;

            // Se mantiene en modo escucha para determinar el ingreso de datos
            while (true) {
                try {
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                } catch (IOException e) {
                    break;
                }
            }
        }

        //Envio de trama
        boolean write(String input)
        {
            try {
                mmOutStream.write(input.getBytes());
                return true;
            }
            catch (IOException e)
            {
                return false;
            }
        }
    }
}
