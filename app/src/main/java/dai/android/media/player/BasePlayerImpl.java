package dai.android.media.player;

import android.app.Application;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

class BasePlayerImpl implements BasePlayer {

    private Application mApp;
    protected final Handler H;
    private final HandlerThread thread;

    BasePlayerImpl(Application context) {
        mApp = context;

        thread = new HandlerThread("StateWorkThread");
        thread.start();
        H = new Handler(thread.getLooper());
    }

    void beforeAppExit() {
        thread.quitSafely();
    }

    @Override
    public Application getContext() {
        return mApp;
    }

    @Override
    public void handleMessage(Message message) {
        H.handleMessage(message);
    }

    @Override
    public boolean postAtFrontOfQueue(@NonNull Runnable r) {
        return H.postAtFrontOfQueue(r);
    }

    @Override
    public boolean post(@NonNull Runnable r) {
        return H.post(r);
    }

    @Override
    public boolean postDelayed(@NonNull Runnable r, long delayMillis) {
        return H.postAtTime(r, delayMillis);
    }

    @Override
    public boolean postDelayed(@NonNull Runnable r, @Nullable Object token, long delayMillis) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            return H.postDelayed(r, token, delayMillis);
        } else {
            return H.postDelayed(r, delayMillis);
        }
    }

    @Override
    public boolean postAtTime(@NonNull Runnable r, long uptimeMillis) {
        return H.postAtTime(r, uptimeMillis);
    }

    @Override
    public boolean postAtTime(@NonNull Runnable r, @Nullable Object token, long uptimeMillis) {
        return H.postAtTime(r, token, uptimeMillis);
    }

    void onHandlerMessage(Message msg) {
    }

    private static class WorkHandler extends Handler {
        private final BasePlayerImpl impl;

        WorkHandler(BasePlayerImpl impl, Looper looper) {
            super(looper);
            this.impl = impl;
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            impl.onHandlerMessage(msg);
            super.handleMessage(msg);
        }
    }
}
