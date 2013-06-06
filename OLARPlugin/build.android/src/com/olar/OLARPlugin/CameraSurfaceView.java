package com.olar.OLARPlugin;

import java.io.IOException;

import android.content.Context;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraSurfaceView extends SurfaceView implements SurfaceHolder.Callback , PreviewCallback {

	private boolean surfaceCreated;
	private boolean running;
	private Camera cameraDevice;
	private Callback callback;
	private CameraConfig config;
	private PreviewBuffer previewBuffer;

	
	public static interface Callback{
		public void onCaptureStart(CameraSurfaceView view,int w, int h);
		public void onCaptureFrame(CameraSurfaceView view,byte[] data);
		public void onCaptureStop(CameraSurfaceView view);
	}
	
	public CameraConfig getCameraConfig(){
		return config;
	}
	public Camera getDeviceCamera(){
		return cameraDevice;
	}
	
	public CameraSurfaceView(Context context) {
		super(context);
		getHolder().addCallback(this);
		getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		surfaceCreated = false;
		running=false;
		cameraDevice=null;
		config = CameraConfig.getCurrentConfig();
	}

	public boolean isRunning(){
		return running;
	}
	public void setCallback(Callback callback){
		this.callback = callback;
	}

	public boolean start(Camera camera){
		if( running || camera==null ){ return false; }
		cameraDevice = camera;
		config.applyConfig(cameraDevice);
		config.printConfig();
		
		if( !surfaceCreated ){
			return false;
		}
		
		try{
			cameraDevice.setPreviewDisplay(getHolder());
		}catch (IOException e) {
			L.e(Log.getStackTraceString(e));
			return false;
		}
		if( config.supportsCallbackBuffer() ){
			cameraDevice.setPreviewCallbackWithBuffer(this);
			previewBuffer = new PreviewBuffer(config.width,config.height, 2, 2);
			for(int i=0;i<previewBuffer.bufferNum();i++){
				cameraDevice.addCallbackBuffer(previewBuffer.nextBuffer());
			}
			L.d(">>>>>>>>>>> Camera#start(2.2)");
		}else{
			cameraDevice.setPreviewCallback(this);
			L.d(">>>>>>>>>>> Camera#start(2.1)");
		}
		

		if( callback!=null ){
			callback.onCaptureStart(this,config.width,config.height);
		}
		cameraDevice.startPreview();	
		running = true;
	
		return true;
	}

	public boolean stop() {
		if( cameraDevice==null ) { return false; }
		if( running ){ 
			if(config.supportsCallbackBuffer()){
				cameraDevice.setPreviewCallbackWithBuffer(null);
			}else{
				cameraDevice.setPreviewCallback(null);
			}
		}
		
		cameraDevice.stopPreview();
		running = false;
		callback.onCaptureStop(this);
		cameraDevice = null;
		
		return true;
	}
	
	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		if( callback!=null ){ callback.onCaptureFrame(this,data); }		
		if( config.supportsCallbackBuffer() ){ camera.addCallbackBuffer(data); }
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		if( width > 0 && height > 0 ){
			surfaceCreated = true;
			if( cameraDevice!=null ){
				start(cameraDevice);
			}
		}
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		surfaceCreated = false;
		stop();		
	}
	
	class PreviewBuffer{
		int bufferIndex;
		byte[][] buffers;
		PreviewBuffer(int w, int h, int bpp, int num){
			bufferIndex = -1;
			buffers = new byte[num][w*h*bpp];
		}
		byte[] nextBuffer(){
			bufferIndex = (bufferIndex+1)%buffers.length;
			return buffers[bufferIndex];
		}
		int bufferNum(){ return buffers.length; }
	}
	
}
