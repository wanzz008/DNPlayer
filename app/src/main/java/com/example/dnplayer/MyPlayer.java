package com.example.dnplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

public class MyPlayer implements SurfaceHolder.Callback {

    // Used to load the 'dnplayer' library on application startup.
    static {
        System.loadLibrary("dnplayer");
    }

    private String dataSource;

    public void setDataSource(String path){
        this.dataSource = path ;
    }

    /**
     * 准备
     */
    public void prepare(){
        nativePrepare(dataSource);
    }

    /**
     * 开始播放
     */
    public void start(){
        System.out.println("------start....");
        nativeStart();
    }

    /**
     * 停止播放
     */
    public void stop() {

    }

    public void release() {
        holder.removeCallback(this);
    }

    public void onError(int errorCode){
        System.out.println("----Java接到回调:"+errorCode);
        if (listener != null){
            listener.onError();
        }
    }


    public void onPrepare(){
        System.out.println("------Java接到回调:onPrepare");
        if (listener != null){
            listener.onPrepare();
        }

    }

    SurfaceHolder holder ;
    public void setSurfaceView(SurfaceView surfaceView) {
        holder = surfaceView.getHolder();
        holder.addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        nativeSetSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

    }

    public interface OnPlayerListener {
        void onPrepare();
        void onError();
    }

    private OnPlayerListener listener;
    public void setOnListener(OnPlayerListener listener){
        this.listener= listener ;
    }



    private native void nativePrepare(String dataSource);
    private native void nativeStart();
    private native void nativeSetSurface(Surface surface);
}
