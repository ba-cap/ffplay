package dai.anroid.media.ffplay;

import android.app.Application;
import android.util.Log;

public class ThisApplication extends Application {
    private static final String TAG = "ThisApplication";

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
    public void onCreate() {
        Log.d(TAG, "[onCreate]");
        super.onCreate();
    }
}
