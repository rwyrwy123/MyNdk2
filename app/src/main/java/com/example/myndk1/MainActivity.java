package com.example.myndk1;

import android.annotation.SuppressLint;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.example.myndk1.bean.DurationBean;
import com.example.myndk1.ff.FFNdk;
import com.example.myndk1.ff.IDurationListener;
import com.example.myndk1.ff.IErrorListener;
import com.example.myndk1.ff.IPauseListener;
import com.example.myndk1.ff.IPrepareListenter;
import com.example.myndk1.ff.IStopListener;
import com.example.myndk1.gl.RWGLSurfaceView;
import com.example.myndk1.utils.RWTimeUtil;

public class MainActivity extends AppCompatActivity implements IPrepareListenter, IErrorListener, IDurationListener, IPauseListener, IStopListener {

    private FFNdk ffNdk;
//    private String path2 = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
    private String path2 = "http://ngcdn004.cnr.cn/live/dszs/index.m3u8";
//    private String path = "/mnt/shared/Other/mydream.mp3";
    private String path = "/mnt/shared/Other/movie.mp4";
    private String pathPcm = "/mnt/shared/Other/mydream.pcm";
    private TextView tv_duration;
    private RWGLSurfaceView surfaceView;
    private DurationBean durationBean;
    private DurationBean sendDurationBean;
    private SeekBar seek_duration;
    private SeekBar seek_volume;
    private boolean seekProgress = false;
    private int soloCount = 0;
    private int duration;

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            if (msg != null) {
                switch (msg.what) {
                    case 1:
                        durationBean = (DurationBean) msg.obj;
                        seek_duration.setProgress(durationBean.getCurrentDuration() * 100 / durationBean.getDuration() );
                        tv_duration.setText(RWTimeUtil.secdsToDateFormat(durationBean.getDuration(), durationBean.getDuration())
                                + "/" + RWTimeUtil.secdsToDateFormat(durationBean.getCurrentDuration(), durationBean.getDuration()));
                        break;
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tv_duration = findViewById(R.id.tv_duration);
        seek_duration = findViewById(R.id.seek_duration);
        seek_volume = findViewById(R.id.seek_volume);
        surfaceView = findViewById(R.id.surfaceview);
        ffNdk = new FFNdk();
        ffNdk.setSurfaceView(surfaceView);
        ffNdk.setErrorListener(this);
        ffNdk.setPrepareListenter(this);
        ffNdk.setDurationListener(this);
        ffNdk.setPauseListener(this);
        ffNdk.setStopListener(this);
        seek_duration.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (ffNdk.getDuration() >0 && seekProgress){
                    duration = ffNdk.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                seekProgress = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                ffNdk.seekJtC(duration);
                seekProgress = false;
            }
        });

        seek_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                ffNdk.volumeJtC(i);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    @Override
    public void onPrepared() {
        Log.d("MainActivity", "准备完成 开始生成数据");
        ffNdk.startJtC();
    }

    @Override
    public void onError(int code, String msg) {
        Log.d("MainActivity onError", "code:" + code + "msg:" + msg);
    }

    public void start(View view) {
        ffNdk.prepareJtC(path);
    }

    public void pause(View view) {
        ffNdk.pauseJtC();
    }

    public void playpcm(View view) {
        ffNdk.playPcmJtC(pathPcm);
    }

    public void stop(View view) {
        ffNdk.stopJtC();
    }

    public void continuedplay(View view) {
        ffNdk.continuedplayJtC();
    }

    @Override
    public void onDuration(int duration, int currentDuration) {
        Message message = Message.obtain();
        message.what = 1;
        if (sendDurationBean == null) {
            sendDurationBean = new DurationBean();
        }
        sendDurationBean.setDuration(duration);
        sendDurationBean.setCurrentDuration(currentDuration);
        message.obj = sendDurationBean;
        handler.sendMessage(message);
    }

    public void next(View view) {
        ffNdk.nextJtC(path2);
    }

    @Override
    public void onRwPause() {
        Log.d("MainActivity", "暂停");
    }

    @Override
    public void onRwStop() {
        Log.d("MainActivity", "停止");
    }

    public void mutesolo(View view) {
        soloCount++;
        ffNdk.mutesoloJtC(soloCount % 3);
    }

    public void pitch(View view) {
        ffNdk.pitchspeedJtC(1.5f,1.0f);
    }

    public void speed(View view) {
        ffNdk.pitchspeedJtC(1.0f,1.5f);
    }

    public void pitchspeed(View view) {
        ffNdk.pitchspeedJtC(1.5f,1.5f);
    }
}
