package projectparty.ppandroid;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

public class MainActivity extends Activity {
	private Intent intent;
	private EditText ipAddressField;
	private EditText portField;
	private SharedPreferences preferences;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		this.intent = new Intent(this, GameControlService.class);
		this.ipAddressField = (EditText) findViewById(R.id.ipAddressField);
		this.portField = (EditText) findViewById(R.id.portField);

		this.preferences = getPreferences(MODE_PRIVATE);
		
		this.ipAddressField.setText(preferences.getString("ip", ""));
		this.portField.setText(preferences.getString("port", ""));
	}

	public void startService(View view) {
		preferences.edit().putString("ip", ipAddressField.getText().toString()).commit();
		preferences.edit().putString("port", portField.getText().toString()).commit();

		intent.putExtra("ip", ipAddressField.getText().toString());
		intent.putExtra("port", Integer.parseInt(portField.getText().toString()));

		startService(intent);
	}

	public void stopService(View view) {
		stopService(intent);
	}
}
