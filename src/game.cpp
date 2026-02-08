#include <cstdint>

#include "asm_mfc.h"
#include "player.h"
#include "players_list.h"
#include "server.h"

extern "C" {
	extern Server* Server_instance;
	extern PlayersList* PlayersList_global;
	extern uint8_t byte_6C3A08; // NetStru1

	void* __cdecl sub_401D90();
	void __fastcall unknown_libname_597(void* this_ptr);
	Player* __fastcall sub_496DD0(void* iter, int32_t edx, void* list);
	Player* __fastcall sub_496E20(void* iter);
	int __fastcall sub_496590(void* this_ptr);
	void* __fastcall sub_518544(void* this_ptr, int32_t edx, int id);
	void __fastcall sub_5170B6(void* this_ptr, int32_t edx, void* node);
	void __fastcall sub_51800F(void* this_ptr);
	void __fastcall sub_51D49B(void* this_ptr, int32_t edx, void* player);
	void __fastcall sub_534DDD(void* this_ptr);
	void __stdcall sub_43AA23(LPCTSTR message);
}

void sub_4954EA()
{
	if (Server_instance == nullptr) {
		return;
	}

	uint8_t* main_window = static_cast<uint8_t*>(sub_401D90());
	CString unused;

	uint32_t iter_state[2] = {};

	int target_index = sub_496590(main_window + 0x70C);
	int index = 0;

    Player* selected = nullptr;
	Player* player = sub_496DD0(iter_state, 0, PlayersList_global);
	while (player != nullptr) {
		if (player->is_ai == 0) {
			if (index == target_index) {
				selected = player;
				break;
			}
			++index;
		}

		player = sub_496E20(iter_state);
	}

	player = selected;
	if (player == nullptr) {
		return;
	}

	CString message = CString("Player ") + player->name + " kicked from server";
	sub_43AA23(message);

	uint16_t player_id = player->player_id;
	void* node = sub_518544(&byte_6C3A08, 0, player_id);
	if (node == nullptr) {
		return;
	}

	sub_5170B6(&byte_6C3A08, 0, node);
	sub_51800F(&byte_6C3A08);
	sub_51D49B(&byte_6C3A08, 0, player);
	player->field_0xa50 = Server_instance->tick16 - 1;
	sub_534DDD(PlayersList_global);
}
