#include "StarsGamePlayer.h"

#define COLOR_MONSTER 0xFFFF00FF
#define COLOR_BOSS 0xFFFF00FF
#define COLOR_DOOR 0xFF00FF00
#define COLOR_OBJECT 0xFFFFFF00
#define COLOR_BLOCK 0xFFFF0000
#define COLOR_ROOM_CLOSE 0xFFFF00FF
#define COLOR_ROOM_OPEN 0xFFFF00FF

//enum starsImgColor{ SCOLOR_NONE = 0, SCOLOR_MONSTER = 0xFFFF00FF, SCOLOR_OBJECT = 0xFFFFFF00, SCOLOR_BLOCK = 0xFFFF0000, SCOLOR_PATHGATE = 0xFF00FF00, SCOLOR_ITEM = 0xFF00FF00 };

extern int iScreenShotWidth;
extern int iScreenShotHeight;
extern HWND g_iTargetGameHandle;
extern void PrintLog(const char *format, ...);

StarsGamePlayer::StarsGamePlayer()
{
	m_pkStarsGraphy = nullptr;
	m_pkStarsControl = nullptr;
	m_eBattleState = StarsBattleState_Start;
	m_eRunUpDown = StarsRunDirection_None;
	m_eLeftRight = StarsRunDirection_None;
	m_iUpDownEndTime = 0;
	m_iLeftRightEndTime = 0;
	m_bStartAttack = false;
	m_iLastUpdaetPlayerPos = 0;
	m_eSceneState = StarsSceneState_None;
	m_ePlayerSide = StarsRunDirection_Left;
}

StarsGamePlayer::~StarsGamePlayer()
{
	delete m_pkStarsGraphy;
	delete m_pkStarsControl;
}

bool StarsGamePlayer::Initalize()
{
	m_pkStarsGraphy = new StarsGraphy();
	if (!m_pkStarsGraphy->Initalize())
	{
		MessageBoxA(NULL, "g_kStarsGraphy初始化失败", "Warning", MB_OK);
		return false;
	}

	m_pkStarsControl = new StarsControl();
	if (!m_pkStarsControl->Initalize())
	{
		MessageBoxA(NULL, "g_kStarsControl初始化失败", "Warning", MB_OK);
		return false;
	}
	return true;
}

bool StarsGamePlayer::Finitalize()
{
	m_pkStarsGraphy->Finitalize();
	m_pkStarsControl->Finitalize();
	return true;
}

void StarsGamePlayer::Update()
{
	if (g_iTargetGameHandle == 0)
	{
		m_kGameRect.left = 0;
		m_kGameRect.right = 830;
		m_kGameRect.top = 0;
		m_kGameRect.bottom = 750;
		//return;
	}
	else
	{
		RECT kGameRect;
		GetWindowRect(g_iTargetGameHandle, &kGameRect);
		
		m_kGameRect.left = kGameRect.left;
		m_kGameRect.right = kGameRect.right;
		m_kGameRect.top = kGameRect.top;
		m_kGameRect.bottom = kGameRect.bottom;
	}
	

	m_pkStarsGraphy->Update(m_kGameRect);
	m_pkStarsControl->Update();
	
	switch (m_eSceneState)
	{
	case StarsSceneState_None:
		break;
	case StarsSceneState_BaseRoom:
		break;
	case StarsSceneState_Town:
		break;
	case StarsSceneState_Loading:
		break;
	case StarsSceneState_Battle:
	{
								   UpdateBattle();
								   break;
	}
	case StarsSceneState_BattleEnd:
		break;
	default:
		break;
	}
}

