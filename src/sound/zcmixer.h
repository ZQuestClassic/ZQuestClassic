#pragma once

#include <cstdint>
#include <memory>
#include <iostream>

#include <sound/zcmusic.h>

namespace zcmixer {
    class ZCMIXER {
        static inline std::atomic<int32_t> cnt = 0;
    public:

        int32_t id = cnt++;
        zcmusic::ZCMUSIC_owner_ptr_t oldtrack;
        zcmusic::ZCMUSIC_owner_ptr_t newtrack;
        zcmusic::ZCMUSIC_owner_ptr_t current_track;

        int32_t fadeinframes = 0;
        int32_t fadeinmaxframes = 0;
        int32_t fadeindelay = 0;

        int32_t fadeoutframes = 0;
        int32_t fadeoutmaxframes = 0;
        int32_t fadeoutdelay = 0;

    public :
        virtual ~ZCMIXER();

        void update(int32_t basevol, int32_t uservol, bool oldscriptvol);

        void stop_and_unload_current_track();

        bool setup_transition(int32_t fadeinframes, int32_t fadeoutframes, int32_t fademiddleframes);
    };

    using ZCMIXER_owner_ptr_t = std::unique_ptr<ZCMIXER>;

    std::ostream &operator<<(std::ostream &os, const zcmixer::ZCMIXER &mix);

    ZCMIXER_owner_ptr_t create();

    void update(ZCMIXER *mix, int32_t basevol, int32_t uservol, bool oldscriptvol);

    void exit(ZCMIXER_owner_ptr_t mix);
} // nameaspace zcmixer

inline zcmixer::ZCMIXER_owner_ptr_t g_zcmixer;
