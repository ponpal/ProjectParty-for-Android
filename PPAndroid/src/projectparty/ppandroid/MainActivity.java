package projectparty.ppandroid;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

public class MainActivity extends Activity {
	private Intent sdsIntent;
	public static List<ServerInfo> serverList;
	
	private ListView serverListView;
	private TextView logView;
	private EditText aliasField;
	private String playerName;
	
	private ArrayAdapter<ServerInfo> adapter;
	
	private BroadcastReceiver logReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			String current = logView.getText().toString();
			String message = intent.getStringExtra("message");

			logView.setText(current.length() > 0 ? logView.getText() + "\n" + message : message);
		}
	};
	private BroadcastReceiver serverReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			adapter.notifyDataSetChanged();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		serverList = new ArrayList<ServerInfo>();
		
		this.sdsIntent = new Intent(this, ServerDiscoveryService.class);
		initGUIComponents();
		startService(sdsIntent);
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
		stopService(sdsIntent);
		unregisterReceiver(logReceiver);
		unregisterReceiver(serverReceiver);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	public void refreshServerList(View view) {
		startService(sdsIntent);
	}
	
	public void clearLog(View view) {
		logView.setText("");
	}

	private void initGUIComponents() {
		this.serverListView = (ListView) findViewById(R.id.serverListView);
		this.logView = (TextView) findViewById(R.id.logView);
		this.aliasField = (EditText) findViewById(R.id.aliasField);
		
		this.adapter = new ArrayAdapter<ServerInfo>(this, android.R.layout.simple_list_item_1, serverList);
		
		serverListView.setAdapter(adapter);
		serverListView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				ServerInfo info = serverList.get(arg2);
				playerName = (aliasField.getText().length() > 0) ? aliasField.getText().toString() : "Guest";
				startControllerActivity(info, playerName);
			}
		});
		
		logView.setText("");
		logView.setMovementMethod(new ScrollingMovementMethod());
	}
	
	public void startControllerActivity(ServerInfo server, String playerName) {
		Intent intent = new Intent(this, ControllerActivity.class);
		intent.putExtra("server", server);
		intent.putExtra("playerName", playerName);
		startActivity(intent);
	}
}
