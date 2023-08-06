package org.furrtek.pricehax;

import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.ImageView;
import android.graphics.Color;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.List;

import static org.furrtek.pricehax.DitherBitmap.floydSteinbergDithering;

public class DMConvert extends AsyncTask<Bitmap, Integer, DMImage> {
    private ImageView mImageView;
    private ProgressBar mProgressBar;
    AsyncResponse delegate = null;

    DMConvert(ProgressBar progressBar, AsyncResponse asyncResponse) {
        delegate = asyncResponse;
        mProgressBar = progressBar;
    }
    @Override
    protected void onProgressUpdate(Integer... progress) {
        mProgressBar.setProgress(progress[0]);
    }
    @Override
    protected void onPostExecute(DMImage dmimage) {
        Log.d("PHX", "Convert done, bytestream size: " + dmimage.byteStreamBW.size());
        delegate.processFinish(dmimage);
    }
    @Override
    protected DMImage doInBackground(Bitmap... selectedImage) {
        int x, y, wi, hi;
        int w, h;
        int pixel;
        int idx = 0;
        Bitmap imageBW, imageBWR;
        w = selectedImage[0].getWidth();
        h = selectedImage[0].getHeight();
        DMImage dmimage = new DMImage(w, h);

        /*if (PLType == 1318) {
            w = 0xD0;
            h = 0x70;
        } else {
            w = 172;
            h = 72;
        }*/
        wi = w;
        hi = h;

        BitSet bitstreamBW = new BitSet(wi * hi);
        BitSet bitstreamBWR = new BitSet(wi * hi * 2);

        Bitmap scaledimage = selectedImage[0];

        //Convert to BW
        Log.d("PHX", "Convert to BW");
        imageBW = floydSteinbergDithering(scaledimage, false);
        //dmimage.bitmapBW = imageBW;
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++, idx++) {
                pixel = imageBW.getPixel(x, y);
                if (Color.red(pixel) > 0)
                    bitstreamBW.set(idx);
                else
                    bitstreamBW.clear(idx);
                dmimage.bitmapBW.setPixel(x, y, pixel);
            }
        }

        //Convert to BWR
        Log.d("PHX", "Convert to BWR");
        imageBWR = floydSteinbergDithering(scaledimage, true);
        //dmimage.bitmapBWR = imageBWR;
        idx = 0;
        int idxr = w * h;
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++, idx++, idxr++) {
                pixel = imageBWR.getPixel(x, y);
                if (Color.red(pixel) > 0) {
                    if (Color.green(pixel) > 0) {
                        bitstreamBWR.set(idx);   // White
                        bitstreamBWR.set(idxr);
                        dmimage.bitmapBWR.setPixel(x, y, 0xFFFFFFFF);
                    } else {
                        bitstreamBWR.clear(idx);   // Red
                        bitstreamBWR.clear(idxr);
                        dmimage.bitmapBWR.setPixel(x, y, 0xFFFF0000);
                    }
                } else {
                    bitstreamBWR.clear(idx); // Black
                    bitstreamBWR.set(idxr);
                    dmimage.bitmapBWR.setPixel(x, y, 0xFF000000);
                }
            }
        }

        BitSet bitstreamBW_RLE = RLECompress(bitstreamBW);
        BitSet bitstreamBWR_RLE = RLECompress(bitstreamBWR);

        publishProgress(90);

        dmimage.byteStreamBW = bitstreamBytes(bitstreamBW_RLE);
        dmimage.byteStreamBWR = bitstreamBytes(bitstreamBWR_RLE);

        publishProgress(100);

        return dmimage;
    }

    private List<Byte> bitstreamBytes(BitSet bitstream) {
        List<Byte> result = new ArrayList<Byte>();
        byte[] bytes = bitstream.toByteArray();
        for (byte b : bytes) {
            int x = 0;
            for (int c = 0; c < 8; c++) {
                x >>= 1;
                if (b < 0)
                    x |= 0x80;
                b <<= 1;
            }
            result.add((byte)x);
        }
        int idx = result.size();
        int klp = idx % 20;
        if (klp > 0) klp = 20 - klp;
        Log.d("PHX", String.format("Padding %d to %d", idx, idx + klp));
        for (int bsc = 0; bsc < klp; bsc++)
            result.add(new Byte((byte)0));

        return result;
    }

    private BitSet RLECompress(BitSet bitstream) {
        int j = bitstream.length() - 1;
        int cnt = 1;
        boolean p = bitstream.get(0);
        boolean n;
        List<Integer> RLErun = new ArrayList<Integer>();

        // RLE compress
        Log.d("PHX", String.format("RLE compress, raw size = %d", bitstream.length()));
        for (int m = 1; m <= j; m++) {
            n = bitstream.get(m);
            if (n == p) {
                cnt++;
            } else {
                RLErun.add(cnt);
                cnt = 1;
                p = n;
            }
        }
        if (cnt > 1) RLErun.add(Integer.valueOf(cnt));

        String bs;
        BitSet bitstreamRLE = new BitSet();
        bitstreamRLE.set(0, bitstream.get(0));

        Log.d("PHX", "Gen unary coded runs");
        int idx = 1;
        for (int cnts : RLErun) {
            bs = Integer.toBinaryString(cnts);
            for (int bsc = 0; bsc < bs.length() - 1; bsc++) {
                bitstreamRLE.clear(idx++);
            }
            for (int bsc = 0; bsc < bs.length(); bsc++) {
                if (bs.charAt(bsc) == '1')
                    bitstreamRLE.set(idx++);
                else
                    bitstreamRLE.clear(idx++);
            }
        }

        String dbg = "";
        for (int i = 0; i < 256; i++) {
            dbg += (bitstreamRLE.get(i) ? "1" : "0");
        }
        Log.d("PHX", dbg);
        return bitstreamRLE;
    }
}