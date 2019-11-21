package dai.anroid.media.ffplay;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

public class RecorderActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String AUDIO_FILE_NAME = "my_record_audio.pcm";
    private static String AUDIO_FILE;

    private static final int CHANNELS = 1;
    private static final int SAMPLE_RATE = 44100;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder);

        findViewById(R.id.btnPlayStart).setOnClickListener(this);
        findViewById(R.id.btnPlayStop).setOnClickListener(this);

        findViewById(R.id.btnRecordStart).setOnClickListener(this);
        findViewById(R.id.btnRecordStop).setOnClickListener(this);

        AUDIO_FILE = getCacheDir() + "/" + AUDIO_FILE_NAME;

        _init();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        _destroy();
    }

    @Override
    public void onClick(View v) {

        switch (v.getId()) {

            // play start
            case R.id.btnPlayStart: {
                if (!_isPlaying()) {
                    _startPlay(AUDIO_FILE);
                }
            }
            break;

            case R.id.btnPlayStop: {
                _stopPlay();
            }
            break;

            // record
            case R.id.btnRecordStart: {
                if (!_isRecord()) {
                    _startRecord(AUDIO_FILE);
                }
            }
            break;

            case R.id.btnRecordStop: {
                _stopRecord();
            }
            break;
        }

    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    private native void _init();

    private native void _destroy();

    private native boolean _isRecord();

    private native void _startRecord(String file);

    private native void _stopRecord();

    private native boolean _isPlaying();

    private native void _startPlay(String file);

    private native void _stopPlay();
}
