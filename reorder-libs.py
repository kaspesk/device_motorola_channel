#!/usr/bin/env python
#
# Copyright (C) 2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

from functools import cmp_to_key
from locale import LC_ALL, setlocale, strcoll
from pathlib import Path

# List of files to process
FILES = [Path(file) for file in [
    "proprietary-files.txt",
]]

# Set locale for consistent string comparison
setlocale(LC_ALL, "C")

def strcoll_extract_utils(string1: str, string2: str) -> int:
    """
    Custom comparison function for sorting paths. Handles directories
    and build targets by comparing directory paths separately.
    """
    if not string1 or not string2:
        return strcoll(string1, string2)

    # Remove leading hyphens used to indicate build targets
    string1 = string1.removeprefix('-')
    string2 = string2.removeprefix('-')

    # If no directories in either string, compare normally
    if not "/" in string1 and not "/" in string2:
        return strcoll(string1, string2)

    # Extract directory paths
    string1_dir = string1.rsplit("/", 1)[0] + "/"
    string2_dir = string2.rsplit("/", 1)[0] + "/"

    if string1_dir == string2_dir:
        # Same directory, compare normally
        return strcoll(string1, string2)

    if string1_dir.startswith(string2_dir):
        # First string dir is a subdirectory of the second one
        return -1

    if string2_dir.startswith(string1_dir):
        # Second string dir is a subdirectory of the first one
        return 1

    # Compare normally if directories are unrelated
    return strcoll(string1, string2)

def section_sort_key(section: str) -> str:
    """
    Key function for sorting sections. Uses the section string
    itself as the sort key.
    """
    return section

# Ask the user if they want to sort sections as well
response = input("Would you like to sort the sections alphabetically as well as the files within them? (yes/no): ").strip().lower()
sort_sections = response in ['yes', 'y']

for file in FILES:
    if not file.is_file():
        print(f"File {file} not found")
        continue

    try:
        # Read the file and split into sections
        with open(file, 'r') as f:
            sections = f.read().split("\n\n")

        # Sort files within each section
        ordered_sections = []
        for section in sections:
            # Split the section into lines, remove extra spaces
            # and ensure to only include non-empty lines
            section_list = [line.strip() for line in section.splitlines() if line.strip()]
            
            # Sort the section list
            section_list.sort(key=cmp_to_key(strcoll_extract_utils))
            
            # Append the sorted section to the list of ordered sections
            ordered_sections.append("\n".join(section_list))

        # Sort the sections themselves if the user chose to
        if sort_sections:
            ordered_sections.sort(key=section_sort_key)

        # Write the sorted content back to the file
        with open(file, 'w') as f:
            f.write("\n\n".join(ordered_sections).strip() + "\n")

    except Exception as e:
        print(f"Error processing file {file}: {e}")