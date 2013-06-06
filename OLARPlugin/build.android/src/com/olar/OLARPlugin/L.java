package com.olar.OLARPlugin;

import android.util.Log;

public class L {
	public static String Tag = "DebugJava";
	public static void d(Object... msg){
		Log.d(Tag,str(msg));
	}
	public static void e(Object... msg){
		Log.e(Tag,str(msg));
	}
	private static String str(Object... msg){
		if( msg.length == 0 ){ 
			return "";
		}
		String s = ""+msg[0];
		for(int i=1; i < msg.length; i++){
			s += " "+msg[i];
		}
		return s;
	}
}
