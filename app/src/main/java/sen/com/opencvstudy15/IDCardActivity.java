package sen.com.opencvstudy15;

import android.app.ProgressDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.googlecode.tesseract.android.TessBaseAPI;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import imagepicker.ImagePicker;
import imagepicker.bean.ImageItem;
import imagepicker.loader.GlideImageLoader;
import imagepicker.ui.ImageGridActivity;
import imagepicker.util.ToastUtils;
import imagepicker.view.CropImageView;

public class IDCardActivity extends AppCompatActivity {

    private TextView tv_res;
    private ImageView idnumb_bitmap;
    private ImageView src_bitmap;
    private ImagePicker imagePicker;
    public static final int REQUEST_CODE_SELECT = 100;
    public static final int REQUEST_CODE_PREVIEW = 101;
    private Bitmap srcBitmap;
    private Bitmap tempBitmap;
    private Bitmap idCarNumber;
    private TessBaseAPI baseApi;
    private ProgressDialog progressDialog;

    private String language = "cn";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_idcard);
        tv_res = (TextView) findViewById(R.id.tv_res);
        idnumb_bitmap = (ImageView) findViewById(R.id.idnumb_bitmap);
        src_bitmap = (ImageView) findViewById(R.id.src_bitmap);
        initImagePicker();
        tempBitmap = BitmapFactory.decodeResource(getResources(), R.mipmap.temple);
        initTess();
    }

    private void showProgress() {
        if (null != progressDialog) {
            progressDialog.show();
        } else {
            progressDialog = new ProgressDialog(this);
            progressDialog.setMessage("请稍候");
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.show();
        }
    }

    private void dismissProgress() {
        if (null != progressDialog) {
            progressDialog.dismiss();
        }
    }


    private void initTess() {
        new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected void onPreExecute() {
                showProgress();
            }

            @Override
            protected void onPostExecute(Boolean result) {
                if (result) {
                    dismissProgress();
                } else {
                    Toast.makeText(IDCardActivity.this, "load trainedData failed", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            protected Boolean doInBackground(Void... params) {
                baseApi = new TessBaseAPI();
                try {
                    InputStream is = null;
                    is = getAssets().open(language + ".traineddata");
                    File file = new File("/sdcard/tess/tessdata/" + language + ".traineddata");
                    if (!file.exists()) {
                        file.getParentFile().mkdirs();
                        FileOutputStream fos = new FileOutputStream(file);
                        byte[] buffer = new byte[2048];
                        int len;
                        while ((len = is.read(buffer)) != -1) {
                            fos.write(buffer, 0, len);
                        }
                        fos.close();
                    }
                    is.close();
                    return baseApi.init("/sdcard/tess", language);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                return null;
            }
        }.execute();
    }

    private void initImagePicker() {
        imagePicker = ImagePicker.getInstance();
        imagePicker.setImageLoader(new GlideImageLoader()); // 设置图片加载器
        imagePicker.setShowCamera(false); // 显示拍照按钮
        imagePicker.setCrop(false); // 允许裁剪（单选才有效）
        imagePicker.setSaveRectangle(true); // 是否按矩形区域保存
        imagePicker.setSelectLimit(1); // 选中数量限制
        imagePicker.setStyle(CropImageView.Style.RECTANGLE); // 裁剪框的形状
        imagePicker.setFocusWidth(800); // 裁剪框的宽度。单位像素（圆形自动取宽高最小值）
        imagePicker.setFocusHeight(800); // 裁剪框的高度。单位像素（圆形自动取宽高最小值）
        imagePicker.setOutPutX(1000); // 保存文件的宽度。单位像素
        imagePicker.setOutPutY(1000); // 保存文件的高度。单位像素
    }

    //选图
    public void startChoose(View view) {
        imagePicker.clear();
        ImagePicker.getInstance().setSelectLimit(1);
        Intent intent = new Intent(IDCardActivity.this, ImageGridActivity.class);
        startActivityForResult(intent, REQUEST_CODE_SELECT);
        overridePendingTransition(0, 0);
    }

    //开始识别
    public void start_regin(View view) {
        if (srcBitmap!=null && tempBitmap!=null){
            if (idCarNumber!=null) {
                idCarNumber.recycle();
            }
            idCarNumber = OpencvUtils.getIdCardNum(srcBitmap,tempBitmap, Bitmap.Config.ARGB_8888);
            if (idCarNumber!=null) {
                idnumb_bitmap.setImageBitmap(idCarNumber);
                baseApi.setImage(idCarNumber);
                tv_res.setText(baseApi.getUTF8Text());
                baseApi.clear();
            }

        }
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        ToastUtils.showTextToast(this, "请稍等");
        if (resultCode == ImagePicker.RESULT_CODE_ITEMS) {
            // 添加图片返回
            if (data != null && requestCode == REQUEST_CODE_SELECT) {

                ArrayList<ImageItem> images = (ArrayList<ImageItem>) data
                        .getSerializableExtra(ImagePicker.EXTRA_RESULT_ITEMS);
                imagePicker.clear();
                imagePicker.addSelectedImageItem(0, images.get(0), true);
            }
        } else if (resultCode == ImagePicker.RESULT_CODE_BACK) {
            // 预览图片返回
            if (data != null && requestCode == REQUEST_CODE_PREVIEW) {
                ArrayList<ImageItem> images = (ArrayList<ImageItem>) data
                        .getSerializableExtra(ImagePicker.EXTRA_IMAGE_ITEMS);

            }
        } else if (resultCode == RESULT_OK
                && requestCode == ImagePicker.REQUEST_CODE_TAKE) {
            // 发送广播通知图片增加了
            ImagePicker.galleryAddPic(this, imagePicker.getTakeImageFile());
            ImageItem imageItem = new ImageItem();
            imageItem.path = imagePicker.getTakeImageFile().getAbsolutePath();
            imageItem.name = imagePicker.getTakeImageFile().getName();
            imagePicker.clear();
            imagePicker.addSelectedImageItem(0, imageItem, true);
        }

        String imagePath = imagePicker.getSelectedImages().get(0).path;
        if (!TextUtils.isEmpty(imagePath)) {
            srcBitmap = toBitmap(imagePath);
            src_bitmap.setImageBitmap(srcBitmap);
        }
    }

    public Bitmap toBitmap(String pathName) {
        if (TextUtils.isEmpty(pathName))
            return null;
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(pathName, o);
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp <= 640 && height_tmp <= 480) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inSampleSize = scale;
        opts.outHeight = height_tmp;
        opts.outWidth = width_tmp;
        return BitmapFactory.decodeFile(pathName, opts);
    }


    @Override
    protected void onDestroy() {
        if (imagePicker != null) {
            imagePicker.clearSelectedImages();
        }
        if (srcBitmap != null)
            srcBitmap.recycle();

        if (tempBitmap != null)
            tempBitmap.recycle();
        super.onDestroy();
    }
}
