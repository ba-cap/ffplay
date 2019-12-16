package dai.android.media.xplayer;

import android.net.Uri;
import android.view.Surface;
import android.view.SurfaceHolder;

public class XMediaPlayer
{


    public void setDataSource(Uri uri)
    {
    }

    public void setDisplay(SurfaceHolder holder)
    {
    }


    private native void native_setDataSource(String url);

    private native void native_setSurface(Surface surface);

}
