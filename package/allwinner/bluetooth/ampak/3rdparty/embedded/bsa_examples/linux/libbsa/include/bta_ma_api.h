/*****************************************************************************
**
**  Name:           bta_ma_api.h
**
**  Description:    This file contains the common API functions used
**                  for the Message Access Profiles (MAP).
**
**  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef BTA_MA_API_H
#define BTA_MA_API_H

#include "bta_mse_api.h"        /* For tBTA_MA_MSG_TYPE */
#include <stdio.h>

typedef struct
{
    UINT16              size;       /* Size of the buffer */
    UINT8 *             p_buffer;   /* Pointer to buffer */
    UINT8 *             p_next;     /* Pointer to next byte to use in buffer */

}tBTA_MA_MEM_STREAM;

typedef struct
{
    int   fd;

}tBTA_MA_FILE_STREAM;

/* Structure used for streaming data */
typedef struct
{
#define STRM_TYPE_MEMORY        0
#define STRM_TYPE_FILE          1

    UINT8               type;

#define STRM_SUCCESS            0
#define STRM_ERROR_OVERFLOW     1
#define STRM_ERROR_FILE         2

    UINT8               status;

    union
    {
        tBTA_MA_MEM_STREAM  mem;
        tBTA_MA_FILE_STREAM file;
    } u;

} tBTA_MA_STREAM;

