import os.path

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# plt.rcParams["text.usetex"] = True
# plt.rcParams["font.family"] = "serif"
plt.rcParams["font.size"] = 24
plt.rcParams["font.weight"] = "bold"
plt.rcParams["lines.linewidth"] = 3
plt.rcParams["legend.fontsize"] = 14


def add_buffy_df(tc, buf_size, dfs):
    p = f"{tc}.{buf_size}.txt"
    if os.path.exists(p):
        df = pd.read_csv(p)
        df['test_case'] = tc
        df['model'] = 'buffy'
        df.columns = df.columns.str.strip()
        dfs.append(df)


def add_fperf_df(tc, buf_size, dfs):
    rows = []
    p = f"../wls/{tc}.{buf_size}.txt"
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
                        'test_case': tc,
                        "model": "fperf"
                    })
        df = pd.DataFrame(rows)
        dfs.append(df)


dfs = []
for tc in ["prio"]:
    for i in range(1000):
        add_buffy_df(tc, i, dfs)
        add_fperf_df(tc, i, dfs)

df = pd.concat(dfs)
df = df[['test_case', 'model', 'buf_size', 'time_millis']]

sns.lineplot(data=df, x='buf_size', y='time_millis', hue='model', estimator="mean")

mean_df = df.groupby(['test_case', 'buf_size', 'model'])['time_millis'].mean().reset_index()
p95_df = df.groupby(['test_case', 'buf_size', 'model'])['time_millis'].quantile(0.95).reset_index()
p95_df.rename(columns={'time_millis': 'time_millis_p95'}, inplace=True)
merged = pd.merge(mean_df, p95_df, on=['test_case', 'buf_size', 'model'])

melted = pd.melt(
    merged,
    id_vars=['test_case', 'buf_size', 'model'],
    value_vars=['time_millis', 'time_millis_p95'],
    var_name='stat',
    value_name='value'
)

melted['stat'] = melted['stat'].map({
    'time_millis': 'mean',
    'time_millis_p95': '95th percentile'
})

# Plot with seaborn
# plt.figure(figsize=(10, 6))
sns.lineplot(
    data=melted,
    x='buf_size',
    y='value',
    hue='model',
    style='stat',
    markers=True,
    dashes=True
)

plt.title("Test Case: Prio")

plt.xlabel("Buffer Size")
plt.ylabel("Verification Time (ms)")
plt.grid(True)
plt.tight_layout()
plt.legend(title='Model / Stat')
plt.show()
