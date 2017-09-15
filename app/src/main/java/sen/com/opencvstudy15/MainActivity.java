package sen.com.opencvstudy15;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private OpencvUtils opencvUtils;
    private SurfaceView surfaceView;
    String path = Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+
            "Download"+File.separator;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        opencvUtils = new OpencvUtils();
        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);
        opencvUtils.loadFaceXml(path+"haarcascade_frontalface_alt.xml");
    }

    public void start(View view){
        Bitmap bitmap = BitmapFactory.decodeFile(path+"test.png");
        opencvUtils.handleBitmap(bitmap);
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        opencvUtils.setSurface(holder.getSurface(), 640, 480);
        Log.e("sensen","surfaceChanged");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
