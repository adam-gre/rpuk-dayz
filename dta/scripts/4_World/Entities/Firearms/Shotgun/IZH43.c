class Izh43Shotgun_Base : DoubleBarrel_Base
{
	override RecoilBase SpawnRecoilObject()
	{
		return new Izh43Recoil(this);
	}
};

class Izh43Shotgun: Izh43Shotgun_Base {};
class SawedoffIzh43Shotgun: Izh43Shotgun_Base {};

