package com.example.dnplayer;

import android.util.Log;
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
        nativePrepare(dataSource,isEGL);
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

    private boolean isEGL = false ;
    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
        if (isEGL){
            eglInit(surfaceHolder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int width, int height) {
        if (isEGL){
            nativeSurfaceChanged(holder.getSurface(),width,height);
        }else {
            nativeSetSurface(holder.getSurface());
        }
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


    /**
     * 设置数据的回调，使用EGL来渲染，还是用OPENGL ES来渲染
     * @param dataSource
     * @param isEGL
     */
    private native void nativePrepare(String dataSource, boolean isEGL);
    private native void nativeStart();
    private native void nativeSetSurface(Surface surface);

    public native void eglInit(Surface surface) ;
    public native void nativeSurfaceChanged(Surface surface, int width, int height) ;

}
