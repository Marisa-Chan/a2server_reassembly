#include "player_file.h"

#include <cstring>

#include "file.h"
#include "game_app.h"
#include "packet.h"


// 4F5308
extern "C" int32_t __cdecl CheckSum_4F5308(const uint8_t* data, int32_t size)
{
	int32_t checksum = 0;
	for (int32_t index = 0; index < size; index++) {
		checksum = checksum * 2 + data[index];
	}

	return checksum;
}

// 4F535E
extern "C" void __cdecl LoadDecrypt_4F535E(uint8_t* data, int32_t size, uint16_t key)
{
	uint32_t state = key | (static_cast<uint32_t>(key) << 16);
	for (int32_t index = 0; index < size; index++) {
		data[index] ^= static_cast<uint8_t>(state >> 16);
		state <<= 1;
		if ((index & 0x0F) == 0x0F) {
			state |= key;
		}
	}
}

namespace {

constexpr uint32_t kPlayerFileMagic = 0x04507989;
constexpr int32_t kKillStatsSize = 0x0A00;

struct PlayerFileSectionHeader {
	uint32_t tag;
	uint32_t size;
	uint16_t reserved;
	uint16_t key;
	uint32_t checksum;
};

void WriteEncryptedSection(CFile& file, uint32_t tag, uint8_t* data, uint32_t size)
{
	PlayerFileSectionHeader header = {};
	header.tag = tag;
	header.size = size;
	header.key = GetRandS16(0x7FFF);
	header.checksum = CheckSum_4F5308(data, size);

	LoadDecrypt_4F535E(data, size, header.key);
	file.Write(&header, sizeof(header));
	file.Write(data, size);

    // What's the point of this? Remove the unencrypted bytes from memory?
	LoadDecrypt_4F535E(data, size, header.key);
}

uint8_t* GetPacketPayload(PacketUnitStateVec* packet)
{
	return reinterpret_cast<uint8_t*>(packet) + 0x0A;
}

uint32_t GetPacketPayloadSize(const PacketUnitStateVec* packet)
{
	return packet->data_size + 9;
}

void MaybeWriteRandomByte(CFile& file, uint16_t key, uint16_t mask)
{
	if ((key & mask) == 0) {
		return;
	}

	uint8_t random_byte = GetRandS16(0xFF);
	file.Write(&random_byte, sizeof(random_byte));
}

void WriteStatsSection(CFile& file, FileSectionStats* stats)
{
	PlayerFileSectionHeader header = {};
	header.tag = 0x41392521;
	header.key = GetRandS16(0x7FFF);
	header.size = sizeof(FileSectionStats);
	for (int32_t bit = 0; bit < 16; bit++) {
		if ((header.key & (1u << bit)) != 0) {
			header.size += 1;
		}
	}
	header.checksum = CheckSum_4F5308(reinterpret_cast<uint8_t*>(stats), sizeof(FileSectionStats));

	file.Write(&header, sizeof(header));

	MaybeWriteRandomByte(file, header.key, 0x0001);
	uint32_t monster_kills = stats->monster_kills ^ 0x01529251;
	file.Write(&monster_kills, sizeof(monster_kills));

	MaybeWriteRandomByte(file, header.key, 0x0002);
	uint32_t player_kills = stats->player_kills - (stats->monster_kills * 5 + 0x13141516);
	file.Write(&player_kills, sizeof(player_kills));

	MaybeWriteRandomByte(file, header.key, 0x0004);
	uint32_t frags = stats->frags - (stats->player_kills * 7 + 0x00ABCDEF);
	file.Write(&frags, sizeof(frags));

	MaybeWriteRandomByte(file, header.key, 0x0008);
	uint32_t deaths = stats->deaths ^ 0x17FF12AA;
	file.Write(&deaths, sizeof(deaths));

	MaybeWriteRandomByte(file, header.key, 0x0010);
	int32_t money = stats->money - (stats->monster_kills * 3 - 0x21524542);
	file.Write(&money, sizeof(money));

	MaybeWriteRandomByte(file, header.key, 0x0020);
	int8_t body = stats->body - (stats->money * 0x11 + stats->monster_kills * 0x13);
	file.Write(&body, sizeof(body));

	MaybeWriteRandomByte(file, header.key, 0x0040);
	int8_t reaction = stats->reaction - stats->body * 3;
	file.Write(&reaction, sizeof(reaction));

	MaybeWriteRandomByte(file, header.key, 0x0080);
	int8_t mind = stats->mind - (stats->body + stats->reaction * 5);
	file.Write(&mind, sizeof(mind));

	MaybeWriteRandomByte(file, header.key, 0x0100);
	int8_t spirit = stats->spirit - (stats->body * 7 + stats->mind * 9);
	file.Write(&spirit, sizeof(spirit));

	MaybeWriteRandomByte(file, header.key, 0x4000);
	uint32_t spells = stats->spells + 0x10121974;
	file.Write(&spells, sizeof(spells));

	MaybeWriteRandomByte(file, header.key, 0x2000);
	file.Write(&stats->active_spell, sizeof(stats->active_spell));

	for (int32_t index = 0; index < 5; index++) {
		MaybeWriteRandomByte(file, header.key, 0x200 << index);

		uint32_t encoded_exp = 0;
		if (index == 0) {
			encoded_exp = stats->experience[index] ^ 0xDADEDADE;
		} else {
			encoded_exp = stats->experience[index] + stats->experience[index - 1] * 0x771;
		}

		file.Write(&encoded_exp, sizeof(encoded_exp));
	}
}

// Static buffers for ParsePlayerFile_4F62E6
PacketUnitStateVec s_equip_packet;
PacketUnitStateVec s_inv_packet;
FileSectionBasicInfo s_basic_info_buf;
FileSectionStats s_stats_buf;
uint8_t s_kill_stats_buf[kKillStatsSize];
uint8_t s_section_40a_buf[kKillStatsSize];
uint32_t s_section_40a_size;
uint8_t s_compressed_buf[kKillStatsSize];

void MaybeSkipRandomByte(CFile& file, uint16_t key, uint16_t mask)
{
	if ((key & mask) != 0) {
		uint8_t dummy;
		file.Read(&dummy, 1);
	}
}

bool ReadDecryptAndVerify(CFile& file, const PlayerFileSectionHeader& header, uint8_t* buffer)
{
	file.Read(buffer, header.size);
	LoadDecrypt_4F535E(buffer, header.size, header.key);
	return CheckSum_4F5308(buffer, header.size) == static_cast<int32_t>(header.checksum);
}

bool ReadStatsSection(CFile& file, const PlayerFileSectionHeader& header)
{
	MaybeSkipRandomByte(file, header.key, 0x0001);
	file.Read(&s_stats_buf.monster_kills, 4);
	s_stats_buf.monster_kills ^= 0x01529251;

	MaybeSkipRandomByte(file, header.key, 0x0002);
	file.Read(&s_stats_buf.player_kills, 4);
	s_stats_buf.player_kills += s_stats_buf.monster_kills * 5 + 0x13141516;

	MaybeSkipRandomByte(file, header.key, 0x0004);
	file.Read(&s_stats_buf.frags, 4);
	s_stats_buf.frags += s_stats_buf.player_kills * 7 + 0x00ABCDEF;

	MaybeSkipRandomByte(file, header.key, 0x0008);
	file.Read(&s_stats_buf.deaths, 4);
	s_stats_buf.deaths ^= 0x17FF12AA;

	MaybeSkipRandomByte(file, header.key, 0x0010);
	file.Read(&s_stats_buf.money, 4);
	s_stats_buf.money += s_stats_buf.monster_kills * 3 - 0x21524542;

	MaybeSkipRandomByte(file, header.key, 0x0020);
	file.Read(&s_stats_buf.body, 1);
	s_stats_buf.body += static_cast<uint8_t>(s_stats_buf.money * 0x11 + s_stats_buf.monster_kills * 0x13);

	MaybeSkipRandomByte(file, header.key, 0x0040);
	file.Read(&s_stats_buf.reaction, 1);
	s_stats_buf.reaction += static_cast<uint8_t>(s_stats_buf.body * 3);

	MaybeSkipRandomByte(file, header.key, 0x0080);
	file.Read(&s_stats_buf.mind, 1);
	s_stats_buf.mind += static_cast<uint8_t>(s_stats_buf.body + s_stats_buf.reaction * 5);

	MaybeSkipRandomByte(file, header.key, 0x0100);
	file.Read(&s_stats_buf.spirit, 1);
	s_stats_buf.spirit += static_cast<uint8_t>(s_stats_buf.body * 7 + s_stats_buf.mind * 9);

	MaybeSkipRandomByte(file, header.key, 0x4000);
	file.Read(&s_stats_buf.spells, 4);
	s_stats_buf.spells -= 0x10121974;

	MaybeSkipRandomByte(file, header.key, 0x2000);
	file.Read(&s_stats_buf.active_spell, 4);

	for (int32_t i = 0; i < 5; i++) {
		MaybeSkipRandomByte(file, header.key, 0x200 << i);
		file.Read(&s_stats_buf.experience[i], 4);
		if (i == 0) {
			s_stats_buf.experience[i] ^= 0xDADEDADE;
		} else {
			s_stats_buf.experience[i] -= s_stats_buf.experience[i - 1] * 0x771;
		}
	}

	return CheckSum_4F5308(reinterpret_cast<uint8_t*>(&s_stats_buf), sizeof(FileSectionStats)) == header.checksum;
}

bool ReadCompressedKillStats(CFile& file, const PlayerFileSectionHeader& header)
{
	file.Read(s_compressed_buf, header.size);
	LoadDecrypt_4F535E(s_compressed_buf, header.size, header.key);
	if (CheckSum_4F5308(s_compressed_buf, header.size) != header.checksum) {
		return false;
	}

	uint32_t uncompressed_size = *reinterpret_cast<uint32_t*>(s_compressed_buf);
	uint8_t* temp = new uint8_t[uncompressed_size];

	uint8_t* read_ptr = s_compressed_buf + 4;
	int32_t read_offset = 4;
	uint8_t* write_ptr = temp;

	while (read_offset < static_cast<int32_t>(header.size)) {
		if ((*read_ptr & 0x80) != 0) {
			uint8_t count = *read_ptr & 0x7F;
			uint8_t value = read_ptr[1];
			for (int32_t j = 0; j < count; j++) {
				*write_ptr++ = value;
			}
			read_ptr += 2;
			read_offset += 2;
		} else {
			uint8_t count = *read_ptr;
			std::memcpy(write_ptr, read_ptr + 1, count);
			write_ptr += count;
			read_ptr += count + 1;
			read_offset += count + 1;
		}
	}

	std::memcpy(s_kill_stats_buf, temp, uncompressed_size);
	delete[] temp;
	return true;
}

void WriteKillStatsSection(CFile& file, uint8_t* kill_stats)
{
	uint8_t* buffer = new uint8_t[kKillStatsSize];
	if (buffer == nullptr) {
		return;
	}

	buffer[0] = 0;
	buffer[1] = 10;
	buffer[2] = 0;
	buffer[3] = 0;

	uint8_t* write_ptr = buffer + 4;
	int32_t read_offset = 0;
	uint8_t* read_ptr = kill_stats;

	while (read_offset < kKillStatsSize) {
		bool repeated_run = read_offset + 1 < kKillStatsSize && read_ptr[0] == read_ptr[1];
		if (repeated_run) {
			uint8_t run_length = 1;
			while (run_length < 0x7F && read_offset + run_length < kKillStatsSize && read_ptr[0] == read_ptr[run_length]) {
				run_length += 1;
			}

			*write_ptr = run_length | 0x80;
			write_ptr[1] = read_ptr[0];
			write_ptr += 2;
			read_ptr += run_length;
			read_offset += run_length;
			continue;
		}

		uint8_t run_length = 1;
		while (run_length < 0x7F && read_offset + run_length < kKillStatsSize && read_ptr[run_length - 1] != read_ptr[run_length]) {
			run_length += 1;
		}
		if (read_offset + run_length == kKillStatsSize) {
			run_length += 1;
		}

		*write_ptr = run_length - 1;
		std::memcpy(write_ptr + 1, read_ptr, run_length - 1);
		write_ptr += run_length;
		read_ptr += run_length - 1;
		read_offset += run_length - 1;
	}

	uint32_t compressed_size = static_cast<uint32_t>(write_ptr - buffer);
	WriteEncryptedSection(file, 0x55555555, buffer, compressed_size);
	delete[] buffer;
}

} // namespace

