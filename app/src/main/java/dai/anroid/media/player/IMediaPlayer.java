package dai.anroid.media.player;

import android.view.ViewGroup;

import java.util.List;

public interface IMediaPlayer {

    void setDataSource(ViewGroup group, IBasePlayerListener callback, IDataSource source);

    void changeDataSource(IDataSource source);

    void start();

    void pause();

    void release();

    long seekTo(int whereTo);

    List<IDefinition> getSupportDefinitions();

    void changeDefinitionTo(IDefinition definition);

    List<IPlayer> getSupportPlayer();

    void changePlayerTo(IPlayer player);

    void sleep(String reason);

    void wakeUp(String reason);
}
