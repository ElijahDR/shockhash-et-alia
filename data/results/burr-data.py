import pandas as pd

# Load the CSV file
csv_file = 'bur_data.csv'
df = pd.read_csv(csv_file)

# Calculate derived metrics
df['query_time_per_key'] = df['query_time_total'] / df['num_keys']
df['build_time_per_key'] = df['build_time_total'] / df['num_keys']
df['space_overhead'] = df['space_used_bytes'] / (df['num_keys'] * 8)  # assuming 8 bytes per key
df['metadata_percentage_vs_Z'] = (df['space_metadata_bytes'] / df['space_Z_bytes']) * 100

# Round to reasonable decimal places
df = df.round({
    'query_time_per_key': 6,
    'build_time_per_key': 6,
    'space_overhead': 2,
    'metadata_percentage_vs_Z': 2
})

# Export to LaTeX table
latex_table = df.to_latex(index=False, 
                          columns=['dataset', 'num_keys', 'query_time_per_key', 
                                   'build_time_per_key', 'space_overhead', 
                                   'metadata_percentage_vs_Z'],
                          header=['Dataset', '\# Keys', 'Query Time/Key (s)', 
                                  'Build Time/Key (s)', 'Space Overhead (bytes/key)', 
                                  'Metadata \% vs Z'])
print(latex_table)