// 4F53EA
extern "C" int32_t __cdecl WritePlayerFile_4F53EA(const char* filename, FileSectionBasicInfo* basic_info, FileSectionStats* stats, uint8_t* kill_stats, PacketUnitStateVec* equip_pkt, PacketUnitStateVec* inv_pkt, uint8_t* section_40a, uint32_t size_40a)
{
	FileSectionBasicInfo* merged_basic_info = nullptr;
	FileSectionStats* merged_stats = nullptr;
	uint8_t* merged_kill_stats = nullptr;
	PacketUnitStateVec* merged_equip = nullptr;
	PacketUnitStateVec* merged_inventory = nullptr;
	uint8_t* merged_param7 = nullptr;
	uint32_t merged_param8 = 0;

	CFile file;
	if (file.Open(filename, CFile::modeRead, nullptr)) {
		if (file.GetLength() != 0) {
			if (ParsePlayerFile_4F62E6(&file, &merged_basic_info, &merged_stats, &merged_kill_stats, &merged_equip, &merged_inventory, &merged_param7, &merged_param8) == 0) {
				file.Close();
				return 0;
			}
		}

		file.Close();
	}

	if (!file.Open(filename, CFile::modeCreate | CFile::modeWrite, nullptr)) {
		return 0;
	}

	if (basic_info != nullptr) {
		merged_basic_info = basic_info;
	}
	if (merged_basic_info == nullptr) {
		return 0;
	}

	if (stats != nullptr) {
		merged_stats = stats;
	}
	if (kill_stats != nullptr) {
		merged_kill_stats = kill_stats;
	}
	if (equip_pkt != nullptr) {
		merged_equip = equip_pkt;
	}
	if (inv_pkt != nullptr) {
		merged_inventory = inv_pkt;
	}
	if (section_40a != nullptr) {
		merged_param7 = section_40a;
		merged_param8 = size_40a;
	}

	file.Write(&kPlayerFileMagic, sizeof(kPlayerFileMagic));

	WriteEncryptedSection(file, 0xAAAAAAAA, reinterpret_cast<uint8_t*>(merged_basic_info), sizeof(FileSectionBasicInfo));

	if (merged_kill_stats != nullptr) {
		WriteKillStatsSection(file, merged_kill_stats);
	}

	if (merged_param7 != nullptr) {
		WriteEncryptedSection(file, 0x40A40A40, merged_param7, merged_param8);
	}

	if (merged_equip != nullptr) {
		WriteEncryptedSection(file, 0xDE0DE0DE, GetPacketPayload(merged_equip), GetPacketPayloadSize(merged_equip));
	}

	if (merged_stats != nullptr) {
		WriteStatsSection(file, merged_stats);
	}

	if (merged_inventory != nullptr) {
		WriteEncryptedSection(file, 0x3A5A3A5A, GetPacketPayload(merged_inventory), GetPacketPayloadSize(merged_inventory));
	}

	file.Close();
	return 1;
}

