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

/**
 * Activity for finding and connecting to a server.
 * This is also where the player selects his/her name/alias.
 * @author Pontus
 *
 */
public class MainActivity extends Activity {
	
	/** Intent used to start the ServerDiscoveryService */
	private Intent sdsIntent;
	
	/** The list of servers backing the serverListView in the GUI. Updated by ServerDiscoveryService. */
	public static List<ServerInfo> serverList;
	
	/** Accessor for SharedPreferences. */
	private SharedPreferences preferences;

	private EditText aliasField;
	private TextView headerTextView;
	private ProgressBar refreshingIndicator;
	private Button refreshButton;
	private ListView serverListView;
	
	/** The alias (name) of the player that is used for games. */
	private String playerAlias;
	
	/** Connects the datasource (serverList) to the view (serverListView). */
	private ArrayAdapter<ServerInfo> adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		serverList = new ArrayList<ServerInfo>();
		preferences = getPreferences(MODE_PRIVATE);

		this.sdsIntent = new Intent(this, ServerDiscoveryService.class);
		this.playerAlias = preferences.getString("playerAlias", "");
		initGUIComponents();
		refreshServerList(null);
	}

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(serverReceiver, new IntentFilter(ServerDiscoveryService.FOUND_SERVER_MESSAGE));
		registerReceiver(serviceStoppedReceiver, new IntentFilter(ServerDiscoveryService.SEARCH_STOPPED_MESSAGE));
		
		refreshServerList(null);
	}

	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(serverReceiver);
		unregisterReceiver(serviceStoppedReceiver);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	/**
	 * Refreshes the list of servers available to the user.
	 * @param view Not used.
	 */
	public synchronized void refreshServerList(View view) {
		serverList.clear();
		
		refreshButton.setEnabled(false);
		refreshingIndicator.setVisibility(View.VISIBLE);
		headerTextView.setText(R.string.refreshing);
		startService(sdsIntent);

		adapter.notifyDataSetChanged();
	}

	/**
	 * Initializes the various GUI components used in the MainActivity view.
	 */
	private void initGUIComponents() {
		this.serverListView = (ListView) findViewById(R.id.serverListView);
		this.adapter = new ArrayAdapter<ServerInfo>(this, android.R.layout.simple_list_item_1, serverList);

		this.aliasField = (EditText) findViewById(R.id.aliasField);
		
		if(playerAlias != null) {
			aliasField.setText(playerAlias);
		}
		
		serverListView.setAdapter(adapter);
		serverListView.setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				ServerInfo info = serverList.get(arg2);
				playerAlias = aliasField.getText().length() > 0 ? aliasField.getText().toString() : "Guest";
				preferences.edit().putString("playerAlias", aliasField.getText().toString()).commit();
				startControllerActivity(info);
			}
		});

		this.refreshingIndicator = (ProgressBar) findViewById(R.id.refreshingIndicator);
		this.headerTextView = (TextView) findViewById(R.id.serversLabel);
		this.refreshButton = (Button) findViewById(R.id.refreshButton);
	}

	/**
	 * Starts the ControllerActivity.
	 * @param server ServerInfo containing IP and port from the list.
	 */
	public void startControllerActivity(ServerInfo server) {
		Intent intent = new Intent(this, ControllerActivity.class);
		intent.putExtra("server", server);
		intent.putExtra("playerAlias", playerAlias);
		startActivity(intent);
	}
	
	/**
	 * Opens the ManualConnectionActivity with the playerAlias sent via intent.
	 * @param view Not used.
	 */
	public void connectManually(View view) {
		Intent intent = new Intent(this, ManualConnectionActivity.class);
		intent.putExtra("playerAlias", playerAlias);
		startActivity(intent);
	}

	/**
	 * Receives and handles broadcasts when a new server is discovered by the ServerDiscoveryService.
	 */
	private BroadcastReceiver serverReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			adapter.notifyDataSetChanged();
		}
	};

	/**
	 * Receives and handles a broadcast when the ServerDiscoveryService has stopped its search.
	 */
	private BroadcastReceiver serviceStoppedReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			refreshButton.setEnabled(true);
			refreshingIndicator.setVisibility(View.INVISIBLE);
			headerTextView.setText("Servers found: " + serverList.size());
		}
	};
}
