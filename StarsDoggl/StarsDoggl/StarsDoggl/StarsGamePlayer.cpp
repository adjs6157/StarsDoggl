#include "StarsGamePlayer.h"
#include <queue>

#define COLOR_MONSTER 0xFFFF00FF
#define COLOR_BOSS 0xFFFF00CC
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
#define COLOR_ITEM 0xFFFF8400

//enum starsImgColor{ SCOLOR_NONE = 0, SCOLOR_MONSTER = 0xFFFF00FF, SCOLOR_OBJECT = 0xFFFFFF00, SCOLOR_BLOCK = 0xFFFF0000, SCOLOR_PATHGATE = 0xFF00FF00,
//SCOLOR_ITEM = 0xFF00FF00, SCOLOR_MINIMAP = 0xFFAAB450, SCOLOR_MINIMAP_OPEN = 0xFF416E14, SCOLOR_MINIMAP_UNKONW = 0xFF417D8C, SCOLOR_MINIMAP_UNKONW_OPEN = 0xFF142350, SCOLOR_MONSTERICON = 0xFFC8C800};

extern int iScreenShotWidth;
extern int iScreenShotHeight;
extern HWND g_iTargetGameHandle;
extern bool g_bDebugMode;
extern void PrintLog(const char *format, ...);
#define KEY_SLEEP_TIME 25

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
	m_iCantFindNameCount = 0;
	m_iPlayerNotMoveCount = 0;
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
		MessageBoxA(NULL, "g_kStarsGraphy��ʼ��ʧ��", "Warning", MB_OK);
		return false;
	}

	m_pkStarsControl = new StarsControl();
	if (!m_pkStarsControl->Initalize())
	{
		MessageBoxA(NULL, "g_kStarsControl��ʼ��ʧ��", "Warning", MB_OK);
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
		bool bHandleInvald = false;
		RECT kGameRect;
		if (GetWindowRect(g_iTargetGameHandle, &kGameRect))
		{
			m_kGameRect.left = kGameRect.left;
			m_kGameRect.right = kGameRect.right;
			m_kGameRect.top = kGameRect.top;
			m_kGameRect.bottom = kGameRect.bottom;
			if (m_kGameRect.left <= 0 && m_kGameRect.right <= 0)
			{
				bHandleInvald = true;
			}
			if (g_bDebugMode)
			{
				m_kGameRect.left += 485;
				m_kGameRect.right = m_kGameRect.left + 800;
				m_kGameRect.top += 100;
				m_kGameRect.bottom = m_kGameRect.top + 600;
			}
		}
		else
		{
			bHandleInvald = true;
		}

		// ˵����С�����߾����Ч��
		if (bHandleInvald)
		{
			m_kGameRect.left = 0;
			m_kGameRect.right = 830;
			m_kGameRect.top = 0;
			m_kGameRect.bottom = 750;
			if (g_bDebugMode)
			{
				g_iTargetGameHandle = FindWindowA(NULL, "Windows 7 x64 - VMware Workstation");
			}
			else
			{
				g_iTargetGameHandle = FindWindowA(NULL, "���³�����ʿ");
			}
			return;
		}
	}

	//m_pkStarsControl->OnKeyDown(VK_RIGHT);
	//Sleep(30);
	//m_pkStarsControl->OnKeyUp(VK_RIGHT);
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

	// 5��6����BOSS������
	if (GetUserDataInt("iMiniMapState") == 5 || GetUserDataInt("iMiniMapState") == 6)
	{
		UpdateMiniMapState();
		UpdateAttack();
		m_eBattleState = StarsBattleState_Start;
		return;
	}

	if (timeGetTime() - m_iLastUpdaetPlayerPos > 100)
	{
		m_iLastUpdaetPlayerPos = timeGetTime();
		m_kLastPlayerPos = m_kPlayerPos;
		ST_POS kPlayerPos = FindPicture("PlayerName.bmp", ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50));
		if (kPlayerPos.x != -1)
		{
			m_kPlayerPos = kPlayerPos;
			//m_kPlayerPos.y += 140;
			//m_kPlayerPos.x += 45;
			m_kPlayerPos.y += 119;
			m_kPlayerPos.x += 35;
			m_iCantFindNameCount = 0;
		}
		else
		{
			m_iCantFindNameCount++;
			if (m_iCantFindNameCount >= 20)
			{
				m_iCantFindNameCount = 0;
				int iTemp = timeGetTime() % 13;
				if (iTemp < 3)
				{
					ActionRun(-120, -120);
				}
				else if (iTemp < 6)
				{
					ActionRun(120, 120);
				}
				else if (iTemp < 9)
				{
					ActionRun(120, -120);
				}
				else
				{
					ActionRun(-120, 120);
				}
				return;
			}
		}

		m_kNearBOSSPos = FindBOSS(ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50), ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
		m_kNearMonsterPos = FindMonster(ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50), ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
		m_kItemPos = FindItem(ST_RECT(m_kGameRect.left, m_kGameRect.right, m_kGameRect.top + 155, m_kGameRect.bottom - 50), ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
	}
	UpdateMiniMapState();
	if (GetUserDataInt("iMiniMapState") == 3 && m_kItemPos.x == -1 && m_kItemPos.y == -1)
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
										 if (m_kItemPos.x != -1 && m_kItemPos.y != -1)
										 {
											 bool bPickItem = false;
											 if (m_kNearMonsterPos.x != -1 && m_kNearMonsterPos.y != -1)
											 {
												 if (GetDistance(m_kPlayerPos.x, m_kPlayerPos.y, m_kItemPos.x, m_kItemPos.y) < GetDistance(m_kPlayerPos.x, m_kPlayerPos.y, m_kNearMonsterPos.x, m_kNearMonsterPos.y))
												 {
													 bPickItem = true;
												 }
											 }
											 else
											 {
												 bPickItem = true;
											 }
											 
											 if (bPickItem)
											 {
												 ActionRun(m_kItemPos.x - m_kPlayerPos.x, m_kItemPos.y - m_kPlayerPos.y);
												 m_eBattleState = StarsBattleState_PickItem;
												 PrintLog("PlayerPos:%d, %d", m_kPlayerPos.x, m_kPlayerPos.y);
												 PrintLog("ItemPos:%d, %d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
												 PrintLog("ChangeState:PickItem %d,%d", m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
												 break;
											 }
										 }

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
										   if (m_kItemPos.x != -1 && m_kItemPos.y != -1)
										   {
											   bool bPickItem = false;
											   if (m_kNearMonsterPos.x != -1 && m_kNearMonsterPos.y != -1)
											   {
												   if (GetDistance(m_kPlayerPos.x, m_kPlayerPos.y, m_kItemPos.x, m_kItemPos.y) < GetDistance(m_kPlayerPos.x, m_kPlayerPos.y, m_kNearMonsterPos.x, m_kNearMonsterPos.y))
												   {
													   bPickItem = true;
												   }
											   }
											   else
											   {
												   bPickItem = true;
											   }

											   if (bPickItem)
											   {
												   ActionRun(m_kItemPos.x - m_kPlayerPos.x, m_kItemPos.y - m_kPlayerPos.y);
												   m_eBattleState = StarsBattleState_PickItem;
												   PrintLog("PlayerPos:%d, %d", m_kPlayerPos.x, m_kPlayerPos.y);
												   PrintLog("ItemPos:%d, %d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
												   PrintLog("ChangeState:PickItem %d,%d", m_kNearMonsterPos.x - m_kPlayerPos.x, m_kNearMonsterPos.y - m_kPlayerPos.y);
												   break;
											   }
										   }

										   bool bXOutOfRange = fabsf(m_kNearMonsterPos.x - m_kPlayerPos.x) > 160;
										   bool bYOutOfRange = fabsf(m_kNearMonsterPos.y - m_kPlayerPos.y) > 18;
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
											   ActionRun(0, 0);
											   ActionAttack(true);
											   PrintLog("AttackMonster::%d,%d", m_kNearMonsterPos.x, m_kNearMonsterPos.y);
										   }
										   break;
	}
	case StarsBattleState_PickItem:
	{
									  if (m_kItemPos.x == -1 || m_kItemPos.y == -1)
									  {
										  m_eBattleState = StarsBattleState_FindMonster;
										  PrintLog("PickItemOver");
										  PrintLog("ChangeState:FindMonster");
									  }

									  bool bXOutOfRange = fabsf(m_kItemPos.x - m_kPlayerPos.x) > 40;
									  bool bYOutOfRange = fabsf(m_kItemPos.y - m_kPlayerPos.y) > 18;

									  if (bXOutOfRange || bYOutOfRange)
									  {
										  if (bXOutOfRange && bYOutOfRange)
										  {
											  ActionRun(m_kItemPos.x - m_kPlayerPos.x, m_kItemPos.y - m_kPlayerPos.y);
										  }
										  else if (bXOutOfRange && !bYOutOfRange)
										  {
											  ActionRun(m_kItemPos.x - m_kPlayerPos.x, 0);
										  }
										  else
										  {
											  ActionRun(0, m_kItemPos.y - m_kPlayerPos.y);
										  }
									  }
									  else
									  {
										  ActionRun(0, 0);
										  Sleep(KEY_SLEEP_TIME * 2);
										  m_pkStarsControl->OnKeyDown('X');
										  Sleep(KEY_SLEEP_TIME);
										  m_pkStarsControl->OnKeyUp('X');
										  Sleep(KEY_SLEEP_TIME);
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
									  bool bFindDoor = false;
									  if (kDoorPos.x != -1 && kDoorPos.y != -1)
									  {
										  if (eDir == StarsRunDirection_Up || eDir == StarsRunDirection_Down)
										  {
											  ActionRun(kDoorPos.x - m_kPlayerPos.x, kDoorPos.y - m_kPlayerPos.y + (eDir == StarsRunDirection_Down ? 100 : -100));
											  SetUserDataInt("GoDoorEndTime", (m_iLeftRightEndTime > m_iUpDownEndTime ? m_iLeftRightEndTime : m_iUpDownEndTime) + 1000);
											  m_eBattleState = StarsBattleState_GoNextRoom;
											  SetUserDataInt("GoNextRoomBlock", 0);
											  PrintLog("ChangeState:GoNextRoom");
											  PrintLog("DoorPos:%d, %d", kDoorPos.x, kDoorPos.y);
											  bFindDoor = true;
										  }
										  if (eDir == StarsRunDirection_Right && kDoorPos.x > 400 || eDir == StarsRunDirection_Left && kDoorPos.x < 400)
										  {
											  ActionRun(kDoorPos.x - m_kPlayerPos.x + (eDir == StarsRunDirection_Right ? 100 : -100), kDoorPos.y - m_kPlayerPos.y);
											  SetUserDataInt("GoDoorEndTime", (m_iLeftRightEndTime > m_iUpDownEndTime ? m_iLeftRightEndTime : m_iUpDownEndTime) + 1000);
											  m_eBattleState = StarsBattleState_GoNextRoom;
											  SetUserDataInt("GoNextRoomBlock", 0);
											  PrintLog("ChangeState:GoNextRoom");
											  PrintLog("DoorPos:%d, %d", kDoorPos.x, kDoorPos.y);
											  bFindDoor = true;
										  }									  
									  }
									  if(!bFindDoor)
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
	if (eState == StarsSceneState_Battle)
	{
		ResetBattle();
	}
}

#define BLOCK_SIZE 18.0f // �赲���С

void StarsGamePlayer::ResetBattle()
{
	SetUserDataInt("iMiniMapState", -1);
	m_eBattleState = StarsBattleState_Start;

	m_eLeftRight = StarsRunDirection_None;
	m_iLeftRightEndTime = 0;
	m_eRunUpDown = StarsRunDirection_None;
	m_iUpDownEndTime = 0;
}

bool StarsGamePlayer::IsDirectArrive(ST_POS kStartPos, ST_POS kEndPos)
{
	//PrintLog("kStart,%d,%d,kEnd,%d,%d", kStartPos.x, kStartPos.y,kEndPos.x,kEndPos.y);
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
	int iBlockOff = BLOCK_SIZE / 2;
	int iBlockOff1 = BLOCK_SIZE / 4;
	for (int i = 0; i < iCount; ++i)
	{
		//PrintLog("GetColor:%d,%d,%X", int(fTempX), int(fTempY), m_pkStarsGraphy->GetColor(ST_POS(fTempX, fTempY)));
		if (m_pkStarsGraphy->GetColor(ST_POS(fTempX, fTempY)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX - iBlockOff, fTempY - iBlockOff)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX - iBlockOff, fTempY + iBlockOff)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX + iBlockOff, fTempY - iBlockOff)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX + iBlockOff, fTempY + iBlockOff)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX - iBlockOff1, fTempY - iBlockOff1)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX - iBlockOff1, fTempY + iBlockOff1)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX + iBlockOff1, fTempY - iBlockOff1)) == COLOR_BLOCK
			|| m_pkStarsGraphy->GetColor(ST_POS(fTempX + iBlockOff1, fTempY + iBlockOff1)) == COLOR_BLOCK)
		{
			return false;
		}
		fTempX += fDifX;
		fTempY += fDifY;
	}
	return true;
}

