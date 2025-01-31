#include "headers/macro-condition.hpp"
#include "headers/advanced-scene-switcher.hpp"

const std::map<LogicType, LogicTypeInfo> MacroCondition::logicTypes = {
	{LogicType::NONE, {"AdvSceneSwitcher.logic.none"}},
	{LogicType::AND, {"AdvSceneSwitcher.logic.and"}},
	{LogicType::OR, {"AdvSceneSwitcher.logic.or"}},
	{LogicType::AND_NOT, {"AdvSceneSwitcher.logic.andNot"}},
	{LogicType::OR_NOT, {"AdvSceneSwitcher.logic.orNot"}},
	{LogicType::ROOT_NONE, {"AdvSceneSwitcher.logic.rootNone"}},
	{LogicType::ROOT_NOT, {"AdvSceneSwitcher.logic.not"}},
};

void DurationModifier::Save(obs_data_t *obj, const char *condName,
			    const char *secondsName, const char *unitName)
{
	obs_data_set_int(obj, condName, static_cast<int>(_type));
	_dur.Save(obj, secondsName, unitName);
}

void DurationModifier::Load(obs_data_t *obj, const char *condName,
			    const char *secondsName, const char *unitName)
{
	// For backwards compatability check if duration value exist without
	// time constraint condition - if so assume DurationCondition::MORE
	if (!obs_data_has_user_value(obj, condName) &&
	    obs_data_has_user_value(obj, secondsName)) {
		obs_data_set_int(obj, condName, static_cast<int>(Type::MORE));
	}

	_type = static_cast<Type>(obs_data_get_int(obj, condName));
	_dur.Load(obj, secondsName, unitName);
}

bool DurationModifier::DurationReached()
{
	switch (_type) {
	case DurationModifier::Type::NONE:
		return true;
		break;
	case DurationModifier::Type::MORE:
		return _dur.DurationReached();
		break;
	case DurationModifier::Type::EQUAL:
		if (_dur.DurationReached() && !_timeReached) {
			_timeReached = true;
			return true;
		}
		break;
	case DurationModifier::Type::LESS:
		return !_dur.DurationReached();
		break;
	case DurationModifier::Type::WITHIN:
		if (_dur.IsReset()) {
			return false;
		}
		return !_dur.DurationReached();
		break;
	default:
		break;
	}
	return false;
}

void DurationModifier::Reset()
{
	_timeReached = false;
	_dur.Reset();
}

bool MacroCondition::Save(obs_data_t *obj)
{
	MacroSegment::Save(obj);
	obs_data_set_string(obj, "id", GetId().c_str());
	obs_data_set_int(obj, "logic", static_cast<int>(_logic));
	_duration.Save(obj);
	return true;
}

bool MacroCondition::Load(obs_data_t *obj)
{
	MacroSegment::Load(obj);
	_logic = static_cast<LogicType>(obs_data_get_int(obj, "logic"));
	_duration.Load(obj);
	return true;
}

void MacroCondition::ResetDuration()
{
	_duration.Reset();
}

void MacroCondition::CheckDurationModifier(bool &val)
{
	if (_duration.GetType() != DurationModifier::Type::WITHIN && !val) {
		_duration.Reset();
	}
	if (_duration.GetType() == DurationModifier::Type::WITHIN && val) {
		_duration.Reset();
	}
	switch (_duration.GetType()) {
	case DurationModifier::Type::NONE:
	case DurationModifier::Type::MORE:
	case DurationModifier::Type::EQUAL:
	case DurationModifier::Type::LESS:
		if (!val) {
			_duration.Reset();
		}
		val = val && _duration.DurationReached();
		return;
	case DurationModifier::Type::WITHIN:
		if (val) {
			_duration.SetTimeRemaining(
				_duration.GetDuration().seconds);
		}
		val = val || _duration.DurationReached();
		break;
	default:
		break;
	}
}

void MacroCondition::SetDurationModifier(DurationModifier::Type m)
{
	_duration.SetModifier(m);
}

void MacroCondition::SetDurationUnit(DurationUnit u)
{
	_duration.SetUnit(u);
}

void MacroCondition::SetDuration(double seconds)
{
	_duration.SetValue(seconds);
}

void MacroRefCondition::ResolveMacroRef()
{
	_macro.UpdateRef();
}
