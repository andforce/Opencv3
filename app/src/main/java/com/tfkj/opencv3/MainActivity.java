package com.tfkj.opencv3;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.tfkj.opencv3.view.FloodFillImageView;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_OPEN_IMAGE = 1;

    static {
        System.loadLibrary("flood-fill");
    }

    private int mStartY = -1;
    private int value = 50;

    private Bitmap floodFillBitmap;
    private Bitmap orgBitmap;
    private Bitmap maskBitmap;
    private Bitmap resultBitmap;

    private FloodFillImageView mSrcImageView;
    private ImageView mMaskImageView;
    private ImageView mResultImageView;

    ArrayList<String> mResult;

    private int[] realPoint;

    private int[] orgPixels;
    private int[] changedPixels;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mMaskImageView = findViewById(R.id.maskImageView);

        mResultImageView = findViewById(R.id.resultImageView);

        mSrcImageView = findViewById(R.id.imageView);
        mSrcImageView.post(new Runnable() {
            @Override
            public void run() {
                mStartY = mSrcImageView.getMeasuredHeight() / 2;
            }
        });

        findViewById(R.id.choose_image).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent getPictureIntent = new Intent(Intent.ACTION_GET_CONTENT);
                getPictureIntent.setType("image/*");
                Intent pickPictureIntent = new Intent(Intent.ACTION_PICK,
                        MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                Intent chooserIntent = Intent.createChooser(getPictureIntent, "Select Image");
                chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, new Intent[] {
                        pickPictureIntent
                });
                startActivityForResult(chooserIntent, REQUEST_OPEN_IMAGE);
            }
        });

        mSrcImageView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()){
                    case MotionEvent.ACTION_DOWN:{
                        mStartY = (int) event.getY();

                        Rect rectDis = mSrcImageView.getImageDisplayRect();

                        Matrix matrix = mSrcImageView.getImageMatrix();

                        Log.d("ACTION_MOVE realSize->", "DiaplyRect:" + rectDis + " Matrix:" + matrix);

                        if (rectDis.contains((int) event.getX(), (int) event.getY())){

                            realPoint = mSrcImageView.pointOnReadImage((int) event.getX(), (int) event.getY());

                            Log.d("ACTION_MOVE imageSize->", "点到了 " + "X:" +event.getX()  + " Y:" + event.getY() + " realX:" + realPoint[0] + " realY:" + realPoint[1]);

                            Bitmap maskBitmap = orgBitmap.copy(Bitmap.Config.ARGB_8888, true);

                            Log.d("ACTION_MOVE imageSize->", "Bitmap W:" + maskBitmap.getWidth() + " Bitmap H:" + maskBitmap.getHeight());

                            if (resultBitmap == null){
                                resultBitmap = Bitmap.createBitmap(orgBitmap.getWidth(), orgBitmap.getHeight(), Bitmap.Config.ARGB_8888);
                            }

                            int[] pixels = FloodFillUtils.floodFillBitmapWithMask(orgBitmap, maskBitmap, realPoint[0], realPoint[1], value, value);

                            int[] maskPixels = new int[maskBitmap.getWidth() * maskBitmap.getHeight()];
                            maskBitmap.getPixels(maskPixels, 0, maskBitmap.getWidth(), 0, 0, maskBitmap.getWidth(), maskBitmap.getHeight());

                            mMaskImageView.setImageBitmap(maskBitmap);
                            resultBitmap.setPixels(pixels, 0, resultBitmap.getWidth(), 0, 0, resultBitmap.getWidth(), resultBitmap.getHeight());
                            mResultImageView.setImageBitmap(resultBitmap);

                        } else {
                            Log.d("ACTION_MOVE imageSize->", "没点到 " + "X:" +event.getX()  + " Y:" + event.getY() + " display:" + rectDis);
                            mMaskImageView.setImageBitmap(orgBitmap);
                        }

                        break;
                    }

                    case MotionEvent.ACTION_MOVE:{

                        if (Math.abs(event.getY() - mStartY) > 100){

                            if (event.getY() > mStartY){
                                // Scroll Down
                                value += 1;

                            } else {
                                // Scroll Up
                                value -= 1;
                            }

                            if (value > 255){
                                value = 255;
                            }

                            if (value < 0){
                                value = 0;
                            }

                            if (maskBitmap == null) {
                                maskBitmap = Bitmap.createBitmap(orgBitmap.getWidth(), orgBitmap.getHeight(), Bitmap.Config.ARGB_8888);
                            }

                            if (resultBitmap == null){
                                resultBitmap = Bitmap.createBitmap(orgBitmap.getWidth(), orgBitmap.getHeight(), Bitmap.Config.ARGB_8888);
                            }

                            int[] pixels =FloodFillUtils.floodFillBitmapWithMask(orgBitmap, maskBitmap, realPoint[0], realPoint[1], value, value);

                            mMaskImageView.setImageBitmap(maskBitmap);

                            resultBitmap.setPixels(pixels, 0, resultBitmap.getWidth(), 0, 0, resultBitmap.getWidth(), resultBitmap.getHeight());
                            mResultImageView.setImageBitmap(resultBitmap);
                        }

                        Log.d("ACTION_MOVE", ">>>>>> floodFill value:" + value + " StartY:" + mStartY + " Y:" + event.getY());
                        break;
                    }

                    case MotionEvent.ACTION_UP:{
                        value = 20;
                        break;
                    }
                }
                return true;
            }
        });

    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_OPEN_IMAGE:
                if (resultCode == RESULT_OK) {
                    Uri imgUri = data.getData();
                    String[] filePathColumn = {MediaStore.Images.Media.DATA};

                    Cursor cursor = getContentResolver().query(imgUri, filePathColumn, null, null, null);
                    cursor.moveToFirst();

                    int colIndex = cursor.getColumnIndex(filePathColumn[0]);
                    final String imagePath = cursor.getString(colIndex);
                    cursor.close();

                    int targetW = mSrcImageView.getWidth();
                    int targetH = mSrcImageView.getHeight();

                    BitmapFactory.Options bmOptions = new BitmapFactory.Options();
                    bmOptions.inJustDecodeBounds = true;
                    BitmapFactory.decodeFile(imagePath, bmOptions);
                    int photoW = bmOptions.outWidth;
                    int photoH = bmOptions.outHeight;

                    int scaleFactor = Math.min(photoW / targetW, photoH / targetH);

                    bmOptions.inJustDecodeBounds = false;
                    bmOptions.inSampleSize = scaleFactor;
                    bmOptions.inPurgeable = true;

                    orgBitmap = BitmapFactory.decodeFile(imagePath, bmOptions);

                    orgPixels = new int[orgBitmap.getWidth() * orgBitmap.getHeight()];
                    orgBitmap.getPixels(orgPixels, 0, orgBitmap.getWidth(), 0, 0, orgBitmap.getWidth(), orgBitmap.getHeight());

                    changedPixels = new int[orgPixels.length];

                    floodFillBitmap = BitmapFactory.decodeFile(imagePath, bmOptions);

                    mSrcImageView.setImageBitmap(orgBitmap);
                }
                break;
        }
    }
}
