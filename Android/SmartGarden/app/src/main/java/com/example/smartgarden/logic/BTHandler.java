package com.example.smartgarden.logic;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import com.example.smartgarden.MainActivity;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class BTHandler {

    private static volatile BTHandler instance = new BTHandler();

    public static BTHandler getInstance() {
        return instance;
    }
    private BTHandler(){
        //En el constructor solo obtengo el adapter
        btAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    private BluetoothAdapter btAdapter;
    private BluetoothSocket btSocket = null;

    private ConnectedThread myConexionBT;
    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    public static final int handlerState = 0;

    public boolean connect() {
        boolean isConnected = true;
        String MAC_ADRESS = "98:D3:31:F7:41:F5";
        String MAC_ADRESS_JONI = "20:16:04:18:17:63";

        BluetoothDevice device1 = btAdapter.getRemoteDevice(MAC_ADRESS);
        BluetoothDevice device2 = btAdapter.getRemoteDevice(MAC_ADRESS_JONI);

        if(notConnect(device1)) {
            if(notConnect(device2)) {
                isConnected = false;
            }
        }

        if(btSocket != null) {
            InputStream inStream = null;
            OutputStream outStream = null;

            try {
                inStream = btSocket.getInputStream();
                outStream = btSocket.getOutputStream();
            } catch (IOException ignored2) {
                isConnected = false;
            }

            if(inStream != null && outStream != null) {

                myConexionBT = new ConnectedThread(inStream, outStream);
                myConexionBT.start();
                sendMsg(new Message(Command.CONEXION));
            }
        }

        return isConnected;
    }

    private boolean notConnect(BluetoothDevice device) {
        try {
            btSocket = device.createRfcommSocketToServiceRecord(BTMODULEUUID);
            btSocket.connect();
            return false;
        } catch (IOException e) {
            btSocket = null;
            return true;
        }
    }

    public void desconnect() throws IOException {
        sendMsg(new Message(Command.DESCONEXION));
        btSocket.close();
    }

    public boolean isBluetoothEnabled() {
        return btAdapter.isEnabled();
    }

    public void sendMsg(Message msg) {
        myConexionBT.write(msg.toString());
    }

    //Crea la clase que permite crear el evento de conexion
    public class ConnectedThread extends Thread
    {

        private InputStream mmInStream;
        private OutputStream mmOutStream;

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
                    MainActivity.bluetoothIN.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                    } catch (IOException e) {
                    break;
                }
            }
        }

        //Envio de trama
        void write(String input)
        {
            try {
                mmOutStream.write(input.getBytes());
            }
            catch (IOException ignored){}
        }
    }
}
