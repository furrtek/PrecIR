package org.furrtek.pricehax;

import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.ImageView;
import android.graphics.Color;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.BitSet;
import java.util.List;

import static org.furrtek.pricehax.DitherBitmap.floydSteinbergDithering;

public class DMGen {
    public static List<IRFrame> DMGenFrames(DMImage dmImage, boolean BWR, Long PLID, int dispPage) {
        List<IRFrame> frames = new ArrayList<IRFrame>();

        // Wake-up frame
        Byte[] payload = {(byte) 0x17, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload), 30, 200));   // TODO: Check delay and repeats

        int datalen = dmImage.byteStreamBW.size();  // TODO: Pass selected BW or BWR bytestream instead of dmImage
        int width = dmImage.bitmapBW.getWidth();
        int height = dmImage.bitmapBW.getHeight();
        Log.d("PHX", String.format("w,h: %d,%d", width, height));
        int x = 0;
        int y = 0;
        Byte[] payload_start = {
                (byte) 0x34, 0, 0, 0, 0x05,
                (byte) (datalen >> 8), (byte) (datalen & 255),
                0,
                0x02,
                (byte) dispPage, //0x01,
                (byte) (width >> 8), (byte) (width & 255),
                (byte) (height >> 8), (byte) (height & 255),
                (byte) (x >> 8), (byte) (x & 255),
                (byte) (y >> 8), (byte) (y & 255),
                0x00, 0x00,
                (byte) 0x88,
                0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
        };
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload_start), 30, 1));   // TODO: Check delay and repeats

        /*ymax = datalen / 40;        // WAS 20

        for (y = 0; y < datalen / 40; y++) {        // WAS 20

            startcode[0] = (byte) 0x85;
            startcode[1] = (byte) (plID & 255);
            startcode[2] = (byte) (plID >> 8);
            startcode[3] = (byte) (plID >> 16);
            startcode[4] = (byte) (plID >> 24);
            startcode[5] = (byte) 0x34;
            startcode[6] = (byte) 0x00;
            startcode[7] = (byte) 0x00;
            startcode[8] = (byte) 0x00;
            startcode[9] = (byte) 0x20;
            startcode[10] = (byte) (y >> 16);
            startcode[11] = (byte) (y & 255);

            for (int cp = 0; cp < 40; cp++) {        // WAS 20
                startcode[12 + cp] = hexlist.get(cp + (y * 40));        // WAS 20
            }

            FrameCRC = CRCCalc.GetCRC(startcode, 52);        // WAS 32
            startcode[52] = FrameCRC[0];        // WAS 32
            startcode[53] = FrameCRC[1];        // WAS 33

            // Send !
            PP4C.sendPP4C(at.getApplicationContext(), startcode, 54, donglever, 1, audioTrack);        // WAS 34
        }*/

        Log.d("PHX", String.format("Datalen %d", datalen));
        int ymax = (int) Math.ceil((double)datalen / 20);
        Log.d("PHX", String.format("ymax %d", ymax));
        for (y = 0; y < ymax; y++) {
            Byte[] payload_data = new Byte[27];
            Log.d("PHX", String.format("Gen data frame %d", y));
            /* (byte) 0x34, 0, 0, 0, 0x20, (byte) (y >> 8), (byte) (y & 255)*/
            payload_data[0] = (byte) 0x34;
            payload_data[1] = (byte) 0;
            payload_data[2] = (byte) 0;
            payload_data[3] = (byte) 0;
            payload_data[4] = (byte) 0x20;
            payload_data[5] = (byte) (y >> 8);
            payload_data[6] = (byte) (y & 255);
            for (int cp = 0; cp < 20; cp++) {        // WAS 20
                payload_data[7 + cp] = dmImage.byteStreamBW.get(cp + (y * 20));
            }
            frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload_data), 30, 1));   // TODO: Check delay and repeats
        }

        /*
        byte[] vercode = {(byte) 0x85, 0, 0, 0, 0, 0x34, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00};

        vercode[1] = (byte) (plID & 255);
        vercode[2] = (byte) (plID >> 8);
        vercode[3] = (byte) (plID >> 16);
        vercode[4] = (byte) (plID >> 24);

        FrameCRC = CRCCalc.GetCRC(vercode, 28);
        vercode[28] = FrameCRC[0];
        vercode[29] = FrameCRC[1];

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                txtworkh.setText("Verify frame...");
            }
        });

        // Send !
        PP4C.sendPP4C(at.getApplicationContext(), vercode, 30, donglever, 10, audioTrack);

        SystemClock.sleep(2000);

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                txtworkh.setText("Done ! ;-)");
            }
        });*/

        Byte[] payloadc = {(byte) 0x34, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payloadc), 30, 1));   // TODO: Check delay and repeats

        return frames;
    }
}