/*
*
*            libkey
*
*/

工作原理:
	读写用户的私密数据（MAC、ID等）
	1. secure storage与private分区同时支持:
		读接口：优先从secure storage读取数据,如果数据错误或者不存在，则从private分区读取数据,否则返回错误
		写接口：同时写到secure storage区域与private 分区
	2. 单独secure storage存在
		读接口：只从secure storage读取数据，如果数据错误或者不存在,则返回错误
		写接口：只写数据到secure storage区域
	3. 单独private分区存在
		读接口：只从private分区读取数据，如果数据错误或者不存在,则返回错误
		写接口：只写数据到private分区

使用方法： 具体接口只有两个，读和写私密数据，详情请看api.h




