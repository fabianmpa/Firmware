
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/getopt.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <drivers/drv_hrt.h>
#include <systemlib/mavlink_log.h>
#include <uORB/Publication.hpp>
#include <uORB/SubscriptionInterval.hpp>
#include <uORB/topics/battery_status.h>
#include <uORB/topics/distance_sensor.h>
#include <uORB/topics/actuator_armed.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/hovergames.h>

#define HOVERGAMES_STANDBY_STATE        0U
#define HOVERGAMES_PREACTIVE_STATE      1U
#define HOVERGAMES_ACTIVE_STATE         2U
#define HOVERGAMES_FAULTED_STATE        3U
#define HOVERGAMES_RECORDING_STATE      0U
#define HOVERGAMES_IMAGE_CAPTURE_STATE  1U
#define HOVERGAMES_PAUSED_STATE         2U

static const uint8_t minimumVoltageFiltered = 12.0F;
static const uint8_t minimumBatteryRemaining= 0.2F;
static const uint16_t fiveMeters            = 500U;
static const uint16_t twentyMeters          = 2000U;
static const uint16_t thirtyFourMeters      = 3400U;

using namespace time_literals;

extern "C" __EXPORT int hovergames_sm_main(int argc, char *argv[]);

typedef struct
{
   bool engineFailure  = 1U;
   bool failSafe       = 1U;
   bool missionFailure = 1U;
}failureValues;

typedef struct
{
   bool armed          = 1U;
   bool notArmed       = 0U;
   bool readyToArm     = 1U;
   bool notReadyToArm  = 0U;
}armingValues;

class HoverGamesSM : public ModuleBase<HoverGamesSM>, public ModuleParams,px4::ScheduledWorkItem
{
public:
	HoverGamesSM();
	~HoverGamesSM();
	static int task_spawn(int argc, char *argv[]);
	static int print_usage(const char *reason = nullptr);
	static int custom_command(int argc, char *argv[]);
	bool init();
	int print_status() override;

private:
	void parameters_update(void);
	void Run() override;
	/* Subscriptions */
	uORB::Subscription _distance_sensor_sub{ORB_ID(distance_sensor)};
	uORB::Subscription _battery_status_sub{ORB_ID(battery_status)  };
	uORB::Subscription _vehicle_status_sub{ORB_ID(vehicle_status)  };
	uORB::Subscription _actuator_armed_sub{ORB_ID(actuator_armed)  };
	battery_status_s  _battery;
	distance_sensor_s _distance;
	actuator_armed_s  _actuator;
	vehicle_status_s  _vehStatus;
	/* hovergames structure */
	hovergames_s _hovergames;
	/* hovergames advertiser */
	orb_advert_t  hovergamesAdv = orb_advertise(ORB_ID(hovergames), &_hovergames);
	/* failure and arming structures*/
	failureValues  _hovergamesFailures;
	armingValues   _hovergamesArmingStates;

	perf_counter_t	_loop_perf{perf_alloc(PC_ELAPSED, MODULE_NAME)};
	perf_counter_t	_loop_interval_perf{perf_alloc(PC_INTERVAL, MODULE_NAME)};
};

