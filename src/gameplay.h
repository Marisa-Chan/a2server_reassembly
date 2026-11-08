#ifndef A2_GAMEPLAY_H
#define A2_GAMEPLAY_H

// In-game screen (BigStruct2), its HUD children and combat/log helpers
// extracted from the vis_core megafile (migration_state/file_split_proposal.md).
#pragma once
#include "vis_core.h"
#include "take_damage.h"

class GM_a28 : public CObject {
public:
	virtual ~GM_a28(); //4020ad (client: 4020ad)
	GM_a28(); //401f50 (client: 401f50)

	void Add(const char* msg, uint16_t* clr, uint32_t t); //4021fe (client: 4021fe)
	void AddWithCheck(const char* msg, uint16_t* clr, uint32_t t); //4023aa (client: 4023aa)
	void Clear(); //4021d2 (client: 4021d2)
	void Draw(); //40261a (client: 40261a)
	void SetRect(const CRect& r); //40212b (client: 40212b)
	void Update(); //402585 (client: 402585)
public:
	CStringArray text;
	CPtrArray color;
	CDWordArray times;
	uint32_t timestamp;
	uint32_t elapsed_time;
	int32_t max_size;
	CRect out_area;
	int32_t dirty; //unk type
};

class BigStruct2 : public CVisualObject {
public:
	BigStruct2(int32_t l, int32_t t, int32_t r, int32_t b); // 402af6 (client: 402af6)
	~BigStruct2(); // 403395 (client: 403418)

