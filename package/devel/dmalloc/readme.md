# dmalloc 简介
	dmalloc: debug memory allocation

	dmalloc是一个简单易用的C/C++内存检查工具，dmalloc通过对系统的malloc、realloc、calloc、free等函数进行替换来检测内存问题。使用dmalloc可以用来跟踪内存泄露、边界写保护、文件/行号报告和一般的统计记录。

	dmalloc主页： http://dmalloc.com

	Dmalloc局限性：
	1. Dmalloc只能检测堆上内存，对栈内存和静态内存无能为力。
	2. dmalloc只用于利用malloc申请的内存，对使用sbrk()或mmap()分配的内存无能为力。
	3. dmalloc不能用于检测读写没有申请的或者没有初始化的内存，也不能检测写只读内存。
	4. 对C++不友好，C++的内存问题生成报告时只能显示地址，不能显示文件名行号等信息。


# 如何使用dmalloc？
	参考文档：http://dmalloc.com/docs/
	经测试dmalloc可检测出memleak、overflow、double free、use after free等类型的内存问题。

	主要步骤：
	1. 在源程序中 #include "dmalloc.h"
	2. 在源程序中设置环境变量， dmalloc_debug_setup("debug=0x4f47d03,log=/tmp/dmalloc_test.log"); 一旦待调试程序结束后，即可生成调试打印文件/tmp/dmalloc_test.log
	3. 如果只想调试一段时间内的内存情况，可以使用dmalloc_mark（起始点）与dmalloc_log_changed（结束点）函数。
	（可参考dmalloc_test.cpp）
