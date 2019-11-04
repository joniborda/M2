package com.example.smartgarden.logic;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Observable;
import java.util.UUID;

public class BTHandler extends Observable {

    public BTHandler(){
        //En el constructor solo obtengo el adapter
        btAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public boolean isConnected = false;

    private BluetoothAdapter btAdapter;
    private BluetoothSocket btSocket = null;

    private ConnectedThread myConexionBT;
    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    public boolean connect() {
        String MAC_ADRESS = "98:D3:31:F7:41:F5";
        String MAC_ADRESS_JONI = "20:16:04:18:17:63";

        BluetoothDevice device1 = btAdapter.getRemoteDevice(MAC_ADRESS);
        BluetoothDevice device2 = btAdapter.getRemoteDevice(MAC_ADRESS_JONI);

        if(notConnect(device1)) {
            if(notConnect(device2)) {
                isConnected = false;
            }
        }

        InputStream inStream = null;
        OutputStream outStream = null;

        try {
            inStream = btSocket.getInputStream();
            outStream = btSocket.getOutputStream();
        } catch (IOException ignored2) {
            isConnected = false;
        }

        if(inStream != null && outStream != null) {

            isConnected = true;

            setChanged();
            notifyObservers();

            myConexionBT = new ConnectedThread(inStream, outStream);
            myConexionBT.start();
            //Le mando un 1 al arduino para que sepa que me conecto por bluetooth
            //MyConexionBT.write("<16,1>");
        }

        return isConnected;
    }

    private boolean notConnect(BluetoothDevice device) {

        try {
            btSocket = device.createRfcommSocketToServiceRecord(BTMODULEUUID);
            btSocket.connect();
            return true;
        } catch (IOException e) {
            btSocket = null;
            return false;
        }
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
