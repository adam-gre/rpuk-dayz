#include "$CurrentDir:\\mpmissions\\dayzOffline.chernarusplus\\Buildings.c"


void SpawnObject(string objectName, vector position, vector orientation)
{
    Object obj;
    obj = Object.Cast(GetGame().CreateObject(objectName, "0 0 0"));
    obj.SetPosition(position);
    obj.SetOrientation(orientation);
 
    // Force update collisions
    if (obj.CanAffectPathgraph())
    {
        obj.SetAffectPathgraph(true, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().UpdatePathgraphRegionByObject, 100, false, obj);
    }
}

<!------------------------------------------------------------------------------------------!>

	//INIT BUILDINGS
	AddBuildings();	
		//GetCEApi().ExportProxyData("7500 0 7500", 10000);  //Centre of map, radius of how far to go out and find buildings.