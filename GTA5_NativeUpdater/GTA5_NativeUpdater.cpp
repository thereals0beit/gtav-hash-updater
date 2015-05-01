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

struct Native {
	DWORD64 hash;
	DWORD64 func;

	DWORD64 GetFunctionRoughHash();
};

struct Namespace {
	std::vector<Native> natives;
	std::string name;
};

// This will keep going until it hits a JMP or RET
// It's super basic, but fits our needs
Opcodes GetFunctionOpcodes(DWORD64 dwFunctionAddress) {
	Opcodes ret;

	for (UCHAR* pStart = (UCHAR*)dwFunctionAddress;;) {
		if (pStart[0] == 0xCC)
			break; // Something else may have happened, lmao

		size_t opcodeSize = LDE(pStart, 64);

		//printf("SIZE: %d\n", opcodeSize);
		
		// This needs serious bug fixing
		if (opcodeSize == -1 || opcodeSize == 0) {
			//printf("Opcode Error: 0x%X\n", pStart[0]);
			break;
		}

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

// This is a function which can sort of vaguely tell us if a native has changed or not.
// It's completely insane, truly, but hey I'm in a damn hurry.
// Uncomment at your peril
DWORD64 Native::GetFunctionRoughHash() {
	DWORD64 pseudoHash = 0;

	/*
	Opcodes ops = GetFunctionOpcodes(this->func);

	for (auto op : ops.op) {
		if (op.bytes.size() <= 0)
			continue; // how

		pseudoHash += op.bytes[0];

		if (op.bytes.size() > 1 && op.bytes[0] == 0x48) {
			pseudoHash += op.bytes[1];
		}
		else if (op.bytes[0] == 0xE8) { // We don't handle JMPs unfortunately, mostly because it can cause serious issues with functions with random JMPs in it
			DWORD64 dwFunctionCallAddress = (*(int*)(op.addr + 1) + op.addr + 5);

			Opcodes funcOps = GetFunctionOpcodes(dwFunctionCallAddress);

			for (auto fop : funcOps.op) {
				if (fop.bytes.size() <= 0)
					continue; // how

				pseudoHash += fop.bytes[0];
			}
		}
	}
	*/

	return pseudoHash;
}

bool isNullStub(eScriptNativeCollectionIndex idx) {
	return (
		idx == _NULLSTUB1 ||
		idx == _NULLSTUB2 ||
		idx == _NULLSTUB3 ||
		idx == _NULLSTUB4 ||
		idx == _NULLSTUB5 ||
		idx == _NULLSTUB6);
}

std::vector<DWORD64> GetHashesInFunction(DWORD64 dwFunctionStart) {
	std::vector<DWORD64> hashes;

	Opcodes ops = GetFunctionOpcodes(dwFunctionStart);

	for (auto op : ops.op) {
		// mov rcx, [hash]
		if (op.bytes.size() > 2 && op.bytes.at(0) == 0x48 && op.bytes.at(1) == 0xB9) {
			hashes.push_back(*(DWORD64*)(op.addr + 2));
		}
	}

	return hashes;
}

std::vector<Native> GetNativesInFunction(DWORD64 dwFunctionStart) {
	std::vector<Native> n;

	Opcodes ops = GetFunctionOpcodes(dwFunctionStart);

	for (auto op : ops.op) {
		// mov rcx, [hash]
		if (op.bytes.size() > 2 && op.bytes.at(0) == 0x48 && op.bytes.at(1) == 0xB9) {
			Native nat;

			nat.hash = *(DWORD64*)(op.addr + 2);
			nat.func = op.addr + *(int*)(op.addr - 4);

			n.push_back(nat);
		}
	}

	return n;
}

std::vector<Namespace> GetNamespaceDataForExe(const char* path) {
	std::vector<Namespace> ns;

	HMODULE hGTA = LoadLibraryA(path);

	MODULEINFO miGTA;

	if (hGTA && GetModuleInformation(GetCurrentProcess(), hGTA, &miGTA, sizeof(MODULEINFO))) {
		//printf("GTA5: %I64X %I64X\n", miGTA.lpBaseOfDll, miGTA.SizeOfImage);

		DWORD64 dwAddressOfScriptSetup = Pattern::Scan(miGTA, "40 53 48 83 EC 20 83 F9 01 0F 85 A3 00 00 00");

		if (dwAddressOfScriptSetup != 0) {
			//printf("Script Setup: %I64X (%I64X)\n", dwAddressOfScriptSetup, (dwAddressOfScriptSetup - (DWORD64)hGTA));

			DWORD64 dwAddressOfAddNatives = dwAddressOfScriptSetup + 0x78;

			dwAddressOfAddNatives = (*(int*)(dwAddressOfAddNatives + 1) + dwAddressOfAddNatives + 5);

			//printf("Add Natives: %I64X (%I64X)\n", dwAddressOfAddNatives, (dwAddressOfAddNatives - (DWORD64)hGTA));

			int scriptNamespaceIndex = -1;

			Opcodes ops = GetFunctionOpcodes(dwAddressOfAddNatives);

			for (auto op : ops.op) {
				if (op.bytes.data()[0] == 0xE8 || op.bytes.data()[0] == 0xE9) { // CALL || JMP
					scriptNamespaceIndex++; // We're in the namespace

					if (isNullStub((eScriptNativeCollectionIndex)scriptNamespaceIndex))
						continue;

					DWORD64 dwFunctionCallAddress = (*(int*)(op.addr + 1) + op.addr + 5);

					Namespace n;

					n.name = gNamespaceNames[scriptNamespaceIndex];
					n.natives = GetNativesInFunction(dwFunctionCallAddress);

					ns.push_back(n);
				}
			}
		}
		else {
			printf("WARNING: Unable to find Script Setup pattern on file \"%s\"\n");
		}
	}
	else {
		printf("Failed LoadLibrary on \"%s\"\n", path);
	}

	return ns;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Please supply an executable name to use...\n");
		return 0;
	}

	if (argc == 2) { // [exe]
		auto namespaces = GetNamespaceDataForExe(argv[1]);

		for (auto ns : namespaces) {
			printf("%s count = %d\n", ns.name.c_str(), ns.natives.size());

			for (size_t i = 0; i < ns.natives.size(); i++) {
				printf("%s [%d] = 0x%I64X [0x%I64X] (0x%I64X)\n", ns.name.c_str(), i, ns.natives[i].hash, ns.natives[i].GetFunctionRoughHash(), ns.natives[i].func);
			}

			printf("------------------\n");
		}
	}
	else if (argc == 4) { // [old_exe] [new_exe] [natives.h]
		printf("Not yet supported\n");
	}

	return 0;
}