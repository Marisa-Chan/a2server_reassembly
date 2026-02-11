#include "resource.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "file.h"

//6a6a20
Resources Resources::ResFiles;

//6a6a60
FatEntry* ResBase::g_lastDstRoot = nullptr;


uint16_t Resources::GetPathType(const char* path)
{
	// 4e6de0

	WIN32_FIND_DATAA fnd;
	HANDLE hfind = FindFirstFileA(path, &fnd);
	FindClose(hfind);
	if (hfind == INVALID_HANDLE_VALUE)
	{
		char letter = tolower(*path);

		char buf[104];
		DWORD num = GetLogicalDriveStringsA(100, buf);
		for (DWORD i = 0; i < num; i++)
		{
			if (isalpha(buf[i]))
				buf[i] = tolower(buf[i]);
		}
		if (isalnum(letter) &&
			strcmp(path + 1, ":\\") == 0 &&
			memchr(buf, letter, num - 2) != NULL)
			return 2;

		return 0;
	}
	else if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return 1;
	
	return 2;
}

Resources::Resources()
{
	//4e5e06

	char path[260];
	GetCurrentDirectoryA(256, path);
	AddPath(path);
}

Resources::~Resources()
{
    //4e5e9f

    for (uint32_t i = 0; i < res_files.GetSize(); i++)
    {
        if (res_files[i] != nullptr)
            delete res_files[i];
    }

    for (uint32_t i = 0; i < files.GetSize(); i++)
    {
        if (files[i] != nullptr)
            delete files[i];
    }
}

void Resources::AddPath(const char* path)
{
	//4e6273

	if (GetPathType(path) != 2)
		AfxThrowFileException(CFileException::badPath);

	char buf[1024];
	pathStrCpy(path, buf);

	field_0x14.Add(buf);
}

void Resources::AddResFile(const char* path)
{
	//4e5950
	char buf[1024];
	pathStrCpy(path, buf);

	if (field_0x14.GetSize() <= 0)
		AfxThrowFileException(CFileException::fileNotFound);

	char pbuf[256];
	strcpy(pbuf, (const char*)field_0x14[0]);
	size_t len = strlen(pbuf);
	if (pbuf[len - 1] != '\\' || pbuf[len - 2] != ':')
		strcat(pbuf, "\\");
	strcat(pbuf, buf);

	res_files.Add(new ResFile(pbuf));
}




void Resources::StaticAddPath(const char* path)
{
	//4e6242
	char buf[1024];
	pathStrCpy(path, buf);
	ResFiles.AddPath(buf);
}

void Resources::StaticAddResFile(const char* path)
{
	//4e6211
	char buf[1024];
	pathStrCpy(path, buf);
	ResFiles.AddResFile(buf);
}


FatInfo* Resources::FUN_004e646d(const char* fpath)
{
    //4e646d
    char buf[1024];
    pathStrCpy(fpath, buf);

    for (uint32_t i = 0; i < res_files.GetSize(); i++)
    {
        FatInfo* inf = res_files[i]->FUN_004e5d55(buf);
        if (inf != nullptr)
        {
            if (inf != FATINFO_T1)
                return inf;
            break;
        }
    }

    CFile* cfile = new CFile();
    for (uint32_t i = 0; i < field_0x14.GetSize(); i++)
    {
        char dbuf[256];
        strcpy(dbuf, field_0x14[i]);

        int ln = strlen(dbuf);
        if (dbuf[ln - 1] != '\\' || dbuf[ln - 2] != ':')
            strcat(dbuf, "\\");

        strcat(dbuf, buf);

        CFileException exc;
        if (cfile->Open(dbuf, 0, &exc) == FALSE)
        {
            if (exc.m_cause != CFileException::fileNotFound && exc.m_cause != CFileException::badPath)
                AfxThrowFileException(exc.m_cause);
        }
        else
        {
            files.Add(cfile);
            FatInfo* inf = new FatInfo();
            inf->file = cfile;
            inf->offset = 0;
            inf->size = cfile->GetLength();
            return inf;
        }
    }

    if (cfile)
        delete cfile;

    return nullptr;
}

