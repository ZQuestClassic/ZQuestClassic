#include <sound/zcmixer.h>
#include <allegro5/allegro_audio.h>
#include <algorithm>
#include <iostream>
#include <base/zc_math.h>

#include "base/general.h"

namespace zcmixer {
	ZCMIXER::~ZCMIXER() {
		std::cout << "Exiting: " << *this << "\n";
		oldtrack.reset();
		newtrack.reset();
		current_track.reset();
		std::cout << "Exited: zcmixer  id: " << id << "\n";
		// newtrack is just zcmusic
	}

	[[nodiscard]] ZCMIXER_owner_ptr_t create() {
		auto ret = std::make_unique<ZCMIXER>();
		std::cout << "zcmixer created " << *ret << "\n";
		return ret;
	}

	void ZCMIXER::update(const int32_t basevol, const int32_t uservol, const bool oldscriptvol) {
		if (fadeinframes) {
			if (fadeindelay) {
				--fadeindelay;
				if (newtrack) {
					newtrack->play(zcmusic::volume_t{0});
				}
			} else {
				--fadeinframes;
				if (newtrack) {
					const int32_t pct = std::clamp(
						int32_t((uint64_t(fadeinframes) * 10000) / uint64_t(fadeinmaxframes)), 0, 10000);
					newtrack->fadevolume = 10000 - pct;
					int32_t temp_volume = basevol;
					if (!oldscriptvol)
						temp_volume = (basevol * uservol) / 10000 / 100;
					temp_volume = (temp_volume * newtrack->fadevolume) / 10000;
					newtrack->play(zcmusic::volume_t{temp_volume});
					if (fadeinframes == 0) {
						newtrack->fadevolume = 10000;
					}
				}
			}
		}

		if (fadeoutframes) {
			if (fadeoutdelay)
				--fadeoutdelay;
			else
				--fadeoutframes;

			if (oldtrack) {
				int32_t pct = 0;
				if (fadeoutframes > 0)
					pct = std::clamp(int32_t((uint64_t(fadeoutframes) * 10000) / uint64_t(fadeoutmaxframes)), 0, 10000);
				oldtrack->fadevolume = pct;
				int32_t temp_volume = basevol;
				if (!oldscriptvol)
					temp_volume = (basevol * uservol) / 10000 / 100;
				temp_volume = (temp_volume * oldtrack->fadevolume) / 10000;
				oldtrack->play(zcmusic::volume_t{temp_volume});
			}

			if (fadeoutframes == 0) {
				oldtrack.reset();
			}
		}
	}

	void update(ZCMIXER *const mix, const int32_t basevol, const int32_t uservol, const bool oldscriptvol) {
		if (mix) {
			mix->update(basevol, uservol, oldscriptvol);
		}
	}

	void exit(ZCMIXER_owner_ptr_t mix) {
		mix.reset();
	}


	void ZCMIXER::stop_and_unload_current_track() {
		if (!current_track)
			return;
		current_track.reset();
		newtrack.reset();
	}

	bool ZCMIXER::setup_transition(const int32_t fadeinframes, const int32_t fadeoutframes,
	                               const int32_t fademiddleframes) {
		double fadeoutpct = 1.0;
		// If there was an old fade going, use that as a multiplier for the new fade out
		if (newtrack) {
			fadeoutpct = static_cast<double>(newtrack->fadevolume) / 10000.0;
		}

		newtrack = std::move(current_track);
		this->fadeinframes = fadeinframes;
		this->fadeoutframes = zc_max(fadeoutframes * fadeoutpct, 1);
		fadeoutmaxframes = fadeoutframes;
		if (fademiddleframes < 0) {
			fadeindelay = 0;
			fadeoutdelay = -fademiddleframes;
		} else {
			fadeindelay = fademiddleframes;
			fadeoutdelay = 0;
		}
		return true;
	}

	std::ostream &operator<<(std::ostream &os, const zcmixer::ZCMIXER &mix) {
		os << "ZCMIXER("
				<< "\n\tid: " << mix.id;
		os << "\n\toldtrack: ";
		print(mix.oldtrack.get(), os);
		os << "\n\tnewtrack: ";
		print(mix.newtrack.get(), os);
		os << "\n\tcurrent_track: ";
		print(mix.newtrack.get(), os);
		return os << "\n)\n";
	}
} // namespace zcmixer
