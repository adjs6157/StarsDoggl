#include "StarsGamePlayer.h"
#include <queue>

#define COLOR_MONSTER 0xFFFF00FF
#define COLOR_BOSS 0xFFFF00FF
#define COLOR_DOOR_LEFTRIGHT 0xFF00FF00
#define COLOR_DOOR_UP 0xFF00EE00
#define COLOR_DOOR_DOWN 0xFF00DD00
#define COLOR_OBJECT 0xFFFFFF00
#define COLOR_BLOCK 0xFFFF0000
#define COLOR_ROOM_CLOSE 0xFF0000EE
#define COLOR_ROOM_OPEN 0xFF0000DD
#define COLOR_ROOM_UNKOWN_CLOSE 0xFF0000AA
#define COLOR_ROOM_UNKOWN_OPEN 0xFF000099
#define COLOR_ROOM_DOOR 0xFF000022
#define COLOR_ROOM_BOSS 0xFF0000CC

//enum starsImgColor{ SCOLOR_NONE = 0, SCOLOR_MONSTER = 0xFFFF00FF, SCOLOR_OBJECT = 0xFFFFFF00, SCOLOR_BLOCK = 0xFFFF0000, SCOLOR_PATHGATE = 0xFF00FF00,
//SCOLOR_ITEM = 0xFF00FF00, SCOLOR_MINIMAP = 0xFFAAB450, SCOLOR_MINIMAP_OPEN = 0xFF416E14, SCOLOR_MINIMAP_UNKONW = 0xFF417D8C, SCOLOR_MINIMAP_UNKONW_OPEN = 0xFF142350, SCOLOR_MONSTERICON = 0xFFC8C800};

