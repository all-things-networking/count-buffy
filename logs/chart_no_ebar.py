import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from logs.chart_ebar import get_buffy_df, get_fperf_df

# buf_sizes = [10, 50, 100, 150, 200, 250, 300, 350, 400, 450]
buf_sizes = [10, 25, 50, 75, 100, 125, 150, 175, 200]
# buf_sizes = [10, 15, 25, 30, 35, 40, 45, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900]


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

    sns.lineplot(data=df, x='buf_size', y='seconds', hue='model', estimator="mean", marker="o", errorbar=None)
    plt.title(f"Test-Case: {tc}")
    plt.ylabel("seconds")
    plt.legend()
    plt.show()


if __name__ == '__main__':
    # gen_chart("loom.mem")
    gen_chart("rr")
