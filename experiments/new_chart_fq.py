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
        df['model'] = 'Ours: With Windows'
        df.columns = df.columns.str.strip()
        dfs.append(df)


def add_nowin_df(tc, buf_size, dfs):
    p = f"{tc}/NoWin/{tc}.{buf_size}.txt"
    if os.path.exists(p):
        df = pd.read_csv(p)
        df['model'] = 'Ours: Without Windows'
        df.columns = df.columns.str.strip()
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
                        "model": "FPerf"
                    })
        df = pd.DataFrame(rows)
        dfs.append(df)


dfs = []
for i in range(501):
    add_buffy_df(TC, i, dfs)
    add_nowin_df(TC, i, dfs)
    add_fperf_df(TC, i, dfs)

df = pd.concat(dfs)
df['time'] = df['time_millis'] / 1000
df = df[['model', 'buf_size', 'time']]

summary = df.groupby(['model', 'buf_size'])['time'].agg(['mean', lambda x: np.percentile(x, 95)])
summary.rename(columns={'<lambda_0>': 'p95'}, inplace=True)
summary = summary.reset_index()
summary = summary.astype({
    'buf_size': int,
    'mean': float,
    'p95': float
})
print(summary)
height = 5.5
plt.figure(figsize=(1.6 * height, height))  # double-column example
# Plot mean line
sns.lineplot(
    x='buf_size',
    y='mean',
    hue='model',
    data=summary,
    markers=True,
)
# plt.gca().yaxis.set_major_formatter(StrMethodFormatter('{x:.0f}k'))
ax = plt.gca()
ax.yaxis.set_major_locator(MaxNLocator(nbins=5))
# ax.set_xticks(list(range(100, 501, 100)))
ax.set_xticks(range(100, 501, 100))
palette = sns.color_palette()
for i, (model, group) in enumerate(summary.groupby("model")):
    ax.fill_between(
        group["buf_size"],
        group["mean"],
        group["p95"],
        alpha=0.3,
        color=palette[i]  # same color as line
    )
# for i,(model, group) in enumerate(summary.groupby("model")):
#     ax.errorbar(
#         group["buf_size"],
#         group["mean"],
#         yerr=[group["mean"] - group["mean"], group["p95"] - group["mean"]],
#         # fmt="none",
#         fmt='-o',
#         color=palette[i],
#         ecolor=palette[i],
#         capsize=4,
#         alpha=0.7
#     )
# plt.fill_between(summary['buf_size'], summary['mean'], summary['p95'], color='red', alpha=0.3, data=summary)
leg = plt.legend()
leg.set_title(None)
# plt.ylim(0, 250)
# plt.ylabel("Verification Time (s)")
plt.ylabel("")
plt.tight_layout()
plt.savefig(f"{TC}.png", dpi=300, bbox_inches='tight')
plt.show()
