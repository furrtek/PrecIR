package org.furrtek.pricehax2;

import android.graphics.Bitmap;

import java.util.ArrayList;
import java.util.List;

public class DMImage {
    Bitmap bitmapBW;
    Bitmap bitmapBWR;
    List<Byte> byteStreamBW = new ArrayList<Byte>();
    List<Byte> byteStreamBWR = new ArrayList<Byte>();

    public DMImage(int width, int height) {
        this.bitmapBW = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        this.bitmapBWR = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
    }
}
