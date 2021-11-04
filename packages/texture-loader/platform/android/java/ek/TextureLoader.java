package ek;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import androidx.annotation.Keep;

import java.io.InputStream;
import java.util.HashMap;

class LoaderState {
    int texture = 0;
    int progress = 0;
    int width = 0;
    int height = 0;
}

public class TextureLoader {

    static int nextLoaderId = 1;
    static HashMap<Integer, LoaderState> map = new HashMap<>();

    @Keep
    public static int load(final Context context,
                           final String basePath,
                           final String[] urls,
                           final int flags,
                           final int formatMask) {

        final int id = nextLoaderId++;
        final LoaderState state = new LoaderState();
        map.put(id, state);
        new Thread(() -> {
            final Bitmap[] bitmaps = new Bitmap[urls.length];

            for (int i = 0; i < urls.length; ++i) {
                String filePath;
                if (basePath != null && basePath.length() > 0) {
                    filePath = basePath;
                    if(filePath.charAt(filePath.length() - 1) != '/') {
                        filePath += '/';
                    }
                    filePath += urls[i];
                }
                else {
                    filePath = urls[i];
                }

                if((formatMask & 2) != 0) {
                    // maybe all android api level >= 19 support WebP decoding
                    int lastDotIndex = filePath.lastIndexOf(".");
                    if (lastDotIndex >= 0) {
                        filePath = filePath.substring(0, lastDotIndex) + ".webp";
                    }
                }

                // Read in the resource
                try {
                    final InputStream stream = context.getAssets().open(filePath, AssetManager.ACCESS_BUFFER);

                    final BitmapFactory.Options options = new BitmapFactory.Options();
                    options.inScaled = false;   // No pre-scaling
                    options.inPremultiplied = (flags & 1) != 0;
                    bitmaps[i] = BitmapFactory.decodeStream(stream, null, options);
                    state.progress = 90 * i / urls.length;
                } catch (Exception ignored) {
                }
            }

            EkActivity.runGLThread(() -> {
                final int[] textureHandle = new int[1];
                GLES20.glGenTextures(1, textureHandle, 0);
                final int texture = textureHandle[0];
                if (texture != 0) {
                    final int[] lastID = new int[1];
                    final boolean cubeMap = (flags & 2) != 0;
                    if (cubeMap) {
                        final int[] faces = {
                                GLES20.GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                GLES20.GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                GLES20.GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                GLES20.GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                GLES20.GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                GLES20.GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
                        };
                        GLES20.glGetIntegerv(GLES20.GL_TEXTURE_BINDING_CUBE_MAP, lastID, 0);

                        GLES20.glBindTexture(GLES20.GL_TEXTURE_CUBE_MAP, texture);
                        for (int faceIndex = 0; faceIndex < faces.length; ++faceIndex) {
                            //GLES20.glTexImage2D(faces[faceIndex], 0, GLES20.GL_RGBA, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, bitmaps[faceIndex]);
                            GLUtils.texImage2D(faces[faceIndex], 0, GLES20.GL_RGBA, bitmaps[faceIndex], 0);
                        }
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_CUBE_MAP, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_CUBE_MAP, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_CUBE_MAP, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_CUBE_MAP, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

                        GLES20.glBindTexture(GLES20.GL_TEXTURE_CUBE_MAP, lastID[0]);
                    } else {
                        GLES20.glGetIntegerv(GLES20.GL_TEXTURE_BINDING_2D, lastID, 0);

                        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture);
                        //GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, image0);
                        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, bitmaps[0], 0);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
                        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

                        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, lastID[0]);
                    }

                    state.texture = texture;
                    state.progress = 100;
                    state.width = bitmaps[0].getWidth();
                    state.height = bitmaps[0].getHeight();
                }

                for (int i = 0; i < urls.length; ++i) {
                    if (bitmaps[i] != null) {
                        bitmaps[i].recycle();
                    }
                }

                if (texture == 0) {
                    throw new RuntimeException("Error loading texture.");
                }
            });
            
        }).start();

        return id;
    }

    @Keep
    public static int getProgress(final int id) {
        final LoaderState state = map.get(id);
        return state != null ? state.progress : 0;
    }

    @Keep
    public static int getWidth(final int id) {
        final LoaderState state = map.get(id);
        return state != null ? state.width : 0;
    }

    @Keep
    public static int getHeight(final int id) {
        final LoaderState state = map.get(id);
        return state != null ? state.height : 0;
    }

    @Keep
    public static int getTexture(final int id) {
        final LoaderState state = map.get(id);
        return state != null ? state.texture : 0;
    }

    @Keep
    public static int destroy(final int id) {
        return map.remove(id) != null ? 0 : 1;
    }
}