	// VTable at 609088.
	virtual void VMethod7() override; // 406f1a (client: 406f89)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 40c232 (client: 40c2b9)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 40c83f (client: 40c8c6)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 40c510 (client: 40c597)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 40c633 (client: 40c6ba)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 40c60f (client: 40c696)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 40c56c (client: 40c5f3)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 40c59c (client: 40c623)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 40c902 (client: 40c989)
	virtual int32_t OnChar(uint32_t wparam) override; // 40d4b2 (client: 40d539)

public:
	void RenderWorldFrame(); //406f7b (client: 406fea)
	void RebuildWorldGrids(); //40403b (client: 4040e2)
	void DrawTerrainFull(); //404e1a (client: 404eb5)
	void DrawTerrainAnimated(int32_t arg); //405399 (client: 405434)
	void DrawTerrainScrolled(int32_t arg); //40587b (client: 405908)
	void RedrawLitTerrain(); //405d0e (client: 405d9b)
	void AddHitRect(CRect* rect, uint16_t unit_id); //404da3 (client: 404e40)
	void DrawTileOverlayFx(int32_t x, int32_t y, uint32_t flags, int32_t flag); //40afc5 (client: 40b010)
	int32_t IsGfxObjectHidden(int32_t gfx_id, int32_t x, int32_t y, int32_t height); //40b173 (client: 40b1be)
	void FreeTileGrids(); //40328e (client: 403311)
	void ApplyTerrainLightOverrides(); //404912 (client: 4049b7)
	void UpdateTileLight(); //404a2c (client: 404ad1)
	void SetScenario(Scenario* scen); //403f08 (client: 403faf)
	void UpdateMapCursor(); //40b314 (client: 40b35f)
	uint32_t UpdateMouseHover(); //41864d (client: 419e12)
	int32_t ScreenToTileRow(int32_t x, int32_t y); //4184b8 (client: 419c7d)
	void IssueOrderAtCursor(CPoint* pt); //417b42 (client: 419313)
	void SelectUnitsInRect(CRect* rect); //417f09 (client: 4196d2)
	void SendGroupMove(uint16_t x, uint16_t y); //419154 (client: 41a90f)
	void SendGroupAttack(uint16_t id); //419246 (client: 41a9ff)
	void SendGroupAttackMove(uint16_t x, uint16_t y); //41930d (client: 41aac4)
	void SendGroupAttackTarget(uint16_t id); //41955c (client: 41ad0d)
	void SendGroupPatrol(uint16_t x, uint16_t y); //41965d (client: 41ae0a)
	void SendGroupCastAtPosition(uint16_t x, uint16_t y, int32_t spell); //41972f (client: 41aeda)
	void SendGroupCastAtUnit(uint16_t id, int32_t spell); //419ba1 (client: 41b346)
	void SendGroupPickup(uint16_t x, uint16_t y); //41a33f (client: 41bae8)
	void SendGroupEnterBuilding(uint16_t id); //41a66e (client: 41befc)
	void SendHotkeyBroadcast(int32_t id); //41ac2d (client: 41c4b9) in asm
	void SendDropGold(int32_t arg1, int32_t arg2); //41ac88 (client: 41c514)
	void SendItemOperation(uint8_t arg1, uint16_t arg2, uint8_t arg3, uint16_t arg4, uint16_t arg5); //41a7c7 (client: 41c053)
	void UpdateWorld(); //40bd34 (client: 40bdc1)
	void TickLightningCells(); //40c112 (client: 40c199)
	void RebuildLandscapeOccupancy(); //40c18b (client: 40c212)
	int32_t StepShadowRay(int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4); //403b8f (client: 403c36)
	void AllocTileGrids(); //402fb8 (client: 40303b)
	void BuildShadowTables(); //403730 (client: 4037d7)
	void CancelOrderMode(); //4168bd (client: 4180d4)
	void ScrollViewTo(int32_t wparam, int32_t lparam); //4167f7 (client: 41800e)
	void SetOrderMode(int32_t arg); //418f93 (client: 41a74e)
	void SendGroupGuard(); //4193e8 (client: 41ab9d)
	void SendGroupAggressive(); //4194a2 (client: 41ac55)
	void SendGroupGoHome(); //41a425 (client: 41bbcc)
	void SelectControlGroup(int32_t digit); //416983 (client: 41819a)
	void AssignControlGroup(int32_t digit); //416a02 (client: 418219)
	void AddToControlGroup(int32_t digit); //416aa6 (client: 4182bd)
	void SelectControlGroupAndCenter(int32_t digit); //416b4a (client: 418361)
	void SendGroupStop(); //41a5b4 (client: 41bd57)
	CUnit* PickVoiceUnit(); //41dfdb (client: 41f85d)
	void SendGroupGather(); //41a4df (client: 41bc84)
	void SendRequestRespawn(); //41b155 (client: 41c9dd)
	void SelectAllOwnUnits(); //416c4c (client: 418463)
	void CycleFormation(); //41f6d0 (client: 420990)
	void ToggleShowHp(); //41f770 (client: 420a30)
	void ToggleFlyingHp(); //41f7b0 (client: 420a70)
	void CycleWimpyMode(); //41f720 (client: 4209e0)
	int32_t GetWimpyMode(); //41f7f0 (client: 420c60)
	int32_t GetFormation(); //41f810 (client: 420ad0)
	void CycleAutoCastGroup(); //41ab05 (client: 41c391)
	void SendShopTakeBack(); //41a9f6 (client: 41c282)
	void SendShopSell(); //41a99c (client: 41c228)
	void SendHurtSelf(); //41ce14 (client: 41e698)
	void SendShopBuy(); //41a942 (client: 41c1ce)

	void UpdateSelectionState(); //416cf7 (client: 41850e)
	void SendMapAnnouncement(const char* mapname); // 41cda3 (client: 41e627)
	uint32_t FindObjectAtTile(int32_t x, int32_t y); // 40c0a8 (client: 40c12f)
	void SendAutoSpellSelect(int32_t spell_id); //41ace2 (client: 41c56e)
	void SendGroupSelfCast(int32_t idx); //41a001 (client: 41b7a0)
	void UpdateSpellEffects(CUnit* unit); //from 416cf7 (client: 41850e)
	void UpdateSpellModifiers(CUnit* unit); //from 416cf7 (client: 41850e)

	void LoadScenario(const char* name); //41c4a1 (client: 41dd29)

	CUnit* GetUnit_3f6c(); //41f830 (client: 420c60)
	void SetUnit_3f6c(CUnit* uni) { this->my_unit = uni; };

