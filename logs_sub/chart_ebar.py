import os.path

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# buf_sizes = [10, 50, 100, 150, 200, 250, 300, 350, 400, 450]
# buf_sizes = [10, 25, 50, 75, 100, 125, 150, 175, 200]
buf_sizes = [10, 15, 25, 30, 35, 40, 45]
# buf_sizes = [10, 15, 25, 30, 35, 40, 45, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900]


def get_buffy_df(tc, buf_size):
    df = pd.read_csv(f"{tc}.{buf_size}.txt")
    df['test_case'] = tc
    df['model'] = 'buffy'
    df.columns = df.columns.str.strip()
    return df


def get_fperf_df(tc, buf_size):
    rows = []
    fperf_log_path = f"../wls/{tc}.{buf_size}.txt"
    if os.path.exists(fperf_log_path):
        with open(fperf_log_path, "r") as file:
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


def gen_chart(tc):
    dfs = []
    for i in buf_sizes:
        dfs.append(get_buffy_df(tc, i))
        dfs.append(get_fperf_df(tc, i))

    df = pd.concat(dfs)
    df = df[['test_case', 'model', 'buf_size', 'time_millis']]
    df['seconds'] = df['time_millis'] / 1000

    print(df.head())

    plt.figure(figsize=(10, 6))

    def draw_metric(metric):
        mean_df = df.groupby(['buf_size', 'model'])[metric].mean().reset_index()
        p99_df = df.groupby(['buf_size', 'model'])[metric].quantile(0.99).reset_index()
        p99_df.rename(columns={metric: f"{metric}_p99"}, inplace=True)
        merged = pd.merge(mean_df, p99_df, on=['buf_size', 'model'])
        merged['error'] = merged[f'{metric}_p99'] - merged[metric]
        for model in merged['model'].unique():
            subset = merged[merged['model'] == model]
            upper_error = subset['error']
            lower_error = [0] * len(subset)
            plt.errorbar(
                subset['buf_size'],
                subset[metric],
                yerr=[lower_error, upper_error],  # asymmetric error
                fmt='-o',
                label=f'{model} (mean, 99th percentile)',
                capsize=5
            )
        plt.title(f"Test-Case: {tc}")
        plt.ylabel(metric)
        plt.legend()
        plt.show()

    draw_metric("seconds")


if __name__ == '__main__':
    gen_chart("loom.mem")
    # gen_chart("rr")
