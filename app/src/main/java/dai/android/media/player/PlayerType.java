package dai.android.media.player;

import androidx.annotation.IntDef;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import static dai.android.media.player.PlayerType.ANDROID_MEDIA_PLAYER;
import static dai.android.media.player.PlayerType.IJK_MEDIA_PLAYER;

@Retention(RetentionPolicy.SOURCE)
@Target({ElementType.METHOD, ElementType.PARAMETER, ElementType.FIELD, ElementType.LOCAL_VARIABLE})
@IntDef({ANDROID_MEDIA_PLAYER, IJK_MEDIA_PLAYER})
public @interface PlayerType {

    int ANDROID_MEDIA_PLAYER = 1;

    int IJK_MEDIA_PLAYER = 2;
}
