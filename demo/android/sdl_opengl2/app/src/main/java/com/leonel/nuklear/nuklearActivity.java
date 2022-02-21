package com.leonel.nuklear;

import android.content.res.AssetManager;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class nuklearActivity extends SDLActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setAssetsNativeManager(getAssets());
    }

    public native void setAssetsNativeManager(AssetManager assetManager);
}
