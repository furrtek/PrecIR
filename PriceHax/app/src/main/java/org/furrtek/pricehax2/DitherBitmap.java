package org.furrtek.pricehax2;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;

public class DitherBitmap {
    // Atkinson dither might give better results on ESLs
    public static Bitmap floydSteinbergDithering(Bitmap sourceimg, boolean withRed) {

        Bitmap img = sourceimg.copy(sourceimg.getConfig(), true);

        C3[] paletteBW = new C3[] {
                new C3(0, 0, 0),
                new C3(255, 255, 255)
        };
        C3[] paletteBWR = new C3[] {
                new C3(0, 0, 0),
                new C3(255, 0, 0),
                new C3(255, 255, 255)
        };

        int w = sourceimg.getWidth();
        int h = sourceimg.getHeight();

        C3[][] d = new C3[h][w];

        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                d[y][x] = new C3(sourceimg.getPixel(x, y));

        for (int y = 0; y < img.getHeight(); y++) {
            for (int x = 0; x < sourceimg.getWidth(); x++) {

                C3 oldColor = d[y][x];
                C3 newColor = findClosestPaletteColor(oldColor, withRed ? paletteBWR : paletteBW);
                img.setPixel(x, y, newColor.toARGB());

                C3 err = oldColor.sub(newColor);

                if (x+1 < w)         d[y  ][x+1] = d[y  ][x+1].add(err.mul(7.f/16));
                if (x-1>=0 && y+1<h) d[y+1][x-1] = d[y+1][x-1].add(err.mul(3.f/16));
                if (y+1 < h)         d[y+1][x  ] = d[y+1][x  ].add(err.mul(5.f/16));
                if (x+1<w && y+1<h)  d[y+1][x+1] = d[y+1][x+1].add(err.mul(1.f/16));
            }
        }

        return img;
    }

    private static C3 findClosestPaletteColor(C3 c, C3[] palette) {
        C3 closest = palette[0];

        for (C3 n : palette)
            if (n.diff(c) < closest.diff(c))
                closest = n;

        return closest;
    }

    static class C3 {
        int r, g, b;

        public C3(int c) {
            this.r = Color.red(c);
            this.g = Color.green(c);
            this.b = Color.blue(c);
        }
        public C3(int r, int g, int b) {
            this.r = r;
            this.g = g;
            this.b = b;
        }

        public C3 add(C3 o) {
            return new C3(r + o.r, g + o.g, b + o.b);
        }
        public C3 sub(C3 o) {
            return new C3(r - o.r, g - o.g, b - o.b);
        }
        public C3 mul(float d) {
            return new C3((int) (d * r), (int) (d * g), (int) (d * b));
        }
        public int diff(C3 o) {
            return Math.abs(r - o.r) + Math.abs(g - o.g) +  Math.abs(b - o.b);
        }

        public int toARGB() {
            return Color.argb(255, clamp(r), clamp(g), clamp(b));
        }
        public int clamp(int c) {
            return Math.max(0, Math.min(255, c));
        }
    }
}