void Resources::FUN_004e6763(CFile* fil)
{
    //4e6763
    for (int i = 0; i < files.GetSize(); i++)
    {
        if (files[i] == fil)
        {
            delete fil;
            files.RemoveAt(i);
            break;
        }
    }
}

void Resources::FUN_004e6409(const char* fname)
{
    //4e6409
    for (int i = 0; i < res_files.GetSize(); i++)
    {
        FatEntry *fat = res_files[i]->FUN_004eca63(fname);
        if (fat)
        {
            fat->flags |= 0x20000000;
            break;
        }
    }
}

void Resources::FUN_004e632d(const char* fname)
{
    //4e632d
    FILE* f = fopen(fname, "r");
    if (!f)
        return;

    char bufline[256];
    while (fgets(bufline, 255, f) && isprint(bufline[0]))
    {
        char* pc = strchr(bufline, '\n');
        if (pc)
            *pc = 0;

        pc = strchr(bufline, '\r');
        if (pc)
            *pc = 0;

        ResFiles.FUN_004e6409(bufline);
    }
    fclose(f);
}




ResFile::ResFile(const char* fpath)
{
    //4e5950
    modify_fat = nullptr;

    char buf[260];
    strcpy(buf, fpath);

    const char *puVar1 = strchr(fpath, '.');

    if (puVar1 == NULL)
        strcat(buf, ".res");    

    field_0x3c = 0;

    
    file = new CFile(buf, field_0x3c | CFile::shareDenyNone);

    char *pext = strchr(buf, '.');
    *pext = 0;

    char* pchar = pext;
    while (*pchar != '\\' && *pchar != '/' && pchar != buf)
    {
        pchar--;
    }

    if (pchar != buf)
        pchar++;

    root.SetName(pchar);

    *pext = '.';

    file->Read(&root.magic, 4);

    if (root.magic != 0x31415926)
        AfxThrowFileException(CFileException::generic);
    
    file->Read(&root.offset, 4);
    file->Read(&root.size, 4);
    file->Read(&root.flags, 4);
    file->Read(&fat_offset, 4);
    file->Read(&fat_size, 4);

    if ((field_0x3c & 0xf) == 0)
        field_0x3c = 0;
    else
        field_0x3c = 1;

    if (field_0x3c == 0)
    {
        size = fat_size;
        fat_data = (FatEntry*)malloc(size * sizeof(FatEntry));

        if (fat_data == nullptr)
            AfxThrowMemoryException();
    }
    else 
    {
        size = fat_size + 10000 + (fat_size / 4);
        fat_data = (FatEntry*)malloc(size * sizeof(FatEntry));

        if (fat_data == nullptr)
            AfxThrowMemoryException();
    }

    if ((root.flags & 0x80000000) == 0)
        file->Seek(fat_offset, 0);

    file->Read(fat_data, fat_size << 5);

    modify_fat = nullptr;
    field_0x30 = 0;
}

ResFile::~ResFile()
{
    //4e5d0b
    if (fat_data)
        free(fat_data);

    if (file)
        delete file;
}


RegFile::RegFile()
{
    //4e84c0
    modify_fat = nullptr;
    fat_offset = 0x20;

    pdata = (uint8_t *)malloc(fat_offset);

    if (pdata == nullptr)
        AfxThrowMemoryException();

    field_0x3c = 0;
    size = 4;

    fat_data = (FatEntry *)malloc(size * sizeof(FatEntry));

    if (fat_data == nullptr)
        AfxThrowMemoryException();

    fat_size = 0;
    root.SetName("");
    root.magic = 0x31415926;
    root.offset = 0;
    root.size = 0;
    root.flags = 1;
    field_0x30 = 0;
}


