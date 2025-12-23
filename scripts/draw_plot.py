import argparse
import os.path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib.ticker import MaxNLocator

plt.rcParams["font.size"] = 24
plt.rcParams["font.weight"] = "bold"
plt.rcParams["lines.linewidth"] = 3
plt.rcParams["legend.fontsize"] = 16
plt.rcParams['legend.title_fontsize'] = 16


class PlotDrawer:
    def __init__(self, wls_dir, logs_dir, save_dir, expr_name, min_buf_size, max_buf_size):
        self.wls_dir = wls_dir
        self.logs_dir = logs_dir
        self.expr_name = expr_name
        self.save_dir = save_dir
        self.min_buf_size = min_buf_size
        self.max_buf_size = max_buf_size

    def __add_buffy_df_win(self, buf_size, dfs):
        p = f"{self.logs_dir}/{self.expr_name}/win/{self.expr_name}.{buf_size}.txt"
        if os.path.exists(p):
            df = pd.read_csv(p)
            df['model'] = 'Count Buffy: Win'
            df.columns = df.columns.str.strip()
            dfs.append(df)

    def __add_buffy_df_no_win(self, buf_size, dfs):
        p = f"{self.logs_dir}/{self.expr_name}/no_win/{self.expr_name}.{buf_size}.txt"
        if os.path.exists(p):
            df = pd.read_csv(p)
            df['model'] = 'Count Buffy: No Win'
            df.columns = df.columns.str.strip()
            dfs.append(df)

    def __add_fperf_df(self, buf_size, dfs):
        rows = []
        p = f"{self.wls_dir}/{self.expr_name}/{self.expr_name}.{buf_size}.txt"
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

    def draw(self):
        dfs = []
        for i in range(self.min_buf_size, self.max_buf_size + 1):
            self.__add_buffy_df_win(i, dfs)
            self.__add_buffy_df_no_win(i, dfs)
            self.__add_fperf_df(i, dfs)

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
        plt.figure(figsize=(1.6 * height, height))
        sns.lineplot(
            x='buf_size',
            y='mean',
            hue='model',
            data=summary,
            markers=True,
        )
        ax = plt.gca()
        ax.yaxis.set_major_locator(MaxNLocator(nbins=5))
        # ax.set_xticks(range(self.min_buf_size, self.max_buf_size, s))
        palette = sns.color_palette()
        for i, (model, group) in enumerate(summary.groupby("model")):
            ax.fill_between(
                group["buf_size"],
                group["mean"],
                group["p95"],
                alpha=0.3,
                color=palette[i]
            )
        leg = plt.legend()
        leg.set_title(None)
        plt.title(f"Experiment: {self.expr_name}")
        # plt.ylim(0, 250)
        plt.ylabel("Verification Time (s)")
        plt.xlabel("Buffer Size")
        plt.tight_layout()
        os.makedirs(self.save_dir, exist_ok=True)
        chart_path = f"{self.save_dir}/{self.expr_name}.png"
        plt.savefig(chart_path, dpi=300, bbox_inches='tight')
        print(f"Saved chart for {self.expr_name} to {chart_path}")
        plt.show()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--save-dir", default="data/plots", help="Plots directory")
    parser.add_argument("-w", "--workloads", required=True, help="Workloads directory")
    parser.add_argument("-l", "--logs", required=True, help="Logs directory")
    parser.add_argument("-n", "--name", required=True, help="Experiment name")
    parser.add_argument("--min", type=int, required=True, help="Min Buffer size")
    parser.add_argument("--max", type=int, required=True, help="Max Buffer size")

    args = parser.parse_args()

    drawer = PlotDrawer(args.workloads, args.logs, args.save_dir, args.name, args.min, args.max)
    drawer.draw()


if __name__ == '__main__':
    main()
