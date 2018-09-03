package com.andforce.opencv3;

import android.graphics.Bitmap;

public class RemoveColors {

    /**
     * @param srcBitmap     需要抠图的bitmap， 必须是ARGB_8888
     * @param maskBitmap    创建时候，需要srcBitmap 宽高一致, 必须是ARGB_8888
     * @param x             对应srcBitmap上想要去掉的区域的x坐标
     * @param y             对应srcBitmap上想要去掉的区域的y坐标
     * @param low           跟上面(x,y)对应的点颜色的向下差值
     * @param up            跟上面(x,y)对应的点颜色的向上差值
     * @return              根据上面参数，处理完毕的颜色数组，可直接调用Bitmap.setPixels()更新bitmap
     */
    public static native int[] removeColors(Bitmap srcBitmap, Bitmap maskBitmap, int x, int y, int low, int up);
}
