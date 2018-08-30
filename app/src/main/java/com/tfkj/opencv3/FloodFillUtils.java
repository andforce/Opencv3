package com.tfkj.opencv3;

import android.graphics.Bitmap;

public class FloodFillUtils {
    public static native void floodFill(Bitmap bitmap, int x, int y, int low, int up);

    public static native int floodFillBitmap(Bitmap bitmap, Bitmap maskBitmap, int x, int y, int low, int up);
}
