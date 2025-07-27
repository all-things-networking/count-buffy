import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

data = []
for i in range(10, 160, 10):
    with open(f"logs/{i}.stats.log") as f:
        for line in f:
            if ":time" in line:
                parts = line.strip().split()
                if parts[0] == ":time":
                    t = parts[1][:-1]
                    data.append({
                        "time": t,
                        "buffer_size": i
                    })

df = pd.DataFrame(data)

# print(df.head(100))

# df = pd.read_csv('data.csv')  # adjust if using a different format

sns.set(style="whitegrid")
plt.figure(figsize=(8, 5))

sns.lineplot(
    data=df,
    x="buffer_size",
    y="time",
    #     # errorbar="sd",  # standard deviation error bars
    marker="o"
)
# sns.boxplot(
#     data=df,
#     x="buffer_size",
#     y="time"
# )
plt.ylim(0, 2)
plt.title("Time vs Buffer Size")
plt.xlabel("Buffer Size")
plt.ylabel("Time")
plt.tight_layout()
plt.show()
