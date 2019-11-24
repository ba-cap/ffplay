package dai.android.media.player;

import androidx.annotation.IntDef;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import static dai.android.media.player.MediaType.LIVE;
import static dai.android.media.player.MediaType.LOOP;
import static dai.android.media.player.MediaType.VOD;

@Retention(RetentionPolicy.SOURCE)
@Target({ElementType.METHOD, ElementType.PARAMETER, ElementType.FIELD, ElementType.LOCAL_VARIABLE})
@IntDef({VOD, LOOP, LIVE})
public @interface MediaType {
    // 点播
    int VOD = 1;

    // 轮播
    int LOOP = 2;

    // 直播
    int LIVE = 3;
}
