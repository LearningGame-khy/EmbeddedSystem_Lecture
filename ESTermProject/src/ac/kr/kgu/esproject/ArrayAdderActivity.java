package ac.kr.kgu.esproject;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.LinearLayout;

import java.util.Random;

public class ArrayAdderActivity extends Activity {
    /** Called when the activity is first created. */
	protected static final int THREAD_0_WAIT = 0;			// 대기모드
	protected static final int THREAD_1_CORRECT = 1;		// 입력값이 정답과 일치
	protected static final int THREAD_2_NOT_CORRECT = 2;	// 입력값이 정답과 일치하지 않음.
	
	BackThread bThread = new BackThread();
	int flag = THREAD_0_WAIT;	// 대기모드 시작
	boolean stop = false;		// Thread 작동제어
	
	Button On;
	LinearLayout LL;
	EditText inputValue;
	TextView TV;
	Button Send;
	Button Clear;
	Spinner mySpinner;
	TextView Answer;
	
	int sum;		// 합계
	int input;		// 사용자 입력
	int arraySize;	// 배열 크기
	int array[];	// 가변배열
		
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        System.loadLibrary("projni");
        
        On = (Button)findViewById(R.id.createarray);
        LL = (LinearLayout)findViewById(R.id.linearLayout1);
        inputValue =(EditText)findViewById(R.id.editText1);
        TV = (TextView)findViewById(R.id.textView2);
        Send = (Button)findViewById(R.id.sendValue);
        Clear = (Button)findViewById(R.id.clear);
        mySpinner = (Spinner)findViewById(R.id.spinner1);
        Answer = (TextView)findViewById(R.id.Answer);
        
        bThread.setDaemon(true);
        bThread.start();
        
        On.setOnClickListener(new Button.OnClickListener(){
        	public void onClick(View v){
        		arraySize = Integer.parseInt(mySpinner.getSelectedItem().toString());
        		array = new int[arraySize];
        		Random random = new Random();
        		sum = 0;
        		
        		for(int i=0; i<arraySize; i++){
        			TextView textArray = new TextView(ArrayAdderActivity.this);
        			array[i] = random.nextInt(10);
        			sum += array[i];
        			String s = String.format("배열요소 #%d : %d",i+1,array[i]);
         			textArray.setText(s);
         			LL.addView(textArray);
        		}
        		TV.setVisibility(View.VISIBLE);
        		inputValue.setVisibility(View.VISIBLE);
        		Send.setVisibility(View.VISIBLE);
        		Clear.setVisibility(View.VISIBLE);        		
        	}
        });
        
        Send.setOnClickListener(new Button.OnClickListener(){
        	public void onClick(View v){
        		input = Integer.parseInt(inputValue.getText().toString());
        		
        		int result = ArraySum(input, array);
        		if (result == 0) {
        			flag = THREAD_1_CORRECT;
        			Answer.setText("정확합니다.");
        		}
        		else {
        			flag = THREAD_2_NOT_CORRECT;
        			Answer.setText("틀렸습니다.");
        		}
        		Answer.setVisibility(View.VISIBLE);
        	}
        });
        
        Clear.setOnClickListener(new Button.OnClickListener(){
        	public void onClick(View v){
        		flag = THREAD_0_WAIT;
        		arraySize = 0;
        		sum = 0;
        		
        		LL.removeAllViews();
        		TV.setVisibility(View.GONE);
        		inputValue.setVisibility(View.GONE);
        		Send.setVisibility(View.GONE);
        		Clear.setVisibility(View.GONE);
        		Answer.setVisibility(View.GONE);
        	}
        });
    }
    
    class BackThread extends Thread {
    	public void run() {
    		while (!stop) {
    			switch (flag) {
	    			case THREAD_0_WAIT:
	    				BuzzerControl(0);
	    				SegmentIOControl(0);
	    				
	    				while (flag == THREAD_0_WAIT) {
	    					for (int i=1; i<15 && flag == THREAD_0_WAIT; i++) {
	    						for (int j=0; j<14; j++) {
	    							SegmentControl(i);
	    						}
	    					}
	    				}
	    				break;
	    				
	    			case THREAD_1_CORRECT:
	    				while (flag == THREAD_1_CORRECT) {
	    					for (int i=0; i<3; i++) {
	    						SegmentIOControl(0);
	    						SegmentControl(0);
	    						BuzzerControl(1);
	    					}
	    					for (int i=0, mod=1; i<6; i++) {
	    						BuzzerControl(0);
	    						SegmentIOControl(mod);
	    						for (int j=0; j<20; j++) {
	    							SegmentControl(sum);
	    						}
	    						mod = 106 - i;
	    					}
	    					for (int i=0; i<3; i++) {
	    						SegmentIOControl(0);
	    						SegmentControl(0);
	    					}
	    				}
	    				break;
	    				
	    			case THREAD_2_NOT_CORRECT:
	    				int result = input*100 + sum;
	    				BuzzerControl(1);
	    				SegmentIOControl(0);
	    				while (flag == THREAD_2_NOT_CORRECT) {
	    					for (int i=0, mod=5; i<6; i++) {
	    						SegmentIOControl(mod);
	    						for (int j=0; j<20; j++) {
	    							SegmentControl(result);
	    						}
	    						mod = 206 - i;
	    					}
	    				}
	    				break;
	    				
    				default: 
    					break;
    			}
    		}
    	}
    }
    
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	if (keyCode == KeyEvent.KEYCODE_BACK) {
    		flag = -1;
    		stop = true;
    		bThread.interrupt();
    	}
    	
    	return super.onKeyDown(keyCode, event);
    }
    
    public native int SegmentControl(int value);
    public native int SegmentIOControl(int value);
    public native int ArraySum(int input, int[] arr);
    public native int BuzzerControl(int value);
}