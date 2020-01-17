class ModInfo
{
	proto native owned string GetName();
	proto native owned string GetPicture();
	proto native owned string GetLogo();
	proto native owned string GetLogoSmall();
	proto native owned string GetLogoOver();
	proto native owned string GetTooltip();
	proto native owned string GetOverview();
	proto native owned string GetAction();
	proto native owned string GetAuthor();
	proto native owned string GetVersion();
	proto native bool GetDefault();
	proto native bool GetIsDLC();
	proto native bool GetIsOwned();
	proto native void GoToStore();
}