extern int iScreenShotWidth;
extern int iScreenShotHeight;
extern HWND g_iTargetGameHandle;
extern void PrintLog(const char *format, ...);
#define KEY_SLEEP_TIME 10

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
		if (GetWindowRect(g_iTargetGameHandle, &kGameRect))
		{
			m_kGameRect.left = kGameRect.left;
			m_kGameRect.right = kGameRect.right;
			m_kGameRect.top = kGameRect.top;
			m_kGameRect.bottom = kGameRect.bottom;
		}
		else
		{
			m_kGameRect.left = 0;
			m_kGameRect.right = 830;
			m_kGameRect.top = 0;
			m_kGameRect.bottom = 750;
			g_iTargetGameHandle = FindWindowA(NULL, "地下城与勇士");
		}
		
	}

	//m_pkStarsControl->OnKeyDown(VK_DOWN);
	//Sleep(30);
	//m_pkStarsControl->OnKeyUp(VK_DOWN);
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

	if (timeGetTime() - m_iLastUpdaetPlayerPos > 100)
	{
		m_iLastUpdaetPlayerPos = timeGetTime();
		ST_POS kPlayerPos = FindPicture("PlayerName.bmp", ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50));
		if (kPlayerPos.x != -1)
		{
			m_kPlayerPos = kPlayerPos;
			m_kPlayerPos.y += 140;
			m_kPlayerPos.x += 35;
		}

		m_kNearMonsterPos = FindMonster(ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50), ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
	}
	UpdateMiniMapState();
	if (GetUserDataInt("iMiniMapState") == 3)
	{
		m_eBattleState = StarsBattleState_AllClear;
	}

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
										 }
										 break;
	}
	case StarsBattleState_GoMonster:
	{
									   //if (m_eLeftRight == StarsRunDirection_None && m_eRunUpDown == StarsRunDirection_None)
									   {
										   m_eBattleState = StarsBattleState_AttackMonster;
										   PrintLog("ChangeState:AttackMonster");
									   }
									   break;
	}
	case StarsBattleState_AttackMonster:
	{
										   bool bXOutOfRange = fabsf(m_kNearMonsterPos.x - m_kPlayerPos.x) > 160;
										   bool bYOutOfRange = fabsf(m_kNearMonsterPos.y - m_kPlayerPos.y) > 25;
										   if (m_kNearMonsterPos.x == -1 && m_kNearMonsterPos.y == -1)
										   {
											   m_eBattleState = StarsBattleState_FindMonster;
											   PrintLog("LostTarget:FindMonster");
											   ActionAttack(false);
										   }
										   else if (bXOutOfRange || bYOutOfRange)
										   {
											   //PrintLog("MonsterXoutOfRange::%d,%d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
											   ActionAttack(false);
											   if (bXOutOfRange && bYOutOfRange)
											   {
												   ActionRun(m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
											   }
											   else if (bXOutOfRange && !bYOutOfRange)
											   {
												   ActionRun(m_kNearMonsterPos.x - m_kPlayerPos.x, 0);
											   }
											   else
											   {
												   ActionRun(0, m_kNearMonsterPos.y - m_kPlayerPos.y);
											   }
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
									  StarsRunDirection eDir = (StarsRunDirection)GetUserDataInt("eRoomDirection");
									  if (eDir == StarsRunDirection_Up)
									  {
										  SetUserDataInt("iNextDoorColor", COLOR_DOOR_UP);
									  }
									  else if (eDir == StarsRunDirection_Down)
									  {
										  SetUserDataInt("iNextDoorColor", COLOR_DOOR_DOWN);
									  }
									  else
									  {
										  SetUserDataInt("iNextDoorColor", COLOR_DOOR_LEFTRIGHT);
									  }
									  Patrol();
									  PrintLog("ChangeState:FindDoor");
									  break;
	}
	case StarsBattleState_FindDoor:
	{
									  if (GetUserDataInt("iMiniMapState") != 4)
									  {
										  m_eBattleState = StarsBattleState_FindMonster;
										  PrintLog("ChangeState:FindMonster");
										  break;
									  }

									  StarsRunDirection eDir = (StarsRunDirection)GetUserDataInt("eRoomDirection");
									  DWORD dwDoorColor = GetUserDataInt("iNextDoorColor");
									  ST_POS kDoorPos = FindColor(dwDoorColor, m_kGameRect);
									  if (kDoorPos.x != -1 && kDoorPos.y != -1)
									  {
										  if (eDir == StarsRunDirection_Up || eDir == StarsRunDirection_Down)
										  {
											  ActionRun(kDoorPos.x - m_kPlayerPos.x, kDoorPos.y - m_kPlayerPos.y);
											  SetUserDataInt("GoDoorEndTime", (m_iLeftRightEndTime > m_iUpDownEndTime ? m_iLeftRightEndTime : m_iUpDownEndTime) + 2500);
											  m_eBattleState = StarsBattleState_GoNextRoom;
											  SetUserDataInt("GoNextRoomBlock", 0);
											  PrintLog("ChangeState:GoNextRoom");
											  PrintLog("DoorPos:%d, %d", kDoorPos.x, kDoorPos.y);
										  }
										  if (eDir == StarsRunDirection_Right && kDoorPos.x > 400 || eDir == StarsRunDirection_Left && kDoorPos.x < 400)
										  {
											  ActionRun(kDoorPos.x - m_kPlayerPos.x, kDoorPos.y - m_kPlayerPos.y);
											  SetUserDataInt("GoDoorEndTime", (m_iLeftRightEndTime > m_iUpDownEndTime ? m_iLeftRightEndTime : m_iUpDownEndTime) + 2500);
											  m_eBattleState = StarsBattleState_GoNextRoom;
											  SetUserDataInt("GoNextRoomBlock", 0);
											  PrintLog("ChangeState:GoNextRoom");
											  PrintLog("DoorPos:%d, %d", kDoorPos.x, kDoorPos.y);
										  }									  
									  }
									  else
									  {
										  Patrol();
									  }
									  break;
	}
	case StarsBattleState_GoNextRoom:
	{
										if (GetUserDataInt("iMiniMapState") != 4)
										{
											m_eBattleState = StarsBattleState_FindMonster;
											PrintLog("ChangeState:FindMonster");
											break;
										}

										if (timeGetTime() >= GetUserDataInt("GoDoorEndTime"))
										{
											int iBlock = GetUserDataInt("GoNextRoomBlock");
											if (iBlock == 0)
											{
												PrintLog("GoNextRoomBlock:0");
												SetUserDataInt("GoNextRoomBlockEndTime", timeGetTime() + 1500);
												SetUserDataInt("GoNextRoomBlock", 1);

												StarsRunDirection eDir = (StarsRunDirection)GetUserDataInt("eRoomDirection");
												if (eDir == StarsRunDirection_Left)
												{
													ActionRun(150, 0);
												}
												else if (eDir == StarsRunDirection_Right)
												{
													ActionRun(-150, 0);
												}
												else if (eDir == StarsRunDirection_Up)
												{
													ActionRun(0, 50);
												}
												else
												{
													ActionRun(0, -50);
												}
											}
											else if (iBlock == 1)
											{
												if (timeGetTime() > GetUserDataInt("GoNextRoomBlockEndTime"))
												{
													m_eBattleState = StarsBattleState_FindDoor;
													PrintLog("ChangeState:FindDoor");
												}
											}
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

#define BLOCK_SIZE 30.0f // 阻挡块大小

bool StarsGamePlayer::IsDirectArrive(const ST_POS& kStartPos, const ST_POS& kEndPos)
{
	float fDis = GetDistance(kStartPos.x, kStartPos.y, kEndPos.x, kEndPos.y);
	int iCount = fDis / BLOCK_SIZE;
	if (iCount <= 0)
	{
		return true;
	}
	float fDifX = (kEndPos.x - kStartPos.x) / iCount;
	float fDifY = (kEndPos.y - kStartPos.y) / iCount;

	float fTempX = kStartPos.x;
	float fTempY = kStartPos.y;
	for (int i = 0; i < iCount; ++i)
	{
		if (m_pkStarsGraphy->GetColor(ST_POS(fTempX, fTempY)) == 0xFFFF0000)
		{
			return false;
		}
		fTempX += fDifX;
		fTempY += fDifY;
	}
	return true;
}

ST_POS StarsGamePlayer::FindPath(const ST_POS& kStartPos, const ST_POS& kEndPos)
{
	// 可以直接到达
	if (IsDirectArrive(kStartPos, kEndPos))
	{
		return kEndPos;
	}

	// 递归查找路径
	std::vector<ST_RECT> akQueue;
	int iCurIndex = 0;
	bool bVisited[100][100];
	int iStartPosX = kStartPos.x / BLOCK_SIZE;
	int iStartPosY = kStartPos.y / BLOCK_SIZE;
	int iEndPosX = kEndPos.x / BLOCK_SIZE;
	int iEndPosY = kEndPos.y / BLOCK_SIZE;
	const int aiDir[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	memset(bVisited, 0, 100 * 100);
	bVisited[iStartPosX][iStartPosY] = true;
	akQueue.push_back(ST_RECT(iStartPosX, iStartPosY, -1, -1));
	bool bFindPath = false;

	ST_RECT kTempPoint;
	while (iCurIndex < akQueue.size())
	{
		ST_RECT kCurPoint = akQueue[iCurIndex];
		iCurIndex++;

		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.left = aiDir[k][0] + kCurPoint.left;
			kTempPoint.right = aiDir[k][1] + kCurPoint.right;
			if (kTempPoint.left >= 0 && kTempPoint.left < 100 && kTempPoint.right >= 0 && kTempPoint.right < 100 && !bVisited[kTempPoint.left][kTempPoint.right])
			{
				bVisited[kTempPoint.left][kTempPoint.right] = true;
				
				if (m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE, kTempPoint.right * BLOCK_SIZE)) != COLOR_BLOCK)
				{
					kTempPoint.bottom = iCurIndex - 1;
					akQueue.push_back(kTempPoint);
					
					if (kTempPoint.left == kEndPos.x && kTempPoint.right == kEndPos.y)
					{
						bFindPath = true;
						PrintLog("bFindPath");
						break;
					}
				}
			}
		}
		if (bFindPath)
		{
			break;
		}
	}

	int iMinDis = 9999;
	int iMinDisIndex = akQueue.size() - 1;
	if (!bFindPath)	// 没寻路到目标点就找最近的
	{
		for (int i = 0; i < akQueue.size(); ++i)
		{
			int iTempDisToEnd = abs(akQueue[i].left - iEndPosX) + abs(akQueue[i].right - iEndPosY);
			if (iTempDisToEnd < iMinDis)
			{
				iMinDis = iTempDisToEnd;
				iMinDisIndex = i;
			}
		}
	}
	else // 找到了最后一个点就是目标点
	{
		iMinDisIndex = akQueue.size() - 1;
	}
	PrintLog("iMinDisIndex:%d", iMinDisIndex);
	while (akQueue[iMinDisIndex].bottom != -1)
	{
		// 找到第一个可以直线到达的点
		if (IsDirectArrive(kStartPos, ST_POS(akQueue[iMinDisIndex].left * BLOCK_SIZE, akQueue[iMinDisIndex].right * BLOCK_SIZE)))
		{
			break;
		}
		iMinDisIndex = akQueue[iMinDisIndex].bottom;
	}
	PrintLog("FindPathPos:%d, %d", akQueue[iMinDisIndex].left * BLOCK_SIZE, akQueue[iMinDisIndex].right * BLOCK_SIZE);
	return ST_POS(akQueue[iMinDisIndex].left * BLOCK_SIZE, akQueue[iMinDisIndex].right * BLOCK_SIZE);
}

void StarsGamePlayer::ActionRun(float fDisX, float fDisY)
{
	ST_POS kPathRet;
	if ((fDisX != 0 || fDisY != 0) && (fabs(fDisX) > 30 || fabs(fDisY) > 30))
	{
		kPathRet = FindPath(m_kPlayerPos, ST_POS(m_kPlayerPos.x + fDisX, m_kPlayerPos.y + fDisY));
		fDisX = kPathRet.x - m_kPlayerPos.x;
		fDisY = kPathRet.y - m_kPlayerPos.y;
	}
	
	if (fDisX != 0)
	{
		StarsRunDirection eTmepLeftRight = fDisX > 0 ? StarsRunDirection_Right : StarsRunDirection_Left;
		if (m_eLeftRight == StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
		}
		else if (eTmepLeftRight != m_eLeftRight)
		{
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_LEFT : VK_RIGHT);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyUp(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyDown(eTmepLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
			Sleep(KEY_SLEEP_TIME);
		}

		m_eLeftRight = eTmepLeftRight;
		m_iLeftRightEndTime = fabsf(fDisX) / 200 * 1000 + timeGetTime();
		
	}
	else if (m_eLeftRight != StarsRunDirection_None)
	{
		ActionStopRun(m_eLeftRight);
	}

	if (fDisY != 0)
	{
		StarsRunDirection eTempUpDown = fDisY > 0 ? StarsRunDirection_Down : StarsRunDirection_Up;

		if (m_eRunUpDown == StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyDown(eTempUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
			Sleep(KEY_SLEEP_TIME);
		}
		else if (eTempUpDown != m_eRunUpDown)
		{
			m_pkStarsControl->OnKeyUp(eTempUpDown == StarsRunDirection_Up ? VK_DOWN : VK_UP);
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyDown(eTempUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
			Sleep(KEY_SLEEP_TIME);
		}

		m_eRunUpDown = eTempUpDown;
		m_iUpDownEndTime = fabsf(fDisY) / 130 * 1000 + timeGetTime();
		
	}
	else if(m_eRunUpDown != StarsRunDirection_None)
	{
		ActionStopRun(m_eRunUpDown);
	}
	PrintLog("ActionRun:%d,%d", (int)fDisX, (int)fDisY);
	PrintLog("RunTime:%d,%d", (int)(fabsf(fDisX) / 200 * 1000), (int)(fabsf(fDisY) / 100 * 1000));
}

void StarsGamePlayer::ActionStopRun(StarsRunDirection eDir)
{
	if (eDir == StarsRunDirection_Left || eDir == StarsRunDirection_Right || eDir == StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
		Sleep(KEY_SLEEP_TIME);
		m_eLeftRight = StarsRunDirection_None;
		m_iLeftRightEndTime = 0;
	}
	if (eDir == StarsRunDirection_Up || eDir == StarsRunDirection_Down || eDir == StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		Sleep(KEY_SLEEP_TIME);
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
			m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
			Sleep(KEY_SLEEP_TIME);
			m_eLeftRight = StarsRunDirection_None;
		}
		m_iLeftRightEndTime = 0;
	}

	if (m_iUpDownEndTime != 0 && timeGetTime() > m_iUpDownEndTime)
	{
		if (m_eRunUpDown != StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
			Sleep(KEY_SLEEP_TIME);
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
		/*if (m_iEndAttackTime != 0)
		{
			m_pkStarsControl->OnKeyDown('V');
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyDown('V');
			Sleep(KEY_SLEEP_TIME);
			m_pkStarsControl->OnKeyUp('V');
			Sleep(KEY_SLEEP_TIME);
		}*/
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
		Sleep(KEY_SLEEP_TIME);
		m_pkStarsControl->OnKeyDown('X');
		Sleep(KEY_SLEEP_TIME);
		m_pkStarsControl->OnKeyUp('X');
		Sleep(KEY_SLEEP_TIME);
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
		if (m_eBattleState >= StarsBattleState_AllClear)
		{
			return;
		}

		ST_POS kMiniMapPlayerPos = FindPicture("PlayerMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 180, m_kGameRect.right, m_kGameRect.top + 20, m_kGameRect.top + 180), false);
		if (kMiniMapPlayerPos.x != -1)
		{
			SetUserDataInt("iMiniMapPlayerPosX", kMiniMapPlayerPos.x + 3);
			SetUserDataInt("iMiniMapPlayerPosY", kMiniMapPlayerPos.y + 2);
			iMiniMapState = 1;
			PrintLog("kMiniMapPlayerPos:%d,%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
			PrintLog("iMiniMapState:1");
		}
	}
	if (iMiniMapState == 1)	// 检查战斗结束
	{
		int iPlayerPosX = GetUserDataInt("iMiniMapPlayerPosX");
		int iPlayerPosY = GetUserDataInt("iMiniMapPlayerPosY");
		int iOffset[4][2] = { { -18, 0 }, { 18, 0 }, { 0, -18 }, {0, 18} };
		for (int i = 0; i < 4; ++i)
		{
			DWORD dwColor = m_pkStarsGraphy->GetColor(ST_POS(iPlayerPosX + iOffset[i][0], iPlayerPosY + iOffset[i][1]));
			if (dwColor == COLOR_ROOM_OPEN || dwColor == COLOR_ROOM_UNKOWN_OPEN)
			{
				iMiniMapState = 2;
				PrintLog("iMiniMapState:2");
				break;
			}
		}
	}
	if (iMiniMapState == 2)	// 查找要进的房间
	{
		ST_POS kMiniMapBossPos = FindPicture("BossMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 150, m_kGameRect.right, m_kGameRect.top + 30, m_kGameRect.top + 180), false);
		if (kMiniMapBossPos.x != -1)
		{
			kMiniMapBossPos.x += 7;
			kMiniMapBossPos.y += 7;
			// 递归查找路径
			std::vector<ST_RECT> akQueue;
			int iCurIndex = 0;
			bool bVisited[50][50];
			int iStartPosX = GetUserDataInt("iMiniMapPlayerPosX");
			int iStartPosY = GetUserDataInt("iMiniMapPlayerPosY");
			const int aiDir[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
			const int aiDir1[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
			memset(bVisited, 0, 50 * 50);
			bVisited[25][25] = true;
			akQueue.push_back(ST_RECT(25, 25, -1, -1));
			bool bFindToBoss = false;

			ST_RECT kTempPoint;
			while (iCurIndex < akQueue.size())
			{
				ST_RECT kCurPoint = akQueue[iCurIndex];
				iCurIndex++;

				for (int k = 0; k < 4; ++k) 
				{
					kTempPoint.left = aiDir[k][0] + kCurPoint.left;
					kTempPoint.right = aiDir[k][1] + kCurPoint.right;
					if (kTempPoint.left >= 0 && kTempPoint.left < 50 && kTempPoint.right >= 0 && kTempPoint.right < 50 && !bVisited[kTempPoint.left][kTempPoint.right])
					{
						// 避免误操作，检测两个点
						if (m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 5, iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 5)) == COLOR_ROOM_DOOR &&
							m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 5 + aiDir1[k][0], iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 5 + aiDir1[k][1])) == COLOR_ROOM_DOOR)
						{
							DWORD dwTempColor = m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 18, iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 18));
							if (dwTempColor == COLOR_ROOM_CLOSE || dwTempColor == COLOR_ROOM_OPEN)
							{
								kTempPoint.top = 1;	// 1代表房间
							}
							else
							{
								kTempPoint.top = 2;	// 2代表小地图问号，未知的房间
							}
							kTempPoint.bottom = iCurIndex - 1;
							akQueue.push_back(kTempPoint);
							bVisited[kTempPoint.left][kTempPoint.right] = true;
							if (dwTempColor == COLOR_ROOM_BOSS && (iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 18) == kMiniMapBossPos.x && (iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 18) == kMiniMapBossPos.y)
							{
								bFindToBoss = true;
								break;
							}
						}
					}
				}
				if (bFindToBoss)
				{
					break;
				}
			}

			int iBossPosX = (kMiniMapBossPos.x - iStartPosX) / 18 + 25;
			int iBossPosY = (kMiniMapBossPos.y - iStartPosY) / 18 + 25;
			int iMinDis = 999;
			int iMinDisIndex = akQueue.size() - 1;
			if (!bFindToBoss)
			{
				for (int i = 0; i < akQueue.size(); ++i)
				{
					if (akQueue[i].top == 2)
					{
						int iTempDisToBoss = abs(akQueue[i].left - iBossPosX) + abs(akQueue[i].right - iBossPosY);
						if (iTempDisToBoss < iMinDis)
						{
							iMinDis = iTempDisToBoss;
							iMinDisIndex = i;
						}
					}
				}
			}
			else
			{
				iMinDisIndex = akQueue.size() - 1;
			}

			int iLastRoomIndex;
			int iStep = 0;
			while (akQueue[iMinDisIndex].bottom != -1)
			{
				iLastRoomIndex = iMinDisIndex;
				iMinDisIndex = akQueue[iMinDisIndex].bottom;
				iStep++;
			}

			
			StarsRunDirection eDoorDir = StarsRunDirection_None;
			for (int i = 0; i < 4; ++i)
			{
				if (akQueue[iLastRoomIndex].left - 25 == aiDir[i][0] && akQueue[iLastRoomIndex].right - 25 == aiDir[i][1])
				{
					eDoorDir = StarsRunDirection(i + 1);
					break;
				}
			}

			SetUserDataInt("eRoomDirection", int(eDoorDir));
			SetUserDataInt("bIsBossRoom", (iStep == 1 && bFindToBoss));
			//save last state
			iMiniMapState = 3;
			PrintLog("iMiniMapState:3");
			char ac[5][40] = { { "StarsRunDirection_None" },{ "StarsRunDirection_Left" }, { "StarsRunDirection_Right" }, { "StarsRunDirection_Up" }, { "StarsRunDirection_Down" } };
			PrintLog("eRoomDirection:%s", ac[int(eDoorDir)]);
			PrintLog("bIsBossRoom:%d,%d,%d", iStep, bFindToBoss, (iStep == 1 && bFindToBoss));
		}

	}
	if (iMiniMapState == 3)
	{
		if (m_eBattleState >= StarsBattleState_AllClear)
		{
			iMiniMapState = 4;
			PrintLog("iMiniMapState:4");
		}
	}
	if (iMiniMapState == 4)	// 等待进入房间
	{
		int iIsBossRoom = GetUserDataInt("bIsBossRoom");
		if (iIsBossRoom == 0)
		{
			ST_POS kMiniMapPlayerPos = FindPicture("PlayerMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 180, m_kGameRect.right, m_kGameRect.top + 30, m_kGameRect.top + 180), false);
			PrintLog("kMiniMapPlayerPos:%d,%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
			if (kMiniMapPlayerPos.x != -1)
			{
				kMiniMapPlayerPos.x += 3;
				kMiniMapPlayerPos.y += 2;
				if (kMiniMapPlayerPos.x != GetUserDataInt("iMiniMapPlayerPosX") || kMiniMapPlayerPos.y != GetUserDataInt("iMiniMapPlayerPosY"))
				{
					SetUserDataInt("iMiniMapPlayerPosXLast", GetUserDataInt("iMiniMapPlayerPosX"));
					SetUserDataInt("iMiniMapPlayerPosYLast", GetUserDataInt("iMiniMapPlayerPosY"));
					SetUserDataInt("iMiniMapPlayerPosX", kMiniMapPlayerPos.x);
					SetUserDataInt("iMiniMapPlayerPosY", kMiniMapPlayerPos.y);
					iMiniMapState = 0;
					PrintLog("iMiniMapState:0");
					PrintLog("iMiniPX:%d, iMiniPY:%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
					PrintLog("iLastPX:%d, iLastPY:%d", GetUserDataInt("iMiniMapPlayerPosXLast"), GetUserDataInt("iMiniMapPlayerPosYLast"));
				}
			}
		}
		else
		{
			ST_POS kMiniMapBossPos = FindPicture("BossMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 180, m_kGameRect.right, m_kGameRect.top + 30, m_kGameRect.top + 180), false);
			PrintLog("kMiniMapBossPos:%d,%d", kMiniMapBossPos.x, kMiniMapBossPos.y);
			if (kMiniMapBossPos.x != -1)
			{
				ST_POS kMiniMapPlayerPos = FindPicture("PlayerMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 180, m_kGameRect.right, m_kGameRect.top + 30, m_kGameRect.top + 180), false);
				PrintLog("kMiniMapPlayerPos:%d,%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
				if (kMiniMapPlayerPos.x == -1)
				{
					iMiniMapState = 0;
					PrintLog("iMiniMapState:0");
				}
			}
		}
	}

	SetUserDataInt("iMiniMapState", iMiniMapState);
}

ST_POS StarsGamePlayer::FindMonster(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_MONSTER, kRect, true, kStartPos);
}

void StarsGamePlayer::SetUserDataInt(std::string kStr, int iValue)
{
	m_akUserDataInt[kStr] = iValue;
}

int StarsGamePlayer::GetUserDataInt(std::string kStr)
{
	std::map<std::string, int>::iterator itr = m_akUserDataInt.find(kStr);
	if (itr != m_akUserDataInt.end())
	{
		return itr->second;
	}
	return -1;
}

void StarsGamePlayer::SetUserDataFloat(std::string kStr, float fValue)
{
	m_akUserDataFloat[kStr] = fValue;
}

float StarsGamePlayer::GetUserDataFloat(std::string kStr)
{
	std::map<std::string, float>::iterator itr = m_akUserDataFloat.find(kStr);
	if (itr != m_akUserDataFloat.end())
	{
		return itr->second;
	}
	return -1;
}