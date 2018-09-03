package com.tfkj.opencv3;

import android.graphics.Bitmap;

public class RemoveColors {
    public static native int[] removeColors(Bitmap srcBitmap, Bitmap maskBitmap, int x, int y, int low, int up);
}
