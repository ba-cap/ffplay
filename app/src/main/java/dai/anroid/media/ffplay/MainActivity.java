package dai.anroid.media.ffplay;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    private TextView mTxtInfo;

    static {
        //System.loadLibrary("avcodec");
        //System.loadLibrary("avfilter");
        //System.loadLibrary("avformat");
        //System.loadLibrary("avutil");
        //System.loadLibrary("swresample");
        //System.loadLibrary("swscale");
        System.loadLibrary("ffplay1.0");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        verifyStoragePermissions();

        mTxtInfo = findViewById(R.id.txtInformation);
        mTxtInfo.setText(getFfplayInfo());
    }

    private native String getFfplayInfo();

    private void verifyStoragePermissions() {
        String[] storagePermissions = {
                "android.permission.READ_EXTERNAL_STORAGE",
                "android.permission.WRITE_EXTERNAL_STORAGE"
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
