package projectparty.ppandroid;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
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

public class ControllerService extends Service implements SensorEventListener {
	public static final String ERROR_MESSAGE = "projectparty.ppandroid.error";

	private Looper looper;
	private ServiceHandler serviceHandler;
	private Timer timer;

	private SocketChannel socketChan;

	private SensorManager sensorManager;
	private Sensor accelerometer;
	private float[] accelerometerData = {0, 0, 0};
	private String playerName;

	private boolean connected = false;

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

		if(connected) {
			timer.cancel();
			sensorManager.unregisterListener(this);
			try {
				socketChan.close();
			} catch (Exception e) {
				e.printStackTrace();
			}

			connected = false;
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
					handleNetworkError(e);
				}
			}
		}

		public synchronized void connect(ServerInfo server) throws UnknownHostException, IOException {
			socketChan = SocketChannel.open();
			socketChan.connect(new InetSocketAddress(InetAddress.getByAddress(server.getIP()), server.getPort()));
			socketChan.finishConnect();

			ByteBuffer sessionIdBuffer = ByteBuffer.allocateDirect(8);
			
			if(socketChan.finishConnect()) {
				socketChan.configureBlocking(true);
				socketChan.read(sessionIdBuffer);
				socketChan.configureBlocking(false);
			}
			
			sessionIdBuffer.flip();
			
			socketChan.write(sessionIdBuffer);
			connected = true;
		}
	}

	public void sendName() throws IOException {
		byte[] byteName = playerName.getBytes("UTF-8");
		
		ByteBuffer nameBuffer = ByteBuffer.allocateDirect(byteName.length + 3);
		nameBuffer.putShort((short) (byteName.length + 1));
		nameBuffer.put((byte) 0);
		nameBuffer.put(byteName);
		
		nameBuffer.position(0);
		socketChan.write(nameBuffer);
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
				//Send messages received on SocketChannel to native here.
			}
		}, 100, 100);
	}

	private void sendAccelerometer() {
		try {
			ByteBuffer accBuffer = ByteBuffer.allocate(15);
			
			accBuffer.putShort((short) 13);
			accBuffer.put((byte) 1);
			accBuffer.putFloat(accelerometerData[0]);
			accBuffer.putFloat(accelerometerData[1]);
			accBuffer.putFloat(accelerometerData[2]);
			
			accBuffer.position(0);
			socketChan.write(accBuffer);
		} catch (Exception e) {
			handleNetworkError(e);
		}
	}

	public void notifyActivity() {
		Intent intent = new Intent(ERROR_MESSAGE);
		sendBroadcast(intent);
	}
	
	public void notifyActivity(String message) {
		Intent intent = new Intent(ERROR_MESSAGE);
		sendBroadcast(intent);
	}

	private void handleNetworkError(Exception e) {
		e.printStackTrace();
		stopSelf();
		notifyActivity();
	}
}
