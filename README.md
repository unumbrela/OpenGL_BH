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

可以切换到Release模式下运行，会报错，需要配置环境，请参考以下博客[https://blog.csdn.net/yiyeyeshenlan/article/details/144697459?spm=1001.2014.3001.5502](https://blog.csdn.net/yiyeyeshenlan/article/details/144697459),相关文件已附到github上，即imgui文件夹
区别：在配置imgui时，可以先删除掉当前和imgui有关的.cpp而后重新添加

参考文献

Papers

Gravitational Lensing by Spinning Black Holes in Astrophysics, and in the Movie Interstellar

Trajectory Around A Spherically Symmetric Non-Rotating Black Hole - Sumanta

Approximating Light Rays In The Schwarzschild Field - O. Semerak

Implementing a Rasterization Framework for a Black Hole Spacetime - Yoshiyuki Yamashita

Articles

Physics of oseiskar.github.io/black-hole - https://oseiskar.github.io/black-hole/docs/physics.html

Schwarzschild geodesics - https://en.wikipedia.org/wiki/Schwarzschild_geodesics

Photons and black holes - https://flannelhead.github.io/posts/2016-03-06-photons-and-black-holes.html

A real-time simulation of the visual appearance of a Schwarzschild Black Hole - http://spiro.fisica.unipd.it/~antonell/schwarzschild/

Ray Tracing a Black Hole in C# by Mikolaj Barwicki - https://www.codeproject.com/Articles/994466/Ray-Tracing-a-Black-Hole-in-Csharp

Ray Marching and Signed Distance Functions - http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/

Einstein's Rings and the Fabric of Space - https://www.youtube.com/watch?v=Rl8H4XEs0hw)

Opus 2, GLSL ray tracing tutorial - http://fhtr.blogspot.com/2013/12/opus-2-glsl-ray-tracing-tutorial.html

Ray Tracing in One Weekend - https://raytracing.github.io/

On ray casting, ray tracing, ray marching and the like - http://hugi.scene.org/online/hugi37/- hugi%2037%20-%20coding%20adok%20on%20ray%20casting,%20ray%20tracing,%20ray%20marching%20and%20the%20like.htm

Other GitHub Projects

https://github.com/ebruneton/black_hole_shader/tree/master?tab=readme-ov-file

https://ebruneton.github.io/black_hole_shader/demo/demo.html

https://github.com/sirxemic/Interstellar

https://github.com/ssloy/tinyraytracer

https://github.com/RayTracing/raytracing.github.io

https://awesomeopensource.com/projects/raytracing

Ray-traced simulation of a black hole - https://github.com/oseiskar/black-hole

Raytracing a blackhole - https://rantonels.github.io/starless/

https://github.com/rantonels/schwarzschild
