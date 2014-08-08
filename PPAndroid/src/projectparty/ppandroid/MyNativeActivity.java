package projectparty.ppandroid;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.WindowManager;

public class MyNativeActivity extends android.app.NativeActivity 
{	
	@Override
	protected void onCreate(Bundle savedInstance) {
		super.onCreate(savedInstance);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
	}
	
	public int getBroadcastAddress()
	{
		WifiManager wm = (WifiManager)getSystemService(Context.WIFI_SERVICE);
		DhcpInfo dhcp = wm.getDhcpInfo();
		
		int broadcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
		if(broadcast == 0) return 0xC0A801FF;
		ByteBuffer buf = ByteBuffer.allocate(4);
		buf.order(ByteOrder.BIG_ENDIAN);
		buf.putInt(broadcast);
		buf.flip();
		buf.order(ByteOrder.LITTLE_ENDIAN);
		return buf.getInt();
	}
	
	public byte[] getDeviceName()
	{
		String name = android.os.Build.MODEL;
		byte[] array  = name.getBytes(Charset.forName("UTF-8"));
		return array;
	}
	
	public int getLanIp()
	{
		WifiManager wm = (WifiManager)getSystemService(Context.WIFI_SERVICE);
		ByteBuffer buf = ByteBuffer.allocate(4);
		buf.order(ByteOrder.BIG_ENDIAN);
		buf.putInt(wm.getDhcpInfo().ipAddress);
		buf.flip();
		buf.order(ByteOrder.LITTLE_ENDIAN);
	
		return buf.getInt();
	}
	
	String inputBuffer;
	
	public byte[] getInputBuffer()
	{
		if(inputBuffer == null) return new byte[0];
		
		byte[] arr = inputBuffer.getBytes(Charset.forName("UTF-8"));
		inputBuffer = null;
		return arr;
	}
	
	//@Override
	//public boolean dispatchKeyEvent(KeyEvent event)
	//{
	//	int code = event.getUnicodeChar();
	//	int action = event.getAction();
	//	if(code == 0 && action == KeyEvent.ACTION_MULTIPLE)
	//	{
	//		String s = event.getCharacters();
	//		if(inputBuffer == null) inputBuffer = s;
	//		else inputBuffer += s;
	//	}
	//	
	//	return true;
	//}
	
	void setOrientation(int orientation)
	{
		if(orientation == 0)
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		else
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);			
	}
	
	@SuppressLint("NewApi")
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
