package com.example.smartgarden.logic;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class BTHandler {

    private BluetoothAdapter btAdapter;
    private BluetoothSocket btSocket = null;

    private ConnectedThread MyConexionBT;
    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private boolean isBtConnected = false;


    public BTHandler(){

        //En el constructor solo obtengo el adapter
        btAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public boolean Connect() {
        String MAC_ADRESS = "00:21:13:00:A2:13";
        BluetoothDevice device = btAdapter.getRemoteDevice(MAC_ADRESS);

        try {
            btSocket = device.createRfcommSocketToServiceRecord(BTMODULEUUID);
            try {
                btSocket.connect();
            } catch (IOException ignored) {
                btSocket.close();
            }
        } catch (IOException e) {
            return isBtConnected;
        }

        MyConexionBT = new ConnectedThread(btSocket);
        MyConexionBT.start();

        //btConeccted la uso para saber si estoy conectado al arduino
        isBtConnected = true;
        //Le mando un 1 al arduino para que sepa que me conecto por bluetooth
        MyConexionBT.write("<16,1>");
        return isBtConnected;

    }

    public void Desconnect() {
        //le mando 0 para que sepa que no desconecto y cierro el socket

        MyConexionBT.write("<16,0>");

        try {
            btSocket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public boolean checkStateBT() {
        return isBtConnected;
    }


    //Crea la clase que permite crear el evento de conexion
    public class ConnectedThread extends Thread
    {
        private InputStream mmInStream;
        private OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket)
        {
            try
            {
                mmInStream = socket.getInputStream();
                mmOutStream = socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
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
        public void write(String input)
        {
            try {
                mmOutStream.write(input.getBytes());
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }
}
