package dai.anroid.media.player;

import androidx.annotation.Nullable;

import dai.anroid.media.player.state.IState;

public abstract class AbstractTypedPlayer<CB extends IBasePlayerListener>
        implements IState, IMediaPlayer {

    protected final static int CMD_DATA_SOURCE_SET = 1000;
    protected final static int CMD_DATA_SOURCE_CHG = 1001;
    protected final static int CMD_START = 1002;
    protected final static int CMD_PAUSE = 1003;
    protected final static int CMD_RELEASE = 1004;
    protected final static int CMD_SEEK = 1005;
    protected final static int CMD_CHG_DEFINITION = 1006;
    protected final static int CMD_CHG_ENGINER = 1007;
    protected final static int CMD_SLEEP = 1008;
    protected final static int CMD_WAKEUP = 1009;
    protected final static int CMD_CUSTOM_BEGIN = 2000;

    protected final BasePlayer BP;
    protected CB callback;

    private long mPlaySessionId = -1;

    protected AbstractTypedPlayer(BasePlayer player) {
        BP = player;
    }

    final BasePlayer getBasePlayer() {
        return BP;
    }

    final void eraseCallback() {
        callback = null;
    }

    protected final long getPlaySessionId() {
        return mPlaySessionId;
    }

    @Nullable
    public abstract IData4Ui getUiData();

    @MediaType
    public abstract int getMediaType();

    @Override
    public void enter() {
    }

    @Override
    public void exit() {
    }
}




