/******************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "socket_db_debug.h"
#include "xml_packet.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

char buff_str_head[32];
char buff_str_rear[32];
char buff_add_cmd_type[512];
char buff_sendStartCase_packet[1024];
char buff_sendResult_packet[1024];
char buff_sendTip_packet[1024];
char buff_sendTestEnd_packet[1024];
char buff_sendCMDscan_packet[1024];
char buff_sendCMDselect_packet[1024];
char buff_sendCMDOperator_packet[1024];
char buff_downloadFile_packet[1024];
char buff_uploadFile_packet[1024];
char buff_sendShowPicture_packet[1024];
char buff_startTestResponse_packet[1024];
char buff_commonResponse_packet[1024];
char buff_sendEdit_packet[1024];

/*********************************************************************
* name: sendStartCase
* function:start test case
* input:testname, id
* return:msgid
*********************************************************************/
char *sendStartCase_packet(const char *testname,int id)
{
    memset(buff_sendStartCase_packet,0,sizeof(buff_sendStartCase_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid", "cmd", "name", NULL};
    char label_val[3][512];
    struct xmlNode baseNode[4];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"start");
    strcpy(label_val[2],testname);
    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendStartCase_packet, buf_temp);
    DEBUG("sendStartCase  buf = %s\n",buff_sendStartCase_packet);

    return buff_sendStartCase_packet;
}


/*********************************************************************
* name: sendResult
* function: send test result
* input:testname, mark, result, id
* return:msgid
*********************************************************************/
char *sendResult_packet(const char *testname, const char *mark, int result,int id)
{
    memset(buff_sendResult_packet,0,sizeof(buff_sendResult_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","result","mark", NULL};
    char label_val[5][512];
    struct xmlNode baseNode[6];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"end");
    strcpy(label_val[2],testname);
    if(result == 1)
    {
        strcpy(label_val[3],"OK");
    }else
    {
        strcpy(label_val[3],"FAIL");
    }
    strcpy(label_val[4],mark);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendResult_packet, buf_temp);
    DEBUG("sendResult  buf = %s\n",buff_sendResult_packet);

    return buff_sendResult_packet;
}


/*********************************************************************
* name: sendTip
* function: send tip message
* input:testname, tip, id
* return:msgid
*********************************************************************/
char *sendTip_packet(const char *testname, const char *tip,int id)
{
    memset(buff_sendTip_packet,0,sizeof(buff_sendTip_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","tip", NULL};
    char label_val[4][512];
    struct xmlNode baseNode[5];

    DEBUG("fun = %s--id = %d\n",__func__,id);
    DEBUG("tip = %s\n", tip);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"tip");
    strcpy(label_val[2],testname);
    strcpy(label_val[3],tip);
    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendTip_packet, buf_temp);
    DEBUG("sendTip  buf = %s\n",buff_sendTip_packet);

    return buff_sendTip_packet;
}


/*********************************************************************
* name: sendTestEnd
* function: send test over message
* input:result, mark, id
* return:msgid
*********************************************************************/
char *sendTestEnd_packet(int result, const char *mark,int id)
{
    memset(buff_sendTestEnd_packet,0,sizeof(buff_sendTestEnd_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","result","mark", NULL};
    char label_val[5][512];
    struct xmlNode baseNode[6];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"finish");
    strcpy(label_val[2],"null");
    if(result == 1)
    {
        strcpy(label_val[3],"OK");
    }else
    {
        strcpy(label_val[3],"FAIL");
    }
    strcpy(label_val[4],mark);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendTestEnd_packet, buf_temp);
    DEBUG("sendTestEnd  buf = %s\n",buff_sendTestEnd_packet);

    return buff_sendTestEnd_packet;
}

