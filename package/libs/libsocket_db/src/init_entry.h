#ifndef __INIT_ENTRY_H
#define __INIT_ENTRY_H

#include "xml_packet.h"

#define PASS 1
#define FAILED 0

#define HEAD_SIZE 20
#define MAGIC 0x54545450
#define VERSION 0x00000001

//head file for init_entry.c
int init_entry();
int waitSocketConnect();
int sendStartCase(const char *testname);
int sendResult(const char *testname, const char *mark, int result);
int sendTip(const char *testname, const char *tip);
int sendTestEnd(int result, const char *mark);
int sendCMDselect(const char *testname, const char *tip, const char* mark, int timeout);
int sendCMDOperator(const char *testname, const char *plugin, const char * datatype, char * data);
int sendEdit(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout);
int downloadFileRequest(struct fileInfo *file, const char *testname, const char *mark);
int uploadFileRequest(struct fileInfo *file, const char *testname, const char *mark);
int sendShowResRequest(struct fileInfo *file, const char *type, const char *testname, const char *mark);
int sendStartTestResponse(struct devInfo *dev, int result, const char *mark, int msgid);
int sendCommonResponse(int result, const char *mark, int msgid);
int getReturn(int  msgid);
int getValue(int msgid, const char* szKey, char*szValue, int *nLen);
int getPacketType(int msgid, char *type);
int msgidInit();
int msgidExit();
int getMsgid();
int sendStartRes2pc(int id);
char *add_head(char* buf, int payloadsize,int externsize,int type);

/* fix bug for testcase */
int sendStartCase_fix(const char *testname, char *buf);
int sendResult_fix(const char *testname, const char *mark, int result, char *buf);
int sendTip_fix(const char *testname, const char *tip, char *buf);
int sendTestEnd_fix(int result, const char *mark, char *buf);
int sendCMDselect_fix(const char *testname, const char *tip, const char* mark, int timeout, char *buf);
int sendCMDOperator_fix(const char *testname, const char *plugin, const char * datatype, char * data, char *buf);
int sendEdit_fix(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout, char *buf);
int downloadFileRequest_fix(struct fileInfo *file, const char* testname, const char *mark, char *buf);
int uploadFileRequest_fix(struct fileInfo *file, const char *testname, const char *mark, char *buf);
int sendShowResRequest_fix(struct fileInfo *file, const char *type, const char *testname, const char *mark, char *buf);
int sendStartTestResponse_fix(struct devInfo *dev, int result, const char *mark, int msgid, char *buf);
int sendCommonResponse_fix(int result, const char *mark, int msgid, char *buf);
int getPacketType_fix(char *buf, char *type);
int getValue_fix(char *buf, const char* szKey, char*szValue, int *nLen);
int getReturn_fix(char *buf);
int getPacketBuff_fix(char *buf, char *xmlBuffer, char *extBuffer, int *xmlLen, int *extLen);

#endif