RegFile::RegFile(const char* fpath)
{
    //4e8585

    modify_fat = nullptr;

    File2 *cfile = new File2();

    CFileException exc;
    if (cfile->Open(fpath, 0, &exc) == FALSE)
    {
        if (cfile)
            delete cfile;
        
        char buf[256];
        sprintf(buf, " Could`t create registry file :%d", exc.m_cause);
        throw std::runtime_error(buf);
    }

    const char *pdot = strchr(fpath, '.');
    const char* pcur = pdot - 1;

    while (*pcur != '\\' && *pcur != '/' && pcur != fpath)
        pcur--;

    if (pcur != fpath)
        pcur++;

    int cpylen = pdot - pcur;

    if (cpylen > 15)
        cpylen = 15;

    strncpy(root.name, pcur, cpylen);

    file = nullptr;
    fat_data = nullptr;

    ReadFromFile(cfile);

    if (cfile != nullptr)
        delete cfile;
}

RegFile::~RegFile()
{
    //4e9989
    if (fat_data)
        free(fat_data);

    if (pdata)
        free(pdata);
}




FatEntry* ResBase::Find(FatEntry* node, const char* fname)
{
    //4eca9e
    if (!node)
        return nullptr;

    FatEntry* searchBase = fat_data + node->offset;

    if ((node->flags & 0x10) != 0 && node->size != 0)
    {
        FatEntry tmp;
        tmp.SetName(fname);
        return (FatEntry*)bsearch(&tmp, searchBase, node->size, sizeof(FatEntry), FatEntry::NameCompare);
    }

    for (int i = 0; i < node->size; i++)
    {
        if (_strnicmp(fname, searchBase[i].name, 15) == 0)
            return &searchBase[i];
    }

    return nullptr;
}


FatEntry* ResBase::FindNext(FatEntry* node, const char** fpath)
{
    //4ecb78
    
    // 10.02.2026
    // fix for const char use
    char buf[256];
    int buflen = 0;

    const char* pchar = *fpath;
    while (*pchar != '\\' && *pchar != '/' && *pchar != 0)
    {
        buf[buflen] = *pchar;
        buflen++;

        pchar++;
    }

    buf[buflen] = 0;

    FatEntry *node_find = Find(node, buf);

    if (*pchar == 0)
        *fpath = pchar;
    else
        *fpath = pchar + 1;

    return node_find;
}

FatEntry* ResBase::TracePath(const char* fpath)
{
    //4ec956
    for (char* pchar = root.name; *fpath != '\\' && *fpath != '/' && *fpath != 0 && *pchar != 0; fpath++, pchar++)
    {
        if (*fpath != *pchar)
            return nullptr;
    }

    if (*fpath != 0)
        fpath++;

    FatEntry* node = &root;

    while (true)
    {
        if (*fpath == 0)
            return node;

        if (node == nullptr) 
            break;

        if ((node->flags & 0x40000001) != 1)
            return nullptr;

        node = FindNext(node, &fpath);
    }
    return nullptr;
}

void ResBase::GrowFat()
{
    //4ec8f4

    if (fat_size + 1 >= size)
    {
        size += 1 + (size / 4);
        fat_data = (FatEntry*)realloc(fat_data, size * sizeof(FatEntry));
        if (fat_data == nullptr)
            AfxThrowMemoryException();
    }
}

void ResBase::FUN_004ecc05(FatEntry* node)
{
    //4ecc05
    static int DAT_006a6a64 = 0;

    if ((node->flags & 0x40000001) == 0)
    {
        node->flags |= 0x40000000;

        int8_t tmp = (node->flags & 0xe) >> 1;
        if (tmp != 1 && tmp != 2)
            field_0x30 += node->size;
    }
    else if ((node->flags & 0x40000001) == 1)
    {
        node->flags |= 0x40000000;
        
        uint32_t cnt = 0;
        while (cnt < node->size)
        {
            FUN_004ecc05(fat_data + node->offset + cnt);

            if (DAT_006a6a64)
                break;

            cnt++;
        }
    }
}

