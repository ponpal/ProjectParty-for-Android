package projectparty.ppandroid;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class GameControlService extends Service implements SensorEventListener {
	private static final int UTF8_DATA = 0;
	private static final int ACCELEROMETER_DATA = 1;

	private Looper looper;
	private ServiceHandler serviceHandler;
	private SensorManager sensorManager;
	private Sensor accelerometer;
	private Socket socket;
	private DataOutputStream out;

	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}

		@Override
		public void handleMessage(Message msg) {
			synchronized (this) {
				try {
//					InetAddress ip = InetAddress.getByName(msg.getData().getString("ip"));
//					Socket socket = new Socket(ip, msg.getData().getInt("port"));
//					DataOutputStream out = new DataOutputStream(socket.getOutputStream());       

					if(msg.what == ACCELEROMETER_DATA) {
						Log.d("PPAndroid", "Trying to send sensor data!");

						float[] acc = msg.getData().getFloatArray("accelerometer");
						out.writeUTF("X: " + acc[0] + "\n" + 
								"Y: " + acc[1] + "\n" +
								"Z: " + acc[2] + "\n");
					} else {
						out.writeUTF("Hello from Android.\n");
					}

					out.flush();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}
	}

	@Override
	public void onCreate() {
		// Start up the thread running the service.  Note that we create a
		// separate thread because the service normally runs in the process's
		// main thread, which we don't want to block.  We also make it
		// background priority so CPU-intensive work will not disrupt our UI.
		HandlerThread thread = new HandlerThread("ServiceStartArguments",
				Thread.MAX_PRIORITY);
		thread.start();

		// Get the HandlerThread's Looper and use it for our Handler
		looper = thread.getLooper();
		serviceHandler = new ServiceHandler(looper);

		setupAccelerometer();
	}

	public void setupAccelerometer() {
		this.sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		this.accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_GAME);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// For each start request, send a message to start a job and deliver the
		// start ID so we know which request we're stopping when we finish the job
		Message msg = serviceHandler.obtainMessage();
		msg.what = UTF8_DATA;
		
		msg.arg1 = startId;
		msg.setData(intent.getExtras());

		Runnable createSocket = new SocketCreator(msg.getData().getString("ip"),
												  msg.getData().getInt("port"));
		
		new Thread(createSocket).start();
		
		serviceHandler.sendMessage(msg);
		return START_STICKY;
	}
	
	class SocketCreator implements Runnable {
		private String ip;
		private int port;
		
		public SocketCreator(String ip, int port) {
			this.ip = ip;
			this.port = port;
		}
		
		@Override
		public void run() {
			try {
				socket = new Socket(InetAddress.getByName(ip), port);
				out = new DataOutputStream(socket.getOutputStream());
			} catch (UnknownHostException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		try {
			socket.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
	}

	@Override
	public void onSensorChanged(SensorEvent e) {
		Message msg = serviceHandler.obtainMessage();
		msg.what = ACCELEROMETER_DATA;

		Bundle data = new Bundle();
		data.putFloatArray("accelerometer", e.values);
		msg.setData(data);
		serviceHandler.sendMessage(msg); 	
	}
}
