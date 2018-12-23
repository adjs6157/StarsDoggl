#include "StarsControl.h"
#include "StarsGraphy.h"

enum StarsBattleState
{
	StarsBattleState_Start = 0,
	StarsBattleState_FindMonster = 1,
	StarsBattleState_GoMonster = 2,
	StarsBattleState_AttackMonster = 3,
	StarsBattleState_AllClear = 4,
	StarsBattleState_FindDoor = 5,
	StarsBattleState_GoNextRoom = 6,
	StarsBattleState_Count,
};

enum StarsActionFlag
{
	StarsActionFlag_Run = 1 << 0,
	StarsActionFlag_Attack = 1 << 1,
};

enum StarsRunDirection
{
	StarsRunDirection_None,
	StarsRunDirection_Left,
	StarsRunDirection_Right,
	StarsRunDirection_Up,
	StarsRunDirection_Down,
};

class StarsGamePlayer
{
public:
	StarsGamePlayer();
	~StarsGamePlayer();
	bool Initalize();
	bool Finitalize();
	void Update();
private:
	void ActionRun(float fDisX, float fDisY);
	void UpdateRun();
	void ActionAttack(bool bStart);
	void UpdateAttack();
private:
	StarsGraphy*		m_pkStarsGraphy;
	StarsControl*		m_pkStarsControl;

	StarsBattleState	m_eBattleState;
	StarsRunDirection	m_eRunUpDown;
	DWORD					m_iUpDownEndTime;
	StarsRunDirection	m_eLeftRight;
	DWORD					m_iLeftRightEndTime;
	bool				m_bStartAttack;
	DWORD				m_iEndAttackTime;
};