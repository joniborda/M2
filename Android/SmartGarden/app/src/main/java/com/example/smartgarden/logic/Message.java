package com.example.smartgarden.logic;

import java.util.ArrayList;

public class Message {

        private Command command;
        private ArrayList<String> values;

        private Message(Command command, ArrayList<String> values) {
            this.command = command;
            this.values  = values;
        }

        @Override
        public String toString() {
            StringBuilder toString = new StringBuilder(String.format("<%s", command.toString()));
            for(int i = 0; i < values.size(); i++) {
                toString.append(String.format(",%s", values.get(i)));
            }
            return String.format("%s>", toString);
        }

}
