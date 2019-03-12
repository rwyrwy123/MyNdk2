package com.example.myndk1.gl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class RWGLSurfaceView extends GLSurfaceView {

    private RWRender render;

    public RWGLSurfaceView(Context context) {
        this(context,null);
    }

    public RWGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        render = new RWRender(context);
        setRenderer(render);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v)
    {
        if(render != null)
        {
            render.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

}
