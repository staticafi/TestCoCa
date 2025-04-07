import json
import os
from pathlib import Path

def update_benchmark_args(benchmarks_dir, new_args_file):
    # Load the new args from the specified file
    with open(new_args_file, 'r') as f:
        new_args = json.load(f)

    # Walk through the benchmarks directory
    for root, dirs, files in os.walk(benchmarks_dir):
        # Look for JSON files in each benchmark directory (ignore test-suite files)
        if Path(root).name == 'test-suite':
            continue

        for file in files:
            if file.endswith('.json') and not file.startswith('metadata'):
                json_path = os.path.join(root, file)

                # Load existing JSON data
                with open(json_path, 'r') as f:
                    data = json.load(f)

                # Update args section
                data['args'] = new_args

                # Write back modified JSON
                with open(json_path, 'w') as f:
                    json.dump(data, f, indent=4)
                print(f"Updated {json_path}")

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Update benchmark args from template')
    parser.add_argument('benchmarks_dir', help='Path to benchmarks directory')
    parser.add_argument('new_args_file', help='Path to JSON file with new args')

    args = parser.parse_args()

    update_benchmark_args(args.benchmarks_dir, args.new_args_file)