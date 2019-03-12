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
	StarsBattleState_PickItem = 4,
	StarsBattleState_AllClear = 10,
	StarsBattleState_FindDoor = 11,
	StarsBattleState_GoNextRoom = 12,
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

struct StarsSpellInfo
{
	int iBeginX;
	int iBeginY;
	std::string kIconName;
	int iCmd;
};

// boos  技能CD监控直接用ICON匹配  写一个窗口移位器 位移定位 用一下设计模式触发器task 特殊AI流程考虑脚本 
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
	void ResetBattle();
	bool IsDirectArrive(ST_POS kStartPos, ST_POS kEndPos);
	ST_POS FindPath(ST_POS kStartPos, ST_POS kEndPos);
	void ActionRun(float fDisX, float fDisY);
	void ActionStopRun(StarsRunDirection eDir);
	void UpdateRun();
	void ActionUseSpell(bool bUltimate);
	void ActionAttack(bool bStart, bool bRepeat = true;);
	void UpdateAttack();
	void UpdateSpellCD();
	// 巡逻
	void Patrol();
	bool FindPlayerMiniMapPos();
	void UpdateMiniMapState();

	ST_POS FindBOSS(const ST_RECT& kRect, ST_POS kStartPos);
	ST_POS FindMonster(const ST_RECT& kRect, ST_POS kStartPos);
	ST_POS FindItem(const ST_RECT& kRect, ST_POS kStartPos);
	ST_POS FindObject(const ST_RECT& kRect, ST_POS kStartPos);
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
	ST_POS				m_kLastPlayerPos;
	int					m_iPlayerNotMoveCount;
	ST_POS				m_kNearMonsterPos;
	ST_POS				m_kNearBOSSPos;
	ST_POS				m_kItemPos;
	ST_POS				m_kNearDoorPos;
	DWORD				m_iLastUpdaetPlayerPos;
	int					m_iCantFindNameCount;
	StarsSceneState		m_eSceneState;
	StarsRunDirection	m_ePlayerSide;
	std::map<std::string, int> m_akUserDataInt;
	std::map<std::string, float> m_akUserDataFloat;
	bool				m_bUseUltimateSpell;
	int					m_iLastUpdateSpell;
};