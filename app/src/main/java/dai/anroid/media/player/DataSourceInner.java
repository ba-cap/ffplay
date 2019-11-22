package dai.anroid.media.player;

import android.view.ViewGroup;

public final class DataSourceInner {

    public final ViewGroup viewGroup;

    public final IBasePlayerListener callback;

    public final IDataSource source;

    public DataSourceInner(ViewGroup view, IBasePlayerListener cb, IDataSource ds) {
        viewGroup = view;
        callback = cb;
        source = ds;
    }
}
