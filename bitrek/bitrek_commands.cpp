#include "bitrek_commands.h"
#include <pokrpak/pack.h>

namespace VisiPlug {
    namespace Bitrek {

	// ---------- version_t --------------------------------------------------

	version_t::version_t() {
	}

	void* version_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(version);

	    return d;
	}

	bool version_t::unpack(const void *d) {
	    return PKR_UNPACK(version);
	}

	bool version_t:: operator == (const version_t& rhs) const {
	    return version == rhs.version;
	}

	// ---------- track_t --------------------------------------------------

	track_t::track_t() {
	    valid = false;
	    satellites = 0;
	    lat = lon = 0.0;
	    height = 0;
	    speed = 0;
	    direction = 0;
	    utc_time = 0.0;
	}

	void* track_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(valid);
	    PKR_ADD_ITEM(satellites);
	    PKR_ADD_ITEM(lat);
	    PKR_ADD_ITEM(lon);
	    PKR_ADD_ITEM(height);
	    PKR_ADD_ITEM(speed);
	    PKR_ADD_ITEM(direction);
	    PKR_ADD_ITEM(utc_time);

	    return d;
	}

	bool track_t::unpack(const void *d) {
	    return PKR_UNPACK(valid) && PKR_UNPACK(satellites) && PKR_UNPACK
		(lat) && PKR_UNPACK(lon) && PKR_UNPACK(height) && PKR_UNPACK
		(speed) && PKR_UNPACK(direction) && PKR_UNPACK(utc_time);
	}

	bool track_t:: operator == (const track_t& rhs) const {
	    return valid == rhs.valid && satellites == rhs.satellites && lat ==
		rhs.lat && lon == rhs.lon && height == rhs.height && speed ==
		rhs.speed && direction == rhs.direction && utc_time ==
		rhs.utc_time;
	}

	// ---------- delgps_t --------------------------------------------------

	delgps_t::delgps_t() {
	    success = false;
	}

	void* delgps_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(success);

	    return d;
	}

	bool delgps_t::unpack(const void *d) {
	    return PKR_UNPACK(success);
	}

	bool delgps_t:: operator == (const delgps_t& rhs) const {
	    return success == rhs.success;
	}

	// ---------- io_t --------------------------------------------------

	io_t::io_t() {
	    d_inputs = 0;
	    d_outputs = 0;
	    analog1 = analog2 = 0.0;
	    power = battary_power = 0.0;
	}

	void* io_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(d_inputs);
	    PKR_ADD_ITEM(d_outputs);
	    PKR_ADD_ITEM(analog1);
	    PKR_ADD_ITEM(analog2);
	    PKR_ADD_ITEM(power);
	    PKR_ADD_ITEM(battary_power);

	    return d;
	}

	bool io_t::unpack(const void *d) {
	    return PKR_UNPACK(d_inputs) && PKR_UNPACK(d_outputs) && PKR_UNPACK
		(analog1) && PKR_UNPACK(analog2) && PKR_UNPACK(power)
		&& PKR_UNPACK(battary_power);
	}

	bool io_t:: operator == (const io_t& rhs) const {
	    return d_outputs == rhs.d_outputs && d_inputs == rhs.d_inputs &&
		analog1 == rhs.analog1 && analog2 == rhs.analog2 && power ==
		rhs.power && battary_power == rhs.battary_power;
	}

	// ---------- setouts_t --------------------------------------------------

	setouts_t::setouts_t() {
	    d_outputs = 0;
	    d_outputs_values = 0;
	}

	void* setouts_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(d_outputs);
	    PKR_ADD_ITEM(d_outputs_values);

	    return d;
	}

	bool setouts_t::unpack(const void *d) {
	    return PKR_UNPACK(d_outputs) && PKR_UNPACK(d_outputs_values);
	}

	bool setouts_t:: operator == (const setouts_t& rhs) const {
	    return d_outputs == rhs.d_outputs && d_outputs_values ==
		rhs.d_outputs_values;
	}

	// ---------- fwupdate_t --------------------------------------------------

	fwupdate_t::fwupdate_t() {
	}

	void* fwupdate_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(str);

	    return d;
	}

	bool fwupdate_t::unpack(const void *d) {
	    return PKR_UNPACK(str);
	}

	bool fwupdate_t:: operator == (const fwupdate_t& rhs) const {
	    return str == rhs.str;
	}

	// ---------- params_t --------------------------------------------------

	enum UsAPNTable {
	    usAPNMin = 60, usAPNMax = 99
	};

	enum UsLoginTable {
	    usLoginMin = 100, usLoginMax = 139
	};

	enum UsPassTable {
	    usPassMin = 100, usPassMax = 179
	};

	enum UsipTable {
	    usipMin = 20, usipMax = 59
	};

	int getTypeForParam(long id) {
	    static const long data4[] = {
		3326, 3327, 3328, 3329, 3330, 3331, 3332, 3333, 3334, 3825, 950,
		951,
		952, 953, 954, 445, 505, 715, 465, 185, 195, 685, 835, 325,
		3815, 5005, 0};
	    static const long strs[] = {
		252, 253, 261, 262, 263, 264, 265, 266, 267, 268, 269, 245, 242,
		1242, 243, 1243, 244, 1244, 910, 20, 188, 0};
	    static const long data2[] = {
		246, 189, 903, 1903, 11, 1011, 12, 1012, 13, 1013, 197, 198,
		199, 208, 206, 201, 270, 1270, 232,
		1232, 284, 1284, 285, 1285, 905, 1905, 906, 1906, 345, 355, 365,
		375, 545, 555, 475, 485, 515, 305, 315, 425, 415, 495, 805,
		585, 595, 405, 635, 645, 655, 665, 675,
		695, 705, 535, 855, 865, 875, 885, 395,
		755, 765, 775, 785, 795, 625, 3805, 3835,
		3625, 3635, 3645, 3655, 3665, 3675, 3685, 3695, 3705, 326, 327,
		328, 329, 330, 331, 332, 333, 334, 342, 343, 352, 353, 362, 363,
		372, 373, 542, 543, 552, 553, 562, 563, 572, 573, 472, 473, 482,
		483, 512, 513, 302, 303, 312, 313, 422, 423, 412, 413, 492, 493,
		452, 453, 442, 443, 502, 503, 712, 713, 462, 463, 582, 583, 592,
		593, 402, 403, 602, 603, 612, 613, 632, 633, 642, 643, 652, 653,
		662, 663, 672, 673, 682, 683, 692, 693, 702, 703, 522, 523, 532,
		533, 852, 853, 862, 863, 872, 873, 882, 883, 892, 893, 392, 393,
		182, 183, 192, 193, 832, 833, 842, 843, 752, 753, 762, 763, 772,
		773, 782, 783, 792, 793, 622, 623, 907, 816, 817, 3622, 3623,
		3632, 3633, 3642,
		3643, 3652, 3653, 3662, 3663, 3672, 3673, 3682, 3683, 3692,
		3693, 3702, 4007, 4008, 4019, 4020, 5045, 3703, 565, 575, 959,
		980,
		475, 485, 305,
		315, 895, 845, 0};

	    if (id >= usAPNMin && id <= usAPNMax ||
		id >= usLoginMin && id <= usLoginMax ||
		id >= usPassMin && id <= usPassMax ||
		id >= usipMin && id <= usipMax
		)
		return 3;

	    int i = 0;
	    while (strs[i] != 0) {
		if (strs[i] == id)
		    return 3;
		++i;
	    }

	    i = 0;
	    while (data2[i] != 0) {
		if (data2[i] == id)
		    return 2;
		++i;
	    }

	    i = 0;
	    while (data4[i] != 0) {
		if (data4[i] == id)
		    return 4;
		++i;
	    }

	    return 1;
	}

	params_t::params_t() {
	    set = false;
	}

	void* params_t::pack() const {
	    void* d = pkr_create_struct();
	    PKR_ADD_ITEM(set);
	    PKR_ADD_ITEM(ids);
	    PKR_ADD_ITEM(types);
	    PKR_ADD_ITEM(data1);
	    PKR_ADD_ITEM(data2);
	    PKR_ADD_ITEM(strs);
	    PKR_ADD_ITEM(data4);

	    return d;
	}

	bool params_t::unpack(const void *d) {
	    return PKR_UNPACK(set) && PKR_UNPACK(ids) && PKR_UNPACK(types)
		&& PKR_UNPACK(data1) && PKR_UNPACK(data2) && PKR_UNPACK(strs)
		&& PKR_UNPACK(data4);
	}

	bool params_t:: operator == (const params_t& rhs) const {
	    return set == rhs.set && ids == rhs.ids && types == rhs.types &&
		data1 == rhs.data1 && data2 == rhs.data2 && strs == rhs.strs &&
		data4 == rhs.data4;
	}

    }
}
