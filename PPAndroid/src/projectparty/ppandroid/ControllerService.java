package projectparty.ppandroid;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class ControllerService extends Service implements SensorEventListener {
	public static final String ERROR_MESSAGE = "projectparty.ppandroid.error";
	
	private Socket socket;
	private DataOutputStream out;
	private DataInputStream in;

	private SensorManager sensorManager;
	private Sensor accelerometer;
	private float[] accelerometerData;

	private Long sessionID;
	
	private Looper looper;
	private ServiceHandler serviceHandler;
	private Timer timer;
	
	private String playerName;

	@Override
	public void onCreate() {
		HandlerThread thread = new HandlerThread("ServiceStartArguments",
				Thread.MAX_PRIORITY);
		thread.start();

		looper = thread.getLooper();
		serviceHandler = new ServiceHandler(looper);
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Message msg = serviceHandler.obtainMessage();

		ServerInfo info = (ServerInfo) intent.getSerializableExtra("server");
		this.playerName = intent.getStringExtra("name");
		msg.obj = info;
		serviceHandler.sendMessage(msg);
		return START_STICKY;
	}
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		
		timer.cancel();
		sensorManager.unregisterListener(this);
		try {
			out.close();
			socket.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		looper.quit();
	}
	
	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
	}

	@Override
	public void onSensorChanged(SensorEvent e) {
		accelerometerData = e.values;
	}
	
	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}
		
		@Override
		public void handleMessage(Message msg) {
			synchronized (this) {
				try {
					connect((ServerInfo) msg.obj);	
					sendName();
					setupAccelerometer();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}
		
		public synchronized void connect(ServerInfo server) throws UnknownHostException, IOException {
			socket = new Socket(InetAddress.getByAddress(server.getIP()), server.getPort());
			out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream(), 8192));
			in = new DataInputStream(socket.getInputStream());

			sessionID = in.readLong();

			out.writeLong(sessionID);
			out.flush();
		}
	}
	
	public void sendName() {
		try {
			byte[] byteName = playerName.getBytes("UTF-8");
			out.writeShort(byteName.length + 1);
			Log.d("LENGTH:", "" + byteName.length);
			out.write(0);
			out.write(byteName);
			out.flush();
		} catch (Exception e) {
			e.printStackTrace();
			notifyActivity();
		}
	}
	
	public void setupAccelerometer() {
		this.sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		this.accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_FASTEST);

		this.timer = new Timer();
		timer.schedule(new TimerTask() {
			@Override
			public void run() {
				sendAccelerometer();
			}
		}, 100, 16);
	}

	private void sendAccelerometer() {
		try {
			out.writeShort(13); //Length of message
			out.write(1);  //Message type
			out.writeFloat(accelerometerData[0]);
			out.writeFloat(accelerometerData[1]);
			out.writeFloat(accelerometerData[2]);
			out.flush();
		} catch (Exception e) {
			e.printStackTrace();
			notifyActivity();
		}
	}
	
	public void notifyActivity() {
		Intent intent = new Intent(ERROR_MESSAGE);
		sendBroadcast(intent);
	}
}