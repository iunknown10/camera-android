package com.harrison.camera;

import java.io.IOException;
import com.harrison.camera.R;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;

@SuppressWarnings("deprecation")
public class CameraPreview extends Activity implements SurfaceHolder.Callback, PreviewCallback {
	
	Camera camera;
	byte[] previewBuffer;
	
	SurfaceHolder previewHolder;

	static {
		System.loadLibrary("core");
	}
	
	@Override
    protected void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.setContentView(R.layout.activity);
        
        SurfaceView svCameraPreview = (SurfaceView) this.findViewById(R.id.surfaceView);        
        this.previewHolder = svCameraPreview.getHolder();
        this.previewHolder.setFixedSize(1, 1);
        this.previewHolder.addCallback(this);
       
        this.findViewById(R.id.start).setOnClickListener(
            	new View.OnClickListener() 
            	{
    				@Override
    				public void onClick(View v) 
    				{
    					startCamera();
    					AudioRecorde.startRecorde();
    				}
    			});
        this.findViewById(R.id.stop).setOnClickListener(
        		new View.OnClickListener()
        		{
        			@Override
        			public void onClick(View v)
        			{
        				stopCamera();
        				AudioRecorde.stopRecorde();
        			}
        		});
    }
	
	@Override
    protected void onPause() 
    {
    	this.stopStream();
    	
    	super.onPause();
    }
	
	private void stopStream()
	{
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {
		// TODO Auto-generated method stub
		
		this.camera.addCallbackBuffer(this.previewBuffer);
		processVideo(data, data.length);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
	}
	
	private void startCamera()
	{        
		int width = 640;
        int height = 480;
        
        /*
		Camera tmpCams = Camera.open();
		Camera.Parameters paramss = tmpCams.getParameters();  
		final List<Size> prevSizess = paramss.getSupportedPreviewSizes();
		tmpCams.release();
		tmpCams = null;
		*/

        this.previewHolder.setFixedSize(width, height);
        initVideo(width, height);
        
    	int stride = (int) Math.ceil(width/16.0f) * 16;
    	int cStride = (int) Math.ceil(width/32.0f)  * 16;
        final int frameSize = stride * height;
        final int qFrameSize = cStride * height / 2;
        
        this.previewBuffer = new byte[frameSize + qFrameSize * 2];
        
        try   
        {  
        	camera = Camera.open();  
        	camera.setPreviewDisplay(this.previewHolder);  
            Camera.Parameters params = camera.getParameters();  
            params.setPreviewSize(width, height);  
            params.setPreviewFormat(ImageFormat.YV12);  
            camera.setParameters(params);   
            camera.addCallbackBuffer(previewBuffer);
            camera.setPreviewCallbackWithBuffer(this);  
            camera.startPreview();                
        }
        catch (IOException e)   
        {  
            //TODO:
        }   
        catch (RuntimeException e)
        {
            //TODO:
        }        		
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		// TODO Auto-generated method stub
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		stopCamera();
	}
	
	private void stopCamera()
	{
    	if (camera != null)
    	{
			camera.setPreviewCallback(null);
			camera.stopPreview();   
	        camera.release();  
	        camera = null;
    	}
    	
    	stopVideo();
    	this.previewHolder.setFixedSize(1, 1);
	}
	
	private native void initVideo(int width, int height);
	private native void processVideo(byte[] data, int size);
	private native void stopVideo();
}
