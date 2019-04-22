package com.islammohamed.minijarvis;

import android.content.Intent;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.firestore.DocumentSnapshot;
import com.google.firebase.firestore.EventListener;
import com.google.firebase.firestore.FirebaseFirestore;
import com.google.firebase.firestore.FirebaseFirestoreException;
import com.google.firebase.firestore.ListenerRegistration;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class MainActivity extends AppCompatActivity {

    private SeekBar mLightingSeekBar;
    private Switch mDoorSwitch;
    private Switch mHangingsSwitch;
    private TextView mWeatherTextView;
    private Map<String, Object> settings;


    FirebaseFirestore db;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



        db = FirebaseFirestore.getInstance();

        mLightingSeekBar = findViewById(R.id.lightingSeekBar);
        mDoorSwitch = findViewById(R.id.doorSwitch);
        mHangingsSwitch = findViewById(R.id.hangingsSwitch);
        mWeatherTextView = findViewById(R.id.weatherTextView);

        getSettings();

        mLightingSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int progress = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progresValue, boolean fromUser) {
                progress = progresValue;
                Toast.makeText(getApplicationContext(), "Changing seekbar's progress", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                Toast.makeText(getApplicationContext(), "Started tracking seekbar", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                Toast.makeText(getApplicationContext(), "Stopped tracking seekbar", Toast.LENGTH_SHORT).show();
                updateLighting(progress);
            }
        });



    }

    public void updateLighting(int value) {
        try {
            HashMap<String, Integer> newValues = new HashMap<String, Integer>();
            newValues.put("lighting", value);
            db.collection("settings")
                    .document("currentSettings")
                    .update("lighting", value)
                    .addOnCompleteListener(new OnCompleteListener<Void>() {
                        @Override
                        public void onComplete(@NonNull Task<Void> task) {
                            Toast.makeText(getApplicationContext(), "Updated the database successfully!", Toast.LENGTH_SHORT).show();
                        }
                    });
        } catch (Exception e) {
            Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    public void getSettings() {
        try {

            db.collection("settings").document("currentSettings")
                    .get().addOnCompleteListener(new OnCompleteListener<DocumentSnapshot>() {
                @Override
                public void onComplete(@NonNull Task<DocumentSnapshot> task) {
                    if (task.isSuccessful()) {
                        Map<String, Object> data = task.getResult().getData();
                        settings = data;
                        String weatherText = "Weather: " + data.get("weather").toString();
                        mWeatherTextView.setText(weatherText);
                        int progress = Integer.parseInt(settings.get("lighting").toString());
                        mLightingSeekBar.setProgress(progress);
                        boolean doorState = Boolean.parseBoolean(settings.get("doorOpenState").toString());
                        boolean hangingsState = Boolean.parseBoolean(settings.get("hangingsOpenState").toString());
                        mDoorSwitch.setChecked(doorState);
                        mHangingsSwitch.setChecked(hangingsState);
                    }
                }
            });

            db.collection("settings").document("currentSettings").addSnapshotListener(new EventListener<DocumentSnapshot>() {
                @Override
                public void onEvent(@Nullable DocumentSnapshot snapshot,
                                    @Nullable FirebaseFirestoreException e) {
                    if (e != null) {
                        Log.w("Firebase", "Listen failed.", e);
                        return;
                    }

                    String source = snapshot != null && snapshot.getMetadata().hasPendingWrites()
                            ? "Local" : "Server";

                    if (snapshot != null && snapshot.exists()) {
                        Log.d("Firebase", source + " data: " + snapshot.getData());
                        settings = snapshot.getData();
                    } else {
                        Log.d("Firebase", source + " data: null");
                    }
                }
            });
        } catch (Exception e) {
            Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }



    public void handleDoorToggle(View view) {
        boolean isChecked = mDoorSwitch.isChecked();

        toggleState("doorOpenState", isChecked);
    }


    public void handleHangingsToggle(View view) {
        boolean isChecked = mHangingsSwitch.isChecked();
        toggleState("hangingsOpenState", isChecked);
    }

    public void toggleState(String field, boolean data) {
        try {

            db.collection("settings")
                    .document("currentSettings")
                    .update(field, data).
                    addOnCompleteListener(new OnCompleteListener<Void>() {
                        @Override
                        public void onComplete(@NonNull Task<Void> task) {
                            if (task.isSuccessful()) {
                                Toast.makeText(getApplicationContext(), "Updated the database successfully!", Toast.LENGTH_SHORT).show();
                            } else {
                                Toast.makeText(getApplicationContext(), "Couldn't update the database!", Toast.LENGTH_SHORT).show();
                            }
                        }
                    });
        } catch (Exception e) {
            Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }
}
