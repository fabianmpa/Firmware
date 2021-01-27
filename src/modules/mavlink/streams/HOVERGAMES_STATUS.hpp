#ifndef HOVERGAMES_STATUS_HPP
#define HOVERGAMES_STATUS_HPP

#include <uORB/SubscriptionMultiArray.hpp>
#include <uORB/topics/hovergames.h>


class MavlinkStreamHoverGames : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink)
	{
		 return new MavlinkStreamHoverGames(mavlink);
	}

	static constexpr const char *get_name_static()
	{
		 return "HOVERGAMES_STATUS";
	 }
	static constexpr uint16_t get_id_static()
	{
		 return MAVLINK_MSG_ID_HOVER_GAMES_STATUS;
	 }

	const char *get_name() const override
	{
		return MavlinkStreamHoverGames::get_name_static();
	}
	uint16_t get_id() override
	{
		 return get_id_static();
	}

	unsigned get_size() override
	{
		return MAVLINK_MSG_ID_HOVER_GAMES_STATUS + MAVLINK_NUM_NON_PAYLOAD_BYTES;
	}

private:
	MavlinkStreamHoverGames(MavlinkStreamHoverGames &);
	MavlinkStreamHoverGames& operator = (const MavlinkStreamHoverGames &);

	uORB::Subscription _hovergames_subs{ORB_ID::hovergames};

protected:
	explicit MavlinkStreamHoverGames(Mavlink *mavlink) : MavlinkStream(mavlink)
	{}

	bool send() override
	{
		hovergames_s _hovergames_s;
		if (_hovergames_subs.update(&_hovergames_s))
		{
			mavlink_hover_games_status_t msg{};
			msg.time_boot_ms = _hovergames_s.timestamp / 1000; /* us to ms */
			msg.HoverGames_ActiveSM = _hovergames_s.hovergames_activesm;
			msg.HoverGames_SM = _hovergames_s.hovergames_sm;
			mavlink_msg_hover_games_status_send_struct(_mavlink->get_channel(), &msg);
			return true;
		}
	return false;
	}
};
#endif
