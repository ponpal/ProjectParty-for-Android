package projectparty.ppandroid.services;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

import projectparty.ppandroid.MainActivity;
import projectparty.ppandroid.ServerInfo;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

/**
 * Service for finding active game servers on the local network.
 * @author Pontus
 *
 */
public class ServerDiscoveryService extends Service {
	/** String constant for broadcasted log messages */
	public static final String LOG_MESSAGE = "projectparty.ppandroid.log";
	
	/** String constant for broadcasted "found server" messages */
	public static final String FOUND_SERVER_MESSAGE = "projectparty.ppandroid.server";
	
	/** String constant for broadcasted "search stopped" message **/
	public static final String SEARCH_STOPPED_MESSAGE = "projectparty.ppandroid.stopped";

	/** Message looper */
	private Looper looper;
	
	/** Handles the messages sent to the service */
	private ServiceHandler serviceHandler;

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

		serviceHandler.sendMessage(msg);
		return START_STICKY;
	}

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	/**
	 * Inner class for handling the initial message sent from onStartCommand.
	 * @author Pontus
	 *
	 */
	private final class ServiceHandler extends Handler {
		public ServiceHandler(Looper looper) {
			super(looper);
		}

		@Override
		public void handleMessage(Message msg) {
			synchronized (this) {
				try {
					refreshServerList();	
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}

		/**
		 * For each local server, this method receives an UDP broadcast message and
		 * adds the server to the list of available servers in MainActivity if the 
		 * list doesn't already contain it. 
		 * @throws UnknownHostException
		 * @throws IOException
		 */
		public void refreshServerList() throws UnknownHostException, IOException {
			int sameRepeated = 0;

			DatagramSocket udpSocket = new DatagramSocket(null);
			udpSocket.setSoTimeout(2500);
			udpSocket.setReuseAddress(true);
			udpSocket.bind(new InetSocketAddress(7331));

			while(true) {
				try {
					byte[] udpBuffer = new byte[32];
					DatagramPacket inPacket = new DatagramPacket(udpBuffer, udpBuffer.length);

					udpSocket.receive(inPacket);
					ServerInfo info = unpackBroadcastMessage(udpBuffer);

					if(!MainActivity.serverList.contains(info)) {
						MainActivity.serverList.add(info);
						notifyActivity(FOUND_SERVER_MESSAGE);
						sameRepeated = 0;
					} else {
						sameRepeated++;
					}

					if(sameRepeated > 2) break;
				} catch (SocketTimeoutException ste) {
					break;
				}
			}
			
			udpSocket.close();
			udpSocket.disconnect();	

			stopSelf();

			notifyActivity(SEARCH_STOPPED_MESSAGE);
		}
		
		private ServerInfo unpackBroadcastMessage(byte[] udpBuffer) throws IOException {
			ServerInfo info = null;
			DataInputStream dis = new DataInputStream(new ByteArrayInputStream(udpBuffer));

			if(dis.readByte() == 'P' && dis.readByte() == 'P' && dis.readByte() == 'S') {
				byte[] ip = new byte[4];
				dis.read(ip, 0, 4);

				int port = dis.readUnsignedShort();

				int serverNameLength = dis.readInt();
				byte[] serverName = new byte[256];

				dis.read(serverName, 0, serverNameLength);

				String name = new String(serverName, "UTF-8");
				info = new ServerInfo(ip, port, name);
			}

			return info;
		}
	}

	private void notifyActivity(String message) {
		Intent intent = new Intent(message);
		sendBroadcast(intent);
	}
}
