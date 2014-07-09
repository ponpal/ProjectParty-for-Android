package projectparty.ppandroid;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
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
	
	public int getBroadcastAddress()
	{
		WifiManager wm = (WifiManager)getSystemService(Context.WIFI_SERVICE);
		DhcpInfo dhcp = wm.getDhcpInfo();
		
		int broadcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
		ByteBuffer buf = ByteBuffer.allocate(4);
		buf.order(ByteOrder.BIG_ENDIAN);
		buf.putInt(broadcast);
		buf.flip();
		buf.order(ByteOrder.LITTLE_ENDIAN);
		return buf.getInt();
	}
	
	static 
	{
		System.loadLibrary("mystery");
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