// 4F62E6
extern "C" int32_t __cdecl ParsePlayerFile_4F62E6(CFile* file, FileSectionBasicInfo** basic_info, FileSectionStats** stats, uint8_t** kill_stats, PacketUnitStateVec** equipment, PacketUnitStateVec** inventory, uint8_t** section_40a, uint32_t* size_40a)
{
	*basic_info = nullptr;
	*stats = nullptr;
	*kill_stats = nullptr;
	*equipment = nullptr;
	*inventory = nullptr;
	*section_40a = nullptr;
	*size_40a = 0;

	uint32_t magic = 0;
	file->Read(&magic, sizeof(magic));
	if (magic != kPlayerFileMagic) {
		return 0;
	}

	while (file->GetPosition() < file->GetLength()) {
		PlayerFileSectionHeader header = {};
		file->Read(&header, sizeof(header));

		switch (header.tag) {
            case 0xAAAAAAAA:
            {
                if (!ReadDecryptAndVerify(*file, header, reinterpret_cast<uint8_t*>(&s_basic_info_buf))) {
                    return 0;
                }
                *basic_info = &s_basic_info_buf;
                break;
            }
            case 0x41392521:
            {
                if (!ReadStatsSection(*file, header)) {
                    return 0;
                }
                *stats = &s_stats_buf;
                break;
            }
            case 0x55555555:
            {
                if (!ReadCompressedKillStats(*file, header)) {
                    return 0;
                }
                *kill_stats = s_kill_stats_buf;
                break;
            }
            case 0x3A5A3A5A:
            {
                if (!ReadDecryptAndVerify(*file, header, GetPacketPayload(&s_inv_packet))) {
                    return 0;
                }
                *inventory = &s_inv_packet;
                break;
            }
            case 0x40A40A40:
            {
                if (!ReadDecryptAndVerify(*file, header, s_section_40a_buf)) {
                    return 0;
                }
                s_section_40a_size = header.size;
                *section_40a = s_section_40a_buf;
                *size_40a = s_section_40a_size;
                break;
            }
            case 0xDE0DE0DE:
            {
                if (!ReadDecryptAndVerify(*file, header, GetPacketPayload(&s_equip_packet))) {
                    return 0;
                }
                *equipment = &s_equip_packet;
                break;
            }
            default:
                return 0;
		}
	}

	return 1;
}
