import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

buf_sizes = [10, 25, 50, 75, 100, 125, 150, 175, 200]


def get_buffy_df(tc, buf_size):
    df = pd.read_csv(f"{tc}.{buf_size}.txt")
    df['test_case'] = tc
    df['model'] = 'buffy'
    df.columns = df.columns.str.strip()
    return df


def get_fperf_df(tc, buf_size):
    rows = []
    with open(f"../wls/{tc}.{buf_size}.txt", "r") as file:
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

    return pd.DataFrame(rows)


dfs = []
for tc in ["rr"]:
    for i in buf_sizes:
        dfs.append(get_buffy_df(tc, i))
        dfs.append(get_fperf_df(tc, i))

df = pd.concat(dfs)
df = df[['test_case', 'model', 'buf_size', 'time_millis']]

print(df.head())

plt.figure(figsize=(10, 6))

sns.lineplot(data=df, x='buf_size', y='time_millis', hue='model', estimator="mean")

mean_df = df.groupby(['test_case', 'buf_size', 'model'])['time_millis'].mean().reset_index()
p95_df = df.groupby(['test_case', 'buf_size', 'model'])['time_millis'].quantile(0.99).reset_index()
p95_df.rename(columns={'time_millis': 'time_millis_p99'}, inplace=True)
merged = pd.merge(mean_df, p95_df, on=['test_case', 'buf_size', 'model'])

melted = pd.melt(
    merged,
    id_vars=['test_case', 'buf_size', 'model'],
    value_vars=['time_millis', 'time_millis_p99'],
    var_name='stat',
    value_name='value'
)

melted['stat'] = melted['stat'].map({
    'time_millis': 'mean',
    'time_millis_p99': '99th percentile'
})

# Plot with seaborn
plt.figure(figsize=(10, 6))
sns.lineplot(
    data=melted,
    x='buf_size',
    y='value',
    hue='model',
    style='stat',
    markers=True,
    dashes=True
)

plt.title("Test Case: Round-Robin")

plt.xlabel("Buffer Size")
plt.ylabel("Verification Time (ms)")
plt.grid(True)
plt.tight_layout()
plt.legend(title='Model / Stat')
plt.show()
