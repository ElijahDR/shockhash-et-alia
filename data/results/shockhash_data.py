import pandas as pd

# Load the CSV file
csv_file = './data/results/shockhash-1000000-3-1.csv'
df = pd.read_csv(csv_file)


grouped = df.groupby(['bucket_size', 'leaf_size']).agg(
    total_bits_avg=('total_bits', 'mean'),
    build_time_avg=('build_time', 'mean'),
    query_time_avg=('query_time', 'mean'),
).reset_index()

# Calculate derived metrics
df['query_time_per_key'] = df['query_time_avg'] / df['num_keys']
df['build_time_per_key'] = df['build_time_avg'] / df['num_keys']
df['bits_per_key'] = df['total_bits_avg'] / df['num_keys']
df['build_throughput'] = df['n_keys'] / (df['build_time_avg'] / 1000000000)
df['query_throughput'] = df['n_keys'] / (df['query_time_avg'] / 1000000000)

# Round to reasonable decimal places
df = df.round({
    'query_time_per_key': 0,
    'build_time_per_key': 0,
    'bits_per_key' : 2,
    'build_throughput' : 0,
    'query_throughput' : 0,
})

# Export to LaTeX table
df.to_csv(csv_file + 'processed_data.csv', index=False)