/*********************************************************************
* name: sendCMDscan
* function: exter detect,PC return scan result
* input:testname, tip, key, id
* return:packet
*********************************************************************/
char *sendCMDscan_packet(const char *testname, const char *tip, const char *key, int id)
{
    memset(buff_sendCMDscan_packet,0,sizeof(buff_sendCMDscan_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","tip", "key", NULL};
    char label_val[6][512];
    struct xmlNode baseNode[7];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"scan");
    strcpy(label_val[2],testname);
    strcpy(label_val[3],tip);
    strcpy(label_val[4],key);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendCMDscan_packet, buf_temp);
    DEBUG("sendCMDscan  buf = %s\n",buff_sendCMDscan_packet);

    return buff_sendCMDscan_packet;
}



/*********************************************************************
* name: sendCMDselect
* function: exter detect,PC return test result
* input:testname, mark, id
* return:msgid
*********************************************************************/
char *sendCMDselect_packet(const char *testname, const char *tip, const char* mark,  int timeout, int id)
{
    memset(buff_sendCMDselect_packet,0,sizeof(buff_sendCMDselect_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","tip", "mark", "timeout", NULL};
    char label_val[6][512];
    struct xmlNode baseNode[7];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"select");
    strcpy(label_val[2],testname);
    strcpy(label_val[3],tip);
	strcpy(label_val[4],mark);
    sprintf(label_val[5], "%d", timeout);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendCMDselect_packet, buf_temp);
    DEBUG("sendCMDselect  buf = %s\n",buff_sendCMDselect_packet);

    return buff_sendCMDselect_packet;
}


/*********************************************************************
* name: sendCMDOperator
* function: request test,pc return test result
* input:testname, plugin, datatype, data, id
* return:msgid
*********************************************************************/
char *sendCMDOperator_packet(const char *testname, const char *plugin, const char *datatype, const char * data,int id)
{
    memset(buff_sendCMDOperator_packet,0,sizeof(buff_sendCMDOperator_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","tip","plugin","datatype","data", NULL};
    char label_val[7][512];
    struct xmlNode baseNode[8];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"operator");
    strcpy(label_val[2],testname);
    strcpy(label_val[3],"operate start");
    strcpy(label_val[4],plugin);
    strcpy(label_val[5],datatype);
    strcpy(label_val[6],data);
    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendCMDOperator_packet, buf_temp);
    DEBUG("sendCMDOperator buf = %s\n", buff_sendCMDOperator_packet);


    return buff_sendCMDOperator_packet;
}

/*********************************************************************
* name: sendEdit
* function: send edit request to pc, pc return the user input
* input:testname, tip, editvalue, timerout, id
* return:msgid
*********************************************************************/
char *sendEdit_packet(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout, int id)
{
    memset(buff_sendEdit_packet,0,sizeof(buff_sendEdit_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","name","tip","editvalue","mark","timeout", NULL};
    char label_val[7][512];
    struct xmlNode baseNode[8];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"edit");
    strcpy(label_val[2],testname);
    strcpy(label_val[3],tip);
    strcpy(label_val[4], editvalue);
    strcpy(label_val[5], mark);
    sprintf(label_val[6], "%d", timeout);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendEdit_packet, buf_temp);
    DEBUG("sendCMDselect  buf = %s\n",buff_sendEdit_packet);

    return buff_sendEdit_packet;
}

/*********************************************************************
* name: downloadFileRequest_packet
* function: download file
* input:filename, directory, id
* return: xml packet
*********************************************************************/
char *downloadFileRequest_packet(struct fileInfo *file, const char *testname, const char *mark, int id)
{
    memset(buff_downloadFile_packet,0,sizeof(buff_downloadFile_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid", "cmd", "filename", "destpath", "name", "mark", NULL};
    char label_val[6][512];
    struct xmlNode baseNode[7];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"download");
    strcpy(label_val[2],file->filename);
    strcpy(label_val[3],file->filedir);
    strcpy(label_val[4],testname);
    strcpy(label_val[5],testname);
    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_downloadFile_packet, buf_temp);
    DEBUG("downloadFile buf = %s\n",buff_downloadFile_packet);

    return buff_downloadFile_packet;
}

