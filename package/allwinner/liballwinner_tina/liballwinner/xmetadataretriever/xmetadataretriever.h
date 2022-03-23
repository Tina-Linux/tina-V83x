
#ifndef X_METADATA_RETRIEVER
#define X_METADATA_RETRIEVER

#ifdef __cplusplus
extern 'C' {
#endif

#include "vdecoder.h"           //* video decode library in "LIBRARY/CODEC/VIDEO/DECODER"
#include "CdxParser.h"          //* parser library in "LIBRARY/DEMUX/PARSER/include/"
#include "CdxStream.h"          //* parser library in "LIBRARY/DEMUX/STREAM/include/"

typedef struct VideoFrame
{
    // Intentional public access modifier:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mDisplayWidth;
    uint32_t mDisplayHeight;
    uint32_t mSize;            // Number of bytes in mData
    uint8_t* mData;            // Actual binary data
    int32_t  mRotationAngle;   // rotation angle, clockwise
}VideoFrame;

enum MetaDataType
{
    METADATA_VIDEO_WIDTH     = 0,
    METADATA_VIDEO_HEIGHT    = 1,
	METADATA_DURATION       = 2,  // file duration (ms)
};

#define MediaScanDedug (0)

typedef void* AwRetriever;

AwRetriever* AwRetrieverCreate();

int  AwRetrieverDestory(AwRetriever* p);

int   AwRetrieverSetDataSource(AwRetriever* p, const char* pUrl);
VideoFrame* AwRetrieverGetFrameAtTime(AwRetriever* p, int64_t timeUs);
int AwRetrieverGetMetaData(AwRetriever* p, int type, void* pVal);


#ifdef __cplusplus
}
#endif

#endif
