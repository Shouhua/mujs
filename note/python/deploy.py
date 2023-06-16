#! /usr/bin/env python3

"""
linux: ./deplpy.py docker_image
windows: python3 .\deploy.py docker_image
"""

import sys
import http.client
import re
import urllib.parse

hosts = {
    'ip_addr': 'project_instace'
}

devops_port = 1234

devops_cookie = 'login=abc'

image = sys.argv[1]

for k, v in hosts.items():
    instance_api = f'/path.{v}'
    conn = http.client.HTTPConnection(k, devops_port)
    headers = {
        'Cookie': devops_cookie 
    }
    conn.request('GET', instance_api, headers=headers)
    response = conn.getresponse()
    instance_info = response.read().decode()

    new_instance_info = re.sub(r'"image":"[^"]+"', f'"image":"{image}"', instance_info)

    headers.update({'Content-Type': 'multipart/form-data'})
    conn.request('PUT', instance_api, body=new_instance_info, headers=headers)
    response = conn.getresponse()
    result = response.read().decode()
    print('更新镜像信息：')
    print(result, end="\n")

    disable_service_api = f'/api/v1/service/{v}/deploy_disable'
    conn.request('POST', disable_service_api, headers=headers)
    response = conn.getresponse()
    result = response.read().decode()
    print('暂停实例：')
    print(result, end="\n")

    operate_body = urllib.parse.urlencode({
        'operate': 'deploy'
    })
    operate_api = instance_api+'/operate'
    headers.update({'Content-Type': 'application/x-www-form-urlencoded'})
    conn.request('POST', operate_api, body=operate_body, headers=headers)
    response = conn.getresponse()
    result = response.read().decode()
    print('部署实例：')
    print(result)