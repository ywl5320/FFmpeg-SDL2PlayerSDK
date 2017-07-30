package com.ywl5320.wlfm;



import java.io.File;

import com.ywl5320.wlsdk.player.SDLSurface;
import com.ywl5320.wlsdk.player.WlPlayer;
import com.ywl5320.wlsdk.player.WlPlayer.OnCompleteListener;
import com.ywl5320.wlsdk.player.WlPlayer.OnErrorListener;
import com.ywl5320.wlsdk.player.WlPlayer.OnPlayerInfoListener;
import com.ywl5320.wlsdk.player.WlPlayer.OnPlayerPrepard;

import android.accounts.OnAccountsUpdateListener;
import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {

	private SDLSurface surface;
	private Button btnPlay;
	private Button btnPause;
	private Button btnStart;
	private Button btnNext;
	private Button btnStop;
	private Button btnSeek;
	private TextView tvInfo;
	private TextView tvTime;
	private TextView tvLog;
	private String log = "";
	private String urlTitle = "英雄联盟";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
//		
//		requestWindowFeature(Window.FEATURE_NO_TITLE);//隐藏标题
//		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
//		WindowManager.LayoutParams.FLAG_FULLSCREEN);//设置全屏
		
		setContentView(R.layout.activity_main);
		surface = (SDLSurface) findViewById(R.id.surface);
		btnPlay = (Button) findViewById(R.id.btn_play);
		btnPause = (Button) findViewById(R.id.btn_pause);
		tvInfo = (TextView) findViewById(R.id.tv_info);
		btnStart = (Button) findViewById(R.id.btn_start);
		btnNext = (Button) findViewById(R.id.btn_next);
		btnStop = (Button) findViewById(R.id.btn_stop);
		btnSeek = (Button) findViewById(R.id.btn_seek);
		tvTime = (TextView) findViewById(R.id.tv_time);
		tvLog = (TextView) findViewById(R.id.tv_log);
		
		String url = "test.mkv";
		url = "cqz01.mp4";
//		url = "jxtg3.mkv";
		String input = new File(Environment.getExternalStorageDirectory(),url).getAbsolutePath();
//		input = "http://lcache.qingting.fm/cache/20170608/386/386_20170608_000000_020000_24_0.aac";
		input = "http://live.g3proxy.lecloud.com/gslb?stream_id=lb_yxlm_1800&tag=live&ext=m3u8&sign=live_tv&platid=10&splatid=1009";
//		input = "/storage/sdcard1/" + url;
		WlPlayer.initPlayer(this);
		WlPlayer.setDataSource(input);
		//WlPlayer.initSurface(surface);
		WlPlayer.prePard();
		WlPlayer.setPrepardListener(new OnPlayerPrepard() {
			
			@Override
			public void onPrepard() {
				// TODO Auto-generated method stub
				//WlPlayer.wlSeekTo(2650);
				WlPlayer.wlStart();
				handler.sendEmptyMessage(1);
				
			}
		});
		
		WlPlayer.setOnCompleteListener(new OnCompleteListener() {
			
			@Override
			public void onConplete() {
				// TODO Auto-generated method stub
				System.out.println("...........complete..............");
				log = "log:--->\n complete";
				handler.sendEmptyMessage(1);
			}
		});
		
		WlPlayer.setOnErrorListener(new OnErrorListener() {
			
			@Override
			public void onError(int code, String msg) {
				// TODO Auto-generated method stub
				System.out.println("code:" + code + ",msg:" + msg);
				log = "log:--->\n" + msg;
				handler.sendEmptyMessage(1);
			}
		});
		
		handler.postDelayed(runnable, 1000);
		
		WlPlayer.setOnPlayerInfoListener(new OnPlayerInfoListener() {
			
			@Override
			public void onPlay() {
				// TODO Auto-generated method stub
				System.out.println("...........play..............");
				log = "playing";
				handler.sendEmptyMessage(1);
			}
			
			@Override
			public void onLoad() {
				// TODO Auto-generated method stub
				System.out.println("...........load..............");
				log = "loading";
				handler.sendEmptyMessage(1);
			}
		});

		
		btnStart.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				log = "start";
				handler.sendEmptyMessage(1);
				WlPlayer.wlStart();
			}
		});
		
		btnPlay.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				log = "start";
				handler.sendEmptyMessage(1);
				WlPlayer.wlPlay();
			}
		});
		
		btnPause.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				log = "pause";
				handler.sendEmptyMessage(1);
				WlPlayer.wlPause();
			}
		});
		
		btnStop.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				log = "stop";
				handler.sendEmptyMessage(1);
				WlPlayer.release();
				
			}
		});
		
		btnNext.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				urlTitle = "中国之声";
				WlPlayer.next("http://cnvod.cnr.cn/audio2017/live/zgzs/201707/qlglx_20170711021143zgzs_h.m4a");
			}
		});
		
		btnSeek.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				int ret = WlPlayer.wlSeekTo(3600);
				System.out.println("ret:" + ret);
			}
		});
		
	}
	
//	@Override
//	protected void onResume() {
//		// TODO Auto-generated method stub
//		super.onResume();
//		WlPlayer.handleResume();
//	}
	
	
	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		WlPlayer.release();
		super.onBackPressed();
	}
	
	Handler handler = new Handler()
	{
		public void handleMessage(android.os.Message msg) {
			tvInfo.setText("time:" + millisToDateFormat(WlPlayer.wlDuration()));
			tvLog.setText("播放：" + urlTitle + "\n状态：" + log);
		};
	};
	
	Runnable runnable = new Runnable() {
		
		@Override
		public void run() {
			// TODO Auto-generated method stub
			tvTime.setText(millisToDateFormat(WlPlayer.wlNowTime()));
			handler.postDelayed(runnable, 1000);
		}
	};
	
	public String millisToDateFormat(int sends) {
        long hours = sends / (60 * 60);
        long minutes = (sends % (60 * 60)) / (60);
        long seconds = sends % (60);

        String sh = "00";
        if (hours > 0) {
            if (hours < 10) {
                sh = "0" + hours;
            } else {
                sh = hours + "";
            }
        }
        String sm = "00";
        if (minutes > 0) {
            if (minutes < 10) {
                sm = "0" + minutes;
            } else {
                sm = minutes + "";
            }
        }

        String ss = "00";
        if (seconds > 0) {
            if (seconds < 10) {
                ss = "0" + seconds;
            } else {
                ss = seconds + "";
            }
        }
        return sh + ":" + sm + ":" + ss;
    }
}
