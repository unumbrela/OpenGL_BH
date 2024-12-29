# 一、无需配置环境，直接运行.exe文件
解压Release.zip,打开Release文件夹，双击运行Blackhole.exe即可

# 二、配置环境，复现源码
## （一）、准备好CMake

## （二）、准备好conan
打开cmd命令行
1，如果已经安装了conan，请输入 pip uninstall conan，卸载当前版本
2，然后 pip install conan==1.8.3，安装正确版本，请不要安装2.x版本，也请不要安装1.6

3，运行以下命令测试conan是否正常  conan --version，如果正确显示，直接跳到（三）

4，如果报错说明PATH配置有问题，输入where conan定位到conan.exe所在文件夹，然后将其添加到PATH环境变量

比如，我输入where conan 显示结果，C:\Users\ZihaoGuo\AppData\Roaming\Python\Python39\Scripts\conan.exe
便将以下路径添加到环境变量C:\Users\ZihaoGuo\AppData\Roaming\Python\Python39\Scripts
![alt text](image.png)

此时再输入 conan --version 便可以正常输出conan 版本

如果之前配置过conan 2.x 并再次报错，只需删除掉C:\Users\你的名字 下面的.conan 和 .conan2文件夹即可

## （三）、在项目的根目录下，输入cmd运行以下命令
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
