package ekapp;

import android.content.res.AssetFileDescriptor;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

public class EKMusic {
    private static final String LOG_TAG = "hl";
    private static final int TIMEOUT_US = 0;

    private MediaCodec decoder;
    public AudioTrack audioTrack;
    private MediaExtractor extractor;
    private int sampleRate;
    private boolean eosReceived = false;
    public float volume = 0.0f;
    public float prevVolume = 0.0f;
    public boolean playing = true;
    public Thread thread;

    EKMusic(AssetFileDescriptor fd) {
        extractor = new MediaExtractor();
        try {
            extractor.setDataSource(fd.getFileDescriptor(), fd.getStartOffset(), fd.getDeclaredLength());
        } catch (IOException e) {
            Log.e(LOG_TAG, "Error music creation", e);
        }

        thread = new Thread(DecodeAndPlay);
        thread.start();
    }

    private Runnable DecodeAndPlay = new Runnable() {
        @Override
        public void run() {
//            android.os.Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
            decodeAndPlay();
        }
    };

    private void decodeAndPlay() {
        MediaFormat format = extractor.getTrackFormat(0);
        String mime = format.getString(MediaFormat.KEY_MIME);
        sampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE);

        Log.i(LOG_TAG, "mime type : " + mime);
        Log.i(LOG_TAG, "sample rate : " + sampleRate);

        try {
            decoder = MediaCodec.createDecoderByType(mime);
        } catch (IOException e) {
            Log.e("hl", "MediaCodec.createDecoderByType", e);
        }
        decoder.configure(format, null, null, 0);
        decoder.start();

        extractor.selectTrack(0);

        int buffsize = AudioTrack.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
        // create an audiotrack object
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                buffsize * 2,
                AudioTrack.MODE_STREAM);
        audioTrack.setStereoVolume(0.0f, 0.0f);
        audioTrack.play();

        ByteBuffer[] inputBuffers = decoder.getInputBuffers();
        ByteBuffer[] outputBuffers = decoder.getOutputBuffers();

        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

//        int buffsize = AudioTrack.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
//        // create an audiotrack object
//        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
//                AudioFormat.CHANNEL_OUT_STEREO,
//                AudioFormat.ENCODING_PCM_16BIT,
//                buffsize,
//                AudioTrack.MODE_STREAM);
//        audioTrack.play();


        while (!eosReceived) {
            int inIndex = decoder.dequeueInputBuffer(TIMEOUT_US);
            boolean inputEnd = false;
            if (inIndex >= 0) {
                ByteBuffer buffer = inputBuffers[inIndex];
                int sampleSize = extractor.readSampleData(buffer, 0);
                if (sampleSize < 0) {
                    // We shouldn't stop the playback at this point, just pass the EOS
                    // flag to mDecoder, we will get it again from the
                    // dequeueOutputBuffer
//                    Log.d("DecodeActivity", "InputBuffer BUFFER_FLAG_END_OF_STREAM");
                    decoder.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    inputEnd = true;
                } else {
                    decoder.queueInputBuffer(inIndex, 0, sampleSize, extractor.getSampleTime(), 0);
                    extractor.advance();
                }

                int outIndex = decoder.dequeueOutputBuffer(info, TIMEOUT_US);
                switch (outIndex) {
                    case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
//                        Log.d("DecodeActivity", "INFO_OUTPUT_BUFFERS_CHANGED");
                        outputBuffers = decoder.getOutputBuffers();
                        break;

                    case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                        format = decoder.getOutputFormat();
//                        Log.d("DecodeActivity", "New format " + format);
                        audioTrack.setPlaybackRate(format.getInteger(MediaFormat.KEY_SAMPLE_RATE));

                        break;
                    case MediaCodec.INFO_TRY_AGAIN_LATER:
//                        Log.d("DecodeActivity", "dequeueOutputBuffer timed out!");
                        break;

                    default:
                        ShortBuffer outBuffer = outputBuffers[outIndex].asShortBuffer();
                        //Log.v("DecodeActivity", "We can't use this buffer but render it due to the API limit, " + outBuffer);

                        final short[] chunk = new short[info.size >>> 1];
                        outBuffer.get(chunk); // Read the buffer all at once
                        outBuffer.clear(); // ** MUST DO!!! OTHERWISE THE NEXT TIME YOU GET THIS SAME BUFFER BAD THINGS WILL HAPPEN
                        audioTrack.write(chunk, 0, chunk.length);
                        updateSamples(chunk, 0, info.size);
                        decoder.releaseOutputBuffer(outIndex, false);
                        break;
                }

                // All decoded frames have been rendered, we can stop playing now
                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0 || inputEnd) {
//                    Log.d("DecodeActivity", "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
                    //break;
                    extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC);
                    decoder.flush();
                }

                if (prevVolume != volume) {
                    audioTrack.setStereoVolume(volume, volume);
                    prevVolume = volume;
                }

                if (playing && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_STOPPED) {
                    audioTrack.play();
                }
            } else {
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                }
            }
        }

        decoder.stop();
        decoder.release();
        decoder = null;

        extractor.release();
        extractor = null;

        audioTrack.stop();
        audioTrack.release();
        audioTrack = null;
    }

    public void pause() {
        audioTrack.pause();
    }

    public void resume() {
        audioTrack.play();
    }

    public static volatile short[] samplesSnapshot;

    private void updateSamples(final short[] waveform, int start, int size) {
//        Log.w("hl", "updateSamples: " + size); // 2492, 4608
        if (size < 512) {
            return;
        }

        final short[] samples = new short[256];
        int ptr = 0;
        for (int i = 0; i < 256; ++i) {
            samples[i] = waveform[ptr];
            ptr += 2;
        }
        samplesSnapshot = samples;
    }

    public void release() {
        eosReceived = true;
    }
}
