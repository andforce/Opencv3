package com.tfkj.opencv3;

import android.app.ProgressDialog;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_OPEN_IMAGE = 1;

    static {
        System.loadLibrary("native-lib");
    }

    private ProgressDialog dlg;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        dlg = new ProgressDialog(this);

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
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_OPEN_IMAGE:
                if (resultCode == RESULT_OK) {
                    Uri imgUri = data.getData();
                    String[] filePathColumn = { MediaStore.Images.Media.DATA };

                    Cursor cursor = getContentResolver().query(imgUri, filePathColumn,
                            null, null, null);
                    cursor.moveToFirst();

                    int colIndex = cursor.getColumnIndex(filePathColumn[0]);
                    final String imagePath = cursor.getString(colIndex);
                    cursor.close();

                    dlg.setMessage("正在查找");
                    dlg.setCancelable(false);
                    dlg.setIndeterminate(true);
                    dlg.show();

                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            Log.d("FIND_OBJ", "start find");
                            final int count = find_objects(imagePath);
                            Log.d("FIND_OBJ", "count: " + count);
                            getWindow().getDecorView().post(new Runnable() {
                                @Override
                                public void run() {
                                    dlg.dismiss();

                                    Toast.makeText(getApplicationContext(),"找到：" + count + "个", Toast.LENGTH_SHORT).show();
                                }
                            });
                        }
                    }).start();
                }
                break;
        }
    }

    //public static native void gray();

    public static native int find_objects(String imagePath);
}
