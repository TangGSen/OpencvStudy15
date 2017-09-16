package com.tz.ndk.compress;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private FFmpegCompress fFmpegCompress;
    private File inputFile;
    private File outputFile;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        fFmpegCompress = new FFmpegCompress();
        File dic = Environment.getExternalStorageDirectory();
        inputFile = new File(dic,"/FFmpeg-Test/ffmpeg_test.mov");
        outputFile = new File(dic,"/FFmpeg-Test/ffmpeg_out.mp4");
    }

    public void clickFFmpegVideoCompress(View v){
        new Thread(new Runnable() {
            @Override
            public void run() {
                //获取视频文件的路径(sdcard路径)
                StringBuilder builder = new StringBuilder();
                builder.append("ffmpeg ");
                builder.append("-i ");
                builder.append(inputFile.getAbsolutePath()+" ");
                builder.append("-b:v 640k ");
                builder.append(outputFile.getAbsolutePath());
                String[] argv = builder.toString().split(" ");
                int argc = argv.length;
                fFmpegCompress.callFFmepgCompress(argc,argv);
            }
        }).start();
    }

}
