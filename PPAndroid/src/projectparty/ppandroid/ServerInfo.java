package projectparty.ppandroid;

import java.io.Serializable;

@SuppressWarnings("serial")
public class ServerInfo implements Serializable {
	private byte[] ip;
	private short port;
	private String name;
	
	public ServerInfo(byte[] ip, short port, String name) {
		this.setIP(ip);
		this.setPort(port);
		this.setName(name);
	}

	public byte[] getIP() {
		return ip;
	}

	public void setIP(byte[] ip) {
		this.ip = ip;
	}

	public short getPort() {
		return port;
	}

	public void setPort(short port) {
		this.port = port;
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
			if(!(this.ip[i] == info.getIP()[i])) {
				return false;
			}
		}
		
		return this.port == info.getPort();
	}
	
	@Override
	public String toString() {
		return "IP: " + ip.toString() + "\nPort: " + port;
	}
}
