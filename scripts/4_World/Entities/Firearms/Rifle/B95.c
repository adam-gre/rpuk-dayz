class B95_base : DoubleBarrel_Base
{
	override RecoilBase SpawnRecoilObject()
	{
		return new B95Recoil(this);
	}
};

class B95 : B95_base
{};