#ifndef  MSIMPLETABLEVIEW_INC
#define  MSIMPLETABLEVIEW_INC

#ifdef __cplusplus
extern "C" {
#endif
#define REGISTER_NCS() \
    MGNCS_INIT_CLASS(mWidgetHostPiece);

#define UNREGISTER_NCS()

typedef struct _mSimpleTableView mSimpleTableView;
typedef struct _mSimpleTableViewClass mSimpleTableViewClass;

#define mSimpleTableViewHeader(clss) \
    mTableViewPieceHeader(clss) \
    void* data; \
	int id_type;

#define mSimpleTableViewClassHeader(clss, superCls) \
    mTableViewPieceClassHeader(clss, superCls)

struct _mSimpleTableView
{
    mSimpleTableViewHeader(mSimpleTableView)
};

struct _mSimpleTableViewClass
{
    mSimpleTableViewClassHeader(mSimpleTableView, mTableViewPiece)
};

MGNCS_EXPORT extern mSimpleTableViewClass g_stmSimpleTableViewCls;

#ifdef __cplusplus
}
#endif

#endif   /* ----- #ifndef MSIMPLETABLEVIEW_INC  ----- */

