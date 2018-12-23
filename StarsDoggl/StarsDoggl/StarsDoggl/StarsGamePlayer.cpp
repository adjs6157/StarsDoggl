#include "StarsGamePlayer.h"

StarsGamePlayer::StarsGamePlayer()
{
	m_pkStarsGraphy = nullptr;
	m_pkStarsControl = nullptr;
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

	m_eBattleState = StarsBattleState_Start;
	m_eRunUpDown = StarsRunDirection_None;
	m_eLeftRight = StarsRunDirection_None;
	m_iUpDownEndTime = 0;
	m_iLeftRightEndTime = 0;
	m_bStartAttack = false;
}

bool StarsGamePlayer::Finitalize()
{
	m_pkStarsGraphy->Finitalize();
	m_pkStarsControl->Finitalize();
	return true;
}

void StarsGamePlayer::Update()
{
	m_pkStarsGraphy->Update();
	m_pkStarsControl->Update();


	switch (m_eBattleState)
	{
	case StarsBattleState_Start:
	{
		ActionRun(500, 0);
		m_eBattleState = StarsBattleState_FindMonster;
		break;
	}
	case StarsBattleState_FindMonster:
	{
										 ST_POS kMonsterPos = m_pkStarsGraphy->FIndPictureORB("test3.bmp");
										 if (kMonsterPos.x != -1 && kMonsterPos.y != -1)
										 {
											 ActionRun(kMonsterPos.x - 400, kMonsterPos.y - 300);
											 m_eBattleState = StarsBattleState_GoMonster;
										 }
										 break;
	}
	case StarsBattleState_GoMonster:
	{
									   if (m_eLeftRight == StarsRunDirection_None && m_eRunUpDown == StarsRunDirection_None)
									   {
										   m_eBattleState = StarsBattleState_AttackMonster;
									   }
									   break;
	}
	case StarsBattleState_AttackMonster:
	{
										   ActionAttack(true);
										   ST_POS kMonsterPos = m_pkStarsGraphy->FIndPictureORB("test3.bmp");
										   if (kMonsterPos.x == -1 && kMonsterPos.y == -1)
										   {
											   m_eBattleState = StarsBattleState_FindMonster;
										   }
										   break;
	}
	case StarsBattleState_AllClear:
		break;
	case StarsBattleState_FindDoor:
		break;
	case StarsBattleState_GoNextRoom:
		break;
	case StarsBattleState_Count:
		break;
	default:
		break;
	}
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
		m_pkStarsControl->OnKeyUp(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
		m_pkStarsControl->OnKeyDown(m_eLeftRight == StarsRunDirection_Right ? VK_RIGHT : VK_LEFT);
	}
	if (fDisY != 0)
	{
		m_eRunUpDown = fDisY > 0 ? StarsRunDirection_Down : StarsRunDirection_Up;
		m_iUpDownEndTime = fabsf(fDisY) / 30 * 1000 + timeGetTime();
		m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		m_pkStarsControl->OnKeyUp(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
		m_pkStarsControl->OnKeyDown(m_eRunUpDown == StarsRunDirection_Up ? VK_UP : VK_DOWN);
	}
	
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
		m_pkStarsControl->OnKeyUp('X');
	}
}