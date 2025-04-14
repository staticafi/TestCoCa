import os
import re
import argparse

def rename_test_files(root_dir):
    """
    Recursively rename files ending with 'test_<number>.xml' to 'test_<number>.xml'
    """
    pattern = re.compile(r'.*test_(\d+)\.xml$', re.IGNORECASE)
    
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            # Match files ending with test_<number>.xml
            match = pattern.fullmatch(filename)
            if match:
                # Extract number and build new name
                number = match.group(1)
                new_name = f"test_{number}.xml"
                
                if new_name == filename:
                    continue  # Skip if already correct name
                
                # Build full paths
                src = os.path.join(dirpath, filename)
                dst = os.path.join(dirpath, new_name)
                
                try:
                    os.rename(src, dst)
                    print(f"Renamed: {src} -> {dst}")
                except Exception as e:
                    print(f"Error renaming {src}: {str(e)}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Rename test XML files to standard format",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument("directory", 
                        help="Root directory to search for XML files")
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print(f"Error: {args.directory} is not a valid directory")
        exit(1)

    rename_test_files(args.directory)
