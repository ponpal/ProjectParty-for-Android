package projectparty.ppandroid;

import java.io.Serializable;

@SuppressWarnings("serial")
public class ServerInfo implements Serializable {
	private byte[] ip;
	private int port;
	private String name;
	
	public ServerInfo(byte[] ip, int port, String name) {
		this.setIP(ip);
		this.setPort(port);
		this.setName(name);
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
	
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
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
		return name;
	}
}
