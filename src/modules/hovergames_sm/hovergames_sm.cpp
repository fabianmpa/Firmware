#include "hovergames_sm.h"

int HoverGamesSM::print_status()
{
	/*Print actual state of both state machines*/
	PX4_INFO("Running");
	PX4_INFO("Global State Machine: %i",_hovergames.hovergames_sm);
	PX4_INFO("Active State Machine: %i",_hovergames.hovergames_activesm);
	perf_print_counter(_loop_perf);
	perf_print_counter(_loop_interval_perf);
	return 0;
}

HoverGamesSM::HoverGamesSM():ModuleParams(nullptr),ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::hp_default)
{
	_hovergames.hovergames_sm       = hovergames_s::STANDBY_STATE; /*Initialize to Standby State*/
	_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;  /*Initialize to Paused State*/
}

int HoverGamesSM::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int HoverGamesSM::task_spawn(int argc, char *argv[])
{
	HoverGamesSM *instance = new HoverGamesSM();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

bool HoverGamesSM::init()
{
	ScheduleOnInterval(1000_us); /*Run every 1000 mseconds*/
	return true;
}

void HoverGamesSM::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
		return;
	}

	perf_begin(_loop_perf);
	perf_count(_loop_interval_perf);

		parameters_update(); /*Update subscription values*/
		switch (_hovergames.hovergames_sm)
		{
			case HOVERGAMES_STANDBY_STATE: /*Sandby State*/
			_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;
			if(_vehStatus.arming_state == vehicle_status_s::ARMING_STATE_ARMED)
			{
				_hovergames.hovergames_sm = hovergames_s::PREACTIVE_STATE;

			}
			else if(
			   (_battery.warning != battery_status_s::BATTERY_WARNING_NONE)
			|| (_vehStatus.engine_failure == _hovergamesFailures.engineFailure)
			|| (_vehStatus.failsafe == _hovergamesFailures.failSafe)
			|| (_vehStatus.failure_detector_status != vehicle_status_s::FAILURE_NONE)
			|| (_vehStatus.mission_failure == _hovergamesFailures.missionFailure)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::FAULTED_STATE;
			}
			break;
			case HOVERGAMES_PREACTIVE_STATE: /*PreActive State*/
			_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;
			if(
			   (_actuator.armed == _hovergamesArmingStates.armed)
			&& (_battery.voltage_filtered_v >= minimumVoltageFiltered)
			&& (_battery.remaining >= minimumBatteryRemaining)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::ACTIVE_STATE;
			}
			else if(
			   (_battery.warning != battery_status_s::BATTERY_WARNING_NONE)
			|| (_vehStatus.engine_failure == _hovergamesFailures.engineFailure)
			|| (_vehStatus.failsafe == _hovergamesFailures.failSafe)
			|| (_vehStatus.failure_detector_status != vehicle_status_s::FAILURE_NONE)
			|| (_vehStatus.mission_failure == _hovergamesFailures.missionFailure)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::FAULTED_STATE;
			}
			else if(
			   (_actuator.armed == _hovergamesArmingStates.notArmed)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::STANDBY_STATE;
			}
			break;
			case HOVERGAMES_ACTIVE_STATE: /*Active State*/
			if(
			   (_battery.warning != battery_status_s::BATTERY_WARNING_NONE)
			|| (_vehStatus.engine_failure == _hovergamesFailures.engineFailure)
			|| (_vehStatus.failsafe  == _hovergamesFailures.failSafe)
			|| (_vehStatus.failure_detector_status != vehicle_status_s::FAILURE_NONE)
			|| (_vehStatus.mission_failure == _hovergamesFailures.missionFailure)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::FAULTED_STATE;
			}
			else if(
			   (_actuator.armed == _hovergamesArmingStates.notArmed)
			&& (_actuator.ready_to_arm == _hovergamesArmingStates.readyToArm)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::PREACTIVE_STATE;
			}
			switch (_hovergames.hovergames_activesm)
			{
				case HOVERGAMES_RECORDING_STATE: /*Recording State*/
				if(
				   (_distance.current_distance > twentyMeters)  /*20 meters*/
				 &&(_distance.current_distance <= thirtyFourMeters) /*34 meters inclusive*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::IMAGE_CAPTURE_STATE;
				}
				else if(
				   (_distance.current_distance < fiveMeters) /*5 meters*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;
				}
				break;
				case HOVERGAMES_IMAGE_CAPTURE_STATE: /*Image Capture State*/
				if(
				   (_distance.current_distance >= fiveMeters)  /*5 meters*/
				 &&(_distance.current_distance <= twentyMeters) /*20 meters inclusive*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::RECORDING_STATE;
				}
				else if(
				   (_distance.current_distance < fiveMeters) /*5 meters*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;
				}
				break;
				case HOVERGAMES_PAUSED_STATE: /*Paused State*/
				if(
				   (_distance.current_distance >= fiveMeters)  /*5 meters inclusive*/
				 &&(_distance.current_distance <= twentyMeters) /*20 meters inclusive*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::RECORDING_STATE;
				}
				else if(
				   (_distance.current_distance > twentyMeters)  /*20 meters*/
				 &&(_distance.current_distance <= thirtyFourMeters) /*34 meters inclusive*/
				)
				{
					_hovergames.hovergames_activesm = hovergames_s::IMAGE_CAPTURE_STATE;
				}
				break;
				default:
				/*Do nothing*/
				break;
			}
			break;
			case HOVERGAMES_FAULTED_STATE: /*Faulted State*/
			_hovergames.hovergames_activesm = hovergames_s::PAUSED_STATE;
			if(
			   (_battery.warning == battery_status_s::BATTERY_WARNING_NONE)
			|| (_vehStatus.engine_failure != _hovergamesFailures.engineFailure)
			|| (_vehStatus.failsafe != _hovergamesFailures.failSafe)
			|| (_vehStatus.failure_detector_status == vehicle_status_s::FAILURE_NONE)
			|| (_vehStatus.mission_failure != _hovergamesFailures.missionFailure)
			)
			{
				_hovergames.hovergames_sm = hovergames_s::STANDBY_STATE;
			}
			break;
			default:
			/* Do nothing*/
			break;
		}
		orb_publish(ORB_ID(hovergames), hovergamesAdv, &_hovergames); /*Write new hovergames message values*/
		perf_end(_loop_perf);
}

void HoverGamesSM::parameters_update(void)
{
 /* Update here topic susbscriptions*/
 	if(
	  ( _distance_sensor_sub.updated())
	||( _battery_status_sub.updated())
	||( _vehicle_status_sub.updated())
	||(_actuator_armed_sub.updated())
	)
	{
		_battery_status_sub.copy(&_battery);
		_distance_sensor_sub.copy(&_distance);
		_actuator_armed_sub.copy(&_actuator);
		_vehicle_status_sub.copy(&_vehStatus);
	}
}

HoverGamesSM::~HoverGamesSM()
{
	perf_free(_loop_perf);
	perf_free(_loop_interval_perf);
}

int HoverGamesSM::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
This module handles a state machine implementation for Hovergames2 contest.
The main purpose of the module is to send states depending on the global status
of the drone to handle an offboard camera running on Navq board with the purpose
of save energy and use camera resources for the best usage.

### Implementation
The module considers states where the camera should be running or not, depending
on the health of the drone like battery status, arming states, distance to ground.

)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("module", "hovergames");
	PRINT_MODULE_USAGE_COMMAND("start");
	return 0;
}

int hovergames_sm_main(int argc, char *argv[])
{
	return HoverGamesSM::main(argc, argv);
}
