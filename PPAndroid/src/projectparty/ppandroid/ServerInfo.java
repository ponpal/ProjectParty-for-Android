package projectparty.ppandroid;

import java.io.Serializable;

@SuppressWarnings("serial")

/**
 * Contains IP address, port and hostname for a server.
 * @author Pontus
 *
 */
public class ServerInfo implements Serializable {
	private byte[] ip;
	private int port;
	private String hostName;
	
	public ServerInfo(byte[] ip, int port, String hostName) {
		this.setIP(ip);
		this.setPort(port);
		this.setHostName(hostName);
	}
	
	public ServerInfo(byte[] ip, int port) {
		this(ip, port, "Default");
	}

	public byte[] getIP() {
		return ip;
	}

	public void setIP(byte[] ip) {
		this.ip = ip;
	}

	public int getPort() {
		return port;
	}

	public void setPort(int port2) {
		this.port = port2;
	}
	
	public String getHostName() {
		return hostName;
	}

	public void setHostName(String name) {
		this.hostName = name;
	}
	
	@Override
	public boolean equals(Object other) {
		ServerInfo info = (ServerInfo) other;
		
		for(int i = 0; i < 4; i++) {
			if(this.ip[i] != info.getIP()[i]) {
				return false;
			}
		}
		
		return this.port == info.getPort();
	}
	
	@Override
	public int hashCode() {
		return port;
	}
	
	@Override
	public String toString() {
		return hostName;
	}
}
