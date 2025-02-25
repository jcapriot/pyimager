import matplotlib.pyplot as plt
import numpy as np

def wiggle(segy, ax=None, color='k'):

    if ax is None:
        plt.figure()
        ax = plt.gca()
        ax.invert_yaxis()

    trace_plots = []
    line_plots = []
    for i, trace in enumerate(segy):
        n1 = trace.ns
        dt = trace.dt / 1_000_000
        s_locs = np.linspace(0, n1*dt, n1, endpoint=False)
        dat = np.asarray(trace)

        fill = ax.fill_betweenx(s_locs, dat+i, i, where=dat>0, interpolate=True, color=color)
        line = ax.plot(dat+i, s_locs, color=color)
        trace_plots.append(fill)
        line_plots.append(line)
    return trace_plots, line_plots