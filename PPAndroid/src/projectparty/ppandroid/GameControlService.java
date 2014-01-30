package projectparty.ppandroid;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.UUID;

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
	private static final int INIT_MESSAGE = 0;
	private static final int UTF8_DATA = 1;
	private static final int ACCELEROMETER_DATA = 2;

	private Looper looper;
	private ServiceHandler serviceHandler;
	private SensorManager sensorManager;
	private Sensor accelerometer;

	private Socket socket;
	private DataOutputStream out;
	private DataInputStream in;

	private UUID sessionID;

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
		msg.what = INIT_MESSAGE;

		msg.arg1 = startId;
		msg.setData(intent.getExtras());

		serviceHandler.sendMessage(msg);
		return START_STICKY;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		try {
			sensorManager.unregisterListener(this);
			socket.close();
			looper.quit();
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

	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}

		@Override
		public void handleMessage(Message msg) {
			synchronized (this) {
				try {
					switch(msg.what) {
					case INIT_MESSAGE:
						//Setup socket, in- and output streams
						socket = new Socket(msg.getData().getString("ip"), msg.getData().getInt("port"));
						out = new DataOutputStream(socket.getOutputStream());
						in = new DataInputStream(socket.getInputStream());

						setupAccelerometer();

						//Read UUID sent from server
						byte[] buffer = new byte[1024];
						int read = in.read(buffer);
						String id = new String(buffer, 0, read);
						sessionID = UUID.fromString(id);
						
						Log.d("UUID: ", sessionID.toString());						
						break;
					case UTF8_DATA:
						break;
					case ACCELEROMETER_DATA:
						float[] acc = msg.getData().getFloatArray("accelerometer");
						
						out.write(0);
						
						out.writeFloat(acc[0]);
						out.writeFloat(acc[1]);
						out.writeFloat(acc[2]);
						break;
					}
					out.flush();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}
	}
}
