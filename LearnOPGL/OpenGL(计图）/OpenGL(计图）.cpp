#include <iostream>		// C++的标准输入输出头文件
using namespace std;

#define GLEW_STATIC     // 预定义：把 glew 这个库放到系统文件夹里面(STATIC:能找到静态库就优先使用静态库)
// 由静态库导出或从静态库导入的函数的签名用关键字 export .从动态库导入的功能必须用 extern __declspec(dllimport).GLEW_STATIC是激活第一种情况的预处理器定义.

#include <GL/glew.h>    // 包含“GL”文件夹里面的“glew.h”的头文件
						// 程序运行到这一段后，先找到“GL”文件，打开后找到“glew.h”头文件，然后
						// 会在“编译”的时候把里面的整段代码复制到下面，只是没有显示出来

#include <GLFW/glfw3.h> // 我们需要和不同的操作系统进行交互，则需要“glfw.h”的头文件
						// 先创建一个窗口，然后在窗口里面创建一个“视口”

const GLint WIDTH = 800, HEIGHT = 600;		// 先设置窗口以及其大小
											// 在openGL里面，数据类型名字前面都有个大写的“GL”,所以“GLint”其实内涵就是“整型int”的意思
											// 而openGL里面的函数都以小写的“gl”开头
int main()
{

	/*
		说明：
			glfw 提供的是环境(变量名或函数名以“glfw”或“GLFW”开头的都是)
			glew 用来绘图(变量名或函数名以“glew”或“GLFW”开头的都是)
	*/

	glfwInit();   //初始化，使用glfw来打开一个窗口

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);		// 设置窗口版本，“MAJOR”代表主版本号

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);		// 设置窗口版本，“MAJOR”代表副版本号

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// GLFW_OPENGL_PROFILE用告诉窗口，这个版本是为了openGL 做准备的。
																	// openGL用的版本用“CORE_PROFILE”来表示，指的是3.1以后的版本 新版的

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// 苹果系统需要加这条语句(Windows可加可不加)。函数作用：向前面的版本兼容

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);				//缩放窗口的关闭

	GLFWwindow* window_1 = glfwCreateWindow(WIDTH, HEIGHT, "Hello, friend! I'm a openGL window!", nullptr, nullptr);
	// 新建一个窗口，"一支王同学再在此出没！"(第三个参数)：设置窗口名字

	//  开始为高清屏做设置
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window_1, &screenWidth, &screenHeight);  // 获得实际占用的帧缓存大小。帧的宽传给screenWidth；帧的高传给screenHeight。

	if (nullptr == window_1)	//判断窗口输出是否成功，失败则给出一段提示
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();		// glfw关闭
		return -1;				// 进程结束
	}

	glfwMakeContextCurrent(window_1);   // 捕获当前窗口，准备对当前窗口进行画图

	glewExperimental = GL_TRUE;			// 如果程序编译的时候说有问题的时候，再打开这条语句(以前解决问题的一条语句)

	if (glewInit() != GLEW_OK)			// 判断glew初始化是否成功，看返回值是否成功 失败则给出一段提示
	{
		cout << "Failed to initialise GLEW" << endl;
		glfwTerminate();				// 关闭glfw
		return -1;
	}

	glViewport(0, 0, screenWidth, screenHeight);	// 设置视口的大小(帧缓存的大小传进去)
													// 原函数声明： glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
													// (x ,y)代表视口(正方形)的左下角的坐标。width、height分别代表视口的宽和高

	while (!glfwWindowShouldClose(window_1))		// 只要当前窗口不关闭，一直执行这个循环
	{
		glfwPollEvents();							// 事件相应的命令，作用：捕获所有的事件。

		glClearColor(0.1f, 0.8f, 0.7f, 1.0f);		// 分别是红、绿、蓝、透明度的四个参数。RGB三原色+透明度(1.0表示不透明，0.1表示完全透明)
													// 一般电脑的RGB显示的都是8位，能表示256*256*256=16777216色

		glClear(GL_COLOR_BUFFER_BIT);				// glClear()：使用 glClearColor 中指定的值设定颜色缓存区的值，即将窗口中的每一个像素设置为背景色GL_COLOR_BUFFER_BIT

		glfwSwapBuffers(window_1);					// 打开双缓存模式(进阶知识),相当于拿出两块“画板”(一块画好的展示在你面前，另一块接着画，反正画好才给你看)
	}

	glfwTerminate();   // 如果循环结束：glfw关闭

	return 0;
}
