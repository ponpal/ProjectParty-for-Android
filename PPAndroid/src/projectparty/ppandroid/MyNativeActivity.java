package projectparty.ppandroid;

import android.os.Bundle;
import android.view.WindowManager;

public class MyNativeActivity extends android.app.NativeActivity 
{
	@Override
	protected void onCreate(Bundle savedInstance) {
		super.onCreate(savedInstance);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}
	
	static 
	{
		System.loadLibrary("mystery");
	}

}
