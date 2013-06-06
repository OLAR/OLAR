package com.olar.example;

import android.os.Bundle;
import com.olar.OLARPlugin.OLARActivity;

public class OLARTest extends OLARActivity{

    static {
         System.loadLibrary("game");
    }
    
	protected void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
	}
	
}
