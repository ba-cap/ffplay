package dai.anroid.media.player;

import android.media.MediaPlayer;

import java.util.concurrent.atomic.AtomicReference;

final class Uid {

    private Uid() {
    }

    static final int BAD_SID = -1;

    private static final int GO_STEP = 2;
    private static long BASE_VALUE = 0;

    private static final AtomicReference<Thread> cas = new AtomicReference<>();

    public static long getId() {
        Thread current = Thread.currentThread();
        // Lock
        while (!cas.compareAndSet(null, current)) {
            // do noting
        }

        long newValue = -1;
        if (BASE_VALUE >= Long.MAX_VALUE) {
            BASE_VALUE = 0;
        }
        BASE_VALUE += GO_STEP;

        newValue = BASE_VALUE;

        // UnLock
        cas.compareAndSet(Thread.currentThread(), null);

        return newValue;
    }
}
