package com.example.android.bluetoothlegatt;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class RegisterNumActivity extends Activity {
    private SharedPreferences preferences;
    private SharedPreferences.Editor editor;
    private EditText editText;
    private TextView datanumber;
    private Button btn_register;
    private Button btn_remove;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register_num);
        getActionBar().setTitle("Register Number");
        getActionBar().setDisplayHomeAsUpEnabled(true);

//        setListenerRgsRmvbtn();

        editText = findViewById(R.id.number_EditText);
        datanumber = findViewById(R.id.data_number);
        btn_register = findViewById(R.id.btnregister);
        btn_remove = findViewById(R.id.btnremove);
        preferences = PreferenceManager.getDefaultSharedPreferences(this);
        // SharedPreferences 수정을 위한 Editor 객체를 얻어옵니다.
        editor = preferences.edit();
        initializeValue();
    }
    public void onClickRgs(View v){
        editor.putString("text", editText.getText().toString());
        editor.apply();
        initializeValue();
        Toast.makeText(this, "Register success", Toast.LENGTH_SHORT).show();
    }
    public void onClickRmv(View v){
        editor.remove("text");
        editor.apply();
        initializeValue();
        Toast.makeText(this, "Remove success", Toast.LENGTH_SHORT).show();
    }

    public void initializeValue()
    {
        datanumber.setText(preferences.getString("text","Registered not yet"));
        editText.setText(preferences.getString("text",""));
    }
}