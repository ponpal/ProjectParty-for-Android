package projectparty.ppandroid;

import java.util.ArrayList;
import java.util.List;

import projectparty.ppandroid.services.ControllerService;
import projectparty.ppandroid.services.ServerDiscoveryService;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Activity for finding and connecting to a local server.
 * This is also where players input their names/aliases.
 * @author Pontus
 *
 */
public class MainActivity extends Activity {
	/** The list of servers backing the serverListView in the GUI. Updated by ServerDiscoveryService. */
	public static List<ServerInfo> serverList = new ArrayList<ServerInfo>();

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

		this.preferences = getPreferences(MODE_PRIVATE);
		this.playerAlias = preferences.getString("playerAlias", "");
		
		initGUI();
		refresh();
	}

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(serverReceiver, new IntentFilter(ServerDiscoveryService.FOUND_SERVER_MESSAGE));
		registerReceiver(serviceStoppedReceiver, new IntentFilter(ServerDiscoveryService.SEARCH_STOPPED_MESSAGE));
		refresh();
	}

	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(serverReceiver);
		unregisterReceiver(serviceStoppedReceiver);
	}
	
	/**
	 * Refreshes the list of server if the device is connected to a network, else shows a notification.
	 */
	private void refresh() {
		if(isConnected()) {
			refreshServerList(null);
		} else {
			Toast t = Toast.makeText(this, "To be able to play, connect to a network.", Toast.LENGTH_LONG);
			t.setGravity(Gravity.CENTER, 0, 0);
			t.show();
		}
	}
	
	/**
	 * Checks if the device is connected to a network.
	 * @return true if connected, false otherwise.
	 */
	private boolean isConnected() {
		ConnectivityManager manager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
		try {
			return manager.getActiveNetworkInfo().isConnected();
		} catch (NullPointerException e) {
			return false;
		}
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
		startService(new Intent(this, ServerDiscoveryService.class));

		adapter.notifyDataSetChanged();
	}

	/**
	 * Initializes the various GUI components used in the MainActivity view.
	 */
	private void initGUI() {
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
				storePlayerAlias();
				startController(info);
			}
		});

		this.refreshingIndicator = (ProgressBar) findViewById(R.id.refreshingIndicator);
		this.headerTextView = (TextView) findViewById(R.id.serversLabel);
		this.refreshButton = (Button) findViewById(R.id.refreshButton);
	}

	/**
	 * Stores the player's alias in SharedPreferences.
	 */
	private void storePlayerAlias() {
		playerAlias = aliasField.getText().length() > 0 ? aliasField.getText().toString() : "Guest";
		preferences.edit().putString("playerAlias", aliasField.getText().toString()).commit();
	}

	/**
	 * Starts the ControllerService.
	 * @param server ServerInfo containing IP and port from the list.
	 */
	public void startController(ServerInfo server) {
		Intent serviceIntent = new Intent(this, ControllerService.class);
		serviceIntent.putExtra("server", server);
		serviceIntent.putExtra("playerAlias", playerAlias);
		startService(serviceIntent);
		
		Intent activityIntent = new Intent(this, MyNativeActivity.class);
		startActivity(activityIntent);
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

	public native int cmain();
}
