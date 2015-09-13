package com.harrison.camera;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.util.Log;

public class AudioRecorde {
	static final String TAG = "AudioRecorde";
	static boolean isRecording = false;
	
	private static AudioRecord audioRecorder = null;
	private static Thread recordingThread = null;

	static void startRecorde() {
		
		initAudio();
		
		int audioSource = AudioSource.MIC;
		int sampleRateInHz = 44100;
		int channelConfig = AudioFormat.CHANNEL_IN_MONO;
		int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
		int bufferSizeInBytes = AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
		
		audioRecorder = new AudioRecord(audioSource,
                sampleRateInHz, 
                channelConfig, 
                audioFormat, 
                bufferSizeInBytes);
		
		isRecording = true;
		audioRecorder.startRecording();
		
		recordingThread = new Thread(new Runnable() {
			public void run(){
				while (isRecording){
					byte sData[] = new byte[1024];
					int nSize = audioRecorder.read(sData, 0, 1024);
					processAudio(sData, nSize);
				}
			}
		});
		recordingThread.start();
	}
	
	static void stopRecorde() {
		
		isRecording = false;
		
		if (recordingThread != null) {
			recordingThread.interrupt();
			recordingThread = null;
		}
		
		audioRecorder.stop();
		audioRecorder.release();
		audioRecorder = null;
		
		stopAudio();
	}
	
	static native void initAudio();
	static native void processAudio(byte[] data, int size);
	static native void stopAudio();
}
