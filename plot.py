import matplotlib.pyplot as plt
import csv

workload = []
tat = []
wt = []
with open("C1_FCFS.csv", "r") as csvfile:
    plots = csv.reader(csvfile, delimiter=",")
    for row in plots:
        workload.append(row[0])
        tat.append(float(row[1]))
        wt.append(float(row[2]))
x = list(range(0, 1000 * len(workload), 1000))
xt = list(i for i in x if i % 100000 == 0)
plt.plot(x, tat)
plt.plot(x, wt)
plt.xlabel("Workload")
plt.ylabel("Time (s)")
plt.legend(["Turn Around Time", "Waiting Time"])
plt.title("Analysis of the scheduling algorithms")
plt.xticks(xt)
plt.show()
