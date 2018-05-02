#!/usr/bin/env python3
"""Downloads the MSVC libs bundle and extracts it to libs/"""

# configuration

bundle_name = "rwe-msvc-libs-3.zip"
sha256hash_hex = "c7d65c78a7e693a8c5471f3c31055c13310a9f407c834bee46ad993838d1ff59"


# code

import binascii
import hashlib
import os
import shutil
import sys
import urllib.request
from zipfile import ZipFile


def extract_file(filename):
    with ZipFile(filename) as zipobj:
        zipobj.extractall()

def download_file(url, filename):
    with urllib.request.urlopen(url) as response, open(filename, 'wb') as fh:
        shutil.copyfileobj(response, fh)

def check_hash(filename, expected_hash):
    actual_hash = get_hash(filename)
    return expected_hash == actual_hash

def get_hash(filename):
    h = hashlib.sha256()
    with open(filename, 'rb') as f:
        while True:
            buf = f.read(65536)
            if not buf:
                break
            h.update(buf)
    return h.digest()

def log(msg):
    print(msg, file=sys.stderr)
    sys.stderr.flush()


if __name__ == "__main__":
    os.chdir("libs")

    sha256hash = binascii.unhexlify(sha256hash_hex)
    bundle_url = "https://rwe.michaelheasell.com/bundles/" + bundle_name

    log("Deleting old files...")
    try:
        os.remove(bundle_name)
    except FileNotFoundError:
        pass

    try:
        shutil.rmtree("_msvc")
    except FileNotFoundError:
        pass

    log("Downloading library bundle...")
    download_file(bundle_url, bundle_name)

    log("Verifying file hash...")
    if not check_hash(bundle_name, sha256hash):
        log("Downloaded file hash did not match", file=sys.stderr)
        sys.exit(1)

    log("Extracting bundle...")
    extract_file(bundle_name)

    log("Deleting bundle file...")
    os.remove(bundle_name)

    log("Done!")
