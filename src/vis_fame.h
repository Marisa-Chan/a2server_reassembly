#ifndef A2_VIS_FAME_H
#define A2_VIS_FAME_H

#pragma once
#include "vis_core.h"

//60b288
class VisFameHall : public VisScreen {
public:
    virtual ~VisFameHall(); // 45cde2 (client: 460a12) - body (??_G deleting dtor at 45DA90)

    virtual const char* GetHint() override; // 45db50 (client: 4616c0)
    virtual void Draw() override; // 45d7f9 (client: 461429)
    virtual void DrawBackground(CRect* rect) override; // 45da6a (client: 461692)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 45d57f (client: 4611af)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 45d5f0 (client: 461220)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 45d6c3 (client: 4612f3)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 45d773 (client: 4613a3)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 45d5be (client: 4611ee)
    virtual int32_t OnChar(uint32_t wparam) override; // 45d5d7 (client: 461207)

    virtual void OnCreate() override; // 45ce6e (client: 460a9e)
    virtual void DoOpen() override; // 45d222 (client: 460e52)
    virtual void DoClose(uint32_t code) override; // 45d2d9 (client: 460f09)

    void LoadBitmaps(); // 45d32e (client: 460f5e)
    void LoadSounds(); // 45d542 (client: 461172)
    void UpdateRects(); // 45cfae (client: 460bde)
    void FreeBitmaps(); // 45d44a (client: 46107a)
    void FreeSounds(); // 45d563 (client: 461193)

    VisFameHall(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 45cd35 (client: 460965)
public:
    CFameHall* fame;
    CArray<CRect> name_rects;
    CArray<CRect> rank_rects;
    CArray<CRect> score_rects;
    CRect close_rect;
    CBmp64* bmp_bkg;
    CBmp64* bmp_close_off;
    CBmp64* bmp_close_on;
    CBmp64* bmp_cur_close;
    SfxSample* snd_close;
    uint32_t visible_flag;
};
ASSERT_SIZE(VisFameHall, 0xd0);

//60cdf8
class VisFameDocument : public VisScreen {
public:
    virtual ~VisFameDocument(); // 4ada2a (client: 4b236a)

    virtual const char* GetHint() override; // 4aeda0 (client: 4b36e0)
    virtual void Draw() override; // 4ae7a3 (client: 4b30e3)
    virtual void DrawBackground(CRect* rect) override; // 4ae926 (client: 4b3266)
    virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4ae57f (client: 4b2ebf)
    virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4ae5be (client: 4b2efe)
    virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4ae607 (client: 4b2f47)
    virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4ae717 (client: 4b3057)
    virtual int32_t OnKeyDown(uint32_t wparam) override; // 4ae791 (client: 4b30d1)

    virtual void OnCreate() override; // 4adab6 (client: 4b23f6)
    virtual void DoOpen() override; // 4ae473 (client: 4b2db3)
    virtual void DoClose(uint32_t code) override; // 4ae534 (client: 4b2e74)

    VisFameDocument(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 4ad961 (client: 4b22a1)

    void LoadBitmaps(); // 4add24 (client: 4b2664)
    void FreeBitmaps(); // 4ae17a (client: 4b2aba)
    void ClearDocs(); // 4ae41b (client: 4b2d5b)
    void LoadDocs(); // 4ae3c6 (client: 4b2d06)
    int32_t UpdateButtons(CPoint pos, bool is_down); // 4ae952 (client: 4b3292)
    void UpdateArrowStates(); // 4aec7a (client: 4b35ba)
    void NextDoc(); // 4ae638 (client: 4b2f78)
    void PrevDoc(); // 4ae6b3 (client: 4b2ff3)
public:
    CFameHall* fame;
    CBmp64* bmp_sheet;
    CArray<CBmp64*> bmp_leftarrow;
    CArray<CBmp64*> bmp_rightarrow;
    CArray<CBmp64*> bmp_okbutton;
    CBmp64* bmp_cur_left;
    CBmp64* bmp_cur_right;
    CBmp64* bmp_cur_ok;
    int32_t selected_doc;
    uint32_t gap_0xbc;
    CRect rect_prev;
    CRect rect_next;
    CRect rect_ok;
    uint32_t gap_0xf0;
    uint32_t visible_flag;
};
ASSERT_SIZE(VisFameDocument, 0xf8);


struct FameHallEntry {
    CString str;
    uint32_t score = 0;
    uint32_t value2 = 0;
    uint32_t value3 = 0;
};

ASSERT_SIZE(FameHallEntry, 0x10);

struct FameDocumentView {
    int32_t doc_id;
    uint32_t is_text;
    CBmp64* picture;
    CString str;
    CStringArray str_arr;
    CRect rect;
    uint32_t gap_x34;
    uint32_t top_line;

    FameDocumentView(); // 4abdab (client: 4afdbb)
    void operator=(const FameDocumentView& b); // 4abd30 (client: 4afd40)

    void Clear(); // 4ac27f (client: 4b028f)
    void SetDocument(int32_t doc_id, uint32_t is_text); // 4abf0f (client: 4aff1f)
    int32_t GetDocId(); // 4ad880 (client: 4b0ca0)
    uint32_t GetIsText(); // 4ad890 (client: 4b0cb0)
    bool ScrollDown(); // 4abf35 (client: 4aff45)
    int32_t ScrollUp(); // 4abf72 (client: 4aff82)
    void Draw(int32_t x, int32_t y); // 4abfb1 (client: 4affc1)
    void LoadDocument(); // 4ac0af (client: 4b00bf)

};

ASSERT_SIZE(FameDocumentView, 0x3C);

struct CFameHall {
    uint32_t difficulty;
    uint32_t m_nTotalTime;
    uint32_t m_nDifficultyWeight;
    uint32_t m_nMaxEntries;
    CArray<FameHallEntry> m_Entries;
    CArray<FameDocumentView> m_Documents;

    CFameHall(); // 4ac2e3 (client: 4b02f3)
    ~CFameHall(); // 4ac352 (client: 4b0362)

    void ZeroFactors(); // 4ac3af (client: 4b03bf)
    void SetDifficulty(uint32_t difficulty); // 497270 (client: 499fd0)
    uint32_t AddTextDocument(int32_t doc_id); // 4ac566 (client: 4b0576)
    void SeedDefaultEntries(); // 4acafa (client: 4b0b0a)
    void LoadEntries(CFile* file); // 4ac945 (client: 4b0955)
    void SaveEntries(CFile* file); // 4ac839 (client: 4b0849) - save m_Entries to file
    void SaveDocuments(CFile* file); // 4ac706 (client: 4b0716)
    void LoadDocuments(CFile* file); // 4ac7a2 (client: 4b07b2)
    void AddEntry(const FameHallEntry& fame); // 4ac3ce (client: 4b03de)

    void AddDifficultyPoint(); // 420050 (client: 420c80)

    void SubmitScore(); // 4ac498 (client: 4b04a8)

    int32_t GetDifficulty() { return this->difficulty; }; // 497290 (client: 49a940)
    void AddMissionElapsedTime(int32_t t) { this->m_nTotalTime += t; } // 497250 (client: 49a860)
};

ASSERT_SIZE(CFameHall, 0x38);
#endif

