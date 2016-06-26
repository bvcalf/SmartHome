package com.example.lf.smarthome;

import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.zhy.http.okhttp.OkHttpUtils;
import com.zhy.http.okhttp.callback.StringCallback;

import org.json.JSONException;
import org.json.JSONObject;

import okhttp3.Call;
import okhttp3.Request;

public class MainActivity extends AppCompatActivity {
    TextView tvTemp,tvHumi,tvLight;
    EditText etKongtiao,etChuanglian,etSetip;
    Button btnKongtiao,btnChuanglian,btnSetip;
    String ip = "";
    //int i=0;
    String url = "";
    Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTemp = (TextView) findViewById(R.id.tv_temp);
        tvHumi = (TextView) findViewById(R.id.tv_humi);
        tvLight = (TextView) findViewById(R.id.tv_light);
        etKongtiao = (EditText) findViewById(R.id.et_kongtiao);
        etChuanglian = (EditText) findViewById(R.id.et_chuanglian);
        etSetip = (EditText) findViewById(R.id.et_setip);
        btnKongtiao = (Button) findViewById(R.id.btn_kongtiao);
        btnChuanglian = (Button) findViewById(R.id.btn_chuanglian);
        btnSetip = (Button) findViewById(R.id.btn_setip);

        btnKongtiao.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(MainActivity.this, "你点击了空调设置", Toast.LENGTH_SHORT).show();
                //设置空调
                String cmdurl = "http://"+ip+":5000/"+"kongtiao("+etKongtiao.getText().toString()+")";
                OkHttpUtils
                        .get()
                        .url(cmdurl)
                        .build()
                        .execute(new StringCallback()
                        {
                            @Override
                            public void onError(Call call, Exception e , int id)
                            {
                                e.printStackTrace();
                            }

                            @Override
                            public void onResponse(String response,int id)
                            {
                                try {
                                    JSONObject jsonobj = new JSONObject(response);
                                    if (jsonobj.getString("result").toString().equals("ok")){
                                        Toast.makeText(MainActivity.this, "空调设置成功", Toast.LENGTH_SHORT).show();
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                        });
            }
        });
        btnChuanglian.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //设置窗帘
                Toast.makeText(MainActivity.this, "你点击了窗帘设置", Toast.LENGTH_SHORT).show();
                String cmdurl = "http://"+ip+":5000/"+"chuanglian("+etChuanglian.getText().toString()+")";
                OkHttpUtils
                        .get()
                        .url(cmdurl)
                        .build()
                        .execute(new StringCallback()
                        {
                            @Override
                            public void onError(Call call, Exception e , int id)
                            {
                                e.printStackTrace();
                            }

                            @Override
                            public void onResponse(String response,int id)
                            {
                                try {
                                    JSONObject jsonobj = new JSONObject(response);
                                    if (jsonobj.getString("result").toString().equals("ok")){
                                        Toast.makeText(MainActivity.this, "窗帘设置成功", Toast.LENGTH_SHORT).show();
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                        });
            }
        });
        btnSetip.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //设置ip
                Toast.makeText(MainActivity.this, "你点击了ip设置", Toast.LENGTH_SHORT).show();
                ip=etSetip.getText().toString();
                url = "http://"+ip+":5000/get";
                //sendGet();
                handler.postDelayed(runnable,1000);//先执行一次
            }
        });
    }
    Runnable runnable = new Runnable() {
        @Override
        public void run() {
            System.out.println("111111111111111111111111111111111111111111111111111");
            sendGet();
            handler.postDelayed(runnable,1000);//每次调自己
            System.out.println("222222222222222222222222222222222222222222222222222");
        }
    };
    void sendGet(){
            OkHttpUtils
                    .get()
                    .url(url)
                    .build()
                    .execute(new StringCallback()
                    {
                        @Override
                        public void onError(Call call, Exception e , int id)
                        {
                            e.printStackTrace();
                            tvTemp.setText("onError:" + e.getMessage());
                        }

                        @Override
                        public void onResponse(String response,int id)
                        {
                            //tvTemp.setText(response+ i);
                            try {
                                JSONObject jsonobj = new JSONObject(response);
                                tvTemp.setText(jsonobj.getInt("temp")+"");
                                tvHumi.setText(jsonobj.getInt("humi")+"");
                                tvLight.setText(jsonobj.getInt("light")+"");
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                            //tvTemp.setText(response);
                        }
                    });
            //i++;
    }
}
