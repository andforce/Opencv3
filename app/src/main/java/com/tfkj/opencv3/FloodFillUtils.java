package com.tfkj.opencv3;

import android.graphics.Bitmap;

public class FloodFillUtils {
    public static native int[] floodFillBitmapWithMask(Bitmap bitmap, Bitmap maskBitmap, int x, int y, int low, int up);
}