void ResBase::FUN_004ec553(FatEntry* newFat, int* pCount, const FatEntry* srcFat, const FatEntry* srcRoot, FatEntry* node, FatEntry* dstRoot)
{
    //4ec553

    int32_t newSize = 0;
    int32_t fatOffset = *pCount;

    if (srcRoot == node)
    {
        fatOffset = fat_size - node->size;
        g_lastDstRoot = dstRoot;
    }

    const uint32_t srcOffset = srcRoot->offset;

    dstRoot->offset = fatOffset;
    dstRoot->SetName(srcRoot->name);
    dstRoot->flags = srcRoot->flags & (~0x10);
    dstRoot->magic = srcRoot->magic;

    for (uint32_t i = 0; i < srcRoot->size; i++)
    {
        if ((srcFat[srcOffset + i].flags & 0x40000001) == 0)
        {            
            newFat[fatOffset] = srcFat[srcOffset + i];

            fatOffset++;
            newSize++;
        }
    }

    for (uint32_t i = 0; i < srcRoot->size; i++)
    {
        if ((srcFat[srcOffset + i].flags & 0x40000001) == 1)
            newSize++;
    }

    if (srcRoot != node)
        *pCount += newSize;

    for (uint32_t i = 0; i < srcRoot->size; i++)
    {
        if ((srcFat[srcOffset + i].flags & 0x40000001) == 1)
        {
            FUN_004ec553(newFat, pCount, srcFat, srcFat + srcOffset + i, node, newFat + fatOffset);
            fatOffset++;
        }
    }

    dstRoot->size = newSize;
}

FatEntry* ResBase::FUN_004ec438(FatEntry* node)
{
    // 4ec438
    if (node == modify_fat)
        return node;

    FatEntry *newFat = (FatEntry*)malloc(size * sizeof(FatEntry));

    if (newFat == nullptr)
    {
        AfxThrowMemoryException();
        return nullptr;
    }

    int exactSize = 0;
    int aproxSize = fat_size - node->size;

    FUN_004ec553(newFat, &exactSize, fat_data, &root, node, &root);

    if (aproxSize != exactSize)
    {
        for (uint32_t i = 0; i < g_lastDstRoot->size; i++)
            newFat[exactSize + i] = newFat[aproxSize + i];
    }

    g_lastDstRoot->offset = exactSize;

    exactSize += g_lastDstRoot->size;

    fat_size = exactSize;
    free(fat_data);

    fat_data = newFat;

    modify_fat = node;

    return g_lastDstRoot;
}



FatEntry* ResBase::FUN_004ec7e0(FatEntry** pOldNode, const char* fname)
{
    //4ec7e0
    FatEntry* oldFat = fat_data; // save old pointer

    GrowFat();

    if (*pOldNode != &root)
        *pOldNode = fat_data + (*pOldNode - oldFat);

    FatEntry *fndNode = Find(*pOldNode, fname);
    if (fndNode != nullptr)
        FUN_004ecc05(fndNode);
    
    fat_size++;

    FatEntry *node = fat_data + (*pOldNode)->offset + (*pOldNode)->size;

    (*pOldNode)->size++;
    (*pOldNode)->flags &= ~0x10;

    node->SetName(fname);
    node->magic = 0;
    return node;
}


void ResBase::SortTree(FatEntry* root)
{
    //4ec736
    for (uint32_t i = 0; i < root->size; i++)
    {
        if ((fat_data[root->offset + root->size].flags & 0x40000001) == 1) {
            SortTree(fat_data + root->offset + root->size);
        }
    }
    if (root->size != 0)
    {
        qsort(fat_data + root->offset, root->size, sizeof(FatEntry), FatEntry::NameCompare);
        root->flags |= 0x10;
    }
}

void ResBase::WriteToFile(CFile* cfile, int32_t offset)
{
    //4e8396
    SortTree(&root);

    if (offset != -1) {
        cfile->Seek(offset, 0);
    }

    cfile->Write(&root.magic, 4);
    cfile->Write(&root.offset, 4);
    cfile->Write(&root.size, 4);
    cfile->Write(&root.flags, 4);
    cfile->Write(&fat_size, 4);
    cfile->Write(&field_0x30, 4);
    cfile->Write(fat_data, fat_size * sizeof(FatEntry));
    cfile->Write(&field_0x3c, 4);
    cfile->Write(pdata, field_0x3c);
}