	int32_t ProcessPackets(uint8_t breakid); //40da14 (client: 40dac9)

	void ComputeSoundVolumePan(int32_t xpos, int32_t ypos, int32_t* pvol, int32_t* ppan); //41b7b7 (client: 41d03f)
	void ApplyTileLightMask(int32_t x, int32_t y, int32_t w, int32_t h, int32_t unk1, uint8_t* dat, int32_t unk2); //41c74b (client: 41dfd3)

	CUnit* FindUnitByServerId(int32_t serv_id); //41df23 (client: 41f7a5)
	int32_t FindUnitByQuestFlag(int32_t id, CUnit** out); //41e2af (client: 41fb31)
	CUnit* FindOrCreateUnitByServerId(int32_t serv_id); //41dfa6 (client: 41f828)
	CGameBitmap* BuildUnitPortrait(int32_t serv_id); //41da52 (client: 41f2d4) in asm
	CUnit* CreateRemoteHeroStub(int32_t serv_id); //41de80 (client: 41f702)
	void LeaveInn(int32_t serv_id); //41ae1c (client: 41c6a8)

	void ApplyLandscapeFlags(CWordArray *data); //41c630 (client: 41deb8)

	void SendRequestFileChunk(int32_t val); //41cb67 (client: 41e3eb)
	void SendRequestMapInfo(); //41cb21 (client: 41e3a5)

	void SendChatPacket(const char* nam, int32_t token, int32_t playerid); //41b2a4 (client: 41cb2c)
	void SendShopClose(); //41a8cc (client: 41c158)

	int32_t SendEnterGameAndWait(); //40d7f3 (client: 40d87a)
	int ConnectAndJoinSession(); //40d4e2 (client: 40d569)

	void SendJoinMap(); //0041cbb8 (client: 41e43c)
	void SendRequestUnitUpdateFile(const CString& str); //41cc78 (client: 41e4fc)
	void ResetWorldState(int32_t arg); //41d2da (client: 41eb5c)
	int32_t IsBookOpen(); //41b4b0 (client: 41cd38)
	int32_t IsBagOpen(); //41b495 (client: 41cd1d)

	void SendPlayerName(CString str); //41afcf (client: 41c857)
	void SendDiplomacyState(); //41a735 (client: 41bfc1)

	void SendGameMode(int32_t m); //41cad0 (client: 41e354)
	void SendWimpyMode(int32_t wim); //41aaaa (client: 41c336)
	void SendFormationMode(int32_t form); // 41abd2 (client: 41c45e)
	void SendAutoCastSetting(); //41ab74 (client: 41c400)

	void RecalcVisibleMapHeight(); //41b6e0 (client: 41cf68)
	int32_t MapMaxY() const { return (this->map_height - 8) - this->view_tiles_y; } //41f6b0 (client: 420310)

	void CloseSpellBook(); //41b636 (client: 41cebe)

	void OpenSpellBook(); //41b509 (client: 41cd91)
	void CloseBag(); //41b40e (client: 41cc96)
	void OpenBag(); //41b381 (client: 41cc09)
	void SendRequestUnitsResend(); //41b10f (client: 41c997)
	void SendCheatShop(int32_t arg1, int32_t arg2); //41b064 (client: 41c8ec)
	void StopAmbientSounds(); //41c39c (client: 41dc24)

	void CleanupCompletedMissionMapState(); //41ce5a (client: 41e6de)

	void BuildShadowField(CGameObject* obj); //403ca0 (client: 403d47)
	void UpdateDayNightCycle(int32_t); //41d97e (client: 41f200)

	uint32_t AvailableOrderMask() const; // 417af6 (client: 4192c7)
	void SendAdjustPlayerGoldAction(int32_t goldDelta) const; //41b0be (client: 41c946)

	int32_t MapMinX() const { return 8; } //41f670 (client: 4201d0)
	int32_t MapMinY() const { return 8; } //41f680 (client: 4201e0)

	int32_t MapMaxX() const { return (this->map_width - 8) - this->view_tiles_x; } //41f690 (client: 4202f0)