/*********************************************************************
* name: uploadFileRequest_packet
* function: upload file
* input:filename, directory, size, compress, checksum, id
* return: xml packet
*********************************************************************/
char *uploadFileRequest_packet(struct fileInfo *file, const char *testname, const char *mark, int id)
{
    memset(buff_uploadFile_packet,0,sizeof(buff_uploadFile_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","cmd","filename", "srcpath","filesize","compress","shecksum", "name", "mark", NULL};
    char label_val[9][512];
    struct xmlNode baseNode[10];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"upload");
    strcpy(label_val[2],file->filename);
    strcpy(label_val[3],file->filedir);
    sprintf(label_val[4],"%d",file->size);
    sprintf(label_val[5],"%d",file->compress);
    sprintf(label_val[6],"%d",file->checksum);
    strcpy(label_val[7],testname);
    strcpy(label_val[8],mark);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_uploadFile_packet, buf_temp);
    DEBUG("uploadFile buf = %s\n",buff_uploadFile_packet);

    return buff_uploadFile_packet;
}

/*********************************************************************
* name: sendShowRes_packet
* function: show resource in pc
* input: resource info, resource type, testcase, mark, id
* return: xml packet
*********************************************************************/
char *sendShowRes_packet(struct fileInfo *file, const char *filetype, const char *testname, const char *mark, int id)
{
    memset(buff_sendShowPicture_packet,0,sizeof(buff_sendShowPicture_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid", "cmd", "resource", "type", "name", "mark", NULL};
    char label_val[6][512];
    struct xmlNode baseNode[7];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],"showres");
    strcpy(label_val[2],file->filename);
    strcpy(label_val[3],filetype);
    strcpy(label_val[4],testname);
    strcpy(label_val[5],mark);

    char *type = "req";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_sendShowPicture_packet, buf_temp);
    DEBUG("sendShowPicture buf = %s\n",buff_sendShowPicture_packet);

    return buff_sendShowPicture_packet;
}

/*********************************************************************
* name: sendStartTestResponse_packet
* function: send response for start test command
* input: device information, result, mark, id
* return: xml packet
*********************************************************************/

char *sendStartTestResponse_packet(struct devInfo *dev, int result, const char *mark, int id)
{
    memset(buff_startTestResponse_packet,0,sizeof(buff_startTestResponse_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","device","apk_version","system","scheme", "sn", "user_def", "result", "mark", NULL};
    char label_val[9][512];
    struct xmlNode baseNode[10];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[1],dev->device);
    strcpy(label_val[2],dev->apkVer);
    strcpy(label_val[3],dev->system);
    strcpy(label_val[4],dev->scheme);
    strcpy(label_val[5],dev->sn);
    strcpy(label_val[6],dev->res);
    strcpy(label_val[8],mark);
    if(result == 1)
    {
        strcpy(label_val[7],"OK");
    }else
    {
        strcpy(label_val[7],"FAIL");
    }
    char *type = "response";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_startTestResponse_packet, buf_temp);
    DEBUG("sendCMDselect  buf = %s\n",buff_startTestResponse_packet);

    return buff_startTestResponse_packet;
}

/*********************************************************************
* name: sendCommonResponse_packet
* function: send response for pc common command, such as download/upload file
* input: result, mark, id
* return: xml packet
*********************************************************************/

char *sendCommonResponse_packet(int result, const char *mark, int id)
{
    memset(buff_commonResponse_packet,0,sizeof(buff_commonResponse_packet));
    char buf_temp[1024];
    int i = 0;
    char *label[] = {"msgid","result","mark", NULL};
    char label_val[3][512];
    struct xmlNode baseNode[4];

    DEBUG("fun = %s---id = %d\n",__func__,id);
    sprintf(label_val[0],"%d",id);
    strcpy(label_val[2],mark);
    if(result == 1)
    {
        strcpy(label_val[1],"OK");
    }else
    {
        strcpy(label_val[1],"FAIL");
    }
    char *type = "response";

    buildXmlNodeInfo(label, label_val, baseNode);
    CreateXmlPacket(type, baseNode, buf_temp);

    strcpy(buff_commonResponse_packet, buf_temp);
    DEBUG("sendCMDselect  buf = %s\n",buff_commonResponse_packet);

    return buff_commonResponse_packet;
}

