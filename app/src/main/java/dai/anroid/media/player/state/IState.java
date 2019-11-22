package dai.anroid.media.player.state;

import android.os.Message;

public interface IState {

    void enter();

    void exit();

    boolean processMessage(Message msg);
}
