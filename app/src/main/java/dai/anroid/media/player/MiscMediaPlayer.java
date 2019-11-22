package dai.anroid.media.player;

import android.app.Application;
import android.os.Message;
import android.util.SparseArray;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

public final class MiscMediaPlayer extends BasePlayerImpl {
    private static final String TAG = "MiscMediaPlayer";

    private static MiscMediaPlayer instance = null;

    public static MiscMediaPlayer createPlayer(Application context) {
        if (null == instance) {
            synchronized (MiscMediaPlayer.class) {
                if (null == instance) {
                    instance = new MiscMediaPlayer(context);
                }
            }
        }
        return instance;
    }

    public static MiscMediaPlayer getPlayer() {
        synchronized (MiscMediaPlayer.class) {
            if (null == instance) {
                throw new RuntimeException("Call MiscMediaPlayer.createPlayer(Application) first");
            }
            return instance;
        }
    }

    private static void appExit() {
        synchronized (MiscMediaPlayer.class) {
            if (null != instance) {
                instance = null;
            }
        }
    }


    @Override
    public void beforeAppExit() {
        super.beforeAppExit();
        appExit();
    }

    private final SparseArray<AbstractTypedPlayer> mTypedPlayer = new SparseArray<>();
    private final AtomicReference<AbstractTypedPlayer> mCurrentTypedPlayer = new AtomicReference<>();

    private IBasePlayerListener mCurrentCallBack = null;

    private MiscMediaPlayer(Application context) {
        super(context);
    }

    public final void registerTypePlayer(@NonNull AbstractTypedPlayer player) throws Exception {
        synchronized (mTypedPlayer) {
            AbstractTypedPlayer existPlayer = mTypedPlayer.get(player.getMediaType());
            if (null != existPlayer) {
                return;
            }

            if (player.getBasePlayer() != this) {
                throw new Exception("TypedPlayer's BasePlayer not myself");
            }

            mTypedPlayer.append(player.getMediaType(), player);
        }
    }


    public void setDataSource(ViewGroup group, IBasePlayerListener callback, IDataSource source)
            throws Exception {
        if (null == source) {
            throw new Exception("must have a instance of IDataSource");
        }

        AbstractTypedPlayer thisPlayer = null;
        @MediaType int type = source.getMediaType();
        synchronized (mTypedPlayer) {
            thisPlayer = mTypedPlayer.get(type);
        }
        if (null == thisPlayer) {
            throw new Exception("Can not play source type equal [" + type + "].");
        }

        DataSourceInner inner = new DataSourceInner(group, callback, source);
        Message message = Message.obtain();
        message.what = AbstractTypedPlayer.CMD_DATA_SOURCE_SET;
        message.obj = inner;
        handleMessage(message);
    }

    public void changeDataSource(IDataSource source) throws Exception {
        if (null == source) {
            throw new Exception("Must have a source when change");
        }

        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null == thisPlayer) {
            throw new Exception("[changeDataSource]: No media type playing");
        }
        thisPlayer.changeDataSource(source);
    }

    public void start() throws Exception {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null == thisPlayer) {
            throw new Exception("[start]: No media type playing");
        }
        thisPlayer.start();
    }

    public void pause() throws Exception {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null == thisPlayer) {
            throw new Exception("[pause]: No media type playing");
        }
        thisPlayer.pause();
    }

    public void release() {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.getAndSet(null);
        if (null != thisPlayer) {
            thisPlayer.release();
        }
        setBaseCallBack(null);
    }

    public void seekTo(int whereTo) {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null != thisPlayer) {
            thisPlayer.seekTo(whereTo);
        }
    }

    @Nullable
    public List<IDefinition> getSupportDefinitions() {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null == thisPlayer) {
            return null;
        }
        return thisPlayer.getSupportDefinitions();
    }

    public void changeDefinitionTo(IDefinition definition) {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null != thisPlayer) {
            thisPlayer.changeDefinitionTo(definition);
        }
    }

    @Nullable
    public List<IPlayer> getSupportPlayer() {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null == thisPlayer) {
            return null;
        }
        return thisPlayer.getSupportPlayer();
    }

    public void changePlayerTo(IPlayer player) {
        AbstractTypedPlayer thisPlayer = mCurrentTypedPlayer.get();
        if (null != thisPlayer) {
            thisPlayer.changePlayerTo(player);
        }
    }

    @Nullable
    public IData4Ui getUiData() {
        AbstractTypedPlayer player = mCurrentTypedPlayer.get();
        if (null == player) {
            return null;
        }
        return player.getUiData();
    }

    public void sleep(AbstractTypedPlayer player, String reason) throws Exception {
        AbstractTypedPlayer current = mCurrentTypedPlayer.get();
        if (current != player) {
            throw new Exception("[sleep]: Not the same type player");
        }

        if (null != current) {
            current.sleep(reason);
        }
    }

    public void wakeUp(AbstractTypedPlayer player, String reason) throws Exception {
        AbstractTypedPlayer current = mCurrentTypedPlayer.get();
        if (current != player) {
            throw new Exception("[wakeUp]: Not the same type player");
        }

        if (null != current) {
            current.wakeUp(reason);
        }
    }

    private void setDataSourceImpl(DataSourceInner src) {
        ViewGroup viewGroup = src.viewGroup;
        IBasePlayerListener callback = src.callback;
        IDataSource source = src.source;

        AbstractTypedPlayer thisPlayer = null;
        synchronized (mTypedPlayer) {
            thisPlayer = mTypedPlayer.get(source.getMediaType());
        }
        AbstractTypedPlayer oldPlayer = mCurrentTypedPlayer.getAndSet(thisPlayer);
        final AbstractTypedPlayer newOne = mCurrentTypedPlayer.get();
        if (null != oldPlayer) {
            notifyReleaseByForce();
            oldPlayer.exit();
            newOne.enter();
        } else {
            newOne.enter();
        }
        setBaseCallBack(callback);
        newOne.setDataSource(viewGroup, callback, source);
    }

    @Override
    void onHandlerMessage(Message msg) {
        switch (msg.what) {
            case AbstractTypedPlayer.CMD_DATA_SOURCE_SET: {
                setDataSourceImpl((DataSourceInner) msg.obj);
                break;
            }
        }
    }

    private void setBaseCallBack(IBasePlayerListener callBack) {
        mCurrentCallBack = callBack;
    }

    private void notifyReleaseByForce() {
        if (null == mCurrentCallBack)
            return;
        AbstractTypedPlayer player = mCurrentTypedPlayer.get();
        if (null != player) {
            mCurrentCallBack.onReleaseByForce(player.getMediaType());
            player.eraseCallback();
        }
    }

}
