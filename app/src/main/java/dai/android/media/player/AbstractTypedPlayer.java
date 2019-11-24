package dai.android.media.player;

import android.os.Message;
import android.view.ViewGroup;

import androidx.annotation.Nullable;

import dai.android.media.player.state.IState;

public abstract class AbstractTypedPlayer<CB extends IBasePlayerListener>
        implements IState, IMiscMediaPlayer {

    protected final static int CMD_DATA_SOURCE_SET = 1000;
    protected final static int CMD_DATA_SOURCE_CHG = 1001;
    protected final static int CMD_START = 1002;
    protected final static int CMD_PAUSE = 1003;
    protected final static int CMD_RELEASE = 1004;
    protected final static int CMD_SEEK = 1005;
    protected final static int CMD_CHG_DEFINITION = 1006;
    protected final static int CMD_CHG_ENGINE = 1007;
    protected final static int CMD_SLEEP = 1008;
    protected final static int CMD_WAKEUP = 1009;
    protected final static int CMD_CUSTOM_BEGIN = 2000;

    protected final BasePlayer base_player;
    protected CB callback;

    private long SID = -1;

    protected AbstractTypedPlayer(BasePlayer player) {
        base_player = player;
    }

    final BasePlayer getBasePlayer() {
        return base_player;
    }

    final void eraseCallback() {
        callback = null;
    }

    protected final long getPlaySessionId() {
        return SID;
    }

    protected final void makePlaySessionId() {
        SID = Uid.getId();
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
        SID = Uid.BAD_SID;
    }

    @Override
    public boolean processMessage(Message msg) {
        if (null == msg) {
            return false;
        }

        switch (msg.what) {
            case CMD_DATA_SOURCE_SET: {
                if (msg.obj instanceof DataSourceInner) {
                    DataSourceInner inner = (DataSourceInner) msg.obj;
                    setDataSourceImpl(inner.viewGroup, inner.callback, inner.source);
                }
            }
            break;

            case CMD_START: {
                startImpl();
            }
            break;

            case CMD_DATA_SOURCE_CHG: {
                if (msg.obj instanceof IDataSource) {
                    changeDataSourceImpl((IDataSource) msg.obj);
                }
            }
            break;

            case CMD_PAUSE: {
                pauseImpl();
            }
            break;

            case CMD_RELEASE: {
                releaseImpl();
            }
            break;

            case CMD_SEEK: {
                seekToImpl(msg.arg1);
            }
            break;

            case CMD_CHG_DEFINITION: {
                if (msg.obj instanceof IDefinition) {
                    changeDefinitionToImpl((IDefinition) msg.obj);
                }
            }
            break;

            case CMD_CHG_ENGINE: {
                if (msg.obj instanceof IPlayer) {
                    changePlayerToImpl((IPlayer) msg.obj);
                }
            }
            break;

            case CMD_SLEEP: {
                sleepImpl((String) msg.obj);
            }
            break;

            case CMD_WAKEUP: {
                wakeUpImpl((String) msg.obj);
            }
            break;
        }

        return true;
    }

    @Override
    public final void setDataSource(ViewGroup view, IBasePlayerListener cb, IDataSource src) {
        DataSourceInner inner = new DataSourceInner(view, cb, src);
        Message message = Message.obtain();
        message.what = CMD_DATA_SOURCE_SET;
        message.obj = inner;
        base_player.handleMessage(message);
    }

    protected abstract void setDataSourceImpl(ViewGroup view, IBasePlayerListener cb, IDataSource src);


    @Override
    public final void start() {
        Message message = Message.obtain();
        message.what = CMD_START;
        base_player.handleMessage(message);
    }

    protected abstract void startImpl();

    @Override
    public final void changeDataSource(IDataSource source) {
        Message message = Message.obtain();
        message.what = CMD_DATA_SOURCE_CHG;
        message.obj = source;
        base_player.handleMessage(message);
    }

    protected abstract void changeDataSourceImpl(IDataSource source);

    @Override
    public final void pause() {
        Message message = Message.obtain();
        message.what = CMD_PAUSE;
        base_player.handleMessage(message);
    }

    protected abstract void pauseImpl();

    @Override
    public final void release() {
        Message message = Message.obtain();
        message.what = CMD_RELEASE;
        base_player.handleMessage(message);
    }

    protected abstract void releaseImpl();

    @Override
    public final void seekTo(int whereTo) {
        Message message = Message.obtain();
        message.what = CMD_RELEASE;
        message.arg1 = whereTo;
        base_player.handleMessage(message);
    }

    protected abstract void seekToImpl(int whereTo);


    @Override
    public final void changeDefinitionTo(IDefinition definition) {
        Message message = Message.obtain();
        message.what = CMD_CHG_DEFINITION;
        message.obj = definition;
        base_player.handleMessage(message);
    }

    protected abstract void changeDefinitionToImpl(IDefinition definition);

    @Override
    public final void changePlayerTo(IPlayer player) {
        Message message = Message.obtain();
        message.what = CMD_CHG_ENGINE;
        message.obj = player;
        base_player.handleMessage(message);
    }

    protected abstract void changePlayerToImpl(IPlayer player);

    @Override
    public final void sleep(String reason) {
        Message message = Message.obtain();
        message.what = CMD_SLEEP;
        message.obj = reason;
        base_player.handleMessage(message);
    }

    protected abstract void sleepImpl(String reason);


    @Override
    public final void wakeUp(String reason) {
        Message message = Message.obtain();
        message.what = CMD_WAKEUP;
        message.obj = reason;
        base_player.handleMessage(message);
    }

    protected abstract void wakeUpImpl(String reason);
}




