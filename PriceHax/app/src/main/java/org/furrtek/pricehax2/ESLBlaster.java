package org.furrtek.pricehax2;

import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.ImageView;
import android.widget.TextView;
import com.hoho.android.usbserial.driver.UsbSerialPort;

import java.io.IOException;
import java.util.ArrayList;
import java.util.BitSet;
import java.util.List;

public class ESLBlaster extends AsyncTask<List<IRFrame>, TXProgress, Boolean> {
    private ImageView mImageView;
    private ProgressBar mProgressBar;
    AsyncResponseTX delegate = null;
    UsbSerialPort usbSerialPort;
    TextView mTextView;
    TXProgress progress = new TXProgress(0, "");

    ESLBlaster(UsbSerialPort usbserialport, ProgressBar progressBar, TextView textView, AsyncResponseTX asyncResponse) {
        delegate = asyncResponse;
        mProgressBar = progressBar;
        mTextView = textView;
        usbSerialPort = usbserialport;
    }
    @Override
    protected void onProgressUpdate(TXProgress... progress) {
        //Log.d("PHX", "Progress: " + progress[0]);
        mTextView.setText(progress[0].text);
        mProgressBar.setProgress(progress[0].percent);
    }
    @Override
    protected void onPostExecute(Boolean result) {
        Log.d("PHX", "TX done");
        endTX();
        delegate.processFinish(result);
    }
    @Override
    protected Boolean doInBackground(List<IRFrame>... frames) {
        int i = 1;
        int cnt = frames[0].size();
        for (IRFrame frame : frames[0]) {
            /*try {
                usbSerialPort.purgeHwBuffers(true, true);
            } catch(IOException e) {
                e.printStackTrace();
                return false;
            }*/

            List<Byte> list = frame.getRawData(false);

            byte[] data = new byte[list.size() + 5 + 1];
            data[0] = (byte)'L';
            data[1] = (byte)list.size();
            data[2] = (byte)frame.delay;            // Delay between repeats
            data[3] = (byte)(frame.repeats & 255);  // Number of repeats
            data[4] = (byte)(frame.repeats >> 8);
            for (int c = 0; c < list.size(); c++)
                data[c + 5] = list.get(c).byteValue();
            data[data.length - 1] = (byte)'T';

            // Debug
            String hex_str = "Blaster frame " + String.format("%d/%d: ", i, cnt);
            for (byte b : data)
                hex_str += String.format("%02X ", b);
            Log.d("PHX", hex_str);
            try {
                byte[] buffer = new byte[256];
                usbSerialPort.write(data, 1000);
                if (frame.repeats > 10) {
                    // Differentiate data vs. wake up frames with their repeat count
                    progress.text = "Waking up ESL, please wait...";
                    publishProgress(progress);
                } else {
                    progress.text = String.format("Transmitting frame %d/%d...", i, cnt);
                    publishProgress(progress);
                }

                // 10s timeout should be enough
                // Split the timeout to check if task was cancelled during the wait
                int w = 0;
                for (w = 0; w < 10; w++) {
                    if (isCancelled()) {
                        endTX();
                        return false;
                    }
                    if (usbSerialPort.read(buffer, 1000) > 0) {
                        //Log.d("PHX", String.valueOf((char)buffer[0]));
                        if (buffer[0] == (byte)'K')
                            break;
                    };
                }
                if (w == 10) {
                    Log.d("PHX", "Comm timeout");
                    endTX();   // Timed out
                    return false;
                }
            } catch(IOException e) {
                e.printStackTrace();
                endTX();
                return false;
            }
            progress.percent = (int)((i * 100) / cnt);
            publishProgress(progress);
            i++;
        }
        endTX();
        return true;
    }

    private void endTX() {
        progress.percent = 0;
        progress.text = "Ready";
        publishProgress(progress);
    }
}