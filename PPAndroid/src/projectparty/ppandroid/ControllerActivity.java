package projectparty.ppandroid;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.view.WindowManager;
import android.widget.Toast;

public class ControllerActivity extends Activity {
	private Intent csIntent;
	private boolean backPressedToExitOnce = false;
	private Toast toast = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_controller);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		ServerInfo server = (ServerInfo) getIntent().getSerializableExtra("server");
		String alias = getIntent().getStringExtra("playerAlias");

		this.csIntent = new Intent(this, ControllerService.class);
		csIntent.putExtra("server", server);
		csIntent.putExtra("playerAlias", alias);
	}

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(errorReceiver, new IntentFilter(ControllerService.ERROR_MESSAGE));
	}

	@Override
	protected void onPause() {
		killToast();
		unregisterReceiver(errorReceiver);
		super.onPause();
	}

	@Override
	protected void onStop() {
		super.onStop();
		stopService(csIntent);
	}

	@Override
	public void onBackPressed() {
		if (backPressedToExitOnce) {
			super.onBackPressed();
		} else {
			this.backPressedToExitOnce = true;
			showToast("Press again to exit");
			new Handler().postDelayed(new Runnable() {
				@Override
				public void run() {
					backPressedToExitOnce = false;
				}
			}, 2000);
		}
	}

	private void showToast(String message) {
		if (this.toast == null) {
			// Create toast if found null, it would he the case of first call only
			this.toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);

		} else if (this.toast.getView() == null) {
			// Toast not showing, so create new one
			this.toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);

		} else {
			// Updating toast message is showing
			this.toast.setText(message);
		}

		// Showing toast finally
		this.toast.show();
	}

	/**
	 * Kill the toast if showing. Supposed to call from onPause() of activity.
	 * So that toast also get removed as activity goes to background, to improve
	 * better app experiance for user
	 */
	private void killToast() {
		if (this.toast != null) {
			this.toast.cancel();
		}
	}

	public void showErrorDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);	
		builder.setTitle("Disconnected")
		.setMessage("The connection to the server was lost.")
		.setPositiveButton("Reconnect", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				startService(csIntent);
			}
		})
		.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
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

	private BroadcastReceiver errorReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			showErrorDialog();
		}
	};

}