	void ScrollMapX(int32_t dx); //403f4f (client: 403ff6)
	void ScrollMapY(int32_t dy); //403fc5 (client: 40406c)

	void OnOpenShopDialog(); //41a856 (client: 41c0e2)
	void NetOnOpenInnDialog(); //41adbb (client: 41c647)

	void UpdateAmbientSounds(); //41b8d0 (client: 41d158)
	void MakeHeightCells(); //4065d3 (client: 406658)
	void ApplyDynamicLight(int32_t tx, int32_t ty, int32_t radius, int32_t brightness); //406340 (client: 4063c5)

	void SendGroupStopAfk(); //client: 41be0f

public:
	int32_t view_x;
	int32_t view_y;
	int32_t view_tiles_x; // 0x64 - visible map width in tiles
	int32_t view_tiles_y; // 0x68 - visible map height in tiles
	int32_t tile_grid_bytes; // 0x6c - byte size of the per-tile arrays ((view_tiles_x + 6) * (view_tiles_y + 10) dwords)
	int32_t corner_grid_bytes; // 0x70 - byte size of the per-corner arrays ((view_tiles_x + 7) * (view_tiles_y + 11) dwords)
	int32_t force_full_redraw; // 0x74
	int32_t scroll_dx; // 0x78
	int32_t scroll_dy; // 0x7c
	Scenario* scenario;
	int32_t map_width; // 0x84 - scenario width in cells
	int32_t map_height; // 0x88 - scenario height in cells
	CGameObject** tile_units; // 0x8c - per-tile ground units
	CGameObject** tile_air_units; // 0x90 - per-tile air units
	CGameObject** tile_structures; // 0x94 - per-tile structures
	CGameObject** tile_backpacks; // 0x98 - per-tile backpacks
	CGameObject** tile_special_units; // 0x9c - per-tile special units (CUnit::VMethod10 bucket 4)
	uint32_t* corner_tile_kind; //unk type
	uint32_t* corner_tile_kind_prev; //unk type
	uint8_t* corner_light; //unk type
	uint8_t* corner_light_prev; //unk type
	uint8_t* tile_light; // 0xb0 - per-tile light level
	uint32_t* tile_corner_y; // 0xb4 - screen Y per tile corner (corner grid)
	uint32_t* tile_top_y; // 0xb8 - screen Y of tile top edge (tile grid)
	uint32_t* tile_bottom_y; // 0xbc - screen Y of tile bottom edge (tile grid)
	uint32_t* height_cells; // 0xc0 - per-corner averaged terrain heights (stride view_tiles_x + 8)
	int32_t fps_frames; //unk type
	int32_t fps_time_ms; //unk type
	int32_t fps_last_tick; //unk type
	double fps;
	int32_t rbtn_drag;
	int32_t minimap_dirty;
	int32_t full_redraw_requested;
	int32_t tile_scroll_x; // 0xe4
	int32_t tile_scroll_y; // 0xe8
	int32_t prev_view_x; // 0xec
	int32_t prev_view_y; // 0xf0
	CRect dirty_rect;
	int32_t cached_msg_count;//unk type
	int32_t lightning_active;//unk type
	CArray<CRect> hit_rects;
	CWordArray hit_unit_ids;
	int32_t hit_count;//unk type
	CGameObject* primary_selected_object; //primarySelectedObject
	POSITION primary_selected_assoc; //primarySelectedAssoc
	int32_t selected_object_count; //selectedObjectCount
	uint32_t selection_flags;
	uint32_t selected_available_spell_mask; //selectedAvailableSpellMask
	int32_t selected_active_spell_mask; //unk type
	uint32_t active_spell_effect_mask; //activeSpellEffectMask
	int32_t spell_damage_min[24];
	int32_t spell_damage_max[24];
	int32_t spell_range_min[24];
	int32_t spell_range_max[24];
	int32_t spell_mana_cost1[24];
	int32_t spell_mana_cost2[24];
	int32_t spell_power_min[24];
	int32_t spell_power_max[24];
	int32_t spell_val1_min[24];
	int32_t spell_val1_max[24];
	int32_t spell_val2_min[24];
	int32_t spell_val2_max[24];
	int32_t spell_val3_min[24];
	int32_t spell_val3_max[24];
	int32_t spell_val4_min[24];
	int32_t spell_val4_max[24];
	int32_t spell_val5_min[24];
	int32_t spell_val5_max[24];
	int32_t spell_val6_min[24];
	int32_t spell_val6_max[24];
	int32_t spell_val7_min[24];
	int32_t spell_val7_max[24];
	CUnit* hovered_unit;
	uint32_t quest_some_id_2;
	uint32_t quest_landmark_some_id;
	uint32_t quest_building_some_id;
	uint32_t quest_some_id;
	uint32_t hovered_unit_id;
	int32_t last_mouse_tile_x;
	int32_t last_mouse_tile_y;
	int32_t order_mode;
	CArray<MapPlayerData*> map_players;
	MapPlayerData* my_main_unit;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> objects;
	CMap<uint16_t, uint16_t, CGameObject*, CGameObject*> projectiles;
	CMap<uint16_t, uint16_t, uint32_t, uint32_t> terrain_light_override_cells; //terrainLightOverrideCells
	int32_t next_projectile_id; //unk type
	GM_a28 msglog;
	int32_t anim_tick;
	int32_t anim_redraw_tick;
	int32_t gap_0xa90;
	CMap<uint16_t, uint16_t, uint32_t, uint32_t> lightning_cells;
	int32_t formation;
	int32_t wimpy;
	int32_t show_hp;
	int32_t flying_hp;
	
