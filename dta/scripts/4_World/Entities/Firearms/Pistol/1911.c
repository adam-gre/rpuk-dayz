class Colt1911_Base : Pistol_Base
{
	override RecoilBase SpawnRecoilObject()
	{
		return new Colt1911Recoil(this);
	}
};

class Colt1911 : Colt1911_Base {};
class Engraved1911 : Colt1911_Base {};