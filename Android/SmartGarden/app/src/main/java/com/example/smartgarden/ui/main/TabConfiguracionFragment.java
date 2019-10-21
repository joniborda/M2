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
public class TabConfiguracionFragment extends Fragment {


    public TabConfiguracionFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.tab2_configuracion, container, false);
        return root;
    }

}
