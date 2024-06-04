## 使用pip3包管理器管理
1. 安装pip3
```shell
sudo apt install python3-pip
```
2. 生成requirements.txt
```shell
touch requirements.txt
```
3. 手动或者自动更新requirements.txt, 还是手动更新吧，自动更新会加入很多不知道的库
```shell
# 自动更新
pip freeze > requirements.txt
# 手动更新
cat 'pyyaml==5.4.1' >> requirements.txt
```
4. 安装包
```shell
pip install -r requirements.txt
```
## open
```python
with open('file_path', 'r+') as f:
    content = f.read()
    f.seek(0, 0)
    f.truncate()
    f.write('sth')
```
如上面所示，基本操作跟C里面的fopen接口差不多，其中mode分为如下：

|mode|解释|
|--|--|
|r|read only, 文件不存在报错|
|r+|read, write, 文件不存在报错, **默认文件游标在开头，从游标位置开始添加，并且覆盖后面原有的**|
|w|在此模式下，文件指针也会放在开头。如果文件不存在，则会创建一个空文件；如果文件已经存在，则会删除原有的内容(truncate)，并从头开始写入。因此，写模式不适合读取文件的内容, **不能read**|
|w+|在w模式基础上，还可以进行read|
|a|append, 文件不存在会新建|
|r+|read, write, **默认文件游标在结尾，可以通过调整seek来读取内容，但是写入是一定在结尾的，跟seek调整无关**|

r, r+, w, w+, a, a+要注意以下几点：
1. 文件不存在是否新建，r和r+不存在报错，其他会新建
2. 文件指针位置。r和r+默认文件指针在开头，其他默认都在结尾，并且a和a+模式下一定会写入结尾，使用seek调整文件指针无效

## unittest
python内置的单元测试库，例子见[test.py](./test.py)。继承```unittest.TestCase```，每个测试用例(test case)就是一个方法，使用各种assert方法测试。

## __pycache__文件夹
当import本地的python文件时，会生成__pycache__文件夹写入相关文件的python字节码(扩展为pyc)，以保持加载效率