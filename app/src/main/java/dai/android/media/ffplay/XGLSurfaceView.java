package dai.android.media.ffplay;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

public class XGLSurfaceView extends GLSurfaceView implements Runnable, SurfaceHolder.Callback {
    private static final String TAG = "XGLSurfaceView";

    private Thread mWorkThread;
    private boolean mStop = false;

    public XGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        Log.d(TAG, "will open native player");
        open_player("/storage/emulated/0/ffmpeg-test/1080.mp4", getHolder().getSurface());
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        super.surfaceCreated(holder);

        Log.d(TAG, ">> surfaceCreated");
        if (null != mWorkThread) {
            mWorkThread.interrupt();
        }

        mWorkThread = new Thread(this);
        mWorkThread.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);

        Log.d(TAG, ">> surfaceDestroyed");
        if (null != mWorkThread) {
            mWorkThread.interrupt();
            mWorkThread = null;
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        super.surfaceChanged(holder, format, w, h);

        Log.d(TAG, ">> surfaceChanged: format=" + format + ", width=" + w + ", height=" + h);
    }


    private native void open_player(String url, Surface surface);
}