const char _Base[]={"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="};

static union
{
    struct
    {
        unsigned long a:6;
        unsigned long b:6;
        unsigned long c:6;
        unsigned long d:6;
    }Sdata;
    unsigned char c[3];
}Udata;


/*********************************************************************
* name: Encbase64
* function:
* input:
* return:
*********************************************************************/
int Encbase64(char * orgdata,unsigned long orglen,char * reValue)
{
    char *p=NULL,*ret=NULL;
    int tlen=0;
    if (orgdata==NULL|| orglen==0)
        return -1;

    tlen=orglen/3;
    if(tlen%3!=0) tlen++;
    tlen=tlen*4;
    // *newlen=tlen;
    if ((ret=(char *)malloc(tlen+1))==NULL)
        return -1;

    memset(ret,0,tlen+1);
    p=orgdata;tlen=orglen;

    int i=0,j=0;
    while(tlen>0)
    {
        Udata.c[0]=Udata.c[1]=Udata.c[2]=0;
        for (i=0;i<3;i++)
        {
            if (tlen<1) break;
            Udata.c[i]=(char)*p;
            tlen--;
            p++;
        }
        if (i==0) break;
        switch (i)
        {
            case 1:
                /*ret[j++]=_Base[Udata.Sdata.d];
                ret[j++]=_Base[Udata.Sdata.c];
                ret[j++]=_Base[64];
                ret[j++]=_Base[64];*/
                ret[j++]=_Base[Udata.c[0]>>2];
                ret[j++]=_Base[((Udata.c[0]&0x03)<<4)|((Udata.c[1]&0xf0)>>4)];
                ret[j++]=_Base[64];
                ret[j++]=_Base[64];
                break;
            case 2:
                /*ret[j++]=_Base[Udata.Sdata.d];
                ret[j++]=_Base[Udata.Sdata.c];
                ret[j++]=_Base[Udata.Sdata.b];
                ret[j++]=_Base[64];*/
                ret[j++]=_Base[Udata.c[0]>>2];
                ret[j++]=_Base[((Udata.c[0]&0x03)<<4)|((Udata.c[1]&0xf0)>>4)];
                ret[j++]=_Base[((Udata.c[1]&0x0f)<<2)|((Udata.c[2]&0xc0)>>6)];
                ret[j++]=_Base[64];
                break;
            case 3:
                /*ret[j++]=_Base[Udata.Sdata.d];
                ret[j++]=_Base[Udata.Sdata.c];
                ret[j++]=_Base[Udata.Sdata.b];
                ret[j++]=_Base[Udata.Sdata.a];*/
                ret[j++]=_Base[Udata.c[0]>>2];
                ret[j++]=_Base[((Udata.c[0]&0x03)<<4)|((Udata.c[1]&0xf0)>>4)];
                ret[j++]=_Base[((Udata.c[1]&0x0f)<<2)|((Udata.c[2]&0xc0)>>6)];
                ret[j++]=_Base[Udata.c[2]&0x3f];
                break;
            default:
                break;
        }
    }
    ret[j]='\0';
    memcpy(reValue,ret,strlen(ret));
    free(ret);
    return 1;
}

