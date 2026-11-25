#include "txtfile.h"
#include "file.h"

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
namespace {

bool g_AllLinesDestroyed = false;

struct AllLinesLifetimeGuard {
    AllLinesLifetimeGuard() {
        g_AllLinesDestroyed = false;
    }
    ~AllLinesLifetimeGuard() {
        g_AllLinesDestroyed = true;
    }
};

} // namespace
#endif

//660d68
CArray<char*> TxtFile::AllLines;

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
namespace {

AllLinesLifetimeGuard g_AllLinesLifetimeGuard;

} // namespace
#endif

// 475c6c (client: 4799b8)
TxtFile::TxtFile() = default;

// 43ccd0 (client: 43f070)
TxtFile::~TxtFile() {

    this->Free();
}

// 475de9 (client: 479b35)
void TxtFile::Free() {
    
    if (this->buffer) {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        if (!g_AllLinesDestroyed) {
            for (int32_t i = 0; i < AllLines.GetSize(); i++) {
                if (AllLines[i] == this->buffer) {
                    int32_t count = this->line_count;
                    if (i + count > AllLines.GetSize()) {
                        count = AllLines.GetSize() - i;
                    }
                    if (count > 0) {
                        AllLines.RemoveAt(i, count);
                    }
                    break;
                }
            }
        }
        free(this->buffer);
        this->buffer = nullptr;
#else
        for (int i = 0; i < AllLines.GetSize(); i++) {
            if (AllLines[i] == this->buffer) {
                AllLines.RemoveAt(i, this->line_count);
                free(this->buffer);
                this->buffer = nullptr;
                return;
            }
        }
#endif
    }
}

// 475ca9 (client: 4799f5)
void TxtFile::LoadChunkFile(const char* fname) {

    ResArchiveFile fil;
    fil.Open(fname, 0);

    uint32_t fsize = fil.GetLength();

    this->buffer = (char*)malloc(fsize + 1);
    this->buffer[fsize] = 0;

    fil.Read(this->buffer, fsize);

    this->line_count = 0;
    this->start_index = AllLines.GetSize();

    for (char *pcur = this->buffer; pcur < this->buffer + fsize; ) {
        char* linestart = pcur;
        AllLines.Add(linestart);

#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
        for (; pcur < this->buffer + fsize && *pcur != '\r'; pcur++)
        {}
#else
        for(; *pcur != '\r' && pcur < this->buffer + fsize; pcur++ )
        {}
#endif

        if (pcur >= this->buffer + fsize) {
#if defined(A2SERVER_PATCH) || defined(A2CLIENT_PATCH)
            this->line_count++;
            CharToOemA(linestart, linestart);
#endif
            break;
        }

        *pcur = 0;
        pcur += 2;

        this->line_count++;
        CharToOemA(linestart, linestart);
    }
}

// 475e57 (client: 479ba3)
char* TxtFile::GetLine(int index) {
    return AllLines[this->start_index + index];
}


// 43cd30 (client: 43f130)
int TxtFile::GetCount() {
    return this->line_count;
}