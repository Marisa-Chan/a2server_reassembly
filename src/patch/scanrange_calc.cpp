// Terrain-aware scan-range vision calculator, ported verbatim from
// my-srvmgr/lib/ScanrangeCalc.cpp (see scanrange_calc.h).

#include "scanrange_calc.h"

#include <math.h>
#include <string.h>

CScanrangeCalc::CScanrangeCalc() {
	this->ulScanShift = 7;
}

void CScanrangeCalc::InitializeTables() {
	uint32_t ulScanShifted = 1 << this->ulScanShift;
	memset(this->pTablesCost, 0, sizeof(this->pTablesCost));
	memset(this->pTablesOffset, 0, sizeof(this->pTablesOffset));

	for (int32_t i = 0; i <= 20; ++i) {
		for (int32_t j = 0; j <= 20; ++j) {
			if (j >= (i / 2)) {
				if (j <= (i * 2)) {
					this->pTablesOffset[20 + i][20 + j][0] = -1;
					this->pTablesOffset[20 + i][20 + j][1] = -1;
					this->pTablesOffset[20 - i][20 - j][0] = 1;
					this->pTablesOffset[20 - i][20 - j][1] = 1;
					this->pTablesOffset[20 + i][20 - j][0] = -1;
					this->pTablesOffset[20 + i][20 - j][1] = 1;
					this->pTablesOffset[20 - i][20 + j][0] = 1;
					this->pTablesOffset[20 - i][20 + j][1] = -1;
				} else {
					this->pTablesOffset[20 + i][20 + j][0] = 0;
					this->pTablesOffset[20 + i][20 + j][1] = -1;
					this->pTablesOffset[20 - i][20 - j][0] = 0;
					this->pTablesOffset[20 - i][20 - j][1] = 1;
					this->pTablesOffset[20 + i][20 - j][0] = 0;
					this->pTablesOffset[20 + i][20 - j][1] = 1;
					this->pTablesOffset[20 - i][20 + j][0] = 0;
					this->pTablesOffset[20 - i][20 + j][1] = -1;
				}
			} else {
				this->pTablesOffset[20 + i][20 + j][0] = -1;
				this->pTablesOffset[20 + i][20 + j][1] = 0;
				this->pTablesOffset[20 - i][20 - j][0] = 1;
				this->pTablesOffset[20 - i][20 - j][1] = 0;
				this->pTablesOffset[20 + i][20 - j][0] = -1;
				this->pTablesOffset[20 + i][20 - j][1] = 0;
				this->pTablesOffset[20 - i][20 + j][0] = 1;
				this->pTablesOffset[20 - i][20 + j][1] = 0;
			}

			int32_t v4;
			if (i > j) {
				v4 = i;
			} else {
				v4 = j;
			}

			int16_t v1 = (int16_t)(sqrt((double)(j * j + i * i)) * (double)ulScanShifted / (double)v4);
			this->pTablesCost[20 + i][20 + j] = v1;
			this->pTablesCost[20 + i][20 - j] = v1;
			this->pTablesCost[20 - i][20 + j] = v1;
			this->pTablesCost[20 - i][20 - j] = v1;
		}
	}

	this->pTablesCost[20][20] = 0;
}

// Character ScanRange
// x, y - cell coordinates inside the 41x41 table
// height_origin - relief at the origin cell
// height_cell - relief at this cell
bool CScanrangeCalc::SetCell(uint8_t x, uint8_t y, uint8_t height_origin, uint8_t height_cell) {
	int16_t vision_previous = this->pTablesVision[x + this->pTablesOffset[x][y][0]][y + this->pTablesOffset[x][y][1]];
	uint16_t cost = this->pTablesCost[x][y];
	// if (this->bDiv2) cost -= 18;
	this->pTablesVision[x][y] = vision_previous - (height_cell - height_origin + cost);
	return (this->pTablesVision[x][y] >= 0);
}

// Character ScanRange Calculation
// x, y - character (?) coords
// scanrange - character vision
// GetHeight - relief lookup
// CheckValid - can we cast there or not
void CScanrangeCalc::CalculateVision(int32_t x, int32_t y, uint16_t scanrange, uint8_t (*func_GetHeight)(int16_t, int16_t), bool (*func_CheckValid)(int16_t, int16_t)) {
	memset(this->pTablesVision, 0, sizeof(this->pTablesVision));
	if (func_GetHeight == NULL) {
		return;
	}
	if (func_CheckValid == NULL) {
		return;
	}

	uint16_t vision = scanrange;

	uint16_t vision2 = (1 << (this->ulScanShift - 1)) + (vision >> (8 - this->ulScanShift));

	int32_t genX = x - 20;
	int32_t genY = y - 20;
	uint8_t ht_origin = func_GetHeight(x, y);

	this->pTablesVision[20][20] = vision2;
	for (int32_t i = 1; i < 20; i++) {
		bool fdisp = false;
		for (int32_t j = -i; j < i + 1; j++) {
			this->bDiv2 = false;
			if (func_CheckValid(genX + (20 + j), genY + (20 - i)) &&
				this->SetCell(20 + j, 20 - i, ht_origin, func_GetHeight(genX + (20 + j), genY + (20 - i)))) {
				fdisp = true;
			}
			if (func_CheckValid(genX + (20 + j), genY + (20 + i)) &&
				this->SetCell(20 + j, 20 + i, ht_origin, func_GetHeight(genX + (20 + j), genY + (20 + i)))) {
				fdisp = true;
			}
			this->bDiv2 = true;
			if (func_CheckValid(genX + (20 - i), genY + (20 + j)) &&
				this->SetCell(20 - i, 20 + j, ht_origin, func_GetHeight(genX + (20 - i), genY + (20 + j)))) {
				fdisp = true;
			}
			if (func_CheckValid(genX + (20 + i), genY + (20 - j)) &&
				this->SetCell(20 + i, 20 - j, ht_origin, func_GetHeight(genX + (20 + i), genY + (20 - j)))) {
				fdisp = true;
			}
		}

		if (!fdisp) {
			break;
		}
	}
}
