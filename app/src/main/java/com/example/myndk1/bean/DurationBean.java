package com.example.myndk1.bean;

public class DurationBean {
    private int duration;
    private int currentDuration;

    public DurationBean() {
    }

    public DurationBean(int duration, int currentDuration) {
        this.duration = duration;
        this.currentDuration = currentDuration;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public int getCurrentDuration() {
        return currentDuration;
    }

    public void setCurrentDuration(int currentDuration) {
        this.currentDuration = currentDuration;
    }
}
