#!/usr/bin/env python3
import json
import sys
from collections import defaultdict


def pad(s, width, align_left=True, fill='-'):
    """
    Pad string s to exactly width characters using fill character.
    Left-align (append fill) if align_left=True; right-align (prepend fill) otherwise.
    """
    if align_left:
        return s.ljust(width, fill)
    else:
        return s.rjust(width, fill)


def main():
    try:
        # Read report.json
        with open('report.json', 'r') as f:
            report_data = json.load(f)
        
        # Read file_path_kw.json
        with open('file_path_kw.json', 'r') as f:
            pr_files_data = json.load(f)
        
        # Extract PR files (files in the PR)
        pr_files = {item['file'] for item in pr_files_data}
        
        # Initialize counters
        pr_severity_count = defaultdict(int)
        other_severity_count = defaultdict(int)
        
        # Process each problem in the report
        problems = report_data.get('errorList', {}).get('problem', [])
        for problem in problems:
            fn = problem.get('filename')
            sev = problem.get('severity')
            if not fn or not sev:
                continue
            
            if fn in pr_files:
                pr_severity_count[sev] += 1
            else:
                other_severity_count[sev] += 1
        
        # Print header information
        print("Klocwork analysis completed.")
        print()  # Empty line
        print("Annotation Index")
        print("HIGH - [Critical, Error, MISRA Mandatory],")
        print("MEDIUM - [Warning, MISRA Required, MISRA Advisory, HIS Metrics]")
        print("LOW - [CERT C Rule, CERT C Recommendation, Review]")
        print()  # Empty line
        print("PR-Files added or modified")
        print("Others-Other Files")
        print()  # Empty line
        print("Results Summary")
        
        # Define severity order
        severity_order = [
            "Critical", "Error", "Warning", "Review",
            "MISRA Mandatory", "MISRA Required", "MISRA Advisory",
            "HIS METRICS", "CERT C Rule", "CERT C Recommendation"
        ]
        
        # Calculate totals and print in the requested format
        total_pr = total_other = 0
        processed_severities = set()
        
        # Print severities in defined order (only if they exist in the data)
        for sev in severity_order:
            p = pr_severity_count[sev]
            o = other_severity_count[sev]
            t = p + o
            print(f'"{sev}"- [PR={p}, Other={o}, Total={t}]')
            total_pr += p
            total_other += o
            processed_severities.add(sev)
        
        # Print any left-over severities not in the predefined order
        all_severities = set(pr_severity_count.keys()) | set(other_severity_count.keys())
        for sev in sorted(all_severities - processed_severities):
            p = pr_severity_count[sev]
            o = other_severity_count[sev]
            t = p + o
            print(f'"{sev}"-[PR={p}, Other={o}, Total={t}]')
            total_pr += p
            total_other += o
        
        # Print summary row
        print()  # Empty line
        print(f"TOTAL[PR={total_pr}, Other={total_other}, Total={total_pr + total_other}]")
        print()  # Empty line
        print("See annotations for details and click the link below for full HTML report.")
        
    except Exception as e:
        sys.stderr.write(f"ERROR generating table: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    main()