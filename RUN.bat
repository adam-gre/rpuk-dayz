@echo off
cls

set watch=DayZ Server

title %watch% Watchdog

cd "E:\SteamLibrary\steamapps\common\DayZServer"

:watchdog
echo (%time%) %watch% started.
start "DayZ Server" /wait "DayZServer_x64.exe" -adminlog -netlog -freezecheck -showScriptErrors "-mod=@CF;@UnlimitedStamina;@ZomBerryAdminTools;@Trader;@BaseFurnitureMods2.0;@BuildAnywhere;@BuilderItems;@Community-Online-Tools;@DayZPlus(Continued);@Ear-Plugs;@WeaponReduxPack" VerifySignatures=3; -profiles=ServerProfiles -config=serverDZ.cfg  -port=2302 -nologs "-profiles=E:\SteamLibrary\steamapps\common\DayZServer\ServerProfiles\TraderFolder"
echo (%time%) %watch% close or crashed, restarting.
goto watchdog
-profiles=E:\SteamLibrary\steamapps\common\DayZServer\log -scrAllowFileWrite -nologs -adminlog