/*********************************************************************
* name: Decbase64
* function:
* input:
* return:
*********************************************************************/
int Decbase64(char * orgdata,unsigned long orglen, char *buf)
{
    char *p = NULL, *ret = NULL;
    int len;
    char ch[4]={0};
    char *pos[4];
    int  offset[4];
    if (orgdata==NULL || orglen==0)
    {
        return -1;
    }
    len=orglen*3/4;
    if ((ret=(char *)malloc(len+1))==NULL)
    {
        return -1;
    }
    p=orgdata;
    len=orglen;
    int j=0;

    while(len>0)
    {
        int i=0;
        while(i<4)
        {
            if (len>0)
            {
                ch[i]=*p;
                p++;
                len--;
                if ((pos[i]=(char *)strchr(_Base,ch[i]))==NULL)
                {
                    free(ret);
                    return -1;
                }
                offset[i]=pos[i]-_Base;

            }
            i++;
        }
        if (ch[0]=='='||ch[1]=='='||(ch[2]=='='&&ch[3]!='='))
        {
            free(ret);
            return -1;
        }
        ret[j++]=(unsigned char)(offset[0]<<2|offset[1]>>4);
        ret[j++]=offset[2]==64?'\0':(unsigned char)(offset[1]<<4|offset[2]>>2);
        ret[j++]=offset[3]==64?'\0':(unsigned char)((offset[2]<<6&0xc0)|offset[3]);
    }
    ret[j]='\0';
    memcpy(buf, ret, strlen(ret));
    free(ret);
    return 1;
}

/*********************************************************************
* name: buildXmlNodeInfo
* function: build xml node information
* input: nodeName - node name, nodeValue - node value
* return: baseNode - xml node information
*********************************************************************/
int buildXmlNodeInfo(char *nodeName[], char nodeValue[][512], struct xmlNode *baseNode)
{
    int i;
    for(i = 0; nodeName[i] != NULL; i++)
    {
        strcpy(baseNode[i].name, nodeName[i]);
        strcpy(baseNode[i].content, nodeValue[i]);
    }
    baseNode[i].name[0] = '\0';
    baseNode[i].content[0] = '\0';

    return 0;
}

/*********************************************************************
* name: CreateXmlPacket
* function: create xml packet
* input: type - req or request, baseNode - xml node information
* return: xmlBuffer - xml packet
*********************************************************************/
int CreateXmlPacket(char *type, struct xmlNode *baseNode, char *xmlBuffer)
{
    int ret = 0;
    int i;

    /* Create xml file */
    xmlDocPtr doc = xmlNewDoc(NULL);
    /* Create xml root node */
    xmlNodePtr rootNode = xmlNewNode(NULL, (xmlChar *)type);
    xmlDocSetRootElement(doc, rootNode);

    for(i = 0; baseNode[i].name[0] != '\0'; i++)
    {
        xmlNodePtr node = xmlNewNode(NULL, (xmlChar *)(baseNode[i].name));
        xmlNodePtr content = xmlNewText((xmlChar *)(baseNode[i].content));
        xmlAddChild(rootNode, node);
        xmlAddChild(node, content);
        DEBUG("node name:%s, node content:%s\n", baseNode[i].name, baseNode[i].content);
    }

    //ret = xmlSaveFile("tempXml.xml", doc);
    //if(ret == -1)
    //{
    //    DEBUG("create xml file failure\n");
    //    return ret;
    //}

    xmlBufferPtr nodeBuffer = xmlBufferCreate();
    if(xmlNodeDump(nodeBuffer, doc, rootNode, 0, 0) > 0)
    {
        DEBUG("node use : %d\n", nodeBuffer->use);
        DEBUG("%s\n", (char *)nodeBuffer->content);
        strncpy(xmlBuffer, (char *)nodeBuffer->content, nodeBuffer->use);
        xmlBuffer[nodeBuffer->use] = '\0';
    }
    xmlBufferFree(nodeBuffer);
    xmlFreeDoc(doc);

    return ret;
}

