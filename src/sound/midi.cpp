#include "midi.h"

#include <cstring>
#include <cstdio>

#include <base/zsys.h>

namespace midi
{
    namespace
    {
        struct mtrkevent
            // sort of... it's actually not a true MTrk event because it can be a
            // combination of events, such as a "note on" event plus a "note off" event
        {
            // basic mtrk event but with total time instead of delta-time
            byte* buf; // buffer for extra data
            dword t; // total time from start in delta-time units
            word dur; // duration of note
            word event; // event + extra byte of info
            word nbytes; // number of bytes in "buf" or note off velocity
            byte type; // meta type, note number, etc.
            byte byte2; // byte 2 or note on velocity
        };

        constexpr double _runtime(const dword beats, const double tempo)
        {
            if (tempo == 0)
                return 0;

            return beats / tempo * 60.0;
        }

        static_assert(_runtime(3, 0) == 0);
        static_assert(_runtime(3, 2) == 90);

        constexpr dword parse_var_len(byte*& data_row)
        {
            dword val = *data_row & 0x7F;

            while (*data_row & 0x80)
            {
                data_row++;
                val <<= 7;
                val += (*data_row & 0x7F);
            }

            data_row++;
            return val;
        }

        // returns length of the <MTrk event>
        constexpr dword parse_mtrk(byte* & data_row, midi_info& mi)
        // returns length of the <MTrk event>
        {
            const byte* const start = data_row;

            mi.dt = parse_var_len(data_row);
            mi.event = *data_row;
            data_row++;
            mi.type = 0;
            mi.byte2 = 0;
            mi.nbytes = 0;
            mi.buf = nullptr;

            if (mi.event < 0x80 && mi.running_status > 0)
            {
                mi.event = mi.running_status;
                data_row--;
            }

            if (mi.event != 0xFF && mi.event != 0xF0 && mi.event != 0xF7)
                mi.running_status = mi.event;

            switch (mi.event)
            {
            case 0xFF: // <meta-event>
                mi.type = *data_row;
                data_row++;

            case 0xF0:
            case 0xF7: // <sysex events>
                mi.nbytes = parse_var_len(data_row);
                mi.buf = data_row;
                data_row += mi.nbytes;
                break;

            case 0xF2:
                data_row += 2;
                break;

            case 0xF3:
                data_row++;
                break;

            default:
                {
                    switch (mi.event & 0xF0)
                    {
                    case 0x80:
                    case 0x90:
                    case 0xA0:
                    case 0xB0:
                    case 0xE0:
                        mi.type = *data_row;
                        data_row++;
                        mi.byte2 = *data_row;
                        data_row++;
                        break;

                    case 0xC0:
                    case 0xD0:
                        mi.type = *data_row;
                        data_row++;
                        break;
                    default:
                        break;
                    }
                }
            }

            return data_row - start;
        }

        constexpr dword beats(const dword dt, const dword divs)
        {
            if (divs <= 0)
                return 1;

            return dt / divs + ((dt % divs) ? 1 : 0); // NOLINT(*-narrowing-conversions)
        }

        constexpr bool eot(const midi_info& mi)
        {
            return (mi.event == 0xFF && mi.type == 0x2F);
        }

        namespace test_odt
        {
            constexpr midi_info make_midi_info(const byte event, const byte type)
            {
                midi_info ret;
                ret.event = event;
                ret.type = type;
                return ret;
            }

            static_assert(eot(make_midi_info(0xFF, 0x2F)) == true);
            static_assert(eot(make_midi_info(0xFF, 0x4)) == false);
            static_assert(eot(make_midi_info(0x43, 0x2F)) == false);
        }


        constexpr std::size_t runtime(const std::size_t beats, const midi_info* const mi)
        {
            const std::size_t c = [beats, mi]
            {
                if (mi->tempo_changes < 0) return static_cast<std::size_t>(0);

                for (std::size_t ret = 0; ret < mi->tempo_changes; ++ret)
                {
                    if (beats < mi->tempo_c[ret])
                        return ret;
                }
                return static_cast<std::size_t>(mi->tempo_changes);
            }();

            double t = 0;
            for (std::size_t i = 1; i < c; ++i)
                t += _runtime(mi->tempo_c[i] - mi->tempo_c[i - 1], mi->tempo[i - 1]);

            t += _runtime(beats - mi->tempo_c[c - 1], mi->tempo[c - 1]);

            return t;
        }

        constexpr bool decode_text_event(char* outputString, const std::size_t length, const byte type,
                                         const std::string_view buf)
        {
            switch (type)
            {
            case 0x01:
                snprintf(outputString, length, "T: %s\n", buf.data());
                return true;

            case 0x02:
                snprintf(outputString, length, "C: %s\n", buf.data());
                return true;

            case 0x03:
                snprintf(outputString, length, "N: %s\n", buf.data());
                return true;

            case 0x04:
                snprintf(outputString, length, "I: %s\n", buf.data());
                return true;

            default:
                return false;
            }
        }

