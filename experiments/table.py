import os.path

import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.ticker import StrMethodFormatter, MaxNLocator

TC = "fq"

plt.rcParams["font.size"] = 24
plt.rcParams["font.weight"] = "bold"
plt.rcParams["lines.linewidth"] = 3
plt.rcParams["legend.fontsize"] = 16
plt.rcParams['legend.title_fontsize'] = 16


def add_buffy_df(tc, buf_size, dfs):
    p = f"{tc}/Ours/{tc}.{buf_size}.txt"
    if os.path.exists(p):
        df = pd.read_csv(p)
        df['tc'] = tc
        df['model'] = 'win'
        df.columns = df.columns.str.strip()
        df['idx'] = df.index
        dfs.append(df)


def add_nowin_df(tc, buf_size, dfs):
    p = f"{tc}/NoWin/{tc}.{buf_size}.txt"
    if os.path.exists(p):
        df = pd.read_csv(p)
        df['model'] = 'nowin'
        df['tc'] = tc
        df.columns = df.columns.str.strip()
        df['idx'] = df.index
        dfs.append(df)


def add_fperf_df(tc, buf_size, dfs):
    rows = []
    p = f"{tc}/FPerf/{tc}.{buf_size}.txt"
    if os.path.exists(p):
        with open(p, "r") as file:
            for line in file:
                line = line.strip()
                if not line:
                    continue
                if line.startswith('### - Time:'):
                    parts = line.split()
                    tm = int(parts[3])
                    rows.append({
                        'time_millis': tm,
                        'buf_size': buf_size,
                        "model": "fperf"
                    })
        df = pd.DataFrame(rows)
        df['idx'] = df.index
        df['tc'] = tc
        dfs.append(df)


dfs = []
for tc in ['fq']:
    for i in range(501):
        add_buffy_df(tc, i, dfs)
        add_nowin_df(tc, i, dfs)
        add_fperf_df(tc, i, dfs)

df = pd.concat(dfs)
df['time'] = df['time_millis'] / 1000
df['count'] = 1
df = df[['model', 'buf_size', 'time', 'idx', 'tc', 'count']]

# fperf_df = df[df['model'] == 'fperf'].copy()
# fperf_df['fperf'] = fperf_df['time']

win_df = df[df['model'] == 'win'].copy()
win_df['win'] = win_df['time']

nowin_df = df[df['model'] == 'nowin'].copy()
nowin_df['nowin'] = nowin_df['time']

# merged = fperf_df.merge(nowin_df, on=["tc", "buf_size", "idx"], how="inner").merge(win_df, on=["tc", "buf_size", "idx"],
#                                                                                    how="inner")
merged = pd.merge(win_df, nowin_df, on=["tc", "buf_size", "idx"], how="inner")
# merged = pd.merge(merged, win_df, on=["tc", "buf_size", "idx"], how="inner")

merged = merged[['tc', 'buf_size', 'win', 'nowin']]
merged['diff'] = merged['win'] - merged['nowin']
print(merged['diff'].mean())
g = merged.groupby(['tc', 'buf_size']).mean().reset_index()
print(g['diff'].mean())
# print(g['win'] - g['nowin'])
# print(g[g['tc'] == 'fq'])
# g['per'] = g['better'] / g['count_x']
# g.to_csv("table.csv")
# print(g['per'].mean())
# print(g['better'].sum())
# print(g['count_x'].sum())
