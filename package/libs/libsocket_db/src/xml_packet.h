#ifndef __XML_PACKET_H
#define __XML_PACKET_H

struct xmlNode{
    char name[20];
    char content[512];
};

struct devInfo{
    char device[64]; /* device num */
    char apkVer[64]; /* apk version */
    char system[64]; /* system version */
    char scheme[16]; /* project scheme */
    char sn[32]; /* SN */
    char res[80]; /* reserve */
};

struct fileInfo{
    char filename[32]; /* file name */
    char filedir[64];  /* file directory */
    int size; /* file size */
    int compress; /* compress flag */
    int checksum; /* checksum value */
};

/*head file for xml packet*/
char *sendStartCase_packet(const char *testname,int id);
char *sendResult_packet(const char *testname, const char *mark, int result,int id);
char *sendTip_packet(const char *testname, const char *tip,int id);
char *sendTestEnd_packet(int result, const char *mark,int id);
char *sendCMDscan_packet(const char *testname, const char *tip, const char *key, int id);
char *sendCMDselect_packet(const char *testname, const char *tip, const char* mark, int timeout, int id);
char *sendCMDOperator_packet(const char *testname, const char *plugin, const char * datatype, const char * data,int id);
char *sendEdit_packet(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout, int id);
int Decbase64(char * orgdata,unsigned long orglen, char *buf);
int Encbase64(char * orgdata,unsigned long orglen,char * reValue);
int libxmltest(void);
int CreateXmlPacket(char *type, struct xmlNode *baseNode, char* xmlBuffer);
int GetXmlNode(char *xmlBuffer, char *type, char *node, char *nodeBuffer);
int GetXmlRootNodeName(char *xmlBuffer, char *name);
int ExtractXmlContent(char *data, char *xmlBuffer, int *xmlLen);
int buildXmlNodeInfo(char *nodeName[], char nodeValue[][512], struct xmlNode *baseNode);
char *downloadFileRequest_packet(struct fileInfo *file, const char *testname, const char *mark, int id);
char *uploadFileRequest_packet(struct fileInfo *file, const char *testname, const char *mark, int id);
char *sendShowRes_packet(struct fileInfo *file, const char *filetype, const char *testname, const char *mark, int id);
char *sendStartTestResponse_packet(struct devInfo *dev, int result, const char *mark, int id);
char *sendCommonResponse_packet(int result, const char *mark, int id);


#endif
