#!/usr/bin/env python3
import numpy as np
import seaborn as sns

import matplotlib.pyplot as plt

# Generate two random datasets
data1 = np.random.normal(loc=0, scale=1, size=1000)
data2 = np.random.normal(loc=2, scale=1.5, size=1000)

# Create a figure
plt.figure(figsize=(8, 6))

# Plot histograms side-by-side
sns.histplot(data1, color='blue', label='Data 1', kde=False, stat='density', bins=30, alpha=0.6)
sns.histplot(data2, color='orange', label='Data 2', kde=False, stat='density', bins=30, alpha=0.6)

plt.legend()
plt.title('Side-by-side Histograms')
plt.xlabel('Value')
plt.ylabel('Density')

# Save to PNG
plt.savefig('histograms.png')
plt.close()