void ResBase::ReadFromFile(CFile* cfile, int32_t offset)
{
    //4e819c
    if (offset != -1) 
        file->Seek(offset, 0);
    
    cfile->Read(&root.magic, 4);

    if (root.magic != 0x31415926)
        throw std::runtime_error("bad signature");

    cfile->Read(&root.offset, 4);
    cfile->Read(&root.size, 4);
    cfile->Read(&root.flags, 4);
    cfile->Read(&fat_size, 4);
    cfile->Read(&field_0x30, 4);

    if (fat_data == nullptr)
        fat_data = (FatEntry*)malloc(fat_size * sizeof(FatEntry));
    else
    {
        if (fat_size == 0)
            fat_data = (FatEntry*)realloc(fat_data, 1 * sizeof(FatEntry));
        else
            fat_data = (FatEntry*)realloc(fat_data, fat_size * sizeof(FatEntry));
    }

    if (fat_data == nullptr)
    {
        AfxThrowMemoryException();
        return;
    }

    cfile->Read(fat_data, fat_size * sizeof(FatEntry));

    size = fat_size;

    cfile->Read(&field_0x3c, 4);

    fat_offset = field_0x3c;

    if (pdata == nullptr)
        pdata = (uint8_t *)malloc(field_0x3c);
    else
    {
        if (field_0x3c == 0)
            pdata = (uint8_t*)realloc(pdata, 1);
        else
            pdata = (uint8_t*)realloc(pdata, field_0x3c);
    }

    if (pdata == nullptr)
    {
        AfxThrowMemoryException();
        return;
    }

    if (field_0x3c != 0)
        cfile->Read(pdata, field_0x3c);
}

FatEntry* ResBase::FUN_004ec8c6(const char* fpath, const char* fname)
{
    //4ec8c6
    FatEntry* pathNode = TracePath(fpath);
    return FUN_004ec7e0(&pathNode, fname);
}

FatEntry* ResBase::FUN_004ea183(const char* fpath, const char* fname)
{
    //4ea183
    FatEntry *pathNode = TracePath(fpath);
    if (pathNode == nullptr)
        return nullptr;

    FUN_004ec438(pathNode);

    FatEntry* node = FUN_004ec8c6(fpath, fname);
    node->offset = fat_size;
    node->size = 0;
    node->flags = 1;
    return node;
}

FatEntry* ResBase::FUN_004eca16(const char* fpath)
{
    //4eca16
    FatEntry* node = TracePath(fpath);
    if (node == nullptr)
        AfxThrowFileException(CFileException::fileNotFound);

    if ((node->flags & 0x40000001) == 0)
        AfxThrowFileException(CFileException::badPath);

    return node;
}

FatEntry* ResBase::FUN_004eca63(const char* fpath)
{
    //4eca63
    FatEntry* node = TracePath(fpath);

    if (node != nullptr && (node->flags & 0x40000010) != 0)
        return nullptr;

    return node;
}

FatInfo* ResBase::FUN_004e5d55(const char* fpath)
{
    //4e5d55
    FatEntry* node = FUN_004eca63(fpath);
    if (node == nullptr)
        return nullptr;
    
    if ((node->flags & 0x20000000) != 0)
        return FATINFO_T1;

    FatInfo *info = new FatInfo();
    info->file = file;
    info->offset = node->offset;
    info->size = node->size;
    return info;
}


