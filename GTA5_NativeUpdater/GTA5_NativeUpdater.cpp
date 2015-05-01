#include "stdafx.h"

#define ECHO_DEF(x) #x

// Find ScriptSetup
// 40 53 48 83 EC 20 83 F9 01 0F 85 A3 00 00 00

//

// Find All Native References
// 48 B9 ? ? ? ? ? ? ? ? E8

enum eScriptNativeCollectionIndex
{
	INDEX_APP			= 0,
	INDEX_AUDIO			= 1,
	INDEX_CAM			= 2,
	_NULLSTUB1			= 3,
	INDEX_UI			= 4,
	INDEX_GAMEPLAY		= 5,
	INDEX_CONTROLS		= 6,
	INDEX_PED			= 7,
	INDEX_PLAYER		= 8,
	INDEX_AI			= 9,
	INDEX_VEHICLE		= 10,
	_NULLSTUB2			= 11,
	INDEX_OBJECT		= 12,
	INDEX_SCRIPT		= 13,
	INDEX_STATS			= 14,
	INDEX_STREAMING		= 15,
	INDEX_PATHFIND		= 16,
	INDEX_WEAPON		= 17,
	INDEX_FIRE			= 18,
	INDEX_ZONE			= 19,
	INDEX_GRAPHICS		= 20,
	_NULLSTUB3			= 21,
	INDEX_CUTSCENE		= 22,
	INDEX_TIME			= 23,
	INDEX_NETWORK		= 24,
	INDEX_BRAIN			= 25,
	_NULLSTUB4			= 26,
	INDEX_WATER			= 27,
	_NULLSTUB5			= 28,
	INDEX_UNK			= 29,
	INDEX_DECISIONEVENT = 30,
	INDEX_INTERIOR		= 31,
	INDEX_ROPE			= 32,
	INDEX_MOBILE		= 33,
	INDEX_ENTITY		= 34,
	INDEX_ITEMSET		= 35,
	_NULLSTUB6			= 36,
	INDEX_WORLDPROBE	= 37,
	INDEX_DECORATOR		= 38,
	INDEX_DATAFILE		= 39,
	INDEX_UNK_SC		= 40,
	INDEX_DLC1			= 41,
	INDEX_DLC2			= 42,
	INDEX_NETWORKCASH	= 43,
	INDEX_UNK1			= 44,
	INDEX_UNK2			= 45,
	INDEX_UNDOC001		= 46,
	INDEX_UNDOC002		= 47,
	MAX
};

int gNativeCount[] = {
	17,						// INDEX_APP
	219,					// INDEX_AUDIO
	210,					// INDEX_CAM
};

char* gNamespaceNames[] = {
	"INDEX_APP",
	"INDEX_AUDIO",
	"INDEX_CAM",
	"_NULLSTUB1",
	"INDEX_UI",
	"INDEX_GAMEPLAY",
	"INDEX_CONTROLS",
	"INDEX_PED",
	"INDEX_PLAYER",
	"INDEX_AI",
	"INDEX_VEHICLE",
	"_NULLSTUB2",
	"INDEX_OBJECT",
	"INDEX_SCRIPT",
	"INDEX_STATS",
	"INDEX_STREAMING",
	"INDEX_PATHFIND",
	"INDEX_WEAPON",
	"INDEX_FIRE",
	"INDEX_ZONE",
	"INDEX_GRAPHICS",
	"_NULLSTUB3",
	"INDEX_CUTSCENE",
	"INDEX_TIME",
	"INDEX_NETWORK",
	"INDEX_BRAIN",
	"_NULLSTUB4",
	"INDEX_WATER",
	"_NULLSTUB5",
	"INDEX_UNK",
	"INDEX_DECISIONEVENT",
	"INDEX_INTERIOR",
	"INDEX_ROPE",
	"INDEX_MOBILE",
	"INDEX_ENTITY",
	"INDEX_ITEMSET",
	"_NULLSTUB6",
	"INDEX_WORLDPROBE",
	"INDEX_DECORATOR",
	"INDEX_DATAFILE",
	"INDEX_UNK_SC",
	"INDEX_DLC1",
	"INDEX_DLC2",
	"INDEX_NETWORKCASH",
	"INDEX_UNK1",
	"INDEX_UNK2",
	"INDEX_UNDOC001",
	"INDEX_UNDOC002",
	"MAX"
};

