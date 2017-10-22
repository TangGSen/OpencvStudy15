package sen.com.opencvstudy15;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

public class NarActivity extends AppCompatActivity {



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nar_choose);
    }


    public void face(View view){
        Intent intent = new Intent(this,FaceActivity.class);
        startActivity(intent);
        overridePendingTransition(0,0);
    }

    public void idcard(View view){
        Intent intent = new Intent(this,IDCardActivity.class);
        startActivity(intent);
        overridePendingTransition(0,0);
    }
}
