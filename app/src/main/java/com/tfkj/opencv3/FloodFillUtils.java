package com.tfkj.opencv3;

import android.graphics.Bitmap;

public class FloodFillUtils {
    public native void floodFill(Bitmap bitmap, int x, int y, int low, int up);
}
