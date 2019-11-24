package com.example.smartgarden.logic;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.MessageQueue;

import com.example.smartgarden.MainActivity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
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

    private ReceiveMessagesThread myReceiveMessagesThread;
    private SendMessagesThread mySendMessagesThread;

    // Identificador unico de servicio - SPP UUID
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    public static final int handlerState = 0;

    public boolean connect() {
        String MAC_ADRESS_JONI = "20:16:04:18:17:73";
        BluetoothDevice device = btAdapter.getRemoteDevice(MAC_ADRESS_JONI);

        try {
            btSocket = device.createRfcommSocketToServiceRecord(BTMODULEUUID);
            btSocket.connect();

        } catch (IOException e) {
            if (btSocket != null) {
                try {
                    btSocket.close();
                } catch (IOException e1) {
                    btSocket = null;
                    return false;
                }
            }
            btSocket = null;
            return false;
        }

        InputStream inStream = null;
        OutputStream outStream = null;

        try {
            inStream = btSocket.getInputStream();
            outStream = btSocket.getOutputStream();
        } catch (IOException ignored2) {
            return false;
        }

        myReceiveMessagesThread = new ReceiveMessagesThread(inStream);
        myReceiveMessagesThread.start();
        mySendMessagesThread = new SendMessagesThread(outStream);
        mySendMessagesThread.start();
        sendMsg(new Message(Command.CONEXION));

        return true;
    }

    public void sendDesconnect() {
        sendMsg(new Message(Command.DESCONEXION));
        //btSocket.close();
    }

    public void desconnect() throws IOException {
        btSocket.close();
    }

    public boolean isBluetoothEnabled() {
        return btAdapter.isEnabled();
    }

    public void sendMsg(Message msg) {
        mySendMessagesThread.write(msg.toString());
    }

    public class ReceiveMessagesThread extends Thread
    {

        private InputStream mmInStream;

        ReceiveMessagesThread(InputStream inputStream)
        {
            mmInStream = inputStream;
        }

        public void run()
        {
            // Se mantiene en modo escucha para determinar el ingreso de datos
            while (true) {
                String readMessage = readUntilEndMsg();
                if(readMessage != null) {
                    MainActivity.bluetoothIN.obtainMessage(handlerState, -1, -1, readMessage).sendToTarget();
                }
            }
        }

        private String readUntilEndMsg() {
            StringBuilder sb = new StringBuilder();

            try {
                BufferedReader buffer=new BufferedReader(new InputStreamReader(mmInStream));

                int r;
                while ((r = buffer.read()) != -1) {
                    char c = (char) r;

                    if (c == '\n'){
                        sb.append(c);
                        break;
                    }

                    sb.append(c);
                }
            } catch(IOException e) {
                return null;
            }

            return sb.toString();
        }

    }

    public class SendMessagesThread extends Thread {
        private OutputStream mmOutStream;
        private String input;


        public SendMessagesThread(OutputStream outputStream){
            mmOutStream = outputStream;
            input = null;
        }

        public void run() {
            while(true) {
                if (input != null) {
                    try {
                        mmOutStream.write(input.getBytes());
                        input = null;
                    } catch (IOException ignored) {
                    }
                }
            }
        }

        //Envio de trama
        void write(String input)
        {
            this.input = input;
        }

    }
}
