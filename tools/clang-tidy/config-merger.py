import yaml
import sys
from pathlib import Path

def parse_checks(raw):
    if not raw:
        return []
    return [line.strip() for line in raw.strip().split(',') if line.strip()]

class FoldedStr(str): pass

def folded_str_representer(dumper, data):
    return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='>')

class CleanDumper(yaml.SafeDumper):
    def ignore_aliases(self, data):
        return True

yaml.add_representer(FoldedStr, folded_str_representer, Dumper=CleanDumper)

def merge_check_options(existing, new):
    option_map = {opt['key']: opt['value'] for opt in existing}
    for opt in new:
        option_map[opt['key']] = opt['value']
    return [{'key': k, 'value': v} for k, v in sorted(option_map.items())]

def merge_files(directory_path, file_basenames):
    merged_data = {}
    all_checks = []
    all_check_options = []

    for base in file_basenames:
        file_path = Path(directory_path) / f"{base}.clang-tidy"
        if not file_path.exists():
            print(f"⚠️  Warning: {file_path} does not exist. Skipping.", file=sys.stderr)
            continue

        data = yaml.safe_load(file_path.read_text()) or {}

        for key, value in data.items():
            if key == "Checks":
                all_checks += parse_checks(value)
            elif key == "CheckOptions":
                if isinstance(value, list):
                    all_check_options = merge_check_options(all_check_options, value)
            else:
                merged_data[key] = value

    # Final merged Checks
    merged_data["Checks"] = FoldedStr(",\n  ".join(dict.fromkeys(all_checks)) + ",")

    if all_check_options:
        merged_data["CheckOptions"] = all_check_options

    return merged_data

def main():
    if len(sys.argv) < 4:
        print("Usage: python merge_clang_tidy_multi.py <directory_path> <file1 file2 ...> <output_name>", file=sys.stderr)
        sys.exit(1)

    directory = sys.argv[1]
    *file_basenames, output_basename = sys.argv[2:]

    merged = merge_files(directory, file_basenames)
    output_path = Path(f"{directory}/{output_basename}.clang-tidy")

    output_text = yaml.dump(merged, Dumper=CleanDumper, sort_keys=False, width=1000)
    output_text = output_text.replace("Checks: >-", "Checks: >")  # fix unwanted '>-'

    output_path.write_text(output_text, encoding='utf-8')
    print(f"✅ Merged YAML written to {output_path}")

if __name__ == "__main__":
    main()
