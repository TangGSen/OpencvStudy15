package com.tz.ndk.compress;

/**
 * 作者: Dream on 2016/12/17 20:43
 * QQ:510278658
 * E-mail:510278658@qq.com
 */

public class FFmpegCompress {

    static {
        System.loadLibrary("MyLib");
    }

    //视频转码压缩
    public native void callFFmepgCompress(int argc,String[] argv);

}
