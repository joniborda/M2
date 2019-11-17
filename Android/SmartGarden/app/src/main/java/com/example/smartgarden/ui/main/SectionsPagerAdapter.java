package com.example.smartgarden.ui.main;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;

import com.example.smartgarden.R;

import java.util.ArrayList;

/**
 * A [FragmentPagerAdapter] that returns a fragment corresponding to
 * one of the sections/tabs/pages.
 */
public class SectionsPagerAdapter extends FragmentPagerAdapter {

    @StringRes
    private static final int[] TAB_TITLES = new int[] {R.string.tab_home, R.string.tab_configuracion, R.string.tab_mantenimiento };
    private final Context mContext;
    private TabHomeFragment home;
    private TabConfiguracionFragment config;
    private TabMantenimientoFragment mantenimiento;

    public SectionsPagerAdapter(Context context, FragmentManager fm) {
        super(fm);
        mContext = context;
        home = new TabHomeFragment();
        config = new TabConfiguracionFragment();
        mantenimiento = new TabMantenimientoFragment();
    }

    @NonNull
    @Override
    public Fragment getItem(int position) {
        Fragment fragment;
        switch (position) {
            case 0:
                fragment = home;
                break;
            case 1:
                fragment = config;
                break;
            case 2:
                fragment = mantenimiento;
                break;
            default:
                fragment = new Fragment();
                break;
        }
        return fragment;
    }

    @Nullable
    @Override
    public CharSequence getPageTitle(int position) {
        return mContext.getResources().getString(TAB_TITLES[position]);
    }

    @Override
    public int getCount() {
        return TAB_TITLES.length;
    }

    public ArrayList<IFragment> getItems() {
        ArrayList<IFragment> fragments = new ArrayList<>();
        fragments.add(home);
        fragments.add(config);
        fragments.add(mantenimiento);
        return fragments;
    }

    public Fragment home() {
        return home;
    }

    public Fragment configuracion() {
        return config;
    }

    public Fragment mantenimiento() {
        return mantenimiento;
    }

}