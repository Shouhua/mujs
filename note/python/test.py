#! /usr/bin/env python3

import unittest
import yaml

from replace import get_version, replace_file

class TestReplace(unittest.TestCase):
    def setup(self):
        """
        每个test case运行前都会运行，主要用于设置fixture 
        """
        with open('artifact.yaml', 'w+') as f:
            f.write('image: {{ image}}')
        with open('version.yaml', 'w+') as f:
            f.write('''
                version: 1.2.4
                dependencies:
                    version: 1.1.1 
            ''')
    def test_get_version(self):
        """
        Test that it can get new version by current max version add 1 
        """
        project_name = 'cbg.sst.sst_fe.diagnosis_web'
        major_minor = '1.3'
        result = get_version(project_name, major_minor)
        self.assertEqual(result, '1.3.1')
    def test_replace_file(self):
        version = '1.2.3'
        image = 'my_image'
        obj = [{
            'path': 'version.yaml',
            're_text': r'^version:\s*\d+\.\d+.\d+',
            'new_content': f'version: {version}'
        },
        {
            'path': 'image.yaml',
            're_text': r'\{\{\s*image\s*\}\}',
            'new_content': image
        }]
        replace_file(obj)
        with open('version.yaml', 'r') as f:
            version_content = f.read()
            version_data = yaml.load(version_content, Loader=yaml.FullLoader)
            expected_version = {
                'version': version,
                'dependencies': {
                    'version': '1.1.1'
                }
            }
            self.assertEqual(version_data, expected_version)
        with open('image.yaml', 'r') as f:
            image_content = f.read()
            image_data = yaml.load(image_content, Loader=yaml.FullLoader)
            expected_image = {
                'name': image
            }
            self.assertEqual(image_data, expected_image)

if __name__ == '__main__':
    unittest.main()