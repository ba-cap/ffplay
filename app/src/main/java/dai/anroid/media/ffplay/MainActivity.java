package dai.anroid.media.ffplay;

import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private TextView mTxtInfo;

    static {
        System.loadLibrary("ffplay1.0");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTxtInfo = findViewById(R.id.txtInformation);
        mTxtInfo.setText(getFfplayInfo());
    }

    private native String getFfplayInfo();
}
