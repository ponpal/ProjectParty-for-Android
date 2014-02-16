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
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

public class MainActivity extends Activity {
	private Intent sdsIntent;
	public static List<ServerInfo> serverList;
	private SharedPreferences preferences;

	private EditText aliasField;
	private TextView serversLabel;
	private ProgressBar refreshingIndicator;
	private Button refreshButton;
	private ListView serverListView;
	
	private String playerName;
	private ArrayAdapter<ServerInfo> adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		serverList = new ArrayList<ServerInfo>();
		preferences = getPreferences(MODE_PRIVATE);

		this.sdsIntent = new Intent(this, ServerDiscoveryService.class);
		initGUIComponents();
		startService(sdsIntent);
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		initGUIComponents();
		
		registerReceiver(serverReceiver, new IntentFilter(ServerDiscoveryService.FOUND_SERVER_MESSAGE));
		registerReceiver(serviceStoppedReceiver, new IntentFilter(ServerDiscoveryService.SEARCH_STOPPED_MESSAGE));
	}

	@Override
	protected void onPause() {
		super.onPause();
		stopService(sdsIntent);
		unregisterReceiver(serverReceiver);
		unregisterReceiver(serviceStoppedReceiver);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	public synchronized void refreshServerList(View view) {
		serverList.clear();
		
		refreshButton.setEnabled(false);
		refreshingIndicator.setVisibility(View.VISIBLE);
		serversLabel.setText(R.string.refreshing);
		startService(sdsIntent);

		adapter.notifyDataSetChanged();
	}

	private void initGUIComponents() {
		this.serverListView = (ListView) findViewById(R.id.serverListView);
		this.adapter = new ArrayAdapter<ServerInfo>(this, android.R.layout.simple_list_item_1, serverList);

		this.aliasField = (EditText) findViewById(R.id.aliasField);
		
		if(playerName != null) {
			aliasField.setText(playerName);
		}
		
		serverListView.setAdapter(adapter);
		serverListView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				ServerInfo info = serverList.get(arg2);
				playerName = aliasField.getText().length() > 0 ? aliasField.getText().toString() : "Guest";
				preferences.edit().putString("name", aliasField.getText().toString()).commit();
				startControllerActivity(info, playerName);
			}
		});

		this.refreshingIndicator = (ProgressBar) findViewById(R.id.refreshingIndicator);
		this.serversLabel = (TextView) findViewById(R.id.serversLabel);
		this.refreshButton = (Button) findViewById(R.id.refreshButton);
	}

	public void startControllerActivity(ServerInfo server, String playerName) {
		Intent intent = new Intent(this, ControllerActivity.class);
		intent.putExtra("server", server);
		intent.putExtra("playerName", playerName);
		startActivity(intent);
	}
	
	public void connectManually(View view) {
		Intent intent = new Intent(this, ManualConnectionActivity.class);
		startActivity(intent);
	}

	private BroadcastReceiver serverReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			adapter.notifyDataSetChanged();
		}
	};

	private BroadcastReceiver serviceStoppedReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			refreshButton.setEnabled(true);
			refreshingIndicator.setVisibility(View.INVISIBLE);
			serversLabel.setText("Servers found: " + serverList.size());
		}
	};
}
