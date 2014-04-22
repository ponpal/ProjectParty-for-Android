package projectparty.ppandroid;

import android.os.Bundle;
import android.os.Vibrator;
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
	
	int vibrate(long milliseconds)
	{
		Vibrator vibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
		if(vibrator.hasVibrator())
		{
			vibrator.vibrate(milliseconds);
			return 1;
		}
		return 0;
	}
}
