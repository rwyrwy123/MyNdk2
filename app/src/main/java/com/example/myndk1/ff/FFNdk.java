package com.example.myndk1.ff;

import android.util.Log;

import com.example.myndk1.gl.RWGLSurfaceView;

public class FFNdk {
    static {
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("native-lib");
    }

    private IPrepareListenter prepareListenter;
    private IErrorListener errorListener;
    private IDurationListener durationListener;
    private IPauseListener pauseListener;
    private IStopListener stopListener;
    private Boolean next = false;
    private String path;
    private int duration = -1;
    private RWGLSurfaceView surfaceView;

    public int getDuration() {
        return duration;
    }

    public void setPrepareListenter(IPrepareListenter prepareListenter) {
        this.prepareListenter = prepareListenter;
    }

    public void setErrorListener(IErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setDurationListener(IDurationListener durationListener) {
        this.durationListener = durationListener;
    }

    public void setPauseListener(IPauseListener pauseListener) {
        this.pauseListener = pauseListener;
    }

    public void setStopListener(IStopListener stopListener) {
        this.stopListener = stopListener;
    }

    public void setSurfaceView(RWGLSurfaceView surfaceView) {
        this.surfaceView = surfaceView;
    }

    public void prepareCtJ(){
        duration = -1;
        if (prepareListenter != null){
            prepareListenter.onPrepared();
        }
    }

    public void errorCtJ(int code,String msg){
        if (errorListener != null){
            errorListener.onError(code,msg);
        }
    }

    public void durationCtJ(int duration,int currentDuration){
        if (this.duration < 0){
            this.duration = duration;
        }
        if (durationListener != null){
            durationListener.onDuration(duration,currentDuration);
        }
    }

    public void pauseCtJ(){
        if (pauseListener != null){
            pauseListener.onRwPause();
        }
    }

    public void stopCtJ(){
        if (stopListener != null){
            stopListener.onRwStop();
        }
        if (next){
            next = false;
            prepareJtC(path);
        }
    }

    public void yuvCtJ(int width,int height,byte[] y,byte[]u,byte[]v){
        if(surfaceView != null)
        {
            surfaceView.setYUVData(width, height, y, u, v);

            Log.d("MainActivity文件","width="+width+"height="+height+"y="+y.toString()+"u="+u.toString()+"v="+v.toString());
        }
    }

    public void prepareJtC(String path){
        prepare(path);
    }

    public void pauseJtC(){
        pause();
    }

    public void stopJtC(){
        stop();
    }

    public void nextJtC(String path){
        this.path = path;
        next = true;
        stopJtC();
    }

    public void pitchspeedJtC(double pitch,double speed){
        pitchspeed(pitch,speed);
    }

    public void volumeJtC(int volume){
        volume(volume);
    }

    public void seekJtC(int perscent){
        Log.d("perscent", "perscent" + perscent);
        seek(perscent);
    }

    public void mutesoloJtC(int solotype){
        mutesolo(solotype);
    }

    public void continuedplayJtC(){
        continuedplay();
    }

    public void startJtC(){
        start();
    }

    public void playPcmJtC(final String path){
        playPcm(path);
    }

    private native void prepare(String path);
    private native void start();
    private native void playPcm(String path);
    private native void pause();
    private native void stop();
    private native void continuedplay();
    private native void next(String path);
    private native void seek(int perscent);
    private native void mutesolo(int solotype);
    private native void pitchspeed(double pitch,double speed);
    private native void volume(int volume);
}
