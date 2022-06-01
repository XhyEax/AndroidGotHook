package com.xhy.androidgothook;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

import com.xhy.nativelib.NativeLib;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView sample = findViewById(R.id.sample_text);
        sample.setText(NativeLib.stringFromJNI());
    }

}