int ResBase::GetSizedString(const char* fpath, const char* fname, const char* defval, char* out, uint32_t sz)
{
    //4ea2d2
    FatEntry *nodepath = Find(&root, fpath);
    FatEntry *node = Find(nodepath, fname);

    if (node == nullptr) 
    {
        uint32_t copylen = strlen(defval);

        if (sz < copylen + 1)
            copylen = sz;
        else
            copylen++;

        strncpy(out, defval, copylen);
        return strlen(out);
    }

    char buf[32];

    int ntype = (node->flags & 0xe) >> 1;
    switch (ntype)
    {
    case 0:
        if (sz < node->size)
            strncpy(out, (char *)pdata + node->offset, sz);
        else
            strncpy(out, (char *)pdata + node->offset, node->size);
        break;

    case 1:
        _itoa(node->offset, buf, 10);
        strncpy(out, buf, sz);
        break;

    case 2:
        sprintf(buf, "%f", *(double *)&node->offset);
        //_itoa(node->offset, buf, 10);
        strncpy(out, buf, sz);
        break;

    case 3:
    {
        char *pout = out;
        for (uint32_t i = 0; i < node->size / 4; i++)
        {
            _itoa(*((int *)(pdata + node->offset) + i), buf, 10);

            if (i < (node->size / 4) - 1)
                strcat(buf, ",");

            uint32_t ln = strlen(buf);

            if (sz < (pout - out) + ln + 2)
                break;

            strcat(pout, buf);

            pout += ln;
        }
    }
        break;

    case 4:
        if (sz - 2 < node->size - 4)
            memcpy(out, (uint8_t *)pdata + 4 + node->offset, sz - 2);
        else
            memcpy(out, (uint8_t*)pdata + 4 + node->offset, node->size - 4);
        out[sz - 2] = 0;
        out[sz - 1] = 0;
        break;

    case 5:
    {
        char *pout = out;
        for (uint32_t i = 0; i < node->size / 8; i++)
        {
            sprintf(buf, "%f", *((double*)(pdata + node->offset) + i));

            if (i < (node->size / 8) - 1)
                strcat(buf, ",");

            int ln = strlen(buf);
            if (sz < (pout - out) + (ln + 2))
                break;
            strcat(pout, buf);
            pout += ln;
        }
    }
        break;

    default:
        throw std::runtime_error("UNKNOWN TYPE. CANT CONVERT");
        break;
    }
    
    return strlen(out);
}


void ResBase::FUN_004e99c4()
{
    //4e99c4
    modify_fat = nullptr;
    field_0x30 = 0;

    FUN_004ec438(&root);

    FatEntry** pnodes = new FatEntry*[fat_size]();

    if (pnodes == nullptr)
    {
        AfxThrowMemoryException();
        return;
    }

    FatEntry tmp;
    tmp.size = 0;
    tmp.offset = 0;

    pnodes[0] = &tmp;

    uint32_t idx = 0;
    uint32_t n = 1;
    for ( idx = 0; idx < fat_size; idx++)
    {
        if ((fat_data[idx].flags & 0x40000001) == 0)
        {
            int tp = (fat_data[idx].flags & 0xe) >> 1;
            if (tp != 1 && tp != 2)
            {
                pnodes[n] = fat_data + idx;
                n++;
            }
        }
    }

    if (n > 1)
    {
        qsort(pnodes, n, sizeof(FatEntry *), FatEntry::SortCompare);

        for (idx = 0; idx < n - 1; idx++)
        {
            uint32_t of = pnodes[idx + 1]->offset - (pnodes[idx]->offset + pnodes[idx]->size);
            if (of != 0)
            {
                FatEntry* nd = pnodes[idx + 1];
                memcpy(pdata + nd->offset, pdata + of + nd->offset, nd->size);
                nd->offset -= of;
            }
        }
    }

    field_0x3c = pnodes[idx]->offset + pnodes[idx]->size;
    field_0x30 = 0;

    delete[] pnodes;
}

uint8_t *ResBase::AllocData(uint32_t size)
{
    //4e9bb6

    if (fat_offset <= field_0x3c + size)
    {
        if (field_0x3c / 8 < field_0x30)
        {
            FUN_004e99c4();

            if (fat_offset <= field_0x3c + size)
            {
                fat_offset += size + (fat_offset / 8);
                pdata = (uint8_t *)realloc(pdata, fat_offset);

                if (pdata == nullptr)
                {
                    AfxThrowMemoryException();
                    return nullptr;
                }
            }
        }
        else
        {
            fat_offset += size + (fat_offset / 8);

            pdata = (uint8_t*)realloc(pdata, fat_offset);

            if (pdata == nullptr)
            {
                AfxThrowMemoryException();
                return nullptr;
            }
        }
    }
    uint32_t off = field_0x3c;
    field_0x3c += size;
    return pdata + off;
}