	int8_t shadow_step[41][41][2];
	uint8_t ____padding1[2];

	int32_t shadow_light[41][41]; // signed!
	uint16_t shadow_ray_len[41][41]; // unsigned!

	uint16_t ____padding2;

	int32_t shadow_scale; //unk type
	int32_t shadow_shift; //unk type
	CArray<TakeDamage> damage_labels;
	CBmp256* scratch_tile;
	CUnit* my_unit;
	std::array<uint8_t, 0xa00> kill_stats;
	QuestMap* quest_map;
	CArray<Item*> hero_inventory;
	std::array<Item*, 12> hero_equipment;
#ifdef A2CLIENT
	// client tail layout (retail class is 0x4a48 bytes, client ctor 402af6): the
	// server's 12-slot item array extends to 32 hat-DM slots (entries 12..31 are
	// the fields below), and the server's 40000 timer triple (field_0x49b8/
	// field_0x49bc/field_0x49c0 at the end of this block) moves behind the hat
	// containers to 0x4a38..0x4a40
	int32_t hat_item_slot12; // client 0x49b8 (ctor sets 0)
	int32_t hat_item_slot13; // client 0x49bc (ctor sets 0xF0000001)
	int32_t hat_item_slot14; // client 0x49c0 (ctor sets 0xF0000001)
	int32_t hat_cursor_slots[16]; // client 0x49c4..0x4a03 = slots 15..30 (ctor 0xF0000003; packet 0xc2 bulk state)
	int32_t hat_player_count; // client 0x4a04 = slot 31 (ctor 0x7fffffff; hat player-list state, packets 0x5c/0xc2)
	int32_t cursor_over_obj_last_index; // client 0x4a08
	int32_t last_focus_obj_index; // client 0x4a0c
	CStringArray hat_name_list; // client 0x4a10 — hat DM player names (packets 0x5a/0x5b/0x5c)
	CArray<uint32_t> hat_score_list; // client 0x4a24 — parallel scores (plain dwords), sorted descending

	void CloseHatOverlays(); // client-only helper wrapping Allods2.exe 41748d + 417902: closes shop/tavern/popup/1200 overlays
#endif
	int32_t ambient_view_x; //unk type
	int32_t ambient_view_y; //unk type
	int32_t ambient_sound_deadline; //unk type
	int32_t gap_0x49c4; //unk type

};

