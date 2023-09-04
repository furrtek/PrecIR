package org.furrtek.pricehax2;

import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.ImageView;
import android.graphics.Color;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.List;

import static org.furrtek.pricehax2.DitherBitmap.floydSteinbergDithering;

public class DMConvert extends AsyncTask<DMConvertParams, Integer, DMImage> {
    // This does the color conversion, dithering, encoding and compression.
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
        Log.d("PHX", "Convert done, BW bytestream size: " + dmimage.byteStreamBW.size());
        delegate.processFinish(dmimage);
    }
    @Override
    protected DMImage doInBackground(DMConvertParams... params) {
        int x, y, w, h;
        int pixel;
        int idx = 0;
        Bitmap imageBW, imageBWR;
        Bitmap imageIn = params[0].imageIn;
        boolean dithering = params[0].dithering;

        w = imageIn.getWidth();
        h = imageIn.getHeight();
        DMImage dmimage = new DMImage(w, h);

        BitSet bitstreamBW = new BitSet(w * h);       // One plane
        BitSet bitstreamBWR = new BitSet(w * h * 2);  // Two planes

        // Convert to BW
        imageBW = dithering ? floydSteinbergDithering(imageIn, false) : imageIn;
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++, idx++) {
                pixel = imageBW.getPixel(x, y);
                if (Color.red(pixel) > 127) {
                    bitstreamBW.set(idx);
                    pixel = Color.WHITE;
                } else {
                    bitstreamBW.clear(idx);
                    pixel = Color.BLACK;
                }
                dmimage.bitmapBW.setPixel(x, y, pixel);
            }
        }

        //Convert to BWR
        imageBWR = dithering ? floydSteinbergDithering(imageIn, true) : imageIn;
        idx = 0;
        int idxr = w * h;   // Offset index for red layer
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++, idx++, idxr++) {
                pixel = imageBWR.getPixel(x, y);
                if (Color.red(pixel) > 127) {
                    if (Color.green(pixel) > 127) {
                        bitstreamBWR.set(idx);   // White
                        bitstreamBWR.set(idxr);
                        pixel = Color.WHITE;
                        //dmimage.bitmapBWR.setPixel(x, y, 0xFFFFFFFF);
                    } else {
                        bitstreamBWR.clear(idx);   // Red
                        bitstreamBWR.clear(idxr);
                        pixel = Color.RED;
                        //dmimage.bitmapBWR.setPixel(x, y, 0xFFFF0000);
                    }
                } else {
                    bitstreamBWR.clear(idx); // Black
                    bitstreamBWR.set(idxr);
                    pixel = Color.BLACK;
                    //dmimage.bitmapBWR.setPixel(x, y, 0xFF000000);
                }
                dmimage.bitmapBWR.setPixel(x, y, pixel);
            }
        }

        // Compress
        BitSet bitstreamBW_RLE = RLECompress(bitstreamBW);
        BitSet bitstreamBWR_RLE = RLECompress(bitstreamBWR);
        publishProgress(90);

        // Pack to bytes
        dmimage.byteStreamBW = bitstreamBytes(bitstreamBW_RLE);
        dmimage.byteStreamBWR = bitstreamBytes(bitstreamBWR_RLE);
        publishProgress(100);

        return dmimage;
    }

    private List<Byte> bitstreamBytes(BitSet bitstream) {
        // Pack bitstream to bytes
        List<Byte> result = new ArrayList<Byte>();
        byte[] bytes = bitstream.toByteArray();
        for (byte b : bytes) {
            int x = 0;
            // Reverse bit order
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
        for (int bsc = 0; bsc < klp; bsc++)
            result.add(new Byte((byte)0));
        Log.d("PHX", String.format("Padded %d to %d", idx, idx + klp));

        return result;
    }

    private BitSet RLECompress(BitSet bitstream) {
        int j = bitstream.length() - 1;
        int cnt = 1;
        boolean p = bitstream.get(0);
        boolean n;
        List<Integer> RLErun = new ArrayList<Integer>();

        // RLE compress
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

        // Gen unary coded runs
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

        Log.d("PHX", String.format("RLE compress, %d -> %d", bitstream.length(), bitstreamRLE.length()));

        /*String dbg = "";
        for (int i = 0; i < 256; i++) {
            dbg += (bitstreamRLE.get(i) ? "1" : "0");
        }
        Log.d("PHX", dbg);*/
        return bitstreamRLE;
    }
}