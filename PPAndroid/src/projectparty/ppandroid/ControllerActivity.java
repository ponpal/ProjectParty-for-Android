package projectparty.ppandroid;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.WindowManager;

public class ControllerActivity extends Activity {
	private Intent csIntent;
	
	private BroadcastReceiver errorReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			showErrorDialog();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_controller);
		
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		ServerInfo server = (ServerInfo) getIntent().getSerializableExtra("server");
		String name = getIntent().getStringExtra("playerName");
		
		this.csIntent = new Intent(this, ControllerService.class);
		csIntent.putExtra("server", server);
		csIntent.putExtra("name", name);
		
		startService(csIntent);
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(errorReceiver, new IntentFilter(ControllerService.ERROR_MESSAGE));
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		stopService(csIntent);
		unregisterReceiver(errorReceiver);
	}
	
	public void showErrorDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);	
		builder.setMessage("Something bad happened...")
			.setTitle("Oh no...")
			.setCancelable(false)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					goBackToMain();
				}
			});
		
		AlertDialog dialog = builder.create();
		dialog.show();
	}
	
	private void goBackToMain() {
		startActivity(new Intent(this, MainActivity.class));
	}
}