ASSERT_OFFSET(BigStruct2, scenario, 0x80);
ASSERT_OFFSET(BigStruct2, objects, 0x9d0);
ASSERT_OFFSET(BigStruct2, damage_labels, 0x3f54);
#ifdef A2CLIENT
ASSERT_OFFSET(BigStruct2, hat_cursor_slots, 0x49c4);
ASSERT_OFFSET(BigStruct2, hat_player_count, 0x4a04);
ASSERT_OFFSET(BigStruct2, hat_name_list, 0x4a10);
ASSERT_OFFSET(BigStruct2, hat_score_list, 0x4a24);
ASSERT_OFFSET(BigStruct2, ambient_view_x, 0x4a38);
ASSERT_SIZE(BigStruct2, 0x4a48);
#else
ASSERT_SIZE(BigStruct2, 0x49c8);
#endif

//60ce80
class VisRightPanel : public CVisualObject {
public:
	virtual ~VisRightPanel(); // 4B46E0 (client: 4b90b0)

	virtual void VMethod7() override; // 4AEE31 (client: 4b3771)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AEE44 (client: 4b3784)

	VisRightPanel(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aedcf (client: 4b370f) in asm
};
ASSERT_SIZE(VisRightPanel, 0x5c);


//60cef8
class VisMiniMap : public CVisualObject {
public:
	virtual ~VisMiniMap(); // 4B4730 (client: 4b9100)

	virtual void VMethod7() override; // 4AFB8E (client: 4b44c4)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4AF006 (client: 4b3946)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B0C2F (client: 4b5561)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B0788 (client: 4b50be)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B0C84 (client: 4b5b7e)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4B0AAD (client: 4b53df)

	VisMiniMap(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4aeec3 (client: 4b3803)

	void RebuildMap(); // 4af0dc (client: 4b3a1c) in asm
	void UpdateCursor(); // 4b042e (client: 4b4d64) in asm

public:
	BigStruct2* map_context;
	CBmp64* bitmap1;
	CBmp64* bitmap2;
	int32_t zoom;
	uint32_t viewer;
};
ASSERT_SIZE(VisMiniMap, 0x70);


//60cf70
class VisOrderToolbar : public CVisualObject {
public:
	virtual ~VisOrderToolbar(); // 4b4790 (client: 4b9160)

	virtual const char* GetHint() override; // 4B0D9A (client: 4b56cc)
	virtual void VMethod7() override; // 4B12E3 (client: 4b5c15)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B0ED1 (client: 4b5803)
	virtual int32_t OnMouseMove(uint32_t wparam, CPoint pos) override; // 4B12B3 (client: 4b5be5)
	virtual int32_t OnLButtonDown(uint32_t wparam, CPoint pos) override; // 4B107B (client: 4b59ad)
	virtual int32_t OnLButtonUp(uint32_t wparam, CPoint pos) override; // 4B124C (client: 4b5b7e)
	virtual int32_t OnLButtonDblClk(uint32_t wparam, CPoint pos) override; // 4B1213 (client: 4b5b45)
	virtual int32_t OnRButtonDown(uint32_t wparam, CPoint pos) override; // 4b1201 (client: 4b5b33)
	virtual int32_t OnRButtonUp(uint32_t wparam, CPoint pos) override; // 4B1287 (client: 4b5bb9)
	virtual int32_t OnRButtonDblClk(uint32_t wparam, CPoint pos) override; // 4b123a (client: 4b5b6c)

	void UpdateCursor(); // 4b1468 (client: 4b5d9a)

	VisOrderToolbar(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b0cfc (client: 4b562e) in asm
public:
	BigStruct2* map_context;
	uint32_t selected_order;
	uint32_t avail_orders_mask;
	uint32_t enabled;
	uint32_t dirty;
};
ASSERT_SIZE(VisOrderToolbar, 0x70);

//609a58 - console-style input line used by Vis1200
class Vis1200obj : public CVisualObject {
public:
	virtual ~Vis1200obj();  // client: 43e3c0