#define BOTTOM_HEIGHT 30

ST_POS StarsGamePlayer::FindPath(ST_POS kStartPos, ST_POS kEndPos)
{
	kStartPos.x += m_kGameRect.left;
	kStartPos.y += m_kGameRect.top;
	kEndPos.x += m_kGameRect.left;
	kEndPos.y += m_kGameRect.top;
	// ����ֱ�ӵ���
	if (IsDirectArrive(kStartPos, kEndPos))
	{
		return ST_POS(kEndPos.x - m_kGameRect.left, kEndPos.y - m_kGameRect.top);
	}

	// ���ҵ�һ������վ���ĵ�
	std::vector<ST_RECT> akQueueStart;
	bool bVisitedStart[200][200];
	const int aiDir[8][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };
	memset(bVisitedStart, 0, 200 * 200 * sizeof(bool));
	akQueueStart.push_back(ST_RECT(kStartPos.x, kStartPos.y, -1, -1));
	bVisitedStart[100][100] = true;
	int iLeftStart = kStartPos.x - 100, iRightStart = kStartPos.x + 100, iTopStart = kStartPos.y - 100, iBottomStart = kStartPos.y + 100;
	if (iLeftStart < m_kGameRect.left) iLeftStart = m_kGameRect.left;
	if (iRightStart > m_kGameRect.right) iRightStart = m_kGameRect.right;
	if (iTopStart < m_kGameRect.top) iTopStart = m_kGameRect.top;
	if (iBottomStart > m_kGameRect.bottom - BOTTOM_HEIGHT) iBottomStart = m_kGameRect.bottom - BOTTOM_HEIGHT;
	ST_RECT kTempPoint;
	bool bFindStartPoint = false;
	int iCurIndexStart = 0;
	while (iCurIndexStart < akQueueStart.size() && akQueueStart.size() < 1600)
	{
		ST_RECT kCurPoint = akQueueStart[iCurIndexStart];
		iCurIndexStart++;

		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.left = aiDir[k][0] + kCurPoint.left;
			kTempPoint.right = aiDir[k][1] + kCurPoint.right;
			if (kTempPoint.left >= iLeftStart && kTempPoint.left < iRightStart && kTempPoint.right >= iTopStart && kTempPoint.right < iBottomStart && !bVisitedStart[kTempPoint.left - kStartPos.x + 100][kTempPoint.right - kStartPos.y + 100])
			{
				bVisitedStart[kTempPoint.left - kStartPos.x + 100][kTempPoint.right - kStartPos.y + 100] = true;

				if (m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left , kTempPoint.right)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left - BLOCK_SIZE / 2, kTempPoint.right - BLOCK_SIZE / 2)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left - BLOCK_SIZE / 2, kTempPoint.right + BLOCK_SIZE / 2)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left + BLOCK_SIZE / 2, kTempPoint.right - BLOCK_SIZE / 2)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left + BLOCK_SIZE / 2, kTempPoint.right + BLOCK_SIZE / 2)) != COLOR_BLOCK)
				{
					bFindStartPoint = true;
					kStartPos.x = kTempPoint.left;
					kStartPos.y = kTempPoint.right;
					break;
				}
				else
				{
					kTempPoint.bottom = -2;
					akQueueStart.push_back(kTempPoint);
				}
			}
			else
			{
				kTempPoint.bottom = -2;
				akQueueStart.push_back(kTempPoint);
			}
		}
		if (bFindStartPoint)
		{
			break;
		}
	}


	// �����治Ѱ· ���·��������
	// �ݹ����·��
	std::vector<ST_RECT> akQueue;
	int iCurIndex = 0;
	bool bVisited[200][200];
	int iStartPosX = kStartPos.x / BLOCK_SIZE;
	int iStartPosY = kStartPos.y / BLOCK_SIZE;
	int iStartPosOffX = kStartPos.x % (int)BLOCK_SIZE;
	int iStartPosOffY = kStartPos.x % (int)BLOCK_SIZE;
	int iEndPosX = kEndPos.x / BLOCK_SIZE;
	int iEndPosY = kEndPos.y / BLOCK_SIZE;
	int iLeft = m_kGameRect.left / BLOCK_SIZE, iRight = m_kGameRect.right / BLOCK_SIZE, iTop = m_kGameRect.top / BLOCK_SIZE, iBottom = (m_kGameRect.bottom - BOTTOM_HEIGHT) / BLOCK_SIZE;
	
	memset(bVisited, 0, 200 * 200 * sizeof(bool));
	bVisited[iStartPosX][iStartPosY] = true;
	akQueue.push_back(ST_RECT(iStartPosX, iStartPosY, -1, -1));
	bool bFindPath = false;
	int iBlockOff = BLOCK_SIZE / 2;
	int iBlockOff1 = BLOCK_SIZE / 4;
	//PrintLog("Range:%d, %d, %d, %d", iLeft, iRight, iTop, iBottom);
	bool bFindStart = false;
	while (iCurIndex < akQueue.size() && akQueue.size() < 1600)
	{
		ST_RECT kCurPoint = akQueue[iCurIndex];
		iCurIndex++;

		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.left = aiDir[k][0] + kCurPoint.left;
			kTempPoint.right = aiDir[k][1] + kCurPoint.right;
			//PrintLog("kTempPoint:%d,%d", kTempPoint.left, kTempPoint.right);
			if (kTempPoint.left >= iLeft && kTempPoint.left < iRight && kTempPoint.right >= iTop && kTempPoint.right < iBottom && !bVisited[kTempPoint.left][kTempPoint.right])
			{
				bVisited[kTempPoint.left][kTempPoint.right] = true;
				
				//PrintLog("GetColor:%d,%d,%X", kTempPoint.left * BLOCK_SIZE, kTempPoint.right * BLOCK_SIZE, m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE, kTempPoint.right * BLOCK_SIZE)));
				//PrintLog("GetColor:%d,%d,%X", kTempPoint.left * BLOCK_SIZE - iBlockOff, kTempPoint.right * BLOCK_SIZE - iBlockOff, m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff, kTempPoint.right * BLOCK_SIZE - iBlockOff)));
				//PrintLog("GetColor:%d,%d,%X", kTempPoint.left * BLOCK_SIZE - iBlockOff, kTempPoint.right * BLOCK_SIZE + iBlockOff, m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff, kTempPoint.right * BLOCK_SIZE + iBlockOff)));
				//PrintLog("GetColor:%d,%d,%X", kTempPoint.left * BLOCK_SIZE + iBlockOff, kTempPoint.right * BLOCK_SIZE - iBlockOff, m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff, kTempPoint.right * BLOCK_SIZE - iBlockOff)));
				//PrintLog("GetColor:%d,%d,%X", kTempPoint.left * BLOCK_SIZE + iBlockOff, kTempPoint.right * BLOCK_SIZE + iBlockOff, m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff, kTempPoint.right * BLOCK_SIZE + iBlockOff)));
				if (m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iStartPosOffX, kTempPoint.right * BLOCK_SIZE + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff + iStartPosOffX, kTempPoint.right * BLOCK_SIZE - iBlockOff + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff + iStartPosOffX, kTempPoint.right * BLOCK_SIZE + iBlockOff + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff + iStartPosOffX, kTempPoint.right * BLOCK_SIZE - iBlockOff + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff + iStartPosOffX, kTempPoint.right * BLOCK_SIZE + iBlockOff + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff1 + iStartPosOffX, kTempPoint.right * BLOCK_SIZE - iBlockOff1 + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE - iBlockOff1 + iStartPosOffX, kTempPoint.right * BLOCK_SIZE + iBlockOff1 + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff1 + iStartPosOffX, kTempPoint.right * BLOCK_SIZE - iBlockOff1 + iStartPosOffY)) != COLOR_BLOCK
					&& m_pkStarsGraphy->GetColor(ST_POS(kTempPoint.left * BLOCK_SIZE + iBlockOff1 + iStartPosOffX, kTempPoint.right * BLOCK_SIZE + iBlockOff1 + iStartPosOffY)) != COLOR_BLOCK)
				{
					kTempPoint.bottom = iCurIndex - 1;
					akQueue.push_back(kTempPoint);
					bFindStart = true;
					
					if (kTempPoint.left == iEndPosX && kTempPoint.right == iEndPosY)
					{
						bFindPath = true;
						PrintLog("bFindPath");
						break;
					}
				}
				else if (!bFindStart)
				{
					kTempPoint.bottom = -2;
					akQueue.push_back(kTempPoint);
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
	if (!bFindPath)	// ûѰ·��Ŀ�����������
	{
		for (int i = 0; i < akQueue.size(); ++i)
		{
			// -2�ǲ����õĵ�
			if (akQueue[i].bottom == -2) continue;
			int iTempDisToEnd = abs(akQueue[i].left - iEndPosX) + abs(akQueue[i].right - iEndPosY);
			if (iTempDisToEnd < iMinDis)
			{
				iMinDis = iTempDisToEnd;
				iMinDisIndex = i;
			}
		}
	}
	else // �ҵ������һ�������Ŀ���
	{
		iMinDisIndex = akQueue.size() - 1;
	}
	PrintLog("iMinDisIndex:%d", iMinDisIndex);
	bool bSuccess = false;
	while (akQueue[iMinDisIndex].bottom != -1 && akQueue[iMinDisIndex].bottom != -2)
	{
		PrintLog("Path:%d,%d", (int)(akQueue[iMinDisIndex].left * BLOCK_SIZE), int(akQueue[iMinDisIndex].right * BLOCK_SIZE));
		// �ҵ���һ������ֱ�ߵ���ĵ�
		if (IsDirectArrive(kStartPos, ST_POS(akQueue[iMinDisIndex].left * BLOCK_SIZE, akQueue[iMinDisIndex].right * BLOCK_SIZE)))
		{
			bSuccess = true;
			break;
		}
		iMinDisIndex = akQueue[iMinDisIndex].bottom;
	}
	//// ʧ���˾�������
	//if (!bSuccess)
	//{
	//	return ST_POS(kStartPos.x + (kEndPos.x - kStartPos.x), kStartPos.y + (kEndPos.y - kStartPos.y));
	//}
	PrintLog("FindPathPos:%d, %d", (int)(akQueue[iMinDisIndex].left * BLOCK_SIZE), (int)(akQueue[iMinDisIndex].right * BLOCK_SIZE));
	return ST_POS(akQueue[iMinDisIndex].left * BLOCK_SIZE - m_kGameRect.left, akQueue[iMinDisIndex].right * BLOCK_SIZE - m_kGameRect.top);
}

