#!/usr/bin/env python3
import numpy as np
import seaborn as sns

import matplotlib.pyplot as plt

# Generate two random datasets
data1 = np.random.normal(loc=0, scale=1, size=1000)
data2 = np.random.normal(loc=2, scale=1.5, size=1000)

# Create a figure with two subplots side-by-side
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

# Plot first histogram in left subplot
sns.histplot(data1, color='blue', kde=False, stat='density', bins=30, alpha=0.6, ax=ax1)
ax1.set_title('Data 1')
ax1.set_xlabel('Value')
ax1.set_ylabel('Density')

# Plot second histogram in right subplot
sns.histplot(data2, color='orange', kde=False, stat='density', bins=30, alpha=0.6, ax=ax2)
ax2.set_title('Data 2')
ax2.set_xlabel('Value')
ax2.set_ylabel('Density')

# Add overall title
fig.suptitle('Side-by-side Histograms')

# Save to PNG
plt.savefig('histograms.png')
plt.close()