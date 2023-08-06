package org.furrtek.pricehax;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.AsyncTask;
import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;
import java.io.IOException;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.*;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.google.android.material.snackbar.Snackbar;
import androidx.appcompat.app.AppCompatActivity;
import android.view.View;
import androidx.navigation.ui.AppBarConfiguration;

import android.view.Menu;
import android.view.MenuItem;

import net.sourceforge.zbar.Config;
import net.sourceforge.zbar.Image;
import net.sourceforge.zbar.ImageScanner;
import net.sourceforge.zbar.Symbol;
import org.furrtek.pricehax.databinding.ActivityMainBinding;

import java.nio.charset.StandardCharsets;
import java.util.*;

import static android.provider.Settings.System.getString;
import static androidx.core.app.ActivityCompat.finishAffinity;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;
    private Handler autoFocusHandler;
    private CameraPreview mPreview;
    private long lastPLID = 0;
    FrameLayout preview;
    ImageScanner scanner;
    String lastBarcodeString = "";
    int imageScale = 100;
    DMImage dmImage = null;
    boolean inBWR = false;

    private enum UsbPermission { Unknown, Requested, Granted, Denied }
    private static final String INTENT_ACTION_GRANT_USB = BuildConfig.APPLICATION_ID + ".GRANT_USB";

    private int deviceId, portNum, baudRate;
    private UsbSerialPort usbSerialPort;
    private UsbPermission usbPermission = UsbPermission.Unknown;
    private SerialInputOutputManager usbIoManager;
    private boolean blasterConnected = false;
    char blasterHWVersion;
    int blasterFWVersion;
    CoordinatorLayout mainlayout;
    Uri selectedImageUri = null;
    int dispDurationIdx, dispPage;
    //AsyncResponseTX asyncTaskTX = null;
    boolean transmitting = false;
    boolean loopTX = false;
    boolean autoTX = false;

    static {
        System.loadLibrary("iconv");
    }

    @Override
    public void onResume() {
        super.onResume();
        testConnect();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        setSupportActionBar(binding.toolbar);

        mainlayout = (CoordinatorLayout) findViewById(R.id.coordinatorLayout);

        // App e-stop
        binding.main.fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finishAffinity();
                System.exit(0);
            }
        });

        // Stop TX
        binding.main.buttonTXStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (transmitting) {
                    byte[] data = {(byte)'S'};
                    try {
                        usbSerialPort.write(data, 1000);
                    } catch(IOException ignore) {
                    }
                    binding.main.switchLoopTX.setChecked(false);
                }
            }
        });

        // Populate duration spinner
        Spinner spinner = binding.main.spinnerDuration;
        List<String> arrayList = Arrays.asList("2s", "15s", "15m", "Forever");
        ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, arrayList);
        arrayAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(arrayAdapter);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                dispDurationIdx = position;
            }
            @Override
            public void onNothingSelected(AdapterView <?> parent) {
            }
        });

        // Populate page spinner
        List<String> pageList = new ArrayList<String>();
        for (int i = 0; i < 7; i++)
            pageList.add(Integer.toString(i));
        ArrayAdapter<String> arrayAdapterPage = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, pageList);
        arrayAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        binding.main.spinnerPage.setAdapter(arrayAdapterPage);
        binding.main.spinnerPage.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                dispPage = Integer.parseInt(parent.getItemAtPosition(position).toString());
            }
            @Override
            public void onNothingSelected(AdapterView <?> parent) {
            }
        });

        binding.main.buttonTXPageDM.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // 0x85, 0x00, 0x00, 0x00, 0x00, 0x06, 0xF1, 0x00, 0x00, 0x00, 0x0A, 0x5D, 0x14
                List<IRFrame> frames = new ArrayList<IRFrame>();
                Byte[] payload = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00};
                payload[1] = (byte)(((dispPage & 7) << 3) | 1);

                if (dispDurationIdx != 3) {
                    int[] durations = {2, 15, 15*60, -1};
                    int duration = durations[dispDurationIdx];
                    payload[4] = (byte)(duration >> 8);
                    payload[5] = (byte)(duration & 255);
                } else {
                    payload[1] = (byte)(payload[1] | 0x80);   // "Forever" flag
                }

                frames.add(new IRFrame(0, (byte)0x85, Arrays.asList(payload), 30, 200));
                IRTransmit(frames);
            }
        });

        binding.main.buttonTXPage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // 0x84, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x00, 0x00, 0x00

                List<IRFrame> frames = new ArrayList<IRFrame>();
                Byte[] payload = {(byte)0xAB, 0x00, 0x00, 0x00};

                int[] durations = {1, 3, 5, 0x80};
                int duration = durations[dispDurationIdx];
                payload[1] = (byte)(((dispPage & 7) << 3) | duration);

                frames.add(new IRFrame(0, (byte)0x84, Arrays.asList(payload), 30, 200));
                IRTransmit(frames);
            }
        });

        binding.main.switchLoopTX.setOnCheckedChangeListener(new Switch.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                loopTX = isChecked;
            }
        });

        binding.main.switchTXAuto.setOnCheckedChangeListener(new Switch.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                autoTX = isChecked;
            }
        });

        binding.main.buttonTXImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TransmitImage();
            }
        });

        // Image selection and processing
        ActivityResultLauncher<Intent> someActivityResultLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                new ActivityResultCallback<ActivityResult>() {
                    @Override
                    public void onActivityResult(ActivityResult result) {
                        if (result.getResultCode() == Activity.RESULT_OK) {
                            if (result.getData() != null) {
                                selectedImageUri = result.getData().getData();
                                convertImage();
                            }
                        }
                    }
                });

        binding.main.buttonLoadImg.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent photoPickerIntent = new Intent(Intent.ACTION_PICK);
                photoPickerIntent.setType("image/*");
                photoPickerIntent.setAction(Intent.ACTION_GET_CONTENT);
                someActivityResultLauncher.launch(photoPickerIntent);
            }
        });

        // BW or BWR mode selection
        binding.main.radiogroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                inBWR = binding.main.radioBWR.isChecked();
                updateImage();
            }
        });

        // Image scaling change
        binding.main.seekScale.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekbar, int progress, boolean fromUser) {
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekbar) {
            }
            @Override
            public void onStopTrackingTouch(SeekBar seekbar) {
                imageScale = seekbar.getProgress();
                convertImage();
            }
        });

        // Start camera preview. Ask for camera permission if not already granted.
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(MainActivity.this, new String[] {Manifest.permission.CAMERA}, 123);
        } else {
            setScanPreview();
        }

        // Register USB action receiver
        BroadcastReceiver USBReceiver = new BroadcastReceiver() {
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
                    setDisconnected();
                } else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                    testConnect();
                }
            }
        };
        IntentFilter filter = new IntentFilter();
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(USBReceiver, filter);

        // Load default image
        selectedImageUri = Uri.parse("android.resource://org.furrtek.pricehax/" + R.drawable.dm_128x64);
        convertImage();
    }

    private void TransmitImage() {
        List<IRFrame> frames = DMGen.DMGenFrames(dmImage, inBWR, lastPLID, dispPage);
        IRTransmit(frames);
    }

    private void convertImage() {
        if (selectedImageUri == null) return;
        try {
            Bitmap selectedImage = BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImageUri));
            selectedImage = scaleImage(selectedImage);

            new DMConvert(
                binding.main.progressbar,
                new AsyncResponse() {
                    @Override
                    public void processFinish(DMImage dmimage) {
                        dmImage = dmimage;
                        binding.main.textScale.setText(getString(R.string.image_dimensions, dmimage.bitmapBW.getWidth(), dmimage.bitmapBW.getHeight()));
                        updateImage();
                        Log.d("PHX", "Converted image ok");
                    }
                }).execute(selectedImage);
        } catch (Exception e) {
            Log.d("PHX", e.getLocalizedMessage());
        }
    }
    public void updateImage() {
        if (dmImage != null)
            binding.main.imageviewDm.setImageBitmap(inBWR ? dmImage.bitmapBWR : dmImage.bitmapBW);
    }

    private Bitmap scaleImage(Bitmap selectedImage) {
        float originalWidth = selectedImage.getWidth();
        float originalHeight = selectedImage.getHeight();
        int newWidth, newHeight;
        if (originalWidth < 300) {
            //imageScale = (int)(originalWidth * 100) / 300;
            newWidth = (int)originalWidth;
            newHeight = (int)originalHeight;
        } else {
            newWidth = (300 * imageScale) / 100;
            newHeight = (int)(newWidth * (originalHeight / originalWidth));
        }
        return Bitmap.createScaledBitmap(selectedImage, newWidth, newHeight, true);
    }

    public void IRTransmit(List<IRFrame> frames) {
        if (!blasterConnected) return;
        enableTXWidgets(false);
        transmitting = true;
        new ESLBlaster(
            usbSerialPort,
            binding.main.progressbar,
            binding.main.textStatus2,
            new AsyncResponseTX() {
                @Override
                public void processFinish(boolean result) {
                    if (loopTX) {
                        IRTransmit(frames);
                    } else {
                        transmitting = false;
                        enableTXWidgets(true);
                    }
                }
            }).execute(frames);
    }

    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 123) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Camera permission granted", Toast.LENGTH_LONG).show();
                setScanPreview();
            } else {
                Toast.makeText(this, "Camera permission denied", Toast.LENGTH_LONG).show();
                // TODO: Close app ?
            }
        }
    }

    Camera.PreviewCallback previewCb = new Camera.PreviewCallback() {
        public void onPreviewFrame(byte[] data, Camera camera) {
            String strSupplement;
            Camera.Size size = camera.getParameters().getPreviewSize();
            Image previewimage = new Image(size.width, size.height, "Y800"); // FourCC for monochrome image
            previewimage.setData(data);
            if (MainActivity.this.scanner.scanImage(previewimage) != 0) {
                Iterator<Symbol> it = MainActivity.this.scanner.getResults().iterator();
                //while (it.hasNext()) {
                String barcodeString = ((Symbol) it.next()).getData();
                if (barcodeString != lastBarcodeString) {
                    if (barcodeValid(barcodeString)) {
                        lastPLID = (Integer.parseInt(barcodeString.substring(2, 7))<<16) + Integer.parseInt(barcodeString.substring(7, 12));
                        String PLSerial = Long.toHexString(lastPLID);
                        while (PLSerial.length() < 8)
                            PLSerial = "0" + PLSerial;
                        if (PLSerial.length() > 8)
                            PLSerial = PLSerial.substring(8);
                        //lastPLType = Integer.parseInt(barcodeString.substring(12, 16));
                        strSupplement = "OK " + PLSerial.toUpperCase();
                        if (autoTX && !transmitting)
                            TransmitImage();
                    } else {
                        strSupplement = "INVALID";
                        lastPLID = 0;
                    }
                    binding.main.textLastScanned.setText("Last scanned:\n" + barcodeString + "\n (" + strSupplement + ")");
                }
                lastBarcodeString = barcodeString;

                /*MainActivity.this.previewing = false;
                camera.setPreviewCallback(null);
                camera.stopPreview();
                MainActivity.this.scanButton.setText("Scan another ESL barcode?");*/
                //}
            }
        }
    };
    private Runnable doAutoFocus = new Runnable() {
        public void run() {
            /*if (MainActivity.this.mCamera != null) {
                MainActivity.this.mCamera.autoFocus(MainActivity.this.autoFocusCB);
            }*/
        }
    };

    Camera.AutoFocusCallback autoFocusCB = new Camera.AutoFocusCallback() {
        public void onAutoFocus(boolean success, Camera camera) {
            // postDelayed() will call doAutoFocus() and onAutoFocus() will be called again from mCamera.autoFocus().
            // This will create a timer that will readjust the focus every second.
            MainActivity.this.autoFocusHandler.postDelayed(MainActivity.this.doAutoFocus, 1000);
        }
    };

    public void setScanPreview() {
        // Init Camera preview
        this.autoFocusHandler = new Handler();
        this.mPreview = new CameraPreview(this, this.previewCb, this.autoFocusCB);
        //this.preview = (FrameLayout) findViewById(R.id.cameraPreview);
        this.preview = binding.main.cameraPreview;
        this.preview.addView(this.mPreview);

        // Init ZBar
        this.scanner = new ImageScanner();
        this.scanner.setConfig(0, Config.X_DENSITY, 3);
        this.scanner.setConfig(0, Config.Y_DENSITY, 3);
        this.scanner.setConfig(0, Config.MIN_LEN, 17);
        this.scanner.setConfig(0, Config.MAX_LEN, 17);
        // Disable all symbols except Code128
        this.scanner.setConfig(0, Config.ENABLE, 0);
        this.scanner.setConfig(Symbol.CODE128, Config.ENABLE, 1);
    }
    boolean barcodeValid(String barcodeString) {
        if (barcodeString.charAt(1) != '4') return false;
        if (Integer.parseInt(barcodeString.substring(5, 6)) > 53) return false;
        int sum = 0;
        for (int i = 0; i < barcodeString.length() - 1; i++)
            sum += (int)barcodeString.charAt(i);
        if (sum % 10 != Integer.parseInt(barcodeString.substring(16, 17))) return false;
        return true;
    }

    public void status(String msg) {
        Snackbar.make(mainlayout, msg, Snackbar.LENGTH_LONG).setAction("Action", null).show();
    }

    public void testConnect() {
        UsbDevice device = null;
        UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        /*for(UsbDevice v : usbManager.getDeviceList().values())
            if(v.getDeviceId() == deviceId)
                device = v;*/
        Collection<UsbDevice> devices = usbManager.getDeviceList().values();
        if (devices.size() == 0)
            return;
        device = devices.iterator().next();
        if (device == null) {
            status("ESL Blaster connection failed: device not found");
            return;
        }
        UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
        if (driver == null) {
            //status(view,"connection failed: no driver for device");
            //status(view, String.valueOf(device.getVendorId()));   // 1155 = 0x483 ok
            status(String.valueOf(device.getProductId()));
            return;
        }
        if (driver.getPorts().size() < portNum) {
            status("ESL Blaster connection failed: not enough ports at device");
            return;
        }
        usbSerialPort = driver.getPorts().get(portNum);
        UsbDeviceConnection usbConnection = usbManager.openDevice(driver.getDevice());
        if (usbConnection == null && usbPermission == UsbPermission.Unknown && !usbManager.hasPermission(driver.getDevice())) {
            usbPermission = UsbPermission.Requested;
            int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_IMMUTABLE : 0;
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(INTENT_ACTION_GRANT_USB), flags);
            usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
            return;
        }
        if (usbConnection == null) {
            if (!usbManager.hasPermission(driver.getDevice()))
                status("ESL Blaster connection failed: permission denied");
            else
                status("ESL Blaster connection failed: open failed");
            return;
        }

        try {
            usbSerialPort.open(usbConnection);
            usbSerialPort.setParameters(57600, 8, 1, UsbSerialPort.PARITY_NONE);

            try {
                byte[] buffer = new byte[256];
                byte[] data = {(byte)'?'};
                usbSerialPort.write(data, 1000);
                if (usbSerialPort.read(buffer, 2000) > 0) {  // 2s timeout should be enough
                    String s = new String(buffer, StandardCharsets.UTF_8).substring(0, 12);
                    Log.d("PHX", String.valueOf((char)buffer[0]));
                    if (s.substring(0, 10).equals("ESLBlaster")) {
                        status("ESL Blaster connected !");
                        blasterHWVersion = s.charAt(10);
                        blasterFWVersion = Character.digit(s.charAt(11), 10);
                        blasterConnected = true;
                        enableTXWidgets(true);
                        binding.main.textStatus.setText(String.format("ESLBlaster connected (HW %c, FW %d)", blasterHWVersion, blasterFWVersion));
                    }
                };
            } catch(IOException e) {
                e.printStackTrace();
                setDisconnected();
            }

        } catch (Exception e) {
            status("ESL Blaster connection failed: " + e.getMessage());
            setDisconnected();
        }
    }

    public void enableTXWidgets(boolean enable) {
        binding.main.buttonTXPage.setEnabled(enable);
        binding.main.buttonTXPageDM.setEnabled(enable);
        binding.main.buttonTXImage.setEnabled(enable);
    }

    private void setDisconnected() {
        blasterConnected = false;
        enableTXWidgets(false);
        status("ESL Blaster disconnected !");
        binding.main.textStatus.setText("ESL Blaster not connected");
        if (usbIoManager != null) {
            usbIoManager.setListener(null);
            usbIoManager.stop();
            usbIoManager = null;
        }
        try {
            usbSerialPort.close();
        } catch (IOException ignored) {}
        usbSerialPort = null;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_about) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage("App and ESL Blaster by furrtek\nhttps://github.com/furrtek/PrecIR/\n\nThanks to: Aoi, david4599, Deadbird, Dr.Windaube, Sigmounte, BiduleOhm, Virtualabs, LightSnivy")
                    .setTitle("About");
            AlertDialog dialog = builder.create();
            dialog.show();
            return true;
        } else if (id == R.id.action_help) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setMessage("Use an ESL Blaster with a USB OTG cable to use this app. Phone IR transmitters aren't fast enough to communicate with ESLs.\n\nPage change doesn't require a valid barcode (found printed on front or back of ESL) to be scanned. Changing segments or images do.\n\nDM ESL images won't update if the image is too big or in the wrong mode. A B/W 50x50px image should always work.")
                    .setTitle("Help");
            AlertDialog dialog = builder.create();
            dialog.show();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    /*@Override
    protected void onNewIntent(Intent intent) {
        if("android.hardware.usb.action.USB_DEVICE_ATTACHED".equals(intent.getAction())) {
            //Log.i("PriceHax", "Device detected !");
            //MainActivity.this.scaneibarcode.setText("USB_DEVICE_ATTACHED !");
        }
        super.onNewIntent(intent);
    }*/
}