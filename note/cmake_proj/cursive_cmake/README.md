### 描述
主要练习cmake嵌套联系，calc生成静态库，sort生成动态库，test1调用calc，test2调用sort
- include 目录：头文件目录
- calc 目录：目录中的四个源文件对应的加、减、乘、除算法
- 对应的头文件是 include 中的 calc.h
- sort 目录 ：目录中的两个源文件对应的是插入排序和选择排序算法
- 对应的头文件是 include 中的 sort.h
- test1 目录：测试目录，对加、减、乘、除算法进行测试
- test2 目录：测试目录，对排序算法进行测试