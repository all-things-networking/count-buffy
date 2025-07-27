import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

buf_sizes = [10, 50, 100, 150, 200, 250, 300, 350, 400, 450]


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
plt.figure(figsize=(10, 6))
mean_df = df.groupby(['buf_size', 'model'])['time_millis'].mean().reset_index()

# # Draw grouped boxplots (by buf_size and model)
# sns.boxplot(data=df, x='buf_size', y='time_millis', hue='model', showfliers=False, dodge=False)

custom_errors = {
    'buffy': 500,
    'fperf': 800
}
for model in mean_df['model'].unique():
    subset = mean_df[mean_df['model'] == model]
    error_val = custom_errors.get(model, 0)
    plt.errorbar(
        subset['buf_size'],
        subset['time_millis'],
        yerr=[error_val] * len(subset),
        fmt='o',
        capsize=5,
        label=f'{model} (±{error_val})'
    )

sns.pointplot(data=df, x='buf_size', y='time_millis', hue='model',
              estimator='mean', errorbar=None)


plt.legend()
plt.xlabel("Buffer Size")
plt.ylabel("Time (ms)")
plt.tight_layout()
plt.show()
