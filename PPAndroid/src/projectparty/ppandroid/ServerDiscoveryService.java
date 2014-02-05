package projectparty.ppandroid;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;

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

public class ServerDiscoveryService extends Service implements SensorEventListener {
	private static final int INIT_MESSAGE = 0;
	private static final int UTF8_DATA = 1;
	private static final int UPDATE_ACCELEROMETER_DATA = 2;
	private static final int SEND_DATA = 3;
	private static final short PORT = 1337;
	
	public static final String LOG_MESSAGE = "projectparty.ppandroid";
	public static final String FOUND_SERVER_MESSAGE = "projectparty.ppandroid.server";

	private Looper looper;
	private ServiceHandler serviceHandler;
	private SensorManager sensorManager;
	private Sensor accelerometer;
	private float[] accelerometerData;

	private Socket socket;
	private DataOutputStream out;
	private DataInputStream in;

	private Timer timer;
	private boolean connected = false;
	private List<ServerInfo> servers;
	private Long sessionID;

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
		servers = new ArrayList<ServerInfo>();
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
				socket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			looper.quit();
		}

		logToPhone("Stopped the service.");
	}

	@Override
	public void onSensorChanged(SensorEvent e) {
		Message msg = serviceHandler.obtainMessage();
		msg.what = UPDATE_ACCELEROMETER_DATA;

		msg.obj = e;
		serviceHandler.sendMessage(msg); 	
	}

	public void sendAccelerometer() {
		Message msg = new Message();
		msg.what = SEND_DATA;

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
						refreshServerList();	
						break;
					case UTF8_DATA:
						break;
					case UPDATE_ACCELEROMETER_DATA:
						SensorEvent e = (SensorEvent) msg.obj;
						accelerometerData = e.values;
						break;
					case SEND_DATA:
						out.write(1);

						out.writeFloat(accelerometerData[0]);
						out.writeFloat(accelerometerData[1]);
						out.writeFloat(accelerometerData[2]);

						out.flush();
					}

				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}

		public void refreshServerList() throws UnknownHostException, IOException {
			int numServers = 0;
			int sameRepeated = 0;

			DatagramSocket udpSocket = new DatagramSocket(null);
			udpSocket.setSoTimeout(5000);
			udpSocket.setReuseAddress(true);
			udpSocket.bind(new InetSocketAddress(7331));

			byte[] udpBuffer = new byte[1024];
			DatagramPacket inPacket = new DatagramPacket(udpBuffer, udpBuffer.length);

			logToPhone("Looking for broadcasting servers...");
			
			while(true) {
				try {
					udpSocket.receive(inPacket);
					ServerInfo info = unpackBroadcastMessage(udpBuffer);
					
					if(!servers.contains(info)) {
						addServer(info);
						numServers++;
						sameRepeated = 0;
					} else {
						sameRepeated++;
					}
					
					if(sameRepeated > 5) {
						break;
					}
					
					continue;
				} catch (SocketTimeoutException ste) {
					break;
				}
			}
			
			logToPhone("Found " + numServers + " servers.");
			
			udpSocket.close();
			stopSelf();
		}
		
//		public void doUsefulStuffLater(byte[] ip, short port) throws UnknownHostException, IOException {
//			//CONNECT VIA TCP
//
//			socket = new Socket(InetAddress.getByAddress(ip), port);
//			out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream(), 8192));
//			in = new DataInputStream(socket.getInputStream());
//
//			connected = true;
//
//			//RECEIVE ID
//
//			long id = in.readLong();
//			logToPhone("ID: " + id);
//			sessionID = id;
//
//			out.write(0);
//			out.writeLong(id);
//			out.flush();
//
//			//SETUP ACCELEROMETER AND SEND THE DATA ON INTERVAL
//
//			setupAccelerometer();
//
//			timer = new Timer();
//			timer.schedule(new TimerTask() {
//				@Override
//				public void run() {
//					sendAccelerometer();
//				}
//
//			}, 100, 16);
//		}

		public ServerInfo unpackBroadcastMessage(byte[] udpBuffer) throws IOException {
			ServerInfo info = null;
			DataInputStream dis = new DataInputStream(new ByteArrayInputStream(udpBuffer));

			if(dis.readByte() == 'P' && dis.readByte() == 'P' && dis.readByte() == 'S') {
				byte[] ip = new byte[4];
				dis.read(ip, 0, 4);
				
				short port = dis.readShort();
				
				int serverNameLength = dis.readInt();
				byte[] serverName = new byte[256];
				
				dis.read(serverName, 0, serverNameLength);
				
				String name = new String(serverName, "UTF-8");
				info = new ServerInfo(ip, port, name);
			}

			return info;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		
	}

	private void logToPhone(String message) {
		Intent intent = new Intent(LOG_MESSAGE);
		intent.putExtra("message", message);
		sendBroadcast(intent);
	}
	
	private void addServer(ServerInfo info) {
		servers.add(info);
		
		Intent intent = new Intent(FOUND_SERVER_MESSAGE);
		intent.putExtra("server", info);
		sendBroadcast(intent);
	}
}
