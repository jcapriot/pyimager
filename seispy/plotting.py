import matplotlib.pyplot as plt
import numpy as np

def wiggle(data, ax=None, color='k'):

    if ax is None:
        plt.figure()
        ax = plt.gca()
        ax.invert_yaxis()

    trace_plots = []
    line_plots = []
    for i, trace in enumerate(data):
        n1 = trace.ns
        dt = trace.dt / 1_000_000
        s_locs = np.linspace(0, n1*dt, n1, endpoint=False)
        dat = np.asarray(trace)

        fill = ax.fill_betweenx(s_locs, dat+i, i, where=dat>0, interpolate=True, color=color)
        line = ax.plot(dat+i, s_locs, color=color)
        trace_plots.append(fill)
        line_plots.append(line)
    return trace_plots, line_plots

def image(data, ax=None, cmap='seismic', **kwargs):

    if ax is None:
        plt.figure()
        ax = plt.gca()
        ax.invert_yaxis()

    im_dat = np.array([trace for trace in data])
    if 'clim' not in kwargs or 'vmin' not in kwargs or 'vmax' not in kwargs:
        max_v = np.abs(im_dat).max()
        if 'vmin' not in kwargs:
            kwargs['vmin'] = -max_v
        if 'vmax' not in kwargs:
            kwargs['vmax'] = max_v

    return ax.pcolormesh(im_dat, cmap=cmap, **kwargs)