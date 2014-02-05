package projectparty.ppandroid;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	private Intent gcsIntent;
	private SharedPreferences preferences;
	private List<ServerInfo> servers;
	
	private Button refreshButton;
	private TextView messageView;
	
	private BroadcastReceiver logReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String current = messageView.getText().toString();
			String message = intent.getStringExtra("message");

			messageView.setText(current.length() > 0 ? messageView.getText() + "\n" + message : message);
		}
	};
	
	private BroadcastReceiver serverReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			ServerInfo info = (ServerInfo) intent.getSerializableExtra("server");
			servers.add(info);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		this.gcsIntent = new Intent(this, ServerDiscoveryService.class);
		this.preferences = getPreferences(MODE_PRIVATE);
		this.servers = new ArrayList<ServerInfo>();

		initGUIComponents();
	}

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(logReceiver, new IntentFilter(ServerDiscoveryService.LOG_MESSAGE));
		registerReceiver(serverReceiver, new IntentFilter(ServerDiscoveryService.FOUND_SERVER_MESSAGE));
	}

	@Override
	protected void onPause() {
		super.onPause();
		stopService(gcsIntent);
		unregisterReceiver(logReceiver);
		unregisterReceiver(serverReceiver);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	public void refreshServerList(View view) {
		startService(gcsIntent);
	}
	
	public void clearText(View view) {
		messageView.setText("");
	}

	private void initGUIComponents() {
		this.refreshButton = (Button) findViewById(R.id.startBtn);
		this.messageView = (TextView) findViewById(R.id.textView1);
		messageView.setText("");
		messageView.setMovementMethod(new ScrollingMovementMethod());
	}
}