void StarsGamePlayer::ActionRun(float fDisX, float fDisY)
{
	ST_POS kObjectPos = FindObject(ST_RECT(m_kPlayerPos.x + m_kGameRect.left - 30, m_kPlayerPos.x + m_kGameRect.left + 30, m_kPlayerPos.y + m_kGameRect.top - 30, m_kPlayerPos.y + m_kGameRect.top + 30),
		ST_POS(m_kPlayerPos.x + m_kGameRect.left, m_kPlayerPos.y + m_kGameRect.top));
	if ((fDisX != 0 || fDisY != 0) && kObjectPos.x != -1 && abs(kObjectPos.y - m_kPlayerPos.y) < 15)
	{
		fDisX = 0;
		fDisY = 0;
		m_pkStarsControl->OnKeyDown('X');
		Sleep(KEY_SLEEP_TIME);
		m_pkStarsControl->OnKeyUp('X');
		Sleep(KEY_SLEEP_TIME);
	}

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
	//PrintLog("ActionRun:%d,%d", (int)fDisX, (int)fDisY);
	//PrintLog("RunTime:%d,%d", (int)(fabsf(fDisX) / 200 * 1000), (int)(fabsf(fDisY) / 100 * 1000));
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
	if (m_kLastPlayerPos.x == m_kPlayerPos.x && m_kLastPlayerPos.y == m_kPlayerPos.y)
	{
		m_iPlayerNotMoveCount++;
	}
	else
	{
		m_iPlayerNotMoveCount = 0;
	}
	if (m_iLeftRightEndTime != 0)
	{
		if (timeGetTime() > m_iLeftRightEndTime)
		{
			if (m_eLeftRight != StarsRunDirection_None)
			{
				m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
				Sleep(KEY_SLEEP_TIME);
				m_eLeftRight = StarsRunDirection_None;
			}
			m_iLeftRightEndTime = 0;
		}
		else
		{
			if (!(m_kLastPlayerPos.x >= 343 && m_kLastPlayerPos.x <= 353 || m_kLastPlayerPos.x >= 443 && m_kLastPlayerPos.x <= 453))
			{
				if (m_iPlayerNotMoveCount >= 20)
				{
					m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
					m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
					m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
				}
			}
			else
			{
				if (m_iPlayerNotMoveCount >= 150)
				{
					m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
					m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
					m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
					Sleep(KEY_SLEEP_TIME);
				}
			}
		}
	}

	if (m_iUpDownEndTime != 0)
	{
		if (timeGetTime() > m_iUpDownEndTime)
		{
			if (m_eRunUpDown != StarsRunDirection_None)
			{
				m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
				Sleep(KEY_SLEEP_TIME);
				m_eRunUpDown = StarsRunDirection_None;
			}
			m_iUpDownEndTime = 0;
		}
		else
		{
			if (!(m_kLastPlayerPos.x >= 343 && m_kLastPlayerPos.x <= 353 || m_kLastPlayerPos.x >= 443 && m_kLastPlayerPos.x <= 453))
			{
				if (m_iPlayerNotMoveCount >= 20)
				{
					m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
					Sleep(KEY_SLEEP_TIME);
				}
			}
			else
			{
				if (m_iPlayerNotMoveCount >= 150)
				{
					m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
					Sleep(KEY_SLEEP_TIME);
				}
			}
		}
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
		m_pkStarsControl->OnKeyUp('X');
		Sleep(KEY_SLEEP_TIME);
		m_pkStarsControl->OnKeyDown(VK_SPACE);
		Sleep(KEY_SLEEP_TIME);
		m_pkStarsControl->OnKeyUp(VK_SPACE);
		Sleep(KEY_SLEEP_TIME);
	}
}

