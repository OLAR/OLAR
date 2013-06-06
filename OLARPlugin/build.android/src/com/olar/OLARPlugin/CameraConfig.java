package com.olar.OLARPlugin;

import java.io.InputStream;
import java.util.List;


import org.json.JSONObject;

import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.os.Build;

public class CameraConfig {
	public int width;
	public int height;
	public int minFPS;
	public int maxFPS;
	public double fov;
	public String focusMode;
	public String antibandingMode;
	public String layoutMode;
	public String deviceName;
	public String manufacturer;
	public String currentConfigName;
	public String[] allParams;
	public boolean debugMode;
	
	protected static CameraConfig currentConfig = null; 
	
	public static CameraConfig getCurrentConfig(){
		if( currentConfig == null ){
			currentConfig = new CameraConfig();
		}
		return currentConfig;
	}
	
	public CameraConfig(){
		debugMode = false;
		currentConfigName = "default";
		currentConfig = this;
		deviceName = Build.MODEL;
		manufacturer = Build.MANUFACTURER;
		width = 0;
		height = 0;
		fov = 0;
		minFPS = maxFPS = 30;
		layoutMode = "fit";
		focusMode = Parameters.FOCUS_MODE_AUTO;
		antibandingMode = null;
	}
	
	public boolean loadExternalConfig(InputStream istrm){
		String externalConfig = null;
		try{
			int length = istrm.available();
			byte[] chars = new byte[length];
			istrm.read(chars);
			externalConfig = new String(chars, "UTF-8");
		}catch (Exception e) {
			return false;
		}
		return loadExternalConfig(externalConfig);
	}
	
	public boolean loadExternalConfig(String externalConfig){
		try{
			JSONObject allconf = new JSONObject(externalConfig);
			if( allconf.has("*") ){
				currentConfigName = "*";
				loadExternalConfigItem(allconf.getJSONObject(currentConfigName));
			}
			if( allconf.has(deviceName) ){
				currentConfigName = deviceName;
				loadExternalConfigItem(allconf.getJSONObject(currentConfigName));
			}
			if( allconf.has(manufacturer+":"+deviceName) ){
				currentConfigName = manufacturer+":"+deviceName;
				loadExternalConfigItem(allconf.getJSONObject(currentConfigName));
			}
			layoutMode = ".";
		}catch (Exception e) {
			L.e(e);
			return false;
		}
		return true;
	}
	
	private boolean loadExternalConfigItem(JSONObject conf){
		try{
			if( conf.has("width") ){
				width = conf.getInt("width");
			}
			if( conf.has("height") ){
				height = conf.getInt("height");
			} 
			if( conf.has("fov") ){
				fov = conf.getDouble("fov");
			}
			if( conf.has("focus") ){
				focusMode = conf.getString("focus");
			}
			if( conf.has("antibanding") ){
				antibandingMode = conf.getString("antibanding");
			}
			if( conf.has("layout") ){
				layoutMode = conf.getString("layout");
			}
			if( conf.has("debug") ){
				debugMode = conf.getBoolean("debug");
			}
		}catch (Exception e) {
			L.e(e);
			return false;
		}
		return true;
	}
	
	public void applyConfigToDefaultCamra()
	{
		try{
			Camera camera = Camera.open();
			applyConfig(camera);
			camera.release();
		}catch (Exception e) {
			L.e("Error in CamraConfig.applyConfigToDefaultCamra():",e);
			return;
		}
	}
	
	public void applyConfig(Camera camera)
	{
		try{
			if( width==0 || height==0 ){
				width = 640;
				height = 480;
			}
			Size preferredSize = selectSize(camera,width,height);
			if( preferredSize!=null ){
				width = preferredSize.width;
				height = preferredSize.height;
			}
			Parameters param = camera.getParameters();
			param.setPreviewSize(width, height);
			if( fov <= 0.0 ){ 
				fov = param.getVerticalViewAngle();
				if( fov <= 0.0 ){ fov = 40.0; }
				else if( fov >= 180.0 ){ fov = 40.0; }
			}
			
			int[] fpsRange = selectFPSRange(camera);
			if( fpsRange!=null ){ 
				param.setPreviewFpsRange(fpsRange[0],fpsRange[1]);
				minFPS = fpsRange[0]/1000;
				maxFPS = fpsRange[1]/1000;
			}else{
				int baseFps = selectFPS(camera);
				if( baseFps >= 15 ){
					maxFPS = baseFps;
					minFPS = baseFps;
					param.setPreviewFrameRate(baseFps);
				}
			}
			param.setFocusMode(focusMode);
			if( antibandingMode!=null ){
				param.setAntibanding(antibandingMode);
			}
			camera.setParameters(param);
			String paramString = camera.getParameters().flatten();
			if( paramString!=null ){
				allParams = paramString.split(";");
			}
			
		}catch (Exception e) {
			L.e(e);
		}
	}
	
