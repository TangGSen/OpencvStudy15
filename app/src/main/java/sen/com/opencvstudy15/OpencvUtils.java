package sen.com.opencvstudy15;

import android.graphics.Bitmap;
import android.view.Surface;

/**
 * Author : 唐家森
 * Version: 1.0
 * On     : 2017/9/15 15:36
 * Des    :
 */

public class OpencvUtils {
    //加载脸部分类器
    public native void loadFaceXml(String path);
    //
    public native void handleBitmap(Bitmap bitmap);
    public native void setSurface(Surface surface,int width,int height);
    public static native Bitmap getIdCardNum(Bitmap srcBitmap, Bitmap templeBitmap , Bitmap.Config config );



    static {
        System.loadLibrary("native-lib");
    }
}
