package projectparty.ppandroid;


public class MyNativeActivity extends android.app.NativeActivity 
{
	static 
	{
		System.loadLibrary("mystery");
	}

}