	virtual void VMethod7() override; // 43b32f (client: 43d73e)
	virtual void WriteData(void* buf) override; // 43ad35 (client: 43d144)
	virtual uint32_t DataSize() override; // 43c020 (client: 43e3b0)
	virtual void ReadData(const void* buf) override; // 43acca (client: 43d0d9)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43b283 (client: 43d692)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43b159 (client: 43d564)
	virtual int32_t OnChar(uint32_t wparam) override; // 43b240 (client: 43d64b)

	void AppendInputChar(uint8_t ch); // 43ae1c (client: 43d22b)
	void BackspaceInputChar(); // 43b03a (client: 43d445)
	void CompletePlayerName(); // 43b724 (client: 43db33)
	void DeletePreviousWord(); // 43ba75 (client: 43de84)
	void DrawInputLine(CString* str); // 43b504 (client: 43d913)

	Vis1200obj(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b, CGameFont* _font, uint16_t* _clr, const char* hint); //43ab28 (client: 43cf38)

	void ResetInputState(); //43ac8c (client: 43d09b) - reset input/history state
	void SaveHistoryBackup(); //43ad86 (client: 43d195) - commit current line to history
	void RecallHistoryPrev(); //43ada3 (client: 43d1b2) - recall previous line from history
	int32_t GetMatchPlayerIndex(); //4972f0 (client: 401780)
	int32_t GetMatchNameLength(); //497330 (client: 401780)

public:
	CStringArray history_backup;
	CStringArray history; // history
	CString input_line; // current line
	CGameFont* font; // 0x88
	uint16_t* clr; // 0x8c
	int32_t match_player_index; // 0x90
	int32_t match_name_length; // 0x94
	uint32_t blink_last_tick; // 0x98 - last input tick
	int32_t blink_now_tick; // 0x9c
	int32_t cursor_visible; // 0xa0
};
ASSERT_SIZE(Vis1200obj, 0xa4);

	int32_t IsWordSeparator(char ch); // 43aae0 in asm - chat separator test (' ', ',', '.', ':', ';')
	void MakeLowerDosCp(CString* str); // 476236 - lowercase respecting DOS CP

//609ad0
class Vis1200 : public VisScreen {
public:
	virtual ~Vis1200(); // 43c000 (client: 43e420)

	virtual void VMethod7() override; // 43BE80 (client: 43e28f)
	virtual void VMethod8(CRect* rect) override; // 43be73 (client: 43e282)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 43bdcb (client: 43e1da)
	virtual int32_t OnKeyDown(uint32_t wparam) override; // 43BDEC (client: 43e1fb)

	virtual void VMethod26() override; // 43BC3C (client: 43e04b)
	virtual void VMethod28() override; // 43BD2E (client: 43e13d)
	virtual void DoClose(uint32_t code) override; // 43BD96 (client: 43e1a5)

	int32_t GetMatchPlayerIndex(); //4972d0 (client: 430ac8)
	int32_t GetMatchNameLength(); //497310 (client: 430ac8)
	void RepositionAboveDialogs(); //43be9f (client: 43e2ae)

	Vis1200(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); // 43bb70 (client: 433fda) in asm

public:
	char buffer[256];
	Vis1200obj* text_block;
	uint32_t active;
};
ASSERT_SIZE(Vis1200, 0x170);

//60d070
class VisSideStatus : public CVisualObject {
public:
	virtual ~VisSideStatus(); // 4B4810 (client: 4b91e0)

	virtual const char* GetHint() override; // 4B45BB (client: 4b8f8a)
	virtual void VMethod7() override; // 4B4190 (client: 4b8b61)
	virtual int32_t MsgProc(uint32_t msg, uint32_t wparam, uint32_t lparam) override; // 4B4490 (client: 4b8e5f)

	void UpdateCursor(); //4b4019 (client: 4b89ea) in asm

	VisSideStatus(int32_t _id, int32_t l, int32_t t, int32_t r, int32_t b); //4b3fa3 (client: 4b8974) in asm
public:
	uint32_t dirty;
};
ASSERT_SIZE(VisSideStatus, 0x60);


#endif
