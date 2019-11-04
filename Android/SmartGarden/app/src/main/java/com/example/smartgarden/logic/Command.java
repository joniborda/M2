package com.example.smartgarden.logic;

public enum Command {
    WATERING_RESULTS("9"),
    STATE("10"),
    STOP("9"),
    STANDARDS_PARAM("12"),
    START("7"),
    NO_WATERING("14"),
    MAINTENANCE("15");


    private String value;

    Command(String value) {
            this.value = value;
    }

    @Override
    public String toString() {
        return value;
    }
}
