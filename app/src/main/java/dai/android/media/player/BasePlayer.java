package dai.android.media.player;

import android.app.Application;
import android.os.Message;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public interface BasePlayer {

    Application getContext();

    void handleMessage(Message message);

    boolean postAtFrontOfQueue(@NonNull Runnable r);

    boolean post(@NonNull Runnable r);

    boolean postDelayed(@NonNull Runnable r, long delayMillis);

    boolean postDelayed(@NonNull Runnable r, @Nullable Object token, long delayMillis);

    boolean postAtTime(@NonNull Runnable r, long uptimeMillis);

    boolean postAtTime(@NonNull Runnable r, @Nullable Object token, long uptimeMillis);
}
