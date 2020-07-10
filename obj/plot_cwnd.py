import pandas as pd
import matplotlib.pyplot as plt

my_data = pd.read_csv('cwnd.csv',index_col=[0],dayfirst=True)

my_data.plot()

plt.show()
