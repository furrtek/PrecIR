package org.furrtek.pricehax2;

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

import static org.furrtek.pricehax2.DitherBitmap.floydSteinbergDithering;

public class DMGen {
    public static List<IRFrame> DMGenFrames(DMImage dmImage, boolean BWR, Long PLID, int dispPage) {
        List<IRFrame> frames = new ArrayList<IRFrame>();

        // Wake-up frame
        Byte[] payload = {(byte) 0x17, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload), 10, 400)); // 400 Same parameters as Python tools

        // Start frame
        List<Byte> byteStream = BWR ? dmImage.byteStreamBWR : dmImage.byteStreamBW;
        int datalen = byteStream.size();
        int width = dmImage.bitmapBW.getWidth();
        int height = dmImage.bitmapBW.getHeight();
        Log.d("PHX", String.format("Dimensions: %d*%d", width, height));
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
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload_start), 10, 1));

        // Data frames
        Log.d("PHX", String.format("datalen: %d", datalen));
        int frame_count = (int)Math.ceil((double)datalen / 20);
        Log.d("PHX", String.format("frame_count: %d", frame_count));
        for (int frame_idx = 0; frame_idx < frame_count; frame_idx++) {
            Byte[] payload_data = new Byte[27];
            payload_data[0] = (byte) 0x34;
            payload_data[1] = (byte) 0;
            payload_data[2] = (byte) 0;
            payload_data[3] = (byte) 0;
            payload_data[4] = (byte) 0x20;
            payload_data[5] = (byte) (frame_idx >> 8);
            payload_data[6] = (byte) (frame_idx & 255);
            for (int cp = 0; cp < 20; cp++) {
                payload_data[7 + cp] = byteStream.get(cp + (frame_idx * 20));
            }
            frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payload_data), 10, 1));
        }

        // Refresh frame
        Byte[] payloadc = {(byte) 0x34, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        frames.add(new IRFrame(PLID, (byte)0x85, Arrays.asList(payloadc), 10, 1));

        return frames;
    }
}