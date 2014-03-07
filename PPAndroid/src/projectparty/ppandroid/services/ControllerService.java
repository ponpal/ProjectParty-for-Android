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
	public static final int COULD_NOT_RECONNECT = 500;
	private static final int SERVER_REFUSED_RECONNECT = 250;

	private SocketChannel socketChan;
	private boolean connected = false;

	private ServerInfo latestServer;
	private String playerAlias;

	//Fields accessed by the native code
	public ByteBuffer inBuffer;
	public ByteBuffer outBuffer;
	public static ControllerService instance;
	public static int toAccess;
	public Long sessionID;

	@Override
	public void onCreate() {
		instance = this;
		this.inBuffer  = ByteBuffer.allocateDirect(0xFFFFF);
		this.outBuffer = ByteBuffer.allocateDirect(0xFFFFF);

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
			disconnect();
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
			inBuffer.limit(0xFFFF);
			int read = socketChan.read(inBuffer);
			if (read == -1)
				System.err.println("WTGGGFFFFFF");
			return read;
			//return socketChan.read(inBuffer);
		} catch (Throwable e) {

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
				e.printStackTrace();
				return -1;
			}
		}

		return 0;
	}

	public int isAlive() { 
		boolean b = socketChan.isConnected();
		return b ? 1 : 0;
	}

	public int connect() {
		try {
			socketChan = SocketChannel.open();
			socketChan.socket().connect(new InetSocketAddress(InetAddress.getByAddress(latestServer.getIP()), 
					latestServer.getPort()), 1000);

			ByteBuffer buffer = ByteBuffer.allocateDirect(8);
			buffer.order(ByteOrder.LITTLE_ENDIAN);

			if(socketChan.finishConnect()) {
				socketChan.configureBlocking(true);
				socketChan.read(buffer);
			} else {
				handleNetworkError();
			}

			buffer.flip();
			sessionID = buffer.getLong();

			buffer.position(0);
			socketChan.write(buffer);

			connected = true;
			socketChan.configureBlocking(false);

			sendAlias();
			return 1;
		} catch (IOException e) {
			e.printStackTrace();
			return 0;
		}
	}

	public int reconnect() {
		if(sessionID == null) {
			return COULD_NOT_RECONNECT;
		}

		try {
			socketChan = SocketChannel.open();
			socketChan.connect(new InetSocketAddress(InetAddress.getByAddress(latestServer.getIP()), 
					latestServer.getPort()));

			ByteBuffer buffer = ByteBuffer.allocateDirect(8);
			buffer.order(ByteOrder.LITTLE_ENDIAN);

			if(socketChan.finishConnect()) {
				socketChan.configureBlocking(true);
				socketChan.read(buffer);
			} else {
				handleNetworkError();
			}

			buffer.position(0);
			buffer.putLong(sessionID);
			buffer.flip();
			socketChan.write(buffer);
			
			buffer.position(0);
			buffer.limit(1);
			socketChan.read(buffer);
			buffer.position(0);
			if(buffer.get(0) == 0)
			{
				socketChan.close();
				return SERVER_REFUSED_RECONNECT;
			}
			socketChan.configureBlocking(false);
		} catch (IOException e) {
			e.printStackTrace();
			return 0;
		}

		return 1;
	}

	public int disconnect() {
		try {
			socketChan.close();
		} catch (Exception e) {
			e.printStackTrace();
			return 0;
		}
		connected = false;
		return 1;
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