// Ѳ��
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

bool StarsGamePlayer::FindPlayerMiniMapPos()
{
	ST_POS kMiniMapPlayerPos = FindPicture("PlayerMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 180, m_kGameRect.right, m_kGameRect.top + 20, m_kGameRect.top + 180), false);
	if (kMiniMapPlayerPos.x != -1)
	{
		SetUserDataInt("iMiniMapPlayerPosX", kMiniMapPlayerPos.x + 3);
		SetUserDataInt("iMiniMapPlayerPosY", kMiniMapPlayerPos.y + 2);
		PrintLog("kMiniMapPlayerPos:%d,%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
		return true;
	}
	return false;
}

void StarsGamePlayer::UpdateMiniMapState()
{
	int iMiniMapState = GetUserDataInt("iMiniMapState");
	if (iMiniMapState == -1)	// ��ʼ״̬
	{
		iMiniMapState = 0;
	}
	if (iMiniMapState == 0)	// ��������Լ�С��ͼλ��
	{
		if (m_eBattleState >= StarsBattleState_AllClear)
		{
			return;
		}

		if (FindPlayerMiniMapPos())
		{
			iMiniMapState = 1;
			PrintLog("iMiniMapState:1");
		}
	}
	if (iMiniMapState == 1)	// ���ս������
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
	if (iMiniMapState == 2)	// ����Ҫ���ķ���
	{
		ST_POS kMiniMapBossPos = FindPicture("BossMiniMapIcon.bmp", ST_RECT(m_kGameRect.right - 150, m_kGameRect.right, m_kGameRect.top + 30, m_kGameRect.top + 180), false);
		if (kMiniMapBossPos.x != -1)
		{
			kMiniMapBossPos.x += 7;
			kMiniMapBossPos.y += 7;
			// �ݹ����·��
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
						// ��������������������
						if (m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 5, iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 5)) == COLOR_ROOM_DOOR &&
							m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 5 + aiDir1[k][0], iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 5 + aiDir1[k][1])) == COLOR_ROOM_DOOR)
						{
							DWORD dwTempColor = m_pkStarsGraphy->GetColor(ST_POS(iStartPosX + (kCurPoint.left - 25) * 18 + aiDir[k][0] * 18, iStartPosY + (kCurPoint.right - 25) * 18 + aiDir[k][1] * 18));
							if (dwTempColor == COLOR_ROOM_CLOSE || dwTempColor == COLOR_ROOM_OPEN)
							{
								kTempPoint.top = 1;	// 1������
							}
							else
							{
								kTempPoint.top = 2;	// 2����С��ͼ�ʺţ�δ֪�ķ���
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
	if (iMiniMapState == 4)	// �ȴ����뷿��
	{
		int iIsBossRoom = GetUserDataInt("bIsBossRoom");
		if (iIsBossRoom == 0)
		{
			ST_POS kMiniMapPlayerPos;
			kMiniMapPlayerPos.x = GetUserDataInt("iMiniMapPlayerPosX");
			kMiniMapPlayerPos.y = GetUserDataInt("iMiniMapPlayerPosY");
			if (FindPlayerMiniMapPos())
			{
				if (kMiniMapPlayerPos.x != GetUserDataInt("iMiniMapPlayerPosX") || kMiniMapPlayerPos.y != GetUserDataInt("iMiniMapPlayerPosY"))
				{
					SetUserDataInt("iMiniMapPlayerPosXLast", kMiniMapPlayerPos.x);
					SetUserDataInt("iMiniMapPlayerPosYLast", kMiniMapPlayerPos.y);
					
					iMiniMapState = 0;
					PrintLog("iMiniMapState:0");
					PrintLog("iMiniPX:%d, iMiniPY:%d", GetUserDataInt("iMiniMapPlayerPosXLast"), GetUserDataInt("iMiniMapPlayerPosYLast"));
					PrintLog("iLastPX:%d, iLastPY:%d", kMiniMapPlayerPos.x, kMiniMapPlayerPos.y);
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
	if (iMiniMapState == 6)
	{
		if (FindPlayerMiniMapPos())
		{
			iMiniMapState = 0;
			PrintLog("iMiniMapState:0");
		}
	}

	// һ����һ�μ��ؿ�ʤ��
	int iLastCheckMissionComplete = GetUserDataInt("iLastCheckMissionComplete");
	if (timeGetTime() - iLastCheckMissionComplete > 4500)
	{
		SetUserDataInt("iLastCheckMissionComplete", timeGetTime());
		if (iMiniMapState != 5)
		{
			ST_POS kSkipCard = FindPicture("Skip.bmp", ST_RECT(m_kGameRect.left + 701, m_kGameRect.left + 725, m_kGameRect.top + 69, m_kGameRect.top + 92), false);
			if (kSkipCard.x != -1)
			{

				iMiniMapState = 5;
				PrintLog("iMiniMapState:5");
				m_pkStarsControl->OnKeyDown(VK_ESCAPE);
				Sleep(KEY_SLEEP_TIME);
				m_pkStarsControl->OnKeyUp(VK_ESCAPE);
				Sleep(KEY_SLEEP_TIME*2);
				ActionRun(0, 0);
				m_pkStarsControl->OnKeyDown(VK_NUMPAD6);
				Sleep(KEY_SLEEP_TIME);
				m_pkStarsControl->OnKeyUp(VK_NUMPAD6);
				Sleep(KEY_SLEEP_TIME * 10);
				ActionAttack(true);
			}
		}
		

		ST_POS kRepeatMission = FindPicture("Repeat.bmp", ST_RECT(m_kGameRect.left + 625, m_kGameRect.left + 648, m_kGameRect.top + 69, m_kGameRect.top + 93), false);
		if (kRepeatMission.x != -1)
		{
			iMiniMapState = 6;
			PrintLog("iMiniMapState:6");
			m_pkStarsControl->OnKeyDown(VK_ESCAPE);
			Sleep(KEY_SLEEP_TIME * 2);
			m_pkStarsControl->OnKeyUp(VK_ESCAPE);
			Sleep(KEY_SLEEP_TIME * 2);
			m_pkStarsControl->OnKeyDown(VK_F10);
			Sleep(KEY_SLEEP_TIME * 2);
			m_pkStarsControl->OnKeyUp(VK_F10);
			Sleep(KEY_SLEEP_TIME * 2);
		}


	}

	SetUserDataInt("iMiniMapState", iMiniMapState);
}

ST_POS StarsGamePlayer::FindBOSS(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_BOSS, kRect, true, kStartPos);
}

ST_POS StarsGamePlayer::FindMonster(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_MONSTER, kRect, true, kStartPos);
}

ST_POS StarsGamePlayer::FindItem(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_ITEM, kRect, true, kStartPos);
}

ST_POS StarsGamePlayer::FindObject(const ST_RECT& kRect, ST_POS kStartPos)
{
	return FindColor(COLOR_OBJECT, kRect, true, kStartPos);
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