class TelnyashkaShirt extends Clothing
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionWringClothes);
	}
};