package projectparty.ppandroid.services;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

import projectparty.ppandroid.ServerInfo;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class ControllerService extends Service {
	public static final String ERROR_MESSAGE = "projectparty.ppandroid.error";

	private SocketChannel socketChan;
	private boolean connected = false;

	private ServerInfo latestServer;
	private String playerAlias;

	//Fields accessed by the native code
	public ByteBuffer inBuffer;
	public ByteBuffer outBuffer;
	public static ControllerService instance;
	public static int toAccess;
	public static Long sessionID;

	@Override
	public void onCreate() {
		instance = this;
		this.inBuffer  = ByteBuffer.allocateDirect(0xFFFF);
		this.outBuffer = ByteBuffer.allocateDirect(0xFFFF);

	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		ServerInfo info = (ServerInfo) intent.getSerializableExtra("server");
		this.playerAlias = intent.getStringExtra("playerAlias");
		this.latestServer = info;
		
		return START_STICKY;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();

		if(connected) {
			try {
				socketChan.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
			connected = false;
		}
		instance = null;
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	public int receive() {
		try {
			inBuffer.position(0);
			return socketChan.read(inBuffer);
		} catch (IOException e) {
			e.printStackTrace();
			return 0;
		}
	}

	public int send(int size) {
		if(size > 0) {
			outBuffer.position(0);
			outBuffer.limit(size);
			try {
				int written = socketChan.write(outBuffer);
				
				return written;
			} catch (IOException e) {
				//e.printStackTrace();
				return -1;
			}
		}

		return 0;
	}

	
	public int isAlive() { 
		return socketChan.isConnected() ? 1 : 0;
	}
	
	public int connect() {
		try {
			socketChan = SocketChannel.open();
			socketChan.connect(new InetSocketAddress(InetAddress.getByAddress(latestServer.getIP()), 
					latestServer.getPort()));
		
			ByteBuffer sessionIdBuffer = ByteBuffer.allocateDirect(8);
			sessionIdBuffer.order(ByteOrder.LITTLE_ENDIAN);

			if(socketChan.finishConnect()) {
				socketChan.configureBlocking(true);
				socketChan.read(sessionIdBuffer);
				socketChan.configureBlocking(false);
			} else {
				handleNetworkError();
			}
	
			sessionIdBuffer.flip();
			
			if(sessionID == null) {
				sessionID = sessionIdBuffer.getLong();
			} else {
				sessionIdBuffer.putLong(sessionID);
			}

			sessionIdBuffer.position(0);
			socketChan.write(sessionIdBuffer);

			connected = true;
			return 1;
		} catch (IOException e) {
			e.printStackTrace();
			return 0;
		}
	}

	public int reconnect() {
		return connect();
	}
	
	public int shutdown() {
		stopSelf();
		return 1;
	}

	public void sendAlias() throws IOException {
		byte[] byteAlias = playerAlias.getBytes("UTF-8");

		ByteBuffer aliasBuffer = ByteBuffer.allocateDirect(byteAlias.length + 3);
		aliasBuffer.order(ByteOrder.LITTLE_ENDIAN);
		aliasBuffer.putShort((short) (byteAlias.length + 1));
		aliasBuffer.put((byte) 0);
		aliasBuffer.put(byteAlias);

		aliasBuffer.position(0);
		socketChan.write(aliasBuffer);
	}

	private void handleNetworkError() {
		stopSelf();
		notifyActivity();
	}
	
	public void notifyActivity() {
		Intent intent = new Intent(ERROR_MESSAGE);
		sendBroadcast(intent);
	}
}