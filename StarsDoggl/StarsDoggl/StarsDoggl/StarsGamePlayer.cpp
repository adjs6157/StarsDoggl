#include "StarsGamePlayer.h"

extern int iScreenShotWidth;
extern int iScreenShotHeight;
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
	m_iGameHandle = 0;
	m_iLastUpdaetPlayerPos = 0;
	m_bAllClear = false;
	m_eSceneState = StarsSceneState_None;
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

	m_iGameHandle = FindWindowA(NULL, "DNF");
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
	
	if (m_iGameHandle == 0)
	{
		m_iGameHandle = FindWindowA(NULL, "地下城与勇士");

		m_kGameRect.left = 0;
		m_kGameRect.right = iScreenShotWidth;
		m_kGameRect.top = 0;
		m_kGameRect.bottom = iScreenShotHeight;
		m_pkStarsGraphy->Update(m_kGameRect);
		return;
	}
	else
	{
		RECT kGameRect;
		GetWindowRect(m_iGameHandle, &kGameRect);
		
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

	if (timeGetTime() - m_iLastUpdaetPlayerPos > 2000)
	{
		m_iLastUpdaetPlayerPos = timeGetTime();
		m_kPlayerPos = FindPicture("PlayerName.bmp", m_kGameRect);
		//if (m_kPlayerPos.x != -1)
		//{
		//	ActionRun(500, 0);
		//}

		if (!m_bAllClear)
		{
			ST_POS kMapPos = FindPicture("NextDoor.bmp", m_kGameRect);
			if (kMapPos.x != -1)
			{
				m_bAllClear = true;
				m_eBattleState = StarsBattleState_AllClear;
			}
		}
	}

	switch (m_eBattleState)
	{
	case StarsBattleState_Start:
	{
								   ActionRun(500, 0);
								   m_eBattleState = StarsBattleState_FindMonster;
								   PrintLog("ChangeState:FindMonster");
								   break;
	}
	case StarsBattleState_FindMonster:
	{
										 ST_POS kMonsterPos = FindMonster();
										 if (kMonsterPos.x != -1 && kMonsterPos.y != -1)
										 {
											 ActionRun(kMonsterPos.x - 400, kMonsterPos.y - 300);
											 m_eBattleState = StarsBattleState_GoMonster;
											 PrintLog("ChangeState:GoMonster");
										 }
										 else
										 {
											 ActionRun(500, 0);
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
										   ActionAttack(true);
										   ST_POS kMonsterPos = FindMonster();
										   if (kMonsterPos.x == -1 && kMonsterPos.y == -1)
										   {
											   m_eBattleState = StarsBattleState_FindMonster;
											   PrintLog("ChangeState:FindMonster");
										   }
										   else
										   {
											   PrintLog("MonsterX:%d,MonsterY%d",kMonsterPos.x,kMonsterPos.y);
											   ActionRun(kMonsterPos.x - 400, kMonsterPos.y - 300);
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

void StarsGamePlayer::ActionRun(float fDisX, float fDisY)
{
	if (m_eLeftRight != StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
		m_eLeftRight = StarsRunDirection_None;
		m_iLeftRightEndTime = 0;
	}

	if (m_eRunUpDown != StarsRunDirection_None)
	{
		m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		m_eRunUpDown = StarsRunDirection_None;
		m_iUpDownEndTime = 0;
	}


	if (fDisX != 0)
	{
		m_eLeftRight = fDisX > 0 ? StarsRunDirection_Right : StarsRunDirection_Left;
		m_iLeftRightEndTime = fabsf(fDisX) / 60 * 1000 + timeGetTime();
		m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
		Sleep(20);
		m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
		Sleep(20);
		m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
	}
	if (fDisY != 0)
	{
		m_eRunUpDown = fDisY > 0 ? StarsRunDirection_Down : StarsRunDirection_Up;
		m_iUpDownEndTime = fabsf(fDisY) / 30 * 1000 + timeGetTime();
		m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		Sleep(20);
		m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		Sleep(20);
		m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
	}
	
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

void StarsGamePlayer::UpdateRun()
{
	if (m_iLeftRightEndTime != 0 && timeGetTime() > m_iLeftRightEndTime)
	{
		if (m_eLeftRight != StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Left ? VK_LEFT : VK_RIGHT);
			m_eLeftRight = StarsRunDirection_None;
		}
		m_iLeftRightEndTime = 0;
	}

	if (m_iUpDownEndTime != 0 && timeGetTime() > m_iUpDownEndTime)
	{
		if (m_eRunUpDown != StarsRunDirection_None)
		{
			m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
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
		m_iEndAttackTime = 0;
	}
}

void StarsGamePlayer::UpdateAttack()
{
	if (timeGetTime() > m_iEndAttackTime)
	{
		m_iEndAttackTime = 0;
		m_bStartAttack = false;
	}

	if (m_bStartAttack)
	{
		m_pkStarsControl->OnKeyDown('X');
		Sleep(20);
		m_pkStarsControl->OnKeyUp('X');
	}
}

ST_POS StarsGamePlayer::FindMonster()
{
	char str[50];
	for (int i = 1; i <= 10; ++i)
	{
		sprintf_s(str, "Monster%d.bmp", i);
		ST_POS kMonsterPos = FIndPictureORB(str);
		if (kMonsterPos.x != -1 && kMonsterPos.y != -1)
		{
			PrintLog("FindMonster:%s", str);
			return kMonsterPos;
		}
	}
	return ST_POS(-1, -1);
}

void StarsGamePlayer::SetSceneState(StarsSceneState eState)
{
	m_eSceneState = eState;
}