void ResBase::SetString(const char* fpath, const char* fname, const char* str)
{
    //4ea73d
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 0;
        node->size = 0;
        node->offset = pdata - AllocData(node->size); //??? BUG
    }

    if ((node->flags & 0xe) != 0)
        throw std::runtime_error("not a string associated with specified key");
    
    int ln = strlen(str) + 1;
    if (node->size < ln)
    {
        field_0x30 += node->size;
        node->size = ln;
        node->offset = AllocData(ln) - pdata;
    }
    else
        field_0x30 += (node->size - ln);

    strcpy((char *)pdata + node->offset, str);
}


void ResBase::SetInt(const char* fpath, const char* fname, int32_t val)
{
    //4ea928
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 1 << 1;
        node->size = 0;
        node->offset = val;
    }
    else
    {
        if ((node->flags & 0xe) >> 1 != 1)
            throw std::runtime_error("not an int associated with specified key");
        node->offset = val;
    }
}

void ResBase::SetDouble(const char* fpath, const char* fname, double val)
{
    //4eaa8b
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 2 << 1;
        //node->size = 0;
        *(double *)&node->offset = val;
    }
    else
    {
        if ((node->flags & 0xe) >> 1 != 2)
            throw std::runtime_error("not an double associated with specified key");

        //node->size = 0;
        *(double*)&node->offset = val;
    }
}


void ResBase::SetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>& array)
{
    //4eb2a3
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 3 << 1;
        node->size = 0;
        node->offset = AllocData(node->size) - pdata;
    }

    if ((node->flags & 0xe) >> 1 != 3)
        throw std::runtime_error("not an int array associated with specified key");

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln)
    {
        field_0x30 += node->size;
        node->size = ln;
        node->offset = AllocData(node->size) - pdata;
    }
    else
        field_0x30 += node->size - ln;

    for (uint32_t i = 0; i < array.GetSize(); i++)
        *(uint32_t*)(pdata + node->offset + i * 4) = array[i];
}


void ResBase::SetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>& array)
{
    //4eb43a
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 3 << 1;
        node->size = 0;
        node->offset = AllocData(node->size) - pdata;
    }

    if ((node->flags & 0xe) >> 1 != 3)
        throw std::runtime_error("not an int array associated with specified key");

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln)
    {
        field_0x30 += node->size;
        node->size = ln;
        node->offset = AllocData(node->size) - pdata;
    }
    else
        field_0x30 += node->size - ln;

    for (uint32_t i = 0; i < array.GetSize(); i++)
        *(uint32_t*)(pdata + node->offset + i * 4) = array[i];
}


void ResBase::SetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>& array)
{
    //4eb5d2
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
    {
        if (nodePath == nullptr)
            nodePath = FUN_004ea183(root.name, fpath);

        node = FUN_004ec438(nodePath);
        node = FUN_004ec7e0(&node, fname);
        node->flags = 3 << 1;
        node->size = 0;
        node->offset = AllocData(node->size) - pdata;
    }

    if ((node->flags & 0xe) >> 1 != 3)
        throw std::runtime_error("not an int array associated with specified key");

    uint32_t ln = array.GetSize() * 4;
    if (node->size < ln)
    {
        field_0x30 += node->size;
        node->size = ln;
        node->offset = AllocData(node->size) - pdata;
    }
    else
        field_0x30 += node->size - ln;

    for (uint32_t i = 0; i < array.GetSize(); i++)
        *(uint32_t*)(pdata + node->offset + i * 4) = array[i];
}


CString ResBase::GetValueString(const char* fpath, const char* fname, const char* defval)
{
    //4ea69d

    char buf[0x800];
    GetSizedString(fpath, fname, defval, buf, 0x800);
    return buf;
}

int32_t ResBase::GetInt(const char* fpath, const char* fname, int32_t defval)
{
    //4ea8af

    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return defval;

    if ((node->flags & 0xe) >> 1 != 1)
        throw std::runtime_error("not an int associated with specified key");

    return node->offset;
}

double ResBase::GetDouble(const char* fpath, const char* fname, double defval)
{
    //4eaa06

    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return defval;

    if ((node->flags & 0xe) >> 1 != 2)
        throw std::runtime_error("not an double associated with specified key");

    return *(double *)&node->offset;
}

