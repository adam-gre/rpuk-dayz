class ActionWritePaperCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.DEFAULT);
	}
};

class ActionWritePaper: ActionContinuousBase
{
	const float TARGET_DISTANCE = 2;
	
	void ActionWritePaper()
	{
		m_CallbackClass = ActionWritePaperCB;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
	}
	
	override void CreateConditionComponents()
	{	
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTNonRuined(TARGET_DISTANCE);
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ((item.ConfigIsExisting("writingColor") && target.GetObject().GetType() == "Paper") || (target.GetObject().ConfigIsExisting("writingColor") && item.GetType() == "Paper"))
		{
			return true;
		}
		return false;
	}

	/*override bool HasTarget()
	{
		return false;
	}*/

	override string GetText()
	{
		return "#write_note";
	}

	override void OnFinishProgressClient( ActionData action_data )
	{
		//which is pen and which paper
		if (action_data.m_Target.GetObject().ConfigIsExisting("writingColor"))
		{
			action_data.m_Player.m_writingImplement = EntityAI.Cast(action_data.m_Target.GetObject());
			action_data.m_Player.m_paper = action_data.m_MainItem;
		}
		else
		{
			action_data.m_Player.m_writingImplement = action_data.m_MainItem;
			action_data.m_Player.m_paper = EntityAI.Cast(action_data.m_Target.GetObject());
		}
		action_data.m_Player.enterNoteMenuWrite = true;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{
		action_data.m_Player.GetSoftSkillsManager().AddSpecialty( m_SpecialtyWeight );
	}
};