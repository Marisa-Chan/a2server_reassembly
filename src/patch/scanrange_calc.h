#pragma once

// Terrain-aware scan-range vision calculator, shared by the client fog
// (CalcVisMapCLIENT, my-a2mgr scanrange.cpp) and the server-side player
// vision stamps (my-srvmgr scanrange.cpp / lib/ScanrangeCalc). Ported
// verbatim from my-srvmgr/lib/ScanrangeCalc.{hpp,cpp}.

#include <stdint.h>

// 41 * 41 = 1681
class CScanrangeCalc {
public:
	CScanrangeCalc();

	int8_t pTablesOffset[41][41][2];
	int16_t pTablesCost[41][41];
	int32_t pTablesVision[41][41];

	uint32_t ulScanShift;

	void InitializeTables();
	void CalculateVision(int32_t x, int32_t y, uint16_t scanrange, uint8_t (*func_GetHeight)(int16_t, int16_t) = NULL, bool (*func_CheckValid)(int16_t, int16_t) = NULL);

private:
	bool bDiv2;
	bool SetCell(uint8_t x, uint8_t y, uint8_t height_origin, uint8_t height_cell);
};
