package dai.android.media.ffplay;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";

    private TextView mTxtInfo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        verifyStoragePermissions();

        mTxtInfo = findViewById(R.id.txtInformation);
        mTxtInfo.setText(getFfplayInfo());

        findViewById(R.id.btnAudio).setOnClickListener(this);
        findViewById(R.id.btnPlayPcmAudio).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnAudio: {
                Intent intent = new Intent(MainActivity.this, RecorderActivity.class);
                startActivity(intent);
                break;
            }

            case R.id.btnPlayPcmAudio: {
                new Thread(() -> {
                    playAudio(getAssets(), "demo/demo-audio-test.pcm");
                }).start();

                break;
            }
        }
    }

    private native void playAudio(AssetManager assetManager, String name);

    private native String getFfplayInfo();

    private void verifyStoragePermissions() {
        String[] storagePermissions = {
                "android.permission.READ_EXTERNAL_STORAGE",
                "android.permission.WRITE_EXTERNAL_STORAGE",
                "android.permission.RECORD_AUDIO"
        };

        try {
            int permission = checkCallingPermission(storagePermissions[0]);
            if (permission != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(storagePermissions, 1);
            }

            permission = checkCallingPermission(storagePermissions[1]);
            if (permission != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(storagePermissions, 1);
            }
        } catch (Exception e) {
            Log.e(TAG, "requestPermissions failed.", e);
        }

    }
}
