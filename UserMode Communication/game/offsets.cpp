#include "..\main.h"
#include "netvars.h"

#define EXP(x) x, sizeof(x) - 1

Ofs* ofs = new Ofs();

std::string Ofs::toHex(DWORD offset) {
	std::stringstream ss;
	ss << std::hex << offset;
	std::string x = ss.str();
	for (auto& c : x) c = toupper(c);

	return x;
}

void Ofs::FindOffsets() {
	if (!c_netvars::get().init())
	{
		printf("Can't initialize netvar manager!");
		return;
	}

	/*if (Global->Debug)
		c_netvars::get().dump();*/

	static auto find_ptr = [](ULONG64 mod, const char* sig, DWORD sig_add, DWORD off_add, bool sub_base = true, bool mode_read = true) -> DWORD
	{
		auto off = Memory->FindPattern2(mod, sig);
		auto sb = sub_base ? mod : 0;
		if (mode_read) off = Global->K->ReadMemory<DWORD>(off + sig_add);

		return (!off ? 0 : off + off_add - sb);
	};

	m_dwIndex = 0x64;

	m_dwClientState = find_ptr(Global->Engine, "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0", 0x1, 0); // up
	m_dwClientState_PlayerInfo = find_ptr(Global->Engine, "8B 89 ? ? ? ? 85 C9 0F 84 ? ? ? ? 8B 01", 0x2, 0, false); // up
	m_dwViewAngles = find_ptr(Global->Engine, "F3 0F 11 80 ? ? ? ? D9 46 04 D9 05", 0x4, 0, false); // up
	m_dwGameState = find_ptr(Global->Engine, "83 B8 ? ? ? ? ? 0F 94 C0 C3", 0x2, 0, false); // unknown
	m_nDeltaTick = find_ptr(Global->Engine, "C7 87 ? ? ? ? ? ? ? ? FF 15 ? ? ? ? 83 C4 08", 0x2, 0, false); // up
	m_dwLocalPlayer = find_ptr(Global->Client, "8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF", 0x3, 0x4); // up
	m_dwEntityList = find_ptr(Global->Client, "BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8", 0x1, 0); // up
	m_dwGlowObject = find_ptr(Global->Client, "A1 ? ? ? ? A8 01 75 4B", 0x1, 0x4); // up
	m_dwRadarBase = find_ptr(Global->Client, "A1 ? ? ? ? 8B 0C B0 8B 01 FF 50 ? 46 3B 35 ? ? ? ? 7C EA 8B 0D", 0x1, 0); // up
	m_dwForceJump = find_ptr(Global->Client, "8B 0D ? ? ? ? 8B D6 8B C1 83 CA 02", 0x2, 0); // up
	m_dwForceBackward = find_ptr(Global->Client, "55 8B EC 51 53 8A 5D 08", 287, 0); // up
	m_dwForceForward = find_ptr(Global->Client, "55 8B EC 51 53 8A 5D 08", 285, 0); // up
	m_dwForceLeft = find_ptr(Global->Client, "55 8B EC 51 53 8A 5D 08", 465, 0); // up
	m_dwForceRight = find_ptr(Global->Client, "55 8B EC 51 53 8A 5D 08", 512, 0); // up
	m_dwForceAttack = find_ptr(Global->Client, "89 0D ? ? ? ? 8B 0D ? ? ? ? 8B F2 8B C1 83 CE 04", 0x2, 0); // up
	m_bDormant = find_ptr(Global->Client, "8A 81 ? ? ? ? C3 32 C0", 0x2, 0x8, false); // up
	m_dwGlobalVars = find_ptr(Global->Engine, "68 ? ? ? ? 68 ? ? ? ? FF 50 08 85 C0", 0x1, 0); // up
	m_dwPlayerResource = find_ptr(Global->Client, "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7", 0x2, 0); // up
	m_dwGameRulesProxy = find_ptr(Global->Client, "A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 74 7A", 0x1, 0);
	m_dwClientCMD = find_ptr(Global->Engine, "55 8B EC 8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 0C A1 ? ? ? ? 35 ? ? ? ? EB 05 8B 01 FF 50 34 50", 0, 0, true, false);
	m_dwRankRevealAllFn = find_ptr(Global->Client, "55 8B EC 8B 0D ? ? ? ? 85 C9 75 28 A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 04 85 C0 74 0B 8B C8 E8 ? ? ? ? 8B C8 EB 02 33 C9 89 0D ? ? ? ? 8B 45 08", 0, 0, true, false);
	modelAmbientMin = find_ptr(Global->Engine, "F3 0F 10 0D ? ? ? ? F3 0F 11 4C 24 ? 8B 44 24 20 35 ? ? ? ? 89 44 24 0C", 0x4, 0); // up

	m_hActiveWeapon = GET_NETVAR("DT_BaseCombatCharacter", "m_hActiveWeapon");
	m_totalRoundsPlayed = GET_NETVAR("DT_CSGameRulesProxy", "m_totalRoundsPlayed");
	m_iMatchStats_RoundResults = GET_NETVAR("DT_CSGameRulesProxy", "m_iMatchStats_RoundResults");
	m_bHasMatchStarted = GET_NETVAR("DT_CSGameRulesProxy", "m_bHasMatchStarted");
	m_iCompetitiveRanking = GET_NETVAR("DT_CSPlayerResource", "m_iCompetitiveRanking");
	m_iCompetitiveWins = GET_NETVAR("DT_CSPlayerResource", "m_iCompetitiveWins");
	m_bWarmupPeriod = GET_NETVAR("DT_CSGameRulesProxy", "m_bWarmupPeriod");
	m_iCrossHairID = GET_NETVAR("DT_CSPlayer", "m_bHasDefuser") + 0x5C;
	m_iHealth = GET_NETVAR("DT_BasePlayer", "m_iHealth");
	m_iMaxHealth = GET_NETVAR("DT_CHostage", "m_iMaxHealth");
	m_iTeamNum = GET_NETVAR("DT_BaseEntity", "m_iTeamNum");
	m_nSurvivalTeam = GET_NETVAR("DT_CSPlayer", "m_nSurvivalTeam");
	m_flC4Blow = GET_NETVAR("DT_PlantedC4", "m_flC4Blow");
	m_iShotsFired = GET_NETVAR("DT_CSPlayer", "m_iShotsFired");
	m_totalHitsOnServer = GET_NETVAR("DT_CSPlayer", "m_totalHitsOnServer");
	m_fFlags = GET_NETVAR("DT_BasePlayer", "m_fFlags");
	m_lifeState = GET_NETVAR("DT_BasePlayer", "m_lifeState");
	m_bSpotted = GET_NETVAR("DT_BaseEntity", "m_bSpotted");
	m_bSpottedByMask = GET_NETVAR("DT_BaseEntity", "m_bSpottedByMask");
	m_bIsScoped = GET_NETVAR("DT_CSPlayer", "m_bIsScoped");
	m_zoomLevel = GET_NETVAR("DT_WeaponCSBaseGun", "m_zoomLevel");
	m_vecOrigin = GET_NETVAR("DT_BaseEntity", "m_vecOrigin");
	m_vecViewOffset = GET_NETVAR("DT_BasePlayer", "m_vecViewOffset[0]");
	m_angEyeAngles = GET_NETVAR("DT_CSPlayer", "m_angEyeAngles");
	m_vecVelocity = GET_NETVAR("DT_BasePlayer", "m_vecVelocity[0]");
	m_aimPunchAngle = GET_NETVAR("DT_CSPlayer", "m_aimPunchAngle");
	m_dwBoneMatrix = GET_NETVAR("DT_BaseAnimating", "m_nForceBone") + 0x1C;
	m_flNextPrimaryAttack = GET_NETVAR("DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
	m_SurvivalGameRuleDecisionTypes = GET_NETVAR("DT_CSGameRulesProxy", "m_SurvivalGameRuleDecisionTypes");
	m_nTickBase = GET_NETVAR("DT_BasePlayer", "m_nTickBase");
	m_iPlayerC4 = GET_NETVAR("DT_CSPlayerResource", "m_iPlayerC4");
	m_bIsDefusing = GET_NETVAR("DT_CSPlayer", "m_bIsDefusing");
	m_iItemDefinitionIndex = GET_NETVAR("DT_BaseCombatWeapon", "m_iItemDefinitionIndex");
	m_iWorldModelIndex = GET_NETVAR("DT_BaseCombatWeapon", "m_iWorldModelIndex");
	m_iEntityQuality = GET_NETVAR("DT_BaseAttributableItem", "m_iEntityQuality");
	m_iViewModelIndex = GET_NETVAR("DT_BaseCombatWeapon", "m_iViewModelIndex");
	m_nModelIndex = GET_NETVAR("DT_BaseCombatWeapon", "m_nModelIndex");
	m_hViewModel = GET_NETVAR("DT_CSPlayer", "m_hViewModel[0]");
	m_iAccountID = GET_NETVAR("DT_BaseCombatWeapon", "m_iAccountID");
	m_nFallbackPaintKit = GET_NETVAR("DT_BaseAttributableItem", "m_nFallbackPaintKit");
	m_OriginalOwnerXuidLow = GET_NETVAR("DT_BaseCombatWeapon", "m_OriginalOwnerXuidLow");
	m_iItemIDLow = GET_NETVAR("DT_BaseCombatWeapon", "m_iItemIDLow");
	m_iItemIDHigh = GET_NETVAR("DT_BaseAttributableItem", "m_iItemIDHigh");
	m_flFallbackWear = GET_NETVAR("DT_BaseAttributableItem", "m_flFallbackWear");
	m_hMyWeapons = GET_NETVAR("DT_BaseCombatCharacter", "m_hMyWeapons");
	m_iClip1 = GET_NETVAR("DT_WeaponCSBaseGun", "m_iClip1");

	m_hTonemapController = GET_NETVAR("DT_CSPlayer", "m_hTonemapController");
	m_bUseCustomAutoExposureMin = GET_NETVAR("DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
	m_bUseCustomAutoExposureMax = GET_NETVAR("DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");

	m_flCustomBloomScale = GET_NETVAR("DT_EnvTonemapController", "m_flCustomBloomScale");
	m_flCustomAutoExposureMin = GET_NETVAR("DT_EnvTonemapController", "m_flCustomAutoExposureMin");
	m_flCustomAutoExposureMax = GET_NETVAR("DT_EnvTonemapController", "m_flCustomAutoExposureMax");

	m_bInReload = m_flNextPrimaryAttack + 0x6D;
	m_bReloadVisuallyComplete = GET_NETVAR("DT_WeaponCSBaseGun", "m_bReloadVisuallyComplete");


}

bool Ofs::CheckOffsets()
{
#define CHECK_OFFSET(n) \
	if (Global->Debug) \
		printf("%s = %X\n", #n, n); \
	if (!n) { \
		result = false; }

	bool result = true;

	CHECK_OFFSET(m_dwIndex);

	CHECK_OFFSET(m_dwClientState);
	CHECK_OFFSET(m_dwViewAngles);
	CHECK_OFFSET(m_dwGameState);
	CHECK_OFFSET(m_nDeltaTick);
	CHECK_OFFSET(m_dwLocalPlayer);
	CHECK_OFFSET(m_dwEntityList);
	CHECK_OFFSET(m_dwGlowObject);
	CHECK_OFFSET(m_dwForceJump);
	CHECK_OFFSET(m_dwForceAttack);
	CHECK_OFFSET(m_bDormant);
	CHECK_OFFSET(m_dwGlobalVars);
	CHECK_OFFSET(m_dwPlayerResource);

	CHECK_OFFSET(m_hActiveWeapon);
	CHECK_OFFSET(m_iCrossHairID);
	CHECK_OFFSET(m_iHealth);
	CHECK_OFFSET(m_iTeamNum);
	CHECK_OFFSET(m_iShotsFired);
	CHECK_OFFSET(m_fFlags);
	CHECK_OFFSET(m_lifeState);
	CHECK_OFFSET(m_bSpottedByMask);
	CHECK_OFFSET(m_vecOrigin);
	CHECK_OFFSET(m_vecViewOffset);
	CHECK_OFFSET(m_angEyeAngles);
	CHECK_OFFSET(m_vecVelocity);
	CHECK_OFFSET(m_aimPunchAngle);
	CHECK_OFFSET(m_dwBoneMatrix);
	CHECK_OFFSET(m_flNextPrimaryAttack);
	CHECK_OFFSET(m_nTickBase);
	CHECK_OFFSET(m_iPlayerC4);
	CHECK_OFFSET(m_bIsDefusing);
	CHECK_OFFSET(m_iItemDefinitionIndex);
	CHECK_OFFSET(m_iWorldModelIndex);
	CHECK_OFFSET(m_nModelIndex);
	CHECK_OFFSET(m_hViewModel);
	CHECK_OFFSET(m_iAccountID);
	CHECK_OFFSET(m_nFallbackPaintKit);
	CHECK_OFFSET(m_OriginalOwnerXuidLow);
	CHECK_OFFSET(m_iItemIDLow);
	CHECK_OFFSET(m_hMyWeapons);

	return result;
}

void Ofs::getOffsets() {
	FindOffsets();

	if (!CheckOffsets())
	{
		printf("Offsets outdated!\n");
		std::system("pause");
		exit(1);
	}
	//std::system("pause");
	Sleep(1000);

	m_dwClientState = Global->K->ReadMemory<DWORD_PTR>(Global->Engine + m_dwClientState);
}