	private static Size selectSize(Camera cam,int w,int h){
		//L.d("CameraConfig.selectSize query = "+w+"x"+h);

		Size candidateSize1 = null;
		Size candidateSize2 = null;
		try{
			List<Size> supportedSizes = cam.getParameters().getSupportedPreviewSizes();
			//for(Size size: supportedSizes){
			//	L.d("CameraConfig.selectSize> "+size.width+"x"+size.height);
			//}
			if (supportedSizes == null || supportedSizes.size() <= 0) {
				return null;
			}
			int minArea1 = 0;
			int minArea2 = w*h;
			for (Size size : supportedSizes) {
				if( size.width==w && size.height == h ){
					candidateSize2 = size;
					break;
				}
				int area = size.width * size.height;
				if( (320*240) >= area ){
					if( area > minArea1 ){
						minArea1 = area;
						candidateSize1 = size;
					}
				}
				if( minArea2 >= area && area >= (w*h) ){
					minArea2 = area;
					candidateSize2 = size;
				}
			}
		}catch (Exception e) {
			return null;
		}
		if( candidateSize2!=null ){
			return candidateSize2;
		}
		return candidateSize1;
	}
	
	private static int selectFPS(Camera cam){
		List<Integer> supportedFPS;
		try{
			supportedFPS = cam.getParameters().getSupportedPreviewFrameRates();
		}catch (Exception e) {
			return 0;
		}
		if (supportedFPS == null || supportedFPS.size() <= 0) {
			return 0;
		}
		int maxFPS = 0;
		for (Integer fps : supportedFPS) {
			//L.d("FPS:",fps);
			if( fps > maxFPS ){ 
				maxFPS = fps;
			}
		}
		return maxFPS;
	}
	
	private static int[] selectFPSRange(Camera cam){
		if( Build.VERSION.SDK_INT < 9 ){ return null; }
		int[] range = null;
		List<int[]> supportedRange = cam.getParameters().getSupportedPreviewFpsRange();
		if (supportedRange == null || supportedRange.size() <= 0) {
			return null;
		}
		int maxFPS = 0;
		int minFPS = 0;
		for (int[] r : supportedRange) {
			//L.d("FPS:",r[0],"-",r[1]);
			if( r[1] > maxFPS || (r[1]==maxFPS && r[0] > minFPS ) ){ 
				minFPS = r[0];
				maxFPS = r[1];
				range = r;
			}
		}
		return range;
	}
	
	public int width(){ return width; }
	public int height(){ return height; }
	public double aspect(){ return (double)width/height; }
	public int minFPS(){ return minFPS; }
	public int maxFPS(){ return maxFPS; }
	public double verticalFOV(){ return fov; }
	public String focusMode(){ return focusMode; }
	public String antibandingMode(){ return antibandingMode; }
	public String layoutMode(){ return layoutMode; }
	public boolean supportsCallbackBuffer(){ return ( Build.VERSION.SDK_INT >= 8 )? true : false; }
	public boolean supportsSurfaceTexture(){ return ( Build.VERSION.SDK_INT >= 14 )? true : false; }
	
	public void printConfig(){
		L.d("===============================");
		L.d("         Camera config         ");
		L.d("===============================");
		L.d("Config name   : ["+currentConfigName+"]");
		L.d("Device ID     : ["+manufacturer+":"+deviceName+"]");
		L.d("Device        : "+deviceName);
		L.d("SDK version   : "+Build.VERSION.SDK_INT);
		L.d("Preview Size  : "+width+" x "+height);
		L.d("FPS range     : "+minFPS+"-"+maxFPS);
		L.d("Camera FOV    : "+fov);
		L.d("Focus mode    : "+focusMode);
		L.d("Layout mode   : "+layoutMode);
		L.d("Antibanding   : "+antibandingMode);
		L.d("CBB supported : "+supportsCallbackBuffer());
		L.d("STX supported : "+supportsSurfaceTexture());
		L.d("===============================");
		if( debugMode ){
			for(int i=0;allParams!=null&&i<allParams.length;i++){
				L.d(allParams[i]);
			}
			L.d("===============================");
		}
	}
	
	public static class ARCameraAvailableOption{
		public int[][] size;
		public String[] layoutMode;
		public String[] focusMode;
		public String   deviceName;
		public String   manufacturer;
		public int      sdkVersion;
		public ARCameraAvailableOption()
		{
			deviceName = Build.MODEL;
			manufacturer = Build.MANUFACTURER;
			sdkVersion = Build.VERSION.SDK_INT;
			
			layoutMode = new String[]{"Fit","Fill","NoScale","."};
			
			Parameters param = null;
			try{
				Camera camera = Camera.open();
				param = camera.getParameters();
				camera.release();
			}catch (Exception e) { return; }
			
			List<Size> sizes = param.getSupportedPreviewSizes();
			if ( sizes != null && sizes.size() > 0) {
				int count = 0;
				for( int i=0;i<sizes.size();i++ ){
					Size s = sizes.get(i);
					if( s.width*s.height >= (320*240) ){ count++; }
				}
				if( count > 0 ){
					size = new int[count][2];
					for( int i=0,j=0;i<sizes.size();i++ ){
						Size s = sizes.get(i);
						if( s.width*s.height >= (320*240) ){
							size[j][0] = s.width;
							size[j][1] = s.height;
							j++;
						}
					}
				}
			}
			String[] paramStringArray = param.flatten().split(";");
			for( String s : paramStringArray ){
				if( s.startsWith("focus-mode-values=") ){
					focusMode = s.replaceFirst("focus-mode-values=","").split(",");
				}
			}
		}
	};
	
}