int ResBase::GetAllNodeNames(char* pbuf, int bufsz)
{
    //4eab6b
    int out_len = 0;
    for (int i = 0; i < root.size; i++)
    {
        const char* name = fat_data[root.offset + i].name;
        int ln = strlen(name);
        if (ln + 1 < bufsz - out_len)
            ln++;
        else
            ln = bufsz - out_len;

        if (ln <= 1)
        {
            *pbuf = 0;
            return bufsz - 2;
        }

        *pbuf = 0;
        strncat(pbuf, name, ln);
        pbuf += ln;
        out_len += ln;
    }
    *pbuf = 0;
    return out_len;
}


int ResBase::GetInt8Array(const char* fpath, const char* fname, CArray<uint8_t>* array)
{
    //4eaca7
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return 0;

    int tp = (node->flags & 0xe) >> 1;
    if (tp == 3)
    {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++)
            array->SetAt(i, *(int32_t*)(pdata + node->offset + i * 4));

        return 1;
    }

    if (tp == 1)
    {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}

int ResBase::GetInt16Array(const char* fpath, const char* fname, CArray<uint16_t>* array)
{
    //4ead9d
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return 0;

    int tp = (node->flags & 0xe) >> 1;
    if (tp == 3)
    {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++)
            array->SetAt(i, *(int32_t*)(pdata + node->offset + i * 4));

        return 1;
    }

    if (tp == 1)
    {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}

int ResBase::GetInt32Array(const char* fpath, const char* fname, CArray<uint32_t>* array)
{
    //4eae99
    //4eafb6 same?
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return 0;

    int tp = (node->flags & 0xe) >> 1;
    if (tp == 3)
    {
        array->SetSize(node->size / 4);
        for (int i = 0; i < node->size / 4; i++)
            array->SetAt(i, *(int32_t*)(pdata + node->offset + i * 4));

        return 1;
    }

    if (tp == 1)
    {
        array->SetSize(1);
        array->SetAt(0, node->offset);
        return 1;
    }

    if (tp == 0 && node->size <= 1)
    {
        array->RemoveAll();
        return 1;
    }

    throw std::runtime_error("not an int array associated with specified key");
    return 0;
}


int ResBase::GetStringArray(const char* fpath, const char* fname, CStringArray* array)
{
    //4eb0d3
    FatEntry* nodePath = Find(&root, fpath);
    FatEntry* node = Find(nodePath, fname);

    if (node == nullptr)
        return 0;

    int tp = (node->flags & 0xe) >> 1;

    if (tp != 4 && tp != 0)
    {
        throw std::runtime_error("not an string array associated with specified key");
        return 0;
    }

    int32_t count = 0;
    char* dat = (char *)pdata + node->offset;
    char* datstart = dat;
    if (tp == 0)
    {
        array->SetSize(1);
        count = 1;
    }
    else
    {
        count = *(int32_t*)(dat);
        dat += 4;
        array->SetSize(count);
    }

    for (int i = 0; i < count; i++)
    {
        if ((dat - datstart) >= node->size)
            break;

        array->SetAt(i, dat);
        int ln = strlen(dat);
        dat += ln + 1;

    }

    return 1;
}





void FatEntry::SetName(const char* resname)
{
    //4e6f30
    size_t sVar1;
    int local_8;

    if (resname != name) {
        size_t len = strlen(resname);
        if (len > 15)
            flags |= 0x10000000;

        if (len + 1 > 15)
            len = 15;

        name[len] = 0;
        strncpy(name, resname, len);
    }
}

int FatEntry::NameCompare(const void* key, const void* datum)
{
    // 4ecd2a
    return strcmp(((FatEntry*)key)->name, ((FatEntry*)datum)->name);
}

int FatEntry::SortCompare(const void* a, const void* b)
{
    //4eccd1
    const FatEntry *_a = *(const FatEntry**)a;
    const FatEntry *_b = *(const FatEntry**)b;
    if (_b->offset < _a->offset)
        return 1;
    else if (_a->offset == _b->offset)
    {
        if (_b->size < _a->size)
            return 1;
        else
            return 0;
    }
    else
        return -1;
}