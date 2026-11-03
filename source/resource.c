#include "resource.h"

BOOL FetchResource(WORD resourceId, void** outBytes, DWORD* outSize)
{
    if (outBytes == NULL || outSize == NULL)
    {
        return FALSE;
    }

    HRSRC resource = FindResourceW(NULL, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (resource == NULL)
    {
        return FALSE;
    }

    HGLOBAL loaded = LoadResource(NULL, resource);
    if (loaded == NULL)
    {
        return FALSE;
    }

    DWORD size = SizeofResource(NULL, resource);
    void* bytes = LockResource(loaded);

    if (bytes == NULL || size == 0)
    {
        return FALSE;
    }

    *outBytes = bytes;
    *outSize = size;
    return TRUE;
}
