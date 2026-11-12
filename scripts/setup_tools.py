import urllib.request
import zipfile
import subprocess
import os
import sys

def download_file(url, dest_path, desc):
    print(f"Downloading {desc} from {url}...")
    headers = {'User-Agent': 'Mozilla/5.0'}
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req) as response, open(dest_path, 'wb') as out_file:
        total = response.length or 0
        downloaded = 0
        chunk_size = 1024 * 512
        while True:
            chunk = response.read(chunk_size)
            if not chunk:
                break
            out_file.write(chunk)
            downloaded += len(chunk)
            if total > 0:
                percent = (downloaded / total) * 100
                print(f"\r  {downloaded / (1024*1024):.1f} MB / {total / (1024*1024):.1f} MB ({percent:.1f}%)", end='', flush=True)
    print(f"\n{desc} download complete!\n")

def main():
    tools_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".tools"))
    os.makedirs(tools_dir, exist_ok=True)

    w64_bin = os.path.join(tools_dir, "w64devkit", "bin", "g++.exe")
    if not os.path.exists(w64_bin):
        w64_url = "https://github.com/skeeto/w64devkit/releases/download/v2.11.0/w64devkit-x64-2.11.0.7z.exe"
        exe_path = os.path.join(tools_dir, "w64devkit-sfx.exe")
        download_file(w64_url, exe_path, "w64devkit GCC Toolchain")
        print("Extracting w64devkit...")
        subprocess.run([exe_path, "-y"], cwd=tools_dir, check=True)
        if os.path.exists(exe_path):
            os.remove(exe_path)
        print("w64devkit extraction complete!")
    else:
        print("w64devkit already present.")

    # Check CMake
    cmake_bin = None
    for root, dirs, files in os.walk(tools_dir):
        if "cmake.exe" in files:
            cmake_bin = os.path.join(root, "cmake.exe")
            break

    if not cmake_bin:
        cmake_url = "https://github.com/Kitware/CMake/releases/download/v4.4.5/cmake-4.4.5-windows-x86_64.zip"
        zip_path = os.path.join(tools_dir, "cmake.zip")
        download_file(cmake_url, zip_path, "CMake Windows x64")
        print("Extracting CMake...")
        with zipfile.ZipFile(zip_path, 'r') as zf:
            zf.extractall(tools_dir)
        if os.path.exists(zip_path):
            os.remove(zip_path)
        print("CMake extraction complete!")
    else:
        print("CMake already present.")

    print("\nVerifying tools:")
    if os.path.exists(w64_bin):
        print(f"GCC found: {w64_bin}")
    for root, dirs, files in os.walk(tools_dir):
        if "cmake.exe" in files:
            print(f"CMake found: {os.path.join(root, 'cmake.exe')}")
            break

if __name__ == "__main__":
    main()
