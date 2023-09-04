package org.furrtek.pricehax2;

import android.graphics.Bitmap;

public class DMConvertParams {
    Bitmap imageIn;
    boolean dithering = false;

    DMConvertParams(Bitmap imageIn, boolean dithering) {
        this.imageIn = imageIn;
        this.dithering = dithering;
    }
}
