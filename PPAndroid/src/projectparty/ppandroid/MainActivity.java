package projectparty.ppandroid;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextSwitcher;

public class MainActivity extends Activity {
	private Intent gcsIntent;
	private SharedPreferences preferences;
	private BroadcastReceiver receiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
//			String message = intent.getStringExtra("message");
//			textSwitcher.setText(message);
		}
	};
	
	private EditText ipAddressField, portField;
	private Button startButton, stopButton;
	private TextSwitcher textSwitcher;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		this.gcsIntent = new Intent(this, GameControlService.class);
		this.preferences = getPreferences(MODE_PRIVATE);
		
		initGraphicalComponents();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(receiver, new IntentFilter(GameControlService.NOTIFICATION));
	}
	
	@Override
	  protected void onPause() {
	    super.onPause();
	    unregisterReceiver(receiver);
	  }

	public void startService(View view) {
		toggleButtons(false);
		
		preferences.edit().putString("ip", ipAddressField.getText().toString()).commit();
		preferences.edit().putString("port", portField.getText().toString()).commit();

		gcsIntent.putExtra("ip", ipAddressField.getText().toString());
		gcsIntent.putExtra("port", Integer.parseInt(portField.getText().toString()));

		startService(gcsIntent);
	}

	public void stopService(View view) {
		toggleButtons(true);
		stopService(gcsIntent);
	}
	
	private void initGraphicalComponents() {
		this.ipAddressField = (EditText) findViewById(R.id.ipAddressField);
		this.portField = (EditText) findViewById(R.id.portField);
		this.startButton = (Button) findViewById(R.id.startBtn);
		this.stopButton = (Button) findViewById(R.id.stopBtn);
		
		ipAddressField.setText(preferences.getString("ip", ""));
		portField.setText(preferences.getString("port", ""));
		
		toggleButtons(true);
	}
	
	public void toggleButtons(boolean enabled) {
		startButton.setEnabled(enabled);
		stopButton.setEnabled(!enabled);
	}
}