/*********************************************************************
* name: GetXmlNode
* function: get xml node information from xml packet
* input: xmlBuffer - xml packet, type - req or response
* return: node - node name, nodeBuffer - node value
*********************************************************************/
int GetXmlNode(char *xmlBuffer, char *type, char *node, char *nodeBuffer)
{
    int ret = 0;
    xmlDocPtr doc;
    xmlNodePtr xmlRoot;
    xmlNodePtr xmlChildren;

    doc = xmlParseMemory(xmlBuffer, strlen(xmlBuffer));
    if(!doc)
    {
        return -1;
    }

    xmlRoot = xmlDocGetRootElement(doc);
    if(!xmlRoot)
    {
        DEBUG("%s:don't find root node\n", __func__);
        xmlFreeDoc(doc);
        return -1;
    }
    if((type != NULL) && (xmlStrcmp(xmlRoot->name, (xmlChar *)(type))))
    {
        DEBUG("%s:root node error\n", __func__);
        xmlFreeDoc(doc);
        return -1;
    }

    xmlChildren = xmlRoot->xmlChildrenNode;
    while(xmlChildren != NULL)
    {
        if(!xmlStrcmp(xmlChildren->name, (xmlChar *)node))
        {
            strcpy(nodeBuffer, (char *)xmlNodeGetContent(xmlChildren));
            break;
        }
        xmlChildren = xmlChildren->next;
    }

    xmlFreeDoc(doc);
    return ret;
}

/*********************************************************************
* name: GetXmlRootNodeName
* function: get root node name from xml packet
* input: xmlBuffer - xml packet
* return: name - node name
*********************************************************************/
int GetXmlRootNodeName(char *xmlBuffer, char *name)
{
    int ret = 0;
    xmlDocPtr doc;
    xmlNodePtr xmlRoot;

    doc = xmlParseMemory(xmlBuffer, strlen(xmlBuffer));
    if(!doc)
    {
        return -1;
    }

    xmlRoot = xmlDocGetRootElement(doc);
    if(!xmlRoot)
    {
        DEBUG("%s:don't find root node\n", __func__);
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(name, (char *)(xmlRoot->name));

    xmlFreeDoc(doc);
    return ret;
}

/*********************************************************************
* name: ExtractXmlContent
* function: extract xml content from original data
* input: data - original data
* return: xmlBuffer - xml packet, xmlLen - length of xml packet
*********************************************************************/
int ExtractXmlContent(char *data, char *xmlBuffer, int *xmlLen)
{
    int ret = 0;
    xmlDocPtr doc;
    xmlNodePtr xmlRoot;

    doc = xmlParseMemory(xmlBuffer, strlen(xmlBuffer));
    if(!doc)
    {
        return -1;
    }

    xmlRoot = xmlDocGetRootElement(doc);
    if(!xmlRoot)
    {
        DEBUG("%s:don't find root node\n", __func__);
        xmlFreeDoc(doc);
        return -1;
    }

    xmlBufferPtr nodeBuffer = xmlBufferCreate();
    if(xmlNodeDump(nodeBuffer, doc, xmlRoot, 0, 0) > 0)
    {
        strncpy(xmlBuffer, (char *)nodeBuffer->content, nodeBuffer->use);
        *xmlLen = nodeBuffer->use;
    }
    xmlBufferFree(nodeBuffer);
    xmlFreeDoc(doc);

    return ret;
}

/*********************************************************************
* name: libxmltest
* function: test function for xml library
* input:
* return:
*********************************************************************/
int libxmltest(void)
{
    int ret;
    int i;
    char buff[512];
    char nodebuff[32];
    char* nodeName[] = {"test1", "test2", NULL};
    char value[2][512];
    struct xmlNode baseNode[3];

    strcpy(value[0], "tina");
    strcpy(value[1], "dragonmat");
    buildXmlNodeInfo(nodeName, value, baseNode);

    ret = CreateXmlPacket("req", baseNode, buff);
    if(ret != -1)
        printf("create xml file, size is %d\n", ret);

    printf("XML Packate:\n");
    printf("%s\n", buff);

    ret = GetXmlNode(buff, "req", "test1", nodebuff);
    if(ret == -1)
    {
        printf("Get XML Node %s failure\n", "test1");
        return -1;
    }

    printf("XML Node:\n");
    printf("Node: %s\n", nodebuff);

    return 1;
}
