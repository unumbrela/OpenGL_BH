# 一、无需配置环境，直接运行.exe文件
解压Release.zip,打开Release文件夹，双击运行Blackhole.exe即可

# 二、配置环境，复现源码

# 测试环境：Win11 + 4060笔记本 + visual studio2022

## （一）、准备好CMake
打开cmd命令行
1， pip install conan //也可自行从官网下载安装
2，验证是否安装成功，输入cmake --version 输出结果应该是 cmake version 3.31.2 代表cmake配置成功
## （二）、准备好conan
打开cmd命令行
1， pip install conan
2，验证是否安装成功，输入conan --version 输出结果应该是 Conan version 2.11.0  代表conan配置成功

## （三）、下载并解压项目文件，得到OpenGL_BH-main，在这个文件夹下输入cmd运行以下命令
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

发现当前文件夹下多了Blackhole.sln，双击运行，即可在解决方案资源管理器中查看到项目Blackhole的所有文件

可以切换到Release模式下运行，会报错，需要配置环境，请参考以下博客
区别：在配置imgui时，可以先删除掉当前和imgui有关的.cpp而后重新添加
