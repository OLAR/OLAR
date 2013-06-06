package com.olar.OLARPlugin;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


import org.cocos2dx.lib.Cocos2dxActivity;

import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;

import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;

public class OLARActivity extends Cocos2dxActivity implements CameraSurfaceView.Callback
{
	public static final int MP = ViewGroup.LayoutParams.FILL_PARENT;
	public static final int AROrientationUnknown = 0;
	public static final int AROrientationPortrait = 1;
	public static final int AROrientationPortraitUpsideDown = 2;
	public static final int AROrientationLandscapeLeft = 4;
	public static final int AROrientationLandscapeRight = 3;
	public static final int ARCameraFormatBGRA  = 0x00010000;
	public static final int ARCameraFormatYUVSP = 0x00020000;
	
	protected FrameLayout contentView;
	protected CameraSurfaceView cameraSurfaceView;
	protected Camera camera;
	protected int interfaceOrientation;
	public long nativeCameraAddr;
	
	protected native void nativeOnCreate();
	protected native void nativeOnDestroy();
	protected native void nativeOnResume();
	protected native void nativeOnPause();
	protected native void nativeOnCaptureFrame(long cameraAddr, byte[] data);
	protected native void nativeSetCameraConfig(int width, int height,
			int pixelFormat, double fov);
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		interfaceOrientation = getOrientationFromManifest(getRequestedOrientation());
		if(interfaceOrientation==AROrientationUnknown){
			interfaceOrientation = getOrientationFromConfig(getResources()
					.getConfiguration().orientation);
		}
		

		nativeCameraAddr = 0;

		cameraSurfaceView = new CameraSurfaceView(this);
		cameraSurfaceView.setCallback(this);
		contentView.addView(cameraSurfaceView, 0, new ViewGroup.LayoutParams(1,
				1));

		GLSurfaceView glSurfaceView = searchGLSurfaceView(contentView);
		if( glSurfaceView != null ){
			glSurfaceView.setZOrderMediaOverlay(true);
		}
		
		CameraConfig cnf = cameraSurfaceView.getCameraConfig();
		try {
			InputStream configStream = getAssets().open("android_camera.conf");
			cnf.loadExternalConfig(configStream);
		} catch (FileNotFoundException nf) {
			L.d("android_camera.conf is not found");
		} catch (Exception e) {
			L.d("Failed to open android_camera.conf:"
					+ Log.getStackTraceString(e));
		}

		Camera cam = null;

		cam = Camera.open();
		cnf.applyConfig(cam);
		cnf.printConfig();

		double fov = cnf.fov;
		int pixelFormat = ARCameraFormatYUVSP;
		int format = cam.getParameters().getPreviewFormat();
		if (format != ImageFormat.NV21) {
			L.e("camera preview format != NV21");
		}
		cam.release();

		nativeOnCreate();
		nativeSetCameraConfig(cnf.width, cnf.height, pixelFormat, fov);
	}

	@Override
	protected void onDestroy() {
		nativeOnDestroy();
		super.onDestroy();
	}

	@Override
	protected void onResume() {
		super.onResume();
		nativeOnResume();
	}

	@Override
	protected void onPause() {
		nativeOnPause();
		super.onPause();
	}

	@Override
	public void setContentView(View view, LayoutParams params) {
		super.setContentView(view, params);
		this.contentView = (FrameLayout) view;
	}

	@Override
	public void setContentView(int layoutResID) {
		setContentView(LayoutInflater.from(this).inflate(layoutResID, null));
	}

	@Override
	public void setContentView(View view) {
		setContentView(view, new ViewGroup.LayoutParams(MP, MP));
	}

	protected GLSurfaceView searchGLSurfaceView(View view) {
		if (view instanceof GLSurfaceView)
			return (GLSurfaceView) view;
		if (view instanceof ViewGroup) {
			ViewGroup group = (ViewGroup) view;
			for (int i = 0; i < group.getChildCount(); i++) {
				View child = group.getChildAt(i);
				GLSurfaceView found = searchGLSurfaceView(child);
				if (found != null)
					return found;
			}
		}
		return null;
	}

	public boolean cameraStart() {
		try {
			camera = Camera.open();
		} catch (Exception e) {
			L.e(Log.getStackTraceString(e));
			return false;
		}
		return cameraSurfaceView.start(camera);
	}

	public boolean cameraStop() {
		return cameraSurfaceView.stop();
	}

	public boolean cameraIsRunning() {
		return cameraSurfaceView.isRunning();
	}

	@Override
	public void onCaptureStart(CameraSurfaceView view, int w, int h) {
		L.d("onCaptureStart : " + w + "x" + h);
	}

	@Override
	public void onCaptureFrame(CameraSurfaceView view, byte[] data) {
		if (nativeCameraAddr != 0)
			nativeOnCaptureFrame(nativeCameraAddr, data);
	}

	@Override
	public void onCaptureStop(CameraSurfaceView view) {
		if (camera != null) {
			camera.release();
			camera = null;
		}
	}

	protected int getOrientationFromConfig(int o) {
		switch (o) {
		case Configuration.ORIENTATION_PORTRAIT:
			return AROrientationPortrait;
		case Configuration.ORIENTATION_LANDSCAPE:
			return AROrientationLandscapeRight;
		default:
			return AROrientationUnknown;
		}
	}
	
	protected int getOrientationFromManifest(int o){
		switch(o){
		case ActivityInfo.SCREEN_ORIENTATION_PORTRAIT:
			return AROrientationPortrait;
		case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
			return AROrientationLandscapeRight;
		default:
			return AROrientationUnknown;
		}
	}

	public int getDeviceOrientation() {
		return interfaceOrientation;
	}

	public byte[] loadAssetsData(String path) {
		InputStream is = null;
		byte[] data = null;
		try {
			is = getAssets().open(path);
			data = new byte[is.available()];
			is.read(data);
		} catch (IOException e) {
			return null;
		}
		return data;
	}
}
