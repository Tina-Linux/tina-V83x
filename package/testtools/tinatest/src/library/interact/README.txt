interact库：实现outlog的交互功能

设计框架如下：
==============================================================================

                --------     --------
[测试用例1] --> |  消  |     |  核  |     ------ <-- [outlog插件1]
                |  息  | --> |  心  | --> | 管 |
                |  队  | <-- |  策  | <-- | 道 |
[测试用例1] --> |  列  |     |  略  |     ------ <-- [outlog插件2]
                --------     --------

==============================================================================

添加outlog插件方法：
1. 在outlog插件库的module_init函数中，调用interact_register注册回调函数即可.
2. 不支持的交互，在回调中写为NULL即可

interact库添加新交互命令方法：
1. 头文件: <interact.h>
    a. 定义outlog插件回调接口类型
       eg. typedef int (*f_upfile) (const char *testcase, XXXXX)

    b. 修改interact_register接口
       eg. int interact_register(
               ......
               f_upfile upfile);

2. 头文件: <interact-actor.h>
    a. 定义命令编号（枚举型变量 enum cmd）
       eg. enum cmd {
               ......
               cmd_upfile,
               cmd_cnt,   //作为cmd个数计数，命令务必写在此前
               ......
           }

2. 源文件: <interact-actor.c>
    a. 修改注册接口
        eg. int interact_register(
                ......
                f_upfile upfile)
            {
                ....
                ADD_CMD(upfile); //只需要添加此行即可
            }

    b. 实现核心策略回调次命令的接口函数
        eg. static int interact_do_upfile(struct actor *act)
            {
                ......
            }

    c. 添加到注册回调函数中(intearct_actor_do)
        eg. int intearct_actor_do(struct actor *act)
            {
                ......

                case cmd_upfile:
                    // @upfile：命令
                    // @false/true：是否需要回复给测试用例
                    interact_do(upfile, false);

                ......
            }

3. 源文件: <interact-capi.c>
    a. 在文件最后实现C/C++对此命令的调用接口
        // initenv 从环境变量中获取关键变量值,并初始化通用部分
        // 此接口传递给核心策略层的数据只能保存到pmsg->text中
        // 提供三个函数：
        //  initenv: 初始化环境
        eg int tupfile(const char *filepath, const char *tips)
           {
                if (initenv() < 0)
                    return -1;

                ......
           }

4. 在scripts中实现shell调用的命令
    eg. <upfile.c>
        int main(int argc, char **argv)
        {
            ......
        }

5. 改了注册接口后修改outlog中其他插件对interact_register接口的调用,否则编译其他插件会失败
