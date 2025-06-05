import glob
import os
import zipfile
import shutil
import requests
import sys
import datetime
from urllib.parse import urljoin

# Configuration
VERSION_URL = "https://kendryte-download.canaan-creative.com/developer/tools/canmv_ide_k230/canmv_ide_resources/version.txt"

COPY_MAP = {
    "ai_cube_examples": "16-AI-Cube",
    "ai_demo": "04-AI-Demo",
    "April-tags": "07-April-Tags",
    "cipher": "03-Cipher",
    "Codes": "08-Codes",
    "Color-Tracking": "09-Color-Tracking",
    "Drawing": "10-Drawing",
    "Feature-Detection": "11-Feature-Detection",
    "Image-Filters": "12-Image-Filters",
    "lvgl": "15-LVGL",
    "machine": "02-Machine",
    "media": "01-Media",
    "Micropython-Basics": "00-Micropython-Basics",
    "nncase_runtime": "05-nncase-Runtime",
    "Snapshot": "13-Snapshot",
    "socket": "14-Socket",
}

def get_current_version():
    """Fetch current version from URL or fallback to local version.txt."""
    try:
        headers = {"User-Agent": "Mozilla/5.0"}
        response = requests.get(VERSION_URL, headers=headers, timeout=5)
        response.raise_for_status()
        return response.text.strip()
    except requests.RequestException:
        # Check in generated folder first
        generated_version_path = os.path.join("generated", "version.txt")
        if os.path.exists(generated_version_path):
            with open(generated_version_path, "r") as f:
                return f.read().strip()
        # Fallback to root version.txt if exists
        if os.path.exists("version.txt"):
            with open("version.txt", "r") as f:
                return f.read().strip()
        raise RuntimeError("Could not fetch version from URL or local files")

def smart_bump_version(version):
    """
    Bump version intelligently:
    - Default: bump patch
    - If patch >= 99: bump minor and reset patch
    - If minor >= 99: bump major and reset minor & patch
    """
    try:
        major, minor, patch = map(int, version.split('.'))
        
        if patch < 99:
            patch += 1
        else:
            patch = 0
            if minor < 99:
                minor += 1
            else:
                minor = 0
                major += 1
        
        return f"{major}.{minor}.{patch}"
    except ValueError:
        raise ValueError(f"Invalid version format: {version} (expected X.Y.Z)")

def write_version_files(version):
    """Write version.txt and resources.txt to generated/ folder in current directory."""
    os.makedirs("generated", exist_ok=True)
    
    # Write version.txt
    with open(os.path.join("generated", "version.txt"), "w") as f:
        f.write(version)

def organize_examples(source_dir):
    """Organize example files according to COPY_MAP."""
    for key in COPY_MAP:
        scripts = glob.glob(f"{source_dir}/{key}/*.py", recursive=True)
        target = f"examples/{COPY_MAP[key]}/"
        
        if os.path.exists(target):
            shutil.rmtree(target)
        os.makedirs(target, exist_ok=True)
        
        for script in scripts:
            shutil.copy2(script, target)

def zip_directory(directory_path, zipf, parent_dir='', all_files=None):
    """Recursively add directory contents to ZIP file."""
    if all_files is None:
        all_files = set()
    
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            arcname = os.path.join(parent_dir, os.path.relpath(file_path, directory_path))
            
            if arcname in all_files:
                continue
                
            zipf.write(file_path, arcname)
            all_files.add(arcname)
        
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            zip_directory(dir_path, zipf, parent_dir, all_files)

def main():
    try:
        # Get and bump version
        current_version = get_current_version()
        new_version = smart_bump_version(current_version)
        print(f"Version bumped: {current_version} → {new_version}")
        
        # Write version files to generated folder in current directory
        write_version_files(new_version)
        
        # Organize examples if source directory provided
        if len(sys.argv) > 1:
            organize_examples(sys.argv[1])
        
        # Create ZIP with new version in generated folder
        os.makedirs("generated", exist_ok=True)
        resource_path = os.path.join("generated", f'canmv-ide-resources-{new_version}.zip')
        print(f"Creating resource package: {resource_path}")
        
        with zipfile.ZipFile(resource_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Use current directory as base for resources
            examples_dir = "examples"
            models_dir = "models"
            
            if os.path.exists(examples_dir):
                zip_directory(examples_dir, zipf, 'examples')
            if os.path.exists(models_dir):
                zip_directory(models_dir, zipf, 'models')
        
        print("Resource package created successfully!")
    
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == '__main__':
    main()
