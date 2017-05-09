package com.rosie.androidiconrecognition;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    final static String TAG = "MainActivity";
    final static String filename = "trainedSVM.xml";
    private String filepath;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    // Used to load the 'native-lib' library on application startup.
    static {

        System.loadLibrary("native-lib");
    }

    TextView result;
    ImageView icon;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        icon = (ImageView) findViewById(R.id.icon);
        result = (TextView) findViewById(R.id.result);

        icon.setImageResource(R.drawable.close);

        verifyStoragePermissions(this);

        if(copyFile())
            loadSVM(filepath);

        icon.setOnClickListener(
                new Button.OnClickListener() {
                    public void onClick(View v) {

                        Bitmap bitmap = drawableToBitmap(icon.getDrawable());


                    }
                }
        );


    }

    public static void verifyStoragePermissions(Activity activity) {
        // Check if we have write permission
        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);

        if (permission != PackageManager.PERMISSION_GRANTED) {
            // We don't have permission so prompt the user
            ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    REQUEST_EXTERNAL_STORAGE
            );
        }
    }

    public static Bitmap drawableToBitmap(Drawable drawable) {

        if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }

        int width = drawable.getIntrinsicWidth();
        width = width > 0 ? width : 1;
        int height = drawable.getIntrinsicHeight();
        height = height > 0 ? height : 1;

        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);

        return bitmap;
    }

    public boolean copyFile() {
        String baseDir = Environment.getExternalStorageDirectory().getPath(); // 모델을 복사할 외부 경로 설정
        Log.d(TAG, "base directory : " + baseDir);
        filepath = baseDir + File.separator + filename;
        Log.d(TAG, "newPath : " + filepath);

        File f = new File(filepath);
        if (f.exists()){
            Log.d(TAG, "already copied!");
            return true;
        }
        AssetManager assetManager = this.getAssets(); // asset 설정

        InputStream inputStream = null;
        OutputStream outputStream = null;

        try {

            inputStream = assetManager.open(filename); // asset으로부터 모델을 읽어올 준비
            outputStream = new FileOutputStream(filepath); // 복사할 준비

            byte[] buffer = new byte[1024]; // buffer 생성

            int read;
            while ((read = inputStream.read(buffer)) != -1) { // 버퍼 단위로 읽어 들임
                outputStream.write(buffer, 0, read); // 읽어 들인 데이터를 버퍼 단위로 outputStream에 전달
            }
            inputStream.close();
            inputStream = null;
            outputStream.flush();
            outputStream.close();
            outputStream = null;

            Log.d(TAG, "copy file success!");
            return true;

        } catch (Exception e) {
            e.printStackTrace();
            Log.d(TAG, "copy file failed!");
            return false;
        }

    }

    public native void loadSVM(String filepath);

}