struct Opcode {
	DWORD64				addr;
	std::vector<uint8>	bytes;
};

struct Opcodes {
	std::vector<Opcode> op;
};

// This will keep going until it hits a JMP or RET
// It's super basic, but fits our needs
Opcodes GetFunctionOpcodes(DWORD64 dwFunctionAddress) {
	Opcodes ret;

	for (UCHAR* pStart = (UCHAR*)dwFunctionAddress;;) {
		size_t opcodeSize = LDE(pStart, 64);

		Opcode op;
		
		op.addr = (DWORD64)pStart;

		for (size_t i = 0; i < opcodeSize; i++) {
			op.bytes.push_back(pStart[i]);
		}

		ret.op.push_back(op);
		
		// Add stuff here if it gets funky
		if (pStart[0] == 0xC3 || pStart[0] == 0xC2 || pStart[0] == 0xE9)
			break;

		pStart += opcodeSize;
	}

	return ret;
}

std::vector<DWORD64> GetHashesInFunction(DWORD64 dwFunctionStart) {
	std::vector<DWORD64> hashes;

	Opcodes ops = GetFunctionOpcodes(dwFunctionStart);

	for (auto op : ops.op) {
		if (op.bytes.size() > 2) {
			if (op.bytes.at(0) == 0x48) {
				if (op.bytes.at(1) == 0xB9) {
					hashes.push_back(*(DWORD64*)(op.addr + 2));
				}
			}
		}
	}

	return hashes;
}

void HandleNamespace(eScriptNativeCollectionIndex idx, DWORD64 dwFunctionStart) {
	if (idx == _NULLSTUB1 || idx == _NULLSTUB2) {
		return; // Nope
	}

	printf("%s (%I64X)\n", gNamespaceNames[idx], dwFunctionStart);

	std::vector<DWORD64> hashes = GetHashesInFunction(dwFunctionStart);

	printf("Hash Count: %d\n", hashes.size());

	for (size_t i = 0; i < hashes.size(); i++) {
		DWORD64 hash = hashes[i];

		printf("%s[%d] = 0x%I64X\n", gNamespaceNames[idx], i, hash);
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Please supply an executable name to use...\n");
		return 0;
	}

	char* pszFileName = argv[1];

	printf("Using File: %s\n", pszFileName);

	HMODULE hGTA = LoadLibraryA(pszFileName);

	MODULEINFO miGTA;

	if (hGTA && GetModuleInformation(GetCurrentProcess(), hGTA, &miGTA, sizeof(MODULEINFO))) {
		printf("GTA5: %I64X %I64X\n", miGTA.lpBaseOfDll, miGTA.SizeOfImage);

		DWORD64 dwAddressOfScriptSetup = Pattern::Scan(miGTA, "40 53 48 83 EC 20 83 F9 01 0F 85 A3 00 00 00");

		if (dwAddressOfScriptSetup != 0) {
			printf("Script Setup: %I64X (%I64X)\n", dwAddressOfScriptSetup, (dwAddressOfScriptSetup - (DWORD64)hGTA));

			DWORD64 dwAddressOfAddNatives = dwAddressOfScriptSetup + 0x78;

			dwAddressOfAddNatives = (*(DWORD*)(dwAddressOfAddNatives + 1) + dwAddressOfAddNatives + 5);

			printf("Add Natives: %I64X (%I64X)\n", dwAddressOfAddNatives, (dwAddressOfAddNatives - (DWORD64)hGTA));

			int scriptNamespaceIndex = -1;

			Opcodes ops = GetFunctionOpcodes(dwAddressOfAddNatives);

			for (auto op : ops.op) {
				if (op.bytes.data()[0] == 0xE8) { // CALL
					scriptNamespaceIndex++; // We're in the namespace

					DWORD64 dwFunctionCallAddress = (*(DWORD*)(op.addr + 1) + op.addr + 5);

					HandleNamespace((eScriptNativeCollectionIndex)scriptNamespaceIndex, dwFunctionCallAddress);
				}
				else if (op.bytes.data()[0] == 0xE9) { // JMP
					scriptNamespaceIndex++; // We're in the namespace

					// get jmp dest and do this one too why not

					break;
				}
			}
		}
		else {
			printf("Unable to find Script Setup pattern.\n");
		}
	}
	else {
		printf("Failed LoadLibrary on \"%s\" (No File?)\n", pszFileName);
	}

	return 0;
}