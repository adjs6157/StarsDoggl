#include "StarsControl.h"
#include "StarsGraphy.h"

class StarsGamePlayer
{
public:
	StarsGamePlayer();
	~StarsGamePlayer();
	bool Initalize();
	bool Finitalize();
	void Update();

private:
	StarsGraphy*		m_pkStarsGraphy;
	StarsControl*		m_pkStarsControl;
};