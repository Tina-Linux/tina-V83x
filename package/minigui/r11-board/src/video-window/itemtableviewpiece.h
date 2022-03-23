#ifndef  ITEMTABLEVIEW_INC
#define  ITEMTABLEVIEW_INC

#ifdef __cplusplus
extern "C" {
#endif
#define REGISTER_NCS() \
    MGNCS_INIT_CLASS(mWidgetHostPiece);

#define UNREGISTER_NCS()

typedef struct _mItemTableView mItemTableView;
typedef struct _mItemTableViewClass mItemTableViewClass;

#define mItemTableViewHeader(clss) \
    mTableViewPieceHeader(clss) \
    void* data;

#define mItemTableViewClassHeader(clss, superCls) \
    mTableViewPieceClassHeader(clss, superCls)
struct _mItemTableView
{
    mItemTableViewHeader(mItemTableView)
};

struct _mItemTableViewClass
{
    mItemTableViewClassHeader(mItemTableView, mTableViewPiece)
};

MGNCS_EXPORT extern mItemTableViewClass g_stmItemTableViewViewCls;

#ifdef __cplusplus
}
#endif

#endif   /* ----- #ifndef MSIMPLETABLEVIEW_INC  ----- */

