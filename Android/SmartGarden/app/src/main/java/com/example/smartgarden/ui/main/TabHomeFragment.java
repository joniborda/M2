package com.example.smartgarden.ui.main;


import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.example.smartgarden.R;
import com.example.smartgarden.logic.BTHandler;

import java.util.Observable;
import java.util.Observer;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabHomeFragment extends Fragment implements Observer {

    private LayoutInflater mInflater;
    private ViewGroup mContainer;
    private BTHandler handler;
    private ImageView imageDesconnected;
    private LinearLayout dataLayout;

    public TabHomeFragment(BTHandler handler) {
        this.handler = handler;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mInflater = inflater;
        mContainer = container;

        View v = inflater.inflate(R.layout.tab1_home, container, false);
        imageDesconnected = v.findViewById(R.id.image_state);
        dataLayout = v.findViewById(R.id.data_layout);

        updateView();

        return v;
    }

    @Override
    public void update(Observable observable, Object arg) {
        updateView();
    }

    private void updateView() {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (handler.isConnected) {
                    imageDesconnected.setVisibility(View.INVISIBLE);
                    dataLayout.setVisibility(View.VISIBLE);
                } else {
                    imageDesconnected.setVisibility(View.VISIBLE);
                    dataLayout.setVisibility(View.INVISIBLE);
                }
            }
        });
    }

}
