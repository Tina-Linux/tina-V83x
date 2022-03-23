#ifndef __INFO_H__
#define __INFO_H__
#include <string>

namespace softwinner{

class DLNADeviceInfo
{
public:
    DLNADeviceInfo();
    ~DLNADeviceInfo();
    //for DLNA
    std::string name;
    std::string uuid;
    std::string device_id;
    std::string manufacture_id;
    std::string pre_shared_key;
    unsigned int qplay_ver;

    const static int STRING_NUM = 5;
    DLNADeviceInfo& operator=(const DLNADeviceInfo& info){

        name = info.name;
        uuid = info.uuid;
        manufacture_id = info.manufacture_id;
        pre_shared_key = info.pre_shared_key;
        qplay_ver = info.qplay_ver;
        return *this;
    }

    void Serialization(char** serial, int* serial_len);
    static DLNADeviceInfo unSerialization(char* data, int len);

private:
    char* serializable;
    int serializable_len;

};
class DLNAMediaInfo
{
public:
    std::string Objectclass;
    /* 媒体对象ID */
    std::string ObjectID;
    /* 媒体对象的父节点媒体对象ID */
    std::string ParentID;

    /* 该媒体对象的标题 */
    std::string Title;
    /* 该媒体对象的创建者 */
    std::string Creator;
    /* 该媒体对象的创建日期 */
    std::string Date;

    /*描述整个mediaObject的xml字符串*/
    std::string Meta;

    /* Album info */
    std::string Album;
    std::string AlbumArtUri;

    const static int STRING_NUM = 9;

    DLNAMediaInfo& operator=(const DLNAMediaInfo& info){

        Objectclass = info.Objectclass;
        ObjectID = info.ObjectID;
        ParentID = info.ParentID;
        Title = info.Title;
        Creator = info.Creator;
        Date = info.Date;
        Meta = info.Meta;
        Album = info.Album;
        AlbumArtUri = info.AlbumArtUri;
        return *this;
    }

    //TODO
    /*
    void Serialization(char** serial, int* serial_len);
    static DLNAMediaInfo unSerialization(char* data, int len);
    */
};

/* 解码器使用 */
class MediaItemResource
{
    /* 资源Uri地址 */
    std::string Uri;
    /* 协议信息 */
    std::string ProtocolInfo;
    /* 播放长度，单位秒*/
    int Duration;
    /* 文件大小 */
    long Size;
    /* 比特率 */
    int Bitrate;
    /* 采样比特位 */
    int BitsPerSample;
    /* 采样率 */
    int SampleFrequency;
    /* 音频声道数 */
    int NbAudioChannels;

    /* Ifo 文件的Url地址 */
    std::string IfoFileUrl;
    /* 导入Ifo文件的Url地址 */
    std::string ImportIfoFileUrl;
};

}/*namespace softwinner*/
#endif /*__INFO_H__*/
