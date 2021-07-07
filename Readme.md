# 大学计算机课程学习记录

## 1. 用C语言编写的windows网络服务器(CSAPP网络编程实验 & 计算机网络socket编程实验) (07/06/2021)

编译程序的时候要加 -lwsock32, accept()函数没有中断的原因最可能是bind出了问题。

先运行程序, 再访问网页<a herf = http://localhost:8000>http://localhost:8000, 最后就能看到页面了。

程序使用了字符串处理的技巧, 涉及到VScode task.json 和 launch.json两个文件的配置, 简单html文件的编写, 同时包含了c语言TCP客户端服务器通信的小程序。

## 2. 用C语言编写的windows系统下的多线程搜索程序(CSAPP 多线程实验) (07/07/2021)
笔记本cpu : ```AMD RYZEN 5700U 1.80GHZ 8核16线程```
经实验, **10线程**的运算速度最快, 代码中有一个多线程模板和搜索程序