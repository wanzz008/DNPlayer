package com.example.dnplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.example.dnplayer.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("dnplayer");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        SurfaceView surfaceView = binding.surfaceview;
        tv.setText(stringFromJNI());


        MyPlayer myPlayer = new MyPlayer();
        myPlayer.setSurfaceView(surfaceView);
        myPlayer.setDataSource("/sdcard/aaa.mp4");
        myPlayer.setOnListener(new MyPlayer.OnPlayerListener() {
            @Override
            public void onPrepare() {
                System.out.println("------MainActivity接到回调:onPrepare");
                myPlayer.start();
            }

            @Override
            public void onError() {
                System.out.println("------MainActivity接到回调:onError");
            }
        });
        myPlayer.prepare();

    }

    /**
     * A native method that is implemented by the 'dnplayer' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}