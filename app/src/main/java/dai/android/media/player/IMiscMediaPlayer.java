package dai.android.media.player;

import android.view.ViewGroup;

import java.util.List;

public interface IMiscMediaPlayer {

    void setDataSource(ViewGroup group, IBasePlayerListener callback, IDataSource source);

    void changeDataSource(IDataSource source);

    void start();

    void pause();

    void release();

    void seekTo(int whereTo);

    int getDuration();

    int getCurrentPosition();

    List<IDefinition> getSupportDefinitions();

    void changeDefinitionTo(IDefinition definition);

    List<IPlayer> getSupportPlayer();

    void changePlayerTo(IPlayer player);

    void sleep(String reason);

    void wakeUp(String reason);
}
