#! /usr/bin/env python3

def read_case():
    with open('testt.txt', 'r') as f:
        content = f.read()
        print(content)
def read_plus_case():
    with open('testt.txt', 'r+') as f:
        # f.write('goodie')
        content = f.read()
        print(content)

def write_case():
    with open('test.txt', 'w') as f:
        content = f.read()
        print(content)
        f.seek(0, 2)
        f.write('\nabc')
def write_plus_case():
    with open('test.txt', 'w+') as f:
        content = f.read()
        print(content)
        f.seek(0, 2)
        f.write('\nabc')
def append_case():
    with open('test.txt', 'a') as f:
        # content = f.read()
        # print(content)
        f.seek(2, 0)
        f.write('append')
def append_plus_case():
    with open('test.txt', 'a+') as f:
        # 此时文件指针位于尾部，读取内容为空
        content = f.read()
        print(content)
        f.seek(0)
        content = f.read()
        print(content)
        f.seek(0)
        f.write('\nappend')

with open('test.txt', 'w+') as f:
    f.write('helo\nworld')

# read_case()
read_plus_case()
# append_plus_case()