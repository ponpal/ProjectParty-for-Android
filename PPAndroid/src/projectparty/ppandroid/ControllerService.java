package projectparty.ppandroid;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.Timer;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

public class ControllerService extends Service {
	public static final String ERROR_MESSAGE = "projectparty.ppandroid.error";

	private Looper looper;
	private ServiceHandler serviceHandler;
	private Timer timer;

	private SocketChannel socketChan;
	private boolean connected = false;
	
	private String playerAlias;
	
	public ByteBuffer inBuffer;
	public ByteBuffer outBuffer;
	public static ControllerService instance;

	@Override
	public void onCreate() {
		HandlerThread thread = new HandlerThread("ServiceStartArguments",
				Thread.MAX_PRIORITY);
		thread.start();
		
		instance = this;
		this.inBuffer = ByteBuffer.allocateDirect(1024);
		this.outBuffer = ByteBuffer.allocateDirect(1024);

		looper = thread.getLooper();
		serviceHandler = new ServiceHandler(looper);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Message msg = serviceHandler.obtainMessage();

		ServerInfo info = (ServerInfo) intent.getSerializableExtra("server");
		this.playerAlias = intent.getStringExtra("playerAlias");
		msg.obj = info;
		serviceHandler.sendMessage(msg);
		return START_STICKY;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		
		if(connected) {
			timer.cancel();
			try {
				socketChan.close();
			} catch (Exception e) {
				e.printStackTrace();
			}

			connected = false;
		}
		looper.quit();
		
		instance = null;
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	public int receive() {
		try {
			return socketChan.read(inBuffer);
		} catch (IOException e) {
			e.printStackTrace();
			return 0;
		}
	}

	public void send(int size) {
		if(size > 0) {
			outBuffer.limit(size);
			try {
				socketChan.write(outBuffer);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
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
					sendAlias();
				} catch (Exception e) {
					handleNetworkError(e);
				}
			}
		}

		public synchronized void connect(ServerInfo server) throws UnknownHostException, IOException {
			socketChan = SocketChannel.open();
			socketChan.connect(new InetSocketAddress(InetAddress.getByAddress(server.getIP()), server.getPort()));

			ByteBuffer sessionIdBuffer = ByteBuffer.allocateDirect(8);

			if(socketChan.finishConnect()) {
				socketChan.configureBlocking(true);
				socketChan.read(sessionIdBuffer);
				socketChan.configureBlocking(false);
			} else {
				handleNetworkError();
			}

			sessionIdBuffer.flip();
			socketChan.write(sessionIdBuffer);
			connected = true;
		}
	}

	public void sendAlias() throws IOException {
		byte[] byteAlias = playerAlias.getBytes("UTF-8");

		ByteBuffer aliasBuffer = ByteBuffer.allocateDirect(byteAlias.length + 3);
		aliasBuffer.putShort((short) (byteAlias.length + 1));
		aliasBuffer.put((byte) 0);
		aliasBuffer.put(byteAlias);

		aliasBuffer.position(0);
		socketChan.write(aliasBuffer);
	}

	//	public void setupAccelerometer() {
	//		this.sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
	//		this.accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
	//		sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_FASTEST);
	//
	//		this.timer = new Timer();
	//		timer.schedule(new TimerTask() {
	//			@Override
	//			public void run() {
	//				sendAccelerometer();
	//				//Send messages received on SocketChannel to native here.
	//			}
	//		}, 100, 100);
	//	}
	//
	//	private void sendAccelerometer() {
	//		try {
	//			ByteBuffer accBuffer = ByteBuffer.allocate(15);
	//			
	//			accBuffer.putShort((short) 13);
	//			accBuffer.put((byte) 1);
	//			accBuffer.putFloat(accelerometerData[0]);
	//			accBuffer.putFloat(accelerometerData[1]);
	//			accBuffer.putFloat(accelerometerData[2]);
	//			
	//			accBuffer.position(0);
	//			socketChan.write(accBuffer);
	//		} catch (Exception e) {
	//			handleNetworkError(e);
	//		}
	//	}

	public void notifyActivity() {
		Intent intent = new Intent(ERROR_MESSAGE);
		sendBroadcast(intent);
	}

	private void handleNetworkError(Exception e) {
		e.printStackTrace();
		handleNetworkError();
	}

	private void handleNetworkError() {
		stopSelf();
		notifyActivity();
	}
}