void StarsGamePlayer::UpdateBattle()
{

	if (timeGetTime() - m_iLastUpdaetPlayerPos > 500)
	{
		m_iLastUpdaetPlayerPos = timeGetTime();
		ST_POS kPlayerPos = FindPicture("PlayerName.bmp", ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom));
		if (kPlayerPos.x != -1)
		{
			m_kPlayerPos = kPlayerPos;
			m_kPlayerPos.y += 120;
			m_kPlayerPos.x += 35;
		}

		m_kNearMonsterPos = FindMonster(m_kGameRect, ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
	}
	UpdateMiniMapState();

	switch (m_eBattleState)
	{
	case StarsBattleState_Start:
	{
								   Patrol();
								   m_eBattleState = StarsBattleState_FindMonster;
								   PrintLog("ChangeState:FindMonster");
								   break;
	}
	case StarsBattleState_FindMonster:
	{
										 if (m_kNearMonsterPos.x != -1 && m_kNearMonsterPos.y != -1)
										 {
											 ActionRun(m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
											 m_eBattleState = StarsBattleState_GoMonster;
											 PrintLog("PlayerPos:%d, %d", m_kPlayerPos.x, m_kPlayerPos.y);
											 PrintLog("MonsterPos:%d, %d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
											 PrintLog("ChangeState:GoMonster %d,%d", m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
										 }
										 else
										 {
											 Patrol();
											 PrintLog("Patrol");
										 }
										 break;
	}
	case StarsBattleState_GoMonster:
	{
									   if (m_eLeftRight == StarsRunDirection_None && m_eRunUpDown == StarsRunDirection_None)
									   {
										   m_eBattleState = StarsBattleState_AttackMonster;
										   PrintLog("ChangeState:AttackMonster");
									   }
									   break;
	}
	case StarsBattleState_AttackMonster:
	{
										   if (m_kNearMonsterPos.x == -1 && m_kNearMonsterPos.y == -1)
										   {
											   m_eBattleState = StarsBattleState_FindMonster;
											   PrintLog("LostTarget:FindMonster");
											   ActionAttack(false);
										   }
										   else if (fabsf(m_kNearMonsterPos.x - m_kPlayerPos.x) > 90 || fabsf(m_kNearMonsterPos.y - m_kPlayerPos.y) > 20)
										   {
											   PrintLog("MonsterXoutOfRange::%d,%d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
											   ActionAttack(false);
											   ActionRun(m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
										   }
										   else
										   {
											   ActionAttack(true);
											   PrintLog("AttackMonster::%d,%d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
										   }
										   break;
	}
	case StarsBattleState_AllClear:
	{
									  m_eBattleState = StarsBattleState_FindDoor;
									  PrintLog("ChangeState:FindDoor");
									  break;
	}
	case StarsBattleState_FindDoor:
	{

									  ST_POS kDoorPos = FIndPictureORB("door.bmp");
									  if (kDoorPos.x != -1 && kDoorPos.y != -1)
									  {
										  ActionRun(kDoorPos.x - 400, kDoorPos.y - 300);
									  }
									  else
									  {
										  ActionRun(500, 0);
									  }
									  break;
	}
	case StarsBattleState_GoNextRoom:
	{
										ST_POS kMapPos = FindPicture("nextdoor", ST_RECT(0, 800, 0, 600));
										if (kMapPos.x == -1)
										{
											m_bAllClear = false;
											m_eBattleState = StarsBattleState_Start;
										}
										break;
	}
	case StarsBattleState_Count:
		break;
	default:
		break;
	}

	UpdateRun();
	UpdateAttack();
}

ST_POS StarsGamePlayer::FindPicture(std::string kPictureName, ST_RECT kRect, bool bUseLocalPos)
{
	ST_POS kPos = m_pkStarsGraphy->FindPicture(kPictureName, kRect);
	if (bUseLocalPos && kPos.x != -1)
	{
		kPos.x -= m_kGameRect.left;
		kPos.y -= m_kGameRect.top;
	}
	return kPos;
}

ST_POS StarsGamePlayer::FIndPictureORB(std::string kPictureName, bool bUseLocalPos)
{
	ST_POS kPos = m_pkStarsGraphy->FIndPictureORB(kPictureName);
	if (bUseLocalPos && kPos.x != -1)
	{
		kPos.x -= m_kGameRect.left;
		kPos.y -= m_kGameRect.top;
	}
	return kPos;
}

ST_POS StarsGamePlayer::FindFont(std::string kStr, ST_RECT kRect, bool bUseLocalPos)
{
	ST_POS kPos = m_pkStarsGraphy->FindFont(kStr, kRect);
	if (bUseLocalPos && kPos.x != -1)
	{
		kPos.x -= m_kGameRect.left;
		kPos.y -= m_kGameRect.top;
	}
	return kPos;
}

ST_POS StarsGamePlayer::FindColor(DWORD dwColor, ST_RECT kRect, bool bUseLocalPos, ST_POS kStartPos)
{
	ST_POS kPos = m_pkStarsGraphy->FindColor(dwColor, kRect, true, kStartPos);
	if (bUseLocalPos && kPos.x != -1)
	{
		kPos.x -= m_kGameRect.left;
		kPos.y -= m_kGameRect.top;
	}
	return kPos;
}

void StarsGamePlayer::SetSceneState(StarsSceneState eState)
{
	m_eSceneState = eState;
}

void StarsGamePlayer::ActionRun(float fDisX, float fDisY)
{
	if (fDisX != 0)
	{
		StarsRunDirection eTmepLeftRight = fDisX > 0 ? StarsRunDirection_Right : StarsRunDirection_Left;
		if (m_eLeftRight == StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
		}
		else if (eTmepLeftRight != m_eLeftRight)
		{
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_LEFT : VK_RIGHT);
			Sleep(20);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(20);
		}

		m_eLeftRight = eTmepLeftRight;
		m_iLeftRightEndTime = fabsf(fDisX) / 200 * 1000 + timeGetTime();
		
	}
	else if (m_eLeftRight != StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
		Sleep(20);
		m_eLeftRight = StarsRunDirection_None;
		m_iLeftRightEndTime = 0;
	}

	if (fDisY != 0)
	{
		StarsRunDirection eTempUpDown = fDisY > 0 ? StarsRunDirection_Down : StarsRunDirection_Up;

		if (m_eRunUpDown == StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyDown(eTempUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
			Sleep(20);
		}
		else if (eTempUpDown != m_eRunUpDown)
		{
			m_pkStarsControl->OnKeyUp(eTempUpDown == StarsRunDirection_Up ? VK_DOWN : VK_UP);
			Sleep(20);
			m_pkStarsControl->OnKeyDown(eTempUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
			Sleep(20);
		}

		m_eRunUpDown = eTempUpDown;
		m_iUpDownEndTime = fabsf(fDisY) / 100 * 1000 + timeGetTime();
		
	}
	else if(m_eRunUpDown != StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		Sleep(20);
		m_eRunUpDown = StarsRunDirection_None;
		m_iUpDownEndTime = 0;
	}
}

void StarsGamePlayer::UpdateRun()
{
	if (m_iLeftRightEndTime != 0 && timeGetTime() > m_iLeftRightEndTime)
	{
		if (m_eLeftRight != StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyUp(VK_LEFT);
			Sleep(20);
			m_pkStarsControl->OnKeyUp(VK_RIGHT);
			Sleep(20);
			m_eLeftRight = StarsRunDirection_None;
		}
		m_iLeftRightEndTime = 0;
	}

	if (m_iUpDownEndTime != 0 && timeGetTime() > m_iUpDownEndTime)
	{
		if (m_eRunUpDown != StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyUp(VK_UP);
			Sleep(20);
			m_pkStarsControl->OnKeyUp(VK_DOWN);
			Sleep(20);
			m_eRunUpDown = StarsRunDirection_None;
		}
		m_iUpDownEndTime = 0;
	}
}

void StarsGamePlayer::ActionAttack(bool bStart)
{
	m_bStartAttack = bStart;
	if (bStart)
	{
		m_iEndAttackTime = timeGetTime() + 3000;
	}
	else
	{
		if (m_iEndAttackTime != 0)
		{
			m_pkStarsControl->OnKeyDown('V');
			Sleep(20);
			m_pkStarsControl->OnKeyDown('V');
			Sleep(20);
			m_pkStarsControl->OnKeyUp('V');
			Sleep(20);
		}
		m_iEndAttackTime = 0;
	}
}

void StarsGamePlayer::UpdateAttack()
{
	if (timeGetTime() > m_iEndAttackTime && m_bStartAttack)
	{
		m_iEndAttackTime = 0;
		m_bStartAttack = false;
	}

	if (m_bStartAttack)
	{
		ActionRun(0, 0);
		m_pkStarsControl->OnKeyDown('X');
		Sleep(20);
		m_pkStarsControl->OnKeyDown('X');
		Sleep(20);
		m_pkStarsControl->OnKeyUp('X');
		Sleep(20);
	}
}

// 巡逻
void StarsGamePlayer::Patrol()
{
	if (m_kPlayerPos.x < 260)
	{
		m_ePlayerSide = StarsRunDirection_Left;
	}
	else if (m_kPlayerPos.x > 540)
	{
		m_ePlayerSide = StarsRunDirection_Right;
	}

	if (m_ePlayerSide == StarsRunDirection_Left)
	{
		ActionRun(500, 0);
	}
	else
	{
		ActionRun(-500, 0);
	}
}

void StarsGamePlayer::UpdateMiniMapState()
{
	int iMiniMapState = GetUserDataInt("iMiniMapState");
	if (iMiniMapState == -1)	// 初始状态
	{
		iMiniMapState = 0;
	}
	if (iMiniMapState == 0)	// 查找玩家自己小地图位置
	{
		ST_POS kMiniMapPlayerPos = FindPicture("MiniMapPlayer.bmn", ST_RECT(m_kGameRect.right - 100, m_kGameRect.right, m_kGameRect.top + 50, m_kGameRect.top + 150), false);
		if (kMiniMapPlayerPos.x != -1)
		{
			SetUserDataInt("iMiniMapPlayerPosX", kMiniMapPlayerPos.x);
			SetUserDataInt("iMiniMapPlayerPosY", kMiniMapPlayerPos.y);
			iMiniMapState = 1;
		}
	}
	if (iMiniMapState == 1)	// 检查战斗结束
	{
		int iOffset[4][2] = { { -10, 0 }, { 10, 0 }, { 0, -10 }, {0, 10} };
		for (int i = 0; i < 4; ++i)
		{
			ST_POS kRoomOpen = FindColor(COLOR_ROOM_OPEN, ST_RECT(GetUserDataInt("iMiniMapPlayerPosX") + iOffset[i][0] - 10, GetUserDataInt("iMiniMapPlayerPosX") + iOffset[i][0] + 10,
				GetUserDataInt("iMiniMapPlayerPosY") + iOffset[i][1] - 10, GetUserDataInt("iMiniMapPlayerPosY") + iOffset[i][1] + 10), false);
			if (kRoomOpen.x != -1)
			{
				iMiniMapState = 2;
				break;
			}
		}
	}
	if (iMiniMapState == 2)	// 查找要进的房间
	{
		SetUserDataInt("iLastRoomDirection", int(StarsRunDirection_Left));
		//save last state
		iMiniMapState = 3;
	}
	if (iMiniMapState == 3)	// 等待进入房间
	{
		ST_POS kMiniMapPlayerPos = FindPicture("MiniMapPlayer.bmn", ST_RECT(m_kGameRect.right - 100, m_kGameRect.right, m_kGameRect.top + 50, m_kGameRect.top + 150), false);
		if (kMiniMapPlayerPos.x != -1 && kMiniMapPlayerPos.x != GetUserDataInt("iMiniMapPlayerPosX"))
		{
			SetUserDataInt("iMiniMapPlayerPosXLast", GetUserDataInt("iMiniMapPlayerPosX"));
			SetUserDataInt("iMiniMapPlayerPosYLast", GetUserDataInt("iMiniMapPlayerPosY"));
			SetUserDataInt("iMiniMapPlayerPosX", kMiniMapPlayerPos.x);
			SetUserDataInt("iMiniMapPlayerPosY", kMiniMapPlayerPos.y);
			iMiniMapState = 1;
		}
	}

	SetUserDataInt("iMiniMapState", iMiniMapState);
}

ST_POS StarsGamePlayer::FindMonster(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_MONSTER, kRect, true, kStartPos);
}

void StarsGamePlayer::SetUserDataInt(std::string& kStr, int iValue)
{
	m_akUserDataInt[kStr] = iValue;
}

int StarsGamePlayer::GetUserDataInt(std::string& kStr)
{
	std::map<std::string, int>::iterator itr = m_akUserDataInt.find(kStr);
	if (itr != m_akUserDataInt.end())
	{
		return itr->second;
	}
	return -1;
}

void StarsGamePlayer::SetUserDataFloat(std::string& kStr, float fValue)
{
	m_akUserDataFloat[kStr] = fValue;
}

float StarsGamePlayer::GetUserDataFloat(std::string& kStr)
{
	std::map<std::string, float>::iterator itr = m_akUserDataFloat.find(kStr);
	if (itr != m_akUserDataFloat.end())
	{
		return itr->second;
	}
	return -1;
}