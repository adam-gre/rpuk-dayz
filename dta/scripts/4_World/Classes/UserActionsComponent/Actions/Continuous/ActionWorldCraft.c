class WorldCraftActionReciveData : ActionReciveData
{
	ItemBase	m_MainItem;
	int 		m_RecipeID;
}
class WorldCraftActionData : ActionData
{
	int m_RecipeID;
}

class ActionWorldCraftCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousCraft(UATimeSpent.DEFAULT_CRAFT); //default value can be set in recipes
	}

	/*override void OnFinish(bool pCanceled)	
	{
		super.OnFinish(pCanceled);
		if( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			PlayerBase player;
			if( Class.CastTo(player, GetGame().GetPlayer()) )
			{
				if( player.GetCraftingManager().IsInventoryCraft() )
					player.GetCraftingManager().CancelInventoryCraft();
			}
		}
	}*/
};

class ActionWorldCraft: ActionContinuousBase
{
	private string m_ActionPrompt;

	void ActionWorldCraft()
	{
		m_CallbackClass = ActionWorldCraftCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH;
		m_ActionPrompt = "Craft";
	}
	
	override ActionData CreateActionData()
	{
		ActionData action_data = new WorldCraftActionData;
		return action_data;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}
		
	override string GetText()
	{
		PlayerBase player;
		if( Class.CastTo(player, GetGame().GetPlayer()) )
		{
			PluginRecipesManager module_recipes_manager;
			Class.CastTo(module_recipes_manager,  GetPlugin(PluginRecipesManager) );
			return module_recipes_manager.GetRecipeName( player.GetCraftingManager().GetRecipeID() );
		}

		return "Default worldcraft text";
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		//Client
		if( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{					
			if ( /*player.GetCraftingManager().IsWorldCraft() || player.GetCraftingManager().IsInventoryCraft()*/ player.GetCraftingManager().GetRecipesCount() > 0 )
			{
				return true;
			}
			return false;
		}
		else //Server
		{
			if ( !target.GetObject() || !item )
				return false;
		}
		
		return true;		
	}
	
	override bool SetupAction(PlayerBase player, ActionTarget target, ItemBase item, out ActionData action_data, Param extra_data = NULL )
	{
		if (super.SetupAction(player, target, item, action_data, extra_data ))
		{
			if (GetGame().IsClient() || !GetGame().IsMultiplayer())
			{
				WorldCraftActionData action_data_wc;
				Class.CastTo(action_data_wc, action_data);
				action_data_wc.m_RecipeID = action_data.m_Player.GetCraftingRecipeID();
			}
			return true;
		}
		return false;
	}
	
	override void Start( ActionData action_data ) //Setup on start of action
	{
		super.Start(action_data);
		if ( action_data.m_Player ) action_data.m_Player.GetItemAccessor().HideItemInHands(true);
	}
	
	override void OnEndServer( ActionData action_data )
	{
		if ( action_data.m_Player ) action_data.m_Player.GetItemAccessor().HideItemInHands(false);
	}
	
	override void OnEndClient( ActionData action_data )
	{
		if ( action_data.m_Player ) action_data.m_Player.GetItemAccessor().HideItemInHands(false);
	}	
	
	override void OnFinishProgressServer( ActionData action_data )
	{
		/*if (!GetGame().IsMultiplayer())
		{
			ActionManagerClient am = ActionManagerClient.Cast(action_data.m_Player.GetActionManager());
			am.UnlockInventory(action_data);
		}*/
		
		WorldCraftActionData action_data_wc;
		PluginRecipesManager module_recipes_manager;
		ItemBase item2;
		
		Class.CastTo(action_data_wc, action_data);
		Class.CastTo(module_recipes_manager,  GetPlugin(PluginRecipesManager) );
		Class.CastTo(item2,  action_data.m_Target.GetObject() );
		
		if( action_data.m_MainItem && item2 )
		{
			module_recipes_manager.PerformRecipeServer( action_data_wc.m_RecipeID, action_data.m_MainItem, item2, action_data.m_Player );
		}
	}
	
	override void OnFinishProgressClient( ActionData action_data )
	{
		/*ActionManagerClient am = ActionManagerClient.Cast(action_data.m_Player.GetActionManager());
		am.UnlockInventory(action_data);*/
	}
	
	override void WriteToContext (ParamsWriteContext ctx, ActionData action_data)
	{
		PlayerBase player;
		Class.CastTo(player, action_data.m_Player);
		WorldCraftActionData action_data_wc;
		Class.CastTo(action_data_wc, action_data);

		ctx.Write(action_data_wc.m_MainItem);
		ctx.Write(action_data_wc.m_Target.GetObject());
		ctx.Write(action_data_wc.m_RecipeID);
	}
	
	override bool ReadFromContext(ParamsReadContext ctx, out ActionReciveData action_recive_data )
	{
		if(!action_recive_data)
		{
			action_recive_data = new WorldCraftActionReciveData;
		}
		WorldCraftActionReciveData recive_data_wc = WorldCraftActionReciveData.Cast(action_recive_data);
		ItemBase item1 = null;
		ItemBase item2 = null;
		int recipeID = -1;
		if (!ctx.Read(item1))
				return false;
		if (!ctx.Read(item2))
				return false;
		if (!ctx.Read(recipeID))
				return false;
		
		if ( !item1 || !item2)
		{
			return false;
		}
		
		ActionTarget target;
		target = new ActionTarget(item2 , NULL, -1,vector.Zero, 0);
					
		recive_data_wc.m_MainItem = item1;
		recive_data_wc.m_Target = target;
		recive_data_wc.m_RecipeID = recipeID;
		return true;
	}
	
	override void HandleReciveData(ActionReciveData action_recive_data, ActionData action_data)
	{
		
		WorldCraftActionReciveData recive_data_wc = WorldCraftActionReciveData.Cast(action_recive_data);
		WorldCraftActionData action_data_wc = WorldCraftActionData.Cast(action_data);
		
		action_data_wc.m_MainItem = recive_data_wc.m_MainItem;
		if (!action_recive_data.m_Target)
		{
			Error("Action WC target not created.");
			action_data.m_Target = new ActionTarget(NULL, NULL, -1, vector.Zero, 0); 
		}
		else
		{
			action_data_wc.m_Target = recive_data_wc.m_Target;
		}
		action_data_wc.m_RecipeID = recive_data_wc.m_RecipeID;
	}
	
};

