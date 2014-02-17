package projectparty.ppandroid;

import java.net.InetAddress;
import java.net.UnknownHostException;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

public class ManualConnectionActivity extends Activity {
	private String playerAlias;
	private EditText ipField, portField;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_manual_connection);

		this.playerAlias = getIntent().getStringExtra("playerAlias");
		this.ipField = (EditText) findViewById(R.id.ipField);
		this.portField = (EditText) findViewById(R.id.portField);
	}

	public void connect(View view) {
		boolean noExceptions = true;
		ServerInfo info = null;

		try {
			info = new ServerInfo(InetAddress.getByName(ipField.getText().toString()).getAddress(), 
					Integer.parseInt(portField.getText().toString()));
		} catch (NumberFormatException e) {
			Toast.makeText(this, "Invalid IP address.", Toast.LENGTH_SHORT).show();
			e.printStackTrace();
			noExceptions = false;
		} catch (UnknownHostException e) {
			Toast.makeText(this, "Unknown host.", Toast.LENGTH_SHORT).show();
			e.printStackTrace();
			noExceptions = false;
		}

		if(noExceptions) {
			Intent intent = new Intent(this, ControllerActivity.class);
			intent.putExtra("server", info);
			intent.putExtra("playerAlias", playerAlias);
			startActivity(intent);
		}
	}
}
