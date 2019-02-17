#include "StarsControl.h"
#include "StarsGraphy.h"

enum StarsSceneState
{
	StarsSceneState_None = 0,
	StarsSceneState_BaseRoom,
	StarsSceneState_Town,
	StarsSceneState_Loading,
	StarsSceneState_Battle,
	StarsSceneState_BattleEnd,
};

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
	void UpdateBattle();

	ST_POS FindPicture(std::string kPictureName, ST_RECT kRect, bool bUseLocalPos = true);
	ST_POS FIndPictureORB(std::string kPictureName, bool bUseLocalPos = true);
	ST_POS FindFont(std::string kStr, ST_RECT kRect, bool bUseLocalPos = true);
	ST_POS FindColor(DWORD dwColor, ST_RECT kRect, bool bUseLocalPos = true, ST_POS kStartPos = ST_POS(-1, -1));

	void SetSceneState(StarsSceneState eState);
private:
	bool IsDirectArrive(const ST_POS& kStartPos, const ST_POS& kEndPos);
	ST_POS FindPath(const ST_POS& kStartPos, const ST_POS& kEndPos);
	void ActionRun(float fDisX, float fDisY);
	void ActionStopRun(StarsRunDirection eDir);
	void UpdateRun();
	void ActionAttack(bool bStart);
	void UpdateAttack();
	// Ѳ��
	void Patrol();
	void UpdateMiniMapState();

	ST_POS FindMonster(const ST_RECT& kRect, ST_POS kStartPos);
	void SetUserDataInt(std::string kStr, int iValue);
	int GetUserDataInt(std::string kStr);
	void SetUserDataFloat(std::string kStr, float fValue);
	float GetUserDataFloat(std::string kStr);
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
	ST_RECT				m_kGameRect;
	ST_POS				m_kPlayerPos;
	ST_POS				m_kNearMonsterPos;
	ST_POS				m_kNearDoorPos;
	DWORD				m_iLastUpdaetPlayerPos;
	StarsSceneState		m_eSceneState;
	StarsRunDirection	m_ePlayerSide;
	std::map<std::string, int> m_akUserDataInt;
	std::map<std::string, float> m_akUserDataFloat;
};