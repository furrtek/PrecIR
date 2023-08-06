package org.furrtek.pricehax;

public interface AsyncResponseTX {
    //Add requestCode to identify request.
    public void processFinish(boolean result);
}
