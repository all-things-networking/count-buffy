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
for tc in ["prio"]:
    for i in buf_sizes:
        dfs.append(get_buffy_df(tc, i))
        dfs.append(get_fperf_df(tc, i))

df = pd.concat(dfs)
df = df[['test_case', 'model', 'buf_size', 'time_millis']]

print(df.head())

plt.figure(figsize=(10, 6))

# sns.lineplot(data=df, x='buf_size', y='time_millis', hue='model', estimator="mean",
#              errorbar=('ci', 99), err_style='bars')
# Compute mean and 99th percentile manually
mean_df = df.groupby(['buf_size', 'model'])['time_millis'].mean().reset_index()
p99_df = df.groupby(['buf_size', 'model'])['time_millis'].quantile(0.99).reset_index()
p99_df.rename(columns={'time_millis': 'time_millis_p99'}, inplace=True)
merged = pd.merge(mean_df, p99_df, on=['buf_size', 'model'])
merged['error'] = merged['time_millis_p99'] - merged['time_millis']

# for model in merged['model'].unique():
#     subset = merged[merged['model'] == model]
#     plt.errorbar(
#         subset['buf_size'],
#         subset['time_millis'],
#         yerr=subset['error'],
#         label=f'{model} (mean, 99th percentile)',
#         fmt='-o'
#     )
for model in merged['model'].unique():
    subset = merged[merged['model'] == model]
    upper_error = subset['error']
    lower_error = [0] * len(subset)  # no lower error

    plt.errorbar(
        subset['buf_size'],
        subset['time_millis'],
        yerr=[lower_error, upper_error],  # asymmetric error
        fmt='-o',
        label=f'{model} (mean, 99th percentile)',
        capsize=5
    )
plt.title("Test-Case: Prio")
plt.legend()
plt.show()
