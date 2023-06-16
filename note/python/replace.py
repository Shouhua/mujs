#! /usr/bin/env python3

"""
./replace.py 1.3 docker_image
"""

import urllib.request
import sys
import json
import re
import subprocess

def get_version(project_name, major_minor):
    """
    Get new version from parameter major_minor, and raise value exception if version is not exist
    1. get all versions by project name in chef context
    2. filter and get max version by parameter major_minor 
    3. get new version by add 1 with max version

    Args:
        project_name(str): like 'project_name'
        major_minor(str): like '1.1'

    Returns:
        str: New version

    Raises:
        ValueError: Raised when version is not exist
    """
    url = f'https://example.com/packages/{project_name}/versions?offset=0&page_size=100000'
    res = urllib.request.urlopen(url)
    content = res.read().decode('utf-8')
    versions_obj = json.loads(content)['versions']
    versions = [version['version'] for version in versions_obj]
    max_version = '0.0.0'
    for version in versions:
        if version.startswith(major_minor):
            max_version = version
            break

    if max_version == '0.0.0':
        raise ValueError('version is not exist')
    major, minor, patch = max_version.split('.')
    new_version = '.'.join([major, minor, str(int(patch)+1)])
    return new_version

def replace_file(infos):
    """
    Replace chef config file with new content
    """
    for info in infos:
        with open(info['path'], 'r+') as f:
            content = f.read()
            new_content = re.sub(info['re_text'], info['new_content'], content, flags=re.MULTILINE)
            f.seek(0)
            f.truncate()
            f.write(new_content)

if __name__ == '__main__':
    try:
        project_name = 'cbg.sst.sst_fe.diagnosis_web'
        version = get_version(project_name, sys.argv[1])
        obj = [{
                    'path': 'version.yaml',
                    're_text': r'^version:\s*\d+\.\d+.\d+',
                    'new_content': f'version: {version}'
                },
                {
                    'path': 'image.yaml',
                    're_text': r'\{\{\s*image\s*\}\}',
                    'new_content': sys.argv[2]
                }]
        replace_file(obj)
    except ValueError as e:
        print(e)
    res = subprocess.run(['ls', '-l'], capture_output=True)
    print(res.stdout.decode('utf8'))
