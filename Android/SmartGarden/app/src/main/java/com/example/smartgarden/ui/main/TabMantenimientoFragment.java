package com.example.smartgarden.ui.main;


import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.example.smartgarden.R;

/**
 * A simple {@link Fragment} subclass.
 */
public class TabMantenimientoFragment extends Fragment {


    public TabMantenimientoFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.tab3_mantenimiento, container, false);
        return root;
    }

}