/*******************************************************************************
**
** bMessage functions
**
** Description      The following API functions are generic in a sense that
**                  they do not imply how the data is stored (to memory or
**                  to file, etc.).
**
**                  They operate on a generic set of structure types.  Though
**                  the internal structure of those types are implementation
**                  specific.
**
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
**
** Function         BTA_MaBmsgCreate
**
** Description      Create and initialize an instance of a tBTA_MA_BMSG structure.
**
** Parameters       None
**
** Returns          Pointer to a bMessage object, or NULL if this fails.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG *BTA_MaBmsgCreate(void);

/*******************************************************************************
**
** Function         BTA_MaBmsgFree
**
** Description      Destroy (free) the contents of a tBTA_MA_BMSG structure.
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgFree(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetReadSts
**
** Description      Set the bmessage-readstatus-property value for the bMessage
**                  object. If the 'read_sts' is TRUE then value will be "READ",
**                  otherwise it is "UNREAD".
**
** Parameters       p_bmsg - Pointer to a bMessage object
**                  read_sts - Read status TRUE- read FALSE - unread
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetReadSts(tBTA_MA_BMSG *p_msg, BOOLEAN read_sts);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetReadSts
**
** Description      Get the bmessage-readstatus-property value for the bMessage
**                  object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Read status (TRUE/FALSE) for the specified bMessage.
**
*******************************************************************************/
BTA_API extern BOOLEAN  BTA_MaBmsgGetReadSts(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetMsgType
**
** Description      Set the bmessage-type-property value for the bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**                  msg_type - Message type
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetMsgType(tBTA_MA_BMSG *p_msg, tBTA_MA_MSG_TYPE msg_type);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetMsgType
**
** Description      Get the bmessage-type-property value for the specified
**                  bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Message type
**
*******************************************************************************/
BTA_API extern tBTA_MA_MSG_TYPE BTA_MaBmsgGetMsgType(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetFolder
**
** Description      Set the bmessage-folder-property value for the bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**                  p_folder - Pointer to a folder path
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetFolder(tBTA_MA_BMSG *p_msg, char *p_folder);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetFolder
**
** Description      Get the bmessage-folder-property value for the specified
**                  bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Pointer to folder path string, or NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern char *BTA_MaBmsgGetFolder(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddOrigToBmsg
**
** Description      Add an originator to the bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Pointer to a new vCard structure, or NULL if this function
**                  fails.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_VCARD *BTA_MaBmsgAddOrigToBmsg(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetOrigFromBmsg
**
** Description      Get the first originator vCard information from the specified
**                  bMessage object
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Pointer to first 'originator vCard, or NULL not used.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_VCARD *BTA_MaBmsgGetOrigFromBmsg(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddEnvToBmsg
**
** Description      Add a new envelope to the bMessage object. This is the first
**                  (top) level envelope. bMessage allows up to 3 levels of envelopes.
**                  application should call BTA_MaBmsgAddEnvToEnv to add the 2nd
**                  3rd level enevelope.
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Pointer to a new envelope structure, or NULL if this
**                  function fails.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_ENVELOPE *BTA_MaBmsgAddEnvToBmsg(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddEnvToEnv
**
** Description      Add a child envelope to an existing envelope.
**
** Parameters       p_envelope - Pointer to a parent envelope
**
** Returns          Pointer to an envelope structure, or NULL if this
**                  function fails.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_ENVELOPE *BTA_MaBmsgAddEnvToEnv(tBTA_MA_BMSG_ENVELOPE *p_envelope);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetEnv
**
** Description      Get the pointer of the first level envelope.
**
** Parameters       p_bmsg - Pointer to a bMessage object
**
** Returns          Pointer to the first level envelope structure, or NULL if it
**                  does not exist
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_ENVELOPE *BTA_MaBmsgGetEnv(tBTA_MA_BMSG *p_msg);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetNextEnv
**
** Description      Get the child envelope of the specified parent envelope.
**
** Parameters       p_env - Pointer to a parent envelope
**
** Returns          Pointer to a child enevelope. NULL if the
**                  envelope does not have a 'child' envelope.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_ENVELOPE *BTA_MaBmsgGetNextEnv(tBTA_MA_BMSG_ENVELOPE *p_env);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddRecipToEnv
**
** Description      Add recipient to the specified envelope.
**
** Parameters       p_env - Pointer to a envelope
**
** Returns          Pointer to a vCard structure. NULL if it
**                  fails to allocate a vCard structure.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_VCARD *BTA_MaBmsgAddRecipToEnv(tBTA_MA_BMSG_ENVELOPE *p_env);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetRecipFromEnv
**
** Description      Get the first recipient's vCard from the specified envelope.
**
** Parameters       p_env - Pointer to a envelope
**
** Returns          Pointer to the first recipient's vCard structure. NULL if it
**                  has not be set.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_VCARD *BTA_MaBmsgGetRecipFromEnv(tBTA_MA_BMSG_ENVELOPE *p_env);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddBodyToEnv
**
** Description      Add a message body to the specified envelope.
**
** Parameters       p_env - Pointer to a envelope
**
** Returns          Pointer to a message body structure.
**                  NULL if it fails to allocate a message body structure.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_BODY *BTA_MaBmsgAddBodyToEnv(tBTA_MA_BMSG_ENVELOPE *p_env);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyFromEnv
**
** Description      Get the message body pointer from the specified envelope.
**
** Parameters       p_env - Pointer to a envelope
**
** Returns          Pointer to a message body structure.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_BODY *BTA_MaBmsgGetBodyFromEnv(tBTA_MA_BMSG_ENVELOPE *p_env);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetBodyEncoding
**
** Description      Set the bmessage-body-encoding-property value for the bMessage
**                  body.
**
** Parameters       p_body - Pointer to a bMessage body
**                  encoding - encoding scheme
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetBodyEncoding(tBTA_MA_BMSG_BODY *p_body, tBTA_MA_BMSG_ENCODING encoding);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyEncoding
**
** Description      Get the bmessage-body-encoding-property value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          Message encoding scheme
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_ENCODING BTA_MaBmsgGetBodyEncoding(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetBodyPartid
**
** Description      Set the bmessage-body-part-ID value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**                  part_id - Part ID (range: from 0 to 65535)
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetBodyPartid(tBTA_MA_BMSG_BODY *p_body, UINT16 part_id);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyPartid
**
** Description      Get the bmessage-body-part-ID value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          The value of the Part ID
**
*******************************************************************************/
BTA_API extern UINT16 BTA_MaBmsgGetBodyPartid(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgIsBodyMultiPart
**
** Description      Is this a multi-part body
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          TURE - if this is a multi-part body
**
*******************************************************************************/
BTA_API extern BOOLEAN BTA_MaBmsgIsBodyMultiPart(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyCharset
**
** Description      Get the bmessage-body-charset-property value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          Charset
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetBodyCharset(tBTA_MA_BMSG_BODY *p_body, tBTA_MA_CHARSET charset);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyCharset
**
** Description      Get the bmessage-body-charset-property value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          Charset
**
*******************************************************************************/
BTA_API extern tBTA_MA_CHARSET BTA_MaBmsgGetBodyCharset(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetBodyLanguage
**
** Description      Set the bmessage-body-language-property value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**                  Language - the language of the message
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetBodyLanguage(tBTA_MA_BMSG_BODY *p_body, tBTA_MA_BMSG_LANGUAGE language);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetBodyLanguage
**
** Description      Get the bmessage-body-language-property value for the specified
**                  bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          the language of the message
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_LANGUAGE BTA_MaBmsgGetBodyLanguage(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddContentToBody
**
** Description      Add a message content to the specified bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          Pointer to a message content.
**                  NULL if it fails to allocate a message content buffer
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_CONTENT *BTA_MaBmsgAddContentToBody(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetContentFromBody
**
** Description      Get a message content from the specified bMessage body.
**
** Parameters       p_body - Pointer to a bMessage body
**
** Returns          Pointer to a message content.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_CONTENT *BTA_MaBmsgGetContentFromBody(tBTA_MA_BMSG_BODY *p_body);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetNextContent
**
** Description      Get the next message content from the specified message content.
**
** Parameters       p_content - Pointer to a message content
**
** Returns          Pointer to a message content.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_CONTENT *BTA_MaBmsgGetNextContent(tBTA_MA_BMSG_CONTENT *p_content);

/*******************************************************************************
**
** Function         BTA_MaBmsgAddMsgContent
**
** Description      Add a text string to the specified message content.
**
** Parameters       p_content - Pointer to a message content
**                  p_text - Pointer to a text string
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgAddMsgContent(tBTA_MA_BMSG_CONTENT *p_content, char *p_text);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetMsgContent
**
** Description      Get the next text string from the specified message content.
**
** Parameters       p_content - Pointer to a message content
**
** Returns          Pointer to the next text string.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern char *BTA_MaBmsgGetMsgContent(tBTA_MA_BMSG_CONTENT *p_content);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetNextVcard
**
** Description      Get the next vCard from the specified vCard.
**
** Parameters       p_vcard - Pointer to a vCard
**
** Returns          Pointer to the next vCard.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern char *BTA_MaBmsgGetNextMsgContent(tBTA_MA_BMSG_CONTENT *p_content);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetNextVcard
**
** Description      Get the next vCard from the specified vCard.
**
** Parameters       p_vcard - Pointer to a vCard
**
** Returns          Pointer to the next vCard.
**                  NULL if it has not been set.
**
*******************************************************************************/
BTA_API extern tBTA_MA_BMSG_VCARD *BTA_MaBmsgGetNextVcard(tBTA_MA_BMSG_VCARD *p_vcard);

/*******************************************************************************
**
** Function         BTA_MaBmsgSetVcardVersion
**
** Description      Set the vCard version for the specified vCard.
**
** Parameters       p_vcard - Pointer to a vCard
**                  version - vcard version
**
** Returns          None
**
*******************************************************************************/
BTA_API extern void BTA_MaBmsgSetVcardVersion(tBTA_MA_BMSG_VCARD *p_vcard, tBTA_MA_VCARD_VERSION version);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetVcardVersion
**
** Description      Get the vCard version from the specified vCard.
**
** Parameters       p_vcard - Pointer to a vCard
**
** Returns          vCard version number
**
*******************************************************************************/
BTA_API extern tBTA_MA_VCARD_VERSION BTA_MaBmsgGetVcardVersion(tBTA_MA_BMSG_VCARD *p_vcard);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetVcardProp
**
** Description      Get the vCard property from the specified vCard property enum.
**
** Parameters       p_vcard - Pointer to a vCard
**                  prop - Indicate which vCard property
**
** Returns          Pointer to the vCard property.
**                  NULL if the vCard property does not exist
**
*******************************************************************************/
BTA_API extern tBTA_MA_VCARD_PROPERTY *BTA_MaBmsgAddVcardProp(tBTA_MA_BMSG_VCARD *p_vcard,
                                                              tBTA_MA_VCARD_PROP prop, char *p_value,
                                                              char *p_param);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetVcardProp
**
** Description      Get the next vCard property from the specified vCard property.
**
** Parameters       p_prop - Pointer to a vCard property
**
** Returns          Pointer to the next vCard property.
**                  NULL if the next vCard property does not exist
**
*******************************************************************************/
BTA_API extern tBTA_MA_VCARD_PROPERTY *BTA_MaBmsgGetVcardProp(tBTA_MA_BMSG_VCARD *p_vcard,tBTA_MA_VCARD_PROP prop);
/*******************************************************************************
**
** Function         BTA_MaBmsgGetNextVcardProp
**
** Description      Get the next vCard property from the specified vCard property.
**
** Parameters       p_prop - Pointer to a vCard property
**
** Returns          Pointer to the next vCard property.
**                  NULL if the next vCard property does not exist
**
*******************************************************************************/
BTA_API extern tBTA_MA_VCARD_PROPERTY *BTA_MaBmsgGetNextVcardProp(tBTA_MA_VCARD_PROPERTY *p_prop);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetVcardPropValue
**
** Description      Get the vCard property value from the specified vCard property.
**
** Parameters       p_prop - Pointer to a vCard property
**
** Returns          Pointer to the vCard property value.
**                  NULL if the vCard property value has not been set.
**
*******************************************************************************/
BTA_API extern char *BTA_MaBmsgGetVcardPropValue(tBTA_MA_VCARD_PROPERTY *p_prop);

/*******************************************************************************
**
** Function         BTA_MaBmsgGetVcardPropParam
**
** Description      Get the vCard property parameter from the specified vCard property.
**
** Parameters       p_prop - Pointer to a vCard property
**
** Returns          Poiter to the vCard property parameter.
**                  NULL if the vCard property parameter has not been set.
**
*******************************************************************************/
BTA_API extern char *BTA_MaBmsgGetVcardPropParam(tBTA_MA_VCARD_PROPERTY *p_prop);

/*******************************************************************************
**
** Function         BTA_MaBuildMapBmsgObj
**
** Description      Builds a specification compliant bMessage object given a
**                  generic bMessage internal structure.
**
** Parameters       p_msg - pointer to bMessage object structure (input).
**                  p_stream - Output stream.
**
** Returns          BTA_MA_STATUS_OK if successful.  BTA_MA_STATUS_FAIL if not.
**
*******************************************************************************/
BTA_API extern tBTA_MA_STATUS BTA_MaBuildMapBmsgObj(tBTA_MA_BMSG * p_msg, tBTA_MA_STREAM *p_stream);

/*******************************************************************************
**
** Function         bta_ma_parse_map_bmsg_obj
**
** Description      Parses a bMessage object from a stream into a generic
**                  bMessage internal structure.
**
** Parameters       p_msg - pointer to bMessage object structure (output).
**                  p_stream - Input stream.
**
** Returns          BTA_MA_STATUS_OK if successful.  BTA_MA_STATUS_FAIL if not.
**
*******************************************************************************/
BTA_API extern tBTA_MA_STATUS BTA_MaParseMapBmsgObj(tBTA_MA_BMSG * p_msg, tBTA_MA_STREAM *p_stream);


/*******************************************************************************
**
** Function         BTA_MaInitMemStream
**
** Description      Initializes a memory based stream
**
** Parameters       p_stream - pointer to stream information.
**                  p_buffer - pointer to buffer to be manipulated.
**                  size - size of buffer pointed to by 'p_buffer'.
**
** Returns          TRUE if stream is successfully initialized
**
*******************************************************************************/
BTA_API extern BOOLEAN BTA_MaInitMemStream(tBTA_MA_STREAM *p_stream, UINT8 *p_buffer, UINT16 size);

/*******************************************************************************
**
** Function         BTA_MaInitFileStream
**
** Description      Initializes a file stream
**
** Parameters       p_stream - pointer to stream information.
**                  p_filename - Full pathname to file to use.
**                  oflags - permissions and mode (see constants above)
**
** Returns          void
**
*******************************************************************************/
BTA_API  extern BOOLEAN BTA_MaInitFileStream(tBTA_MA_STREAM * p_stream, const char *p_path, int oflags);

/*******************************************************************************
**
** Function         BTA_MaCloseStream
**
** Description      Close a stream (do any necessary clean-up.
**
** Parameters       p_stream - pointer to stream information.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern  void BTA_MaCloseStream(tBTA_MA_STREAM *p_stream);


#ifdef __cplusplus
}
#endif

#endif /* BTA_BMSG_API_H */
