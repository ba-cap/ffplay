package dai.android.media.ffplay;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewStub;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class YuvPlayActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "YuvPlayActivity";

    private final static int REQUEST_CODE = 100;

    private String mPlayUrl = null;

    private ViewStub mViewStub;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_yuv_play);

        //mSurfaceView = findViewById(R.id.surfaceView);
        //mSurfaceView.getHolder().addCallback(callback);

        mViewStub = findViewById(R.id.ViewStub4Surface);
        findViewById(R.id.btnOpenSelectFile).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.btnOpenSelectFile) {
            /// Intent intent = new Intent(Intent.ACTION_PICK);
            /// intent.setType("*/*");
            /// startActivityForResult(intent, REQUEST_CODE);

            /// Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            /// intent.setType("video/*"); //String VIDEO_UNSPECIFIED = "video/*";
            /// Intent wrapperIntent = Intent.createChooser(intent, null);
            /// startActivityForResult(wrapperIntent, REQUEST_CODE);


            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            startActivityForResult(intent, REQUEST_CODE);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (null == data) {
            Log.e(TAG, "not select any file");
            return;
        }

        // get the data uri
        Uri uri = data.getData();
        if (null == uri) {
            Log.e(TAG, "null Uri in data");
            return;
        }
        Log.e(TAG, "Uri: " + uri);


        // only for document
        if (DocumentsContract.isDocumentUri(this, uri)) {
            String authority = uri.getAuthority();
            Log.d(TAG, "Uri.authority: " + authority);
            if ("com.android.externalstorage.documents".equals(authority)) {
                String docId = DocumentsContract.getDocumentId(uri);
                Log.d(TAG, "doc id: " + docId);

                //String path = "/storage/";
                final String[] split = docId.split(":");
                final String type = split[0];

                String strPath = null;
                if ("primary".equalsIgnoreCase(type)) {
                    strPath = Environment.getExternalStorageDirectory() + "/" + split[1];
                } else {
                    if (!TextUtils.isEmpty(type)) {
                        strPath = "/storage/" + type + "/" + split[1];
                    }
                }

                Log.d(TAG, "Current file path: " + strPath);
                if (!TextUtils.isEmpty(strPath)) {
                    mPlayUrl = strPath;
                    mViewStub.inflate();

                    SurfaceView view = findViewById(R.id.VideoSurfaceView);
                    view.getHolder().addCallback(callback);
                }
            }
        }

    }

    private SurfaceHolder.Callback callback = new SurfaceHolder.Callback() {
        private Thread mThread;

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG, "surfaceCreated");

            mThread = new Thread(() -> playYuvVideo(holder.getSurface(), mPlayUrl));
            mThread.start();
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d(TAG, "surfaceChanged");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG, "surfaceDestroyed");
            if (null != mThread) {
                mThread.interrupt();
            }
        }
    };


    // https://blog.csdn.net/u014788540/article/details/83479240
    // https://www.jianshu.com/p/26dc5c0d174d
    // https://www.jianshu.com/p/98fce48442ee

    // https://blog.csdn.net/u010927932/article/details/50751472


    private native void playYuvVideo(Surface surface, String path);
}