        void get_midi_text(const MIDI* const midi, midi_info* const mi, char* text)
        // must have called get_midi_info() first
        {
            char* out = text;
            std::size_t length = 4096;

            text[0] = 0;

            for (const auto& track : midi->track)
            {
                for (int32_t i = 0; track.len > 0; ++i)
                {
                    byte* data = track.data;

                    if (data == nullptr)
                        break;

                    mi->event = 0;
                    mi->running_status = 0;

                    while (!eot(*mi) && data - track.data < track.len)
                    {
                        parse_mtrk(data, *mi);

                        if (mi->event == 0xFF)
                        {
                            char buf[1024];
                            const dword nbytes = zc_min(mi->nbytes, 1023);
                            memcpy(buf, mi->buf, nbytes);
                            buf[nbytes] = 0;

                            if (decode_text_event(out, length, mi->type, buf))
                            {
                                length -= strlen(out);
                                out += strlen(out);
                            }
                        }
                    }
                }
            }
        }

        constexpr dword getval_(const byte* const buf, const int32_t nbytes)
        {
            dword value = 0;

            for (int32_t i = 0; i < nbytes; i++)
            {
                value <<= 8;
                value += buf[i];
            }

            return value;
        }

        constexpr double get_tempo(const byte* const buf)
        {
            double t = getval_(buf, 3);
            t /= 60e6;

            if (t == 0)
                return 1;

            return 1 / t;
        }
    }

    /* save_midi:
      *  Saves a standard MIDI file, returning 0 on success,
      *  or non-zero on error.
      */

    int32_t save_midi(const std::string_view filename, const MIDI* const midi)
    {
        int32_t num_tracks = 0;

        if (!midi)
            return 1;

        PACKFILE* const fp = pack_fopen_password(filename.data(), F_WRITE, ""); /* open the file */

        if (!fp)
            return 2;

        for (const auto& c : midi->track)
            if (c.len > 0)
                num_tracks++;

        pack_fwrite((void*)"MThd", 4, fp); /* write midi header */

        pack_mputl(6, fp); /* header chunk length = 6 */

        pack_mputw((num_tracks == 1) ? 0 : 1, fp); /* MIDI file type */

        pack_mputw(num_tracks, fp); /* number of tracks */

        pack_mputw(midi->divisions, fp); /* beat divisions (negatives?) */

        for (int32_t c = 0; c < num_tracks; c++) /* write each track */
        {
            pack_fwrite((void*)"MTrk", 4, fp); /* write track header */

            const int32_t len = midi->track[c].len;
            pack_mputl(len, fp); /* length of track chunk */

            if (pack_fwrite(midi->track[c].data, len, fp) != len)
                goto err;
        }

        pack_fclose(fp);
        return 0;

        /* oh dear... */
    err:
        pack_fclose(fp);
        delete_file(filename.data());
        return 3;
    }

    /* ---  All this code just to calculate the length of a MIDI song.  --- */

    /*
        constexpr std::size_t MAX_TEMPO_CHANGES = 512;

        struct midi_info
        {
            // midi info
            int32_t format = 0;
            std::size_t num_tracks = 0;
            int32_t divisions = 0;
            int32_t len_beats = 0;
            double len_sec = 0;
            int32_t tempo_changes = 0;
            std::array<double, MAX_TEMPO_CHANGES> tempo = {}; // tempo can change during song
            std::array<dword, MAX_TEMPO_CHANGES> tempo_c = {}; // store the total delta time before each change in tempo

            // MTrk event info (used by parse_mtrk())
            dword dt{}, nbytes{};
            byte event{}, running_status{}, type{}, byte2{};
            byte* buf{};

            constexpr midi_info() = default;
            explicit midi_info(const MIDI* midi) noexcept;
        };

      */

    dword getval(const byte* const buf, const int32_t nbytes)
    {
        return getval_(buf, nbytes);
    }

    midi_info::midi_info(const MIDI* midi) noexcept
    {
        dword max_dt = 0;
        tempo_changes = 0;
        bool gottempo = false;

        if (midi == nullptr)
            goto done;

        for (int32_t i = 0; midi->track[i].len > 0; i++)
        {
            byte* data = midi->track[i].data;

            if (data == nullptr)
                break;

            dword total_dt = 0;
            event = 0;
            running_status = 0;
            // tempo info should only be in first track, but sometimes it isn't
            bool gettempo = (i == 0) || (!gottempo);

            while (!eot(*this) && data - midi->track[i].data < midi->track[i].len)
            {
                parse_mtrk(data, *this);
                total_dt += dt;

                if (gettempo && event == 0xFF && type == 0x51 && tempo_changes < MAX_TEMPO_CHANGES)
                {
                    tempo[tempo_changes] = get_tempo(buf);
                    const auto new_tempo_c = tempo_c[tempo_changes] = beats(total_dt, midi->divisions);

                    if (tempo_changes == 0 && tempo_c[0]) // make sure there is a tempo at beat 0
                    {
                        tempo_c[0] = 0;
                        tempo_c[1] = new_tempo_c;
                        tempo[1] = tempo[0];
                        tempo_changes++;
                    }

                    tempo_changes++;
                    gottempo = true;
                }
            }

            max_dt = zc_max(max_dt, total_dt);
        }

    done:

        if (tempo_changes == 0) // then guess
        {
            tempo_changes = 1;
            tempo[0] = 120.0;
            tempo_c[0] = 0;
        }

        len_beats = (midi == nullptr) ? 0 : beats(max_dt, midi->divisions);
        len_sec = (midi == nullptr) ? 0 : runtime(len_beats, this);
    }

    char* timestr(double sec)
    {
        static char buf[16];
        auto min = static_cast<int32_t>(sec / 60);
        sec -= min * 60;

        if (sec >= 59.5)
        {
            min++;
            sec = 0;
        }

        sprintf(buf, "%02d:%02.0f", min, sec);
        return buf;
    }
} // namespace midi
