import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

def plot_graphs(csv_file):
    df = pd.read_csv(csv_file)

    grouped = df.groupby('d')

    with PdfPages('graphs.pdf') as pdf:
        for d, group in grouped:
            plt.figure()
            plt.plot(group['t'], group['parallelization_factor'], marker='o', label=f'd={d}')
            
            x_values = group['t']
            plt.plot(x_values, x_values, linestyle='--', color='red', label='f(x) = x')

            plt.xlabel('Number of threads')
            plt.ylabel('Parallelization Factor')
            plt.title(f'd={d}')
            plt.grid(True, which="both", linestyle='--', linewidth=0.5)
            plt.legend()
            plt.tight_layout()
            pdf.savefig()
            plt.close()

plot_graphs('data.csv')