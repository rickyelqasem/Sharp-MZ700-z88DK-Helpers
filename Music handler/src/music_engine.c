#include "music_engine.h"

#define IO_BANKH_VRAM_MMIO 0xe3

#define MMIO_8253_CT0 0xe004
#define MMIO_8253_CTRL 0xe007
#define MMIO_ETC 0xe008

#define MMIO_ETC_GATE_MASK 0x01
#define MMIO_8253_CT0_MODE3 0x36

static const unsigned int note_ratios[] = {
    4238,
    4000,
    3776,
    3564,
    3364,
    3175,
    2997,
    2829,
    2670,
    2520,
    2379,
    2245
};

static const char *const pitch_class_names[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
};

static struct {
    const struct melody_step *steps;
    unsigned int step_count;
    unsigned int current_step;
    unsigned char loop_enabled;
    unsigned char mode;
    note_id_t current_note;
    unsigned int bpm;
    unsigned int tempo_accumulator;
    unsigned int length_remaining_16ths;
} music_state;

static void sound_hw_init(void) __naked
{
__asm
    ld      c, 0 + IO_BANKH_VRAM_MMIO
    out     (c), a

    ld      a, 0 + MMIO_8253_CT0_MODE3
    ld      (MMIO_8253_CTRL), a

    xor     a
    ld      (MMIO_ETC), a

    ret
__endasm;
}

static void sound_hw_set_ratio(unsigned int ratio) __z88dk_fastcall __naked
{
__asm
    ld      d, h
    ld      e, l

    ld      c, 0 + IO_BANKH_VRAM_MMIO
    out     (c), a

    ld      a, d
    or      e
    jr      nz, SOUND_HW_SET_RATIO

    ld      a, 0 + MMIO_8253_CT0_MODE3
    ld      (MMIO_8253_CTRL), a
    xor     a
    ld      (MMIO_ETC), a
    jr      SOUND_HW_SET_END

SOUND_HW_SET_RATIO:
    ld      hl, 0 + MMIO_8253_CT0
    ld      (hl), e
    ld      (hl), d
    ld      a, 0 + MMIO_ETC_GATE_MASK
    ld      (MMIO_ETC), a

SOUND_HW_SET_END:
    ret
__endasm;
}

static unsigned int note_ratio_for(note_id_t note)
{
    unsigned char note_index;
    unsigned char pitch_class;
    unsigned char octave;
    unsigned char shift;
    unsigned int ratio;

    note_index = (unsigned char)note;
    pitch_class = (unsigned char)(note_index % NOTES_PER_OCTAVE);
    octave = (unsigned char)(note_index / NOTES_PER_OCTAVE) + NOTE_OCTAVE_MIN;
    ratio = note_ratios[pitch_class];

    if (octave < 4) {
        shift = (unsigned char)(4 - octave);
        ratio <<= shift;
    } else if (octave > 4) {
        shift = (unsigned char)(octave - 4);
        ratio = (ratio + (1u << (shift - 1))) >> shift;
        if (ratio == 0) {
            ratio = 1;
        }
    }

    return ratio;
}

unsigned char music_read_tempo_bit(void) __naked
{
__asm
    ld      c, 0 + IO_BANKH_VRAM_MMIO
    out     (c), a

    ld      a, (MMIO_ETC)
    and     a, 0 + MMIO_ETC_GATE_MASK
    ld      l, 0
    jr      nz, MUSIC_READ_TEMPO_BIT_END
    inc     l

MUSIC_READ_TEMPO_BIT_END:
    ld      h, 0
    ret
__endasm;
}

static void speaker_stop(void)
{
    sound_hw_set_ratio(0);
}

static void speaker_start_ratio(unsigned int ratio)
{
    sound_hw_set_ratio(ratio);
}

static void speaker_play_note(note_id_t note)
{
    if (note < 0 || note >= NOTE_COUNT) {
        speaker_stop();
        music_state.current_note = NOTE_REST;
        return;
    }

    speaker_start_ratio(note_ratio_for(note));
    music_state.current_note = note;
}

static void music_load_step(void)
{
    const struct melody_step *step;

    if (music_state.step_count == 0) {
        music_stop();
        return;
    }

    if (music_state.current_step >= music_state.step_count) {
        if (music_state.loop_enabled) {
            music_state.current_step = 0;
        } else {
            music_stop();
            return;
        }
    }

    step = &music_state.steps[music_state.current_step];

    music_state.length_remaining_16ths = step->length_16ths;
    if (music_state.length_remaining_16ths == 0) {
        music_state.length_remaining_16ths = 1;
    }

    if (step->note == NOTE_REST) {
        speaker_stop();
        music_state.current_note = NOTE_REST;
    } else {
        speaker_play_note(step->note);
    }
}

void music_init(void)
{
    music_state.steps = 0;
    music_state.step_count = 0;
    music_state.current_step = 0;
    music_state.loop_enabled = 0;
    music_state.mode = MUSIC_MODE_IDLE;
    music_state.current_note = NOTE_REST;
    music_state.bpm = 120;
    music_state.tempo_accumulator = 0;
    music_state.length_remaining_16ths = 0;

    sound_hw_init();
    speaker_stop();
}

void music_shutdown(void)
{
    speaker_stop();
}

void music_set_bpm(unsigned int bpm)
{
    if (bpm == 0) {
        bpm = 120;
    }

    music_state.bpm = bpm;
}

void music_set_melody(const struct melody_step *steps, unsigned int count, unsigned char loop)
{
    music_state.steps = steps;
    music_state.step_count = count;
    music_state.loop_enabled = loop;
    music_state.current_step = 0;
    music_state.tempo_accumulator = 0;
    music_state.length_remaining_16ths = 0;
}

void music_start_melody(void)
{
    if (music_state.steps == 0 || music_state.step_count == 0) {
        return;
    }

    music_state.mode = MUSIC_MODE_MELODY;
    music_state.current_step = 0;
    music_state.tempo_accumulator = 0;
    music_load_step();
}

void music_stop(void)
{
    music_state.mode = MUSIC_MODE_IDLE;
    music_state.tempo_accumulator = 0;
    music_state.length_remaining_16ths = 0;
    speaker_stop();
    music_state.current_note = NOTE_REST;
}

void music_preview_note(note_id_t note)
{
    music_state.mode = MUSIC_MODE_PREVIEW;
    music_state.length_remaining_16ths = 0;
    speaker_play_note(note);
}

void music_preview_off(void)
{
    if (music_state.mode == MUSIC_MODE_PREVIEW) {
        music_stop();
    }
}

void music_tick(void)
{
    if (music_state.mode != MUSIC_MODE_MELODY) {
        return;
    }

    music_state.tempo_accumulator += music_state.bpm;

    while (music_state.tempo_accumulator >= 480u) {
        music_state.tempo_accumulator -= 480u;

        if (music_state.length_remaining_16ths != 0) {
            --music_state.length_remaining_16ths;
        }

        if (music_state.length_remaining_16ths == 0) {
            ++music_state.current_step;
            music_load_step();
            if (music_state.mode != MUSIC_MODE_MELODY) {
                return;
            }
        }
    }
}

unsigned char music_get_mode(void)
{
    return music_state.mode;
}

note_id_t music_get_current_note(void)
{
    return music_state.current_note;
}

unsigned int music_get_bpm(void)
{
    return music_state.bpm;
}

unsigned int music_get_step_index(void)
{
    return music_state.current_step;
}

unsigned int music_get_step_count(void)
{
    return music_state.step_count;
}

note_id_t music_make_note(unsigned char octave, unsigned char pitch_class)
{
    if (octave < NOTE_OCTAVE_MIN || octave > NOTE_OCTAVE_MAX || pitch_class >= NOTES_PER_OCTAVE) {
        return NOTE_REST;
    }

    return NOTE_MAKE(octave, pitch_class);
}

const char *music_note_name(note_id_t note)
{
    static char note_name[5];
    const char *pitch_class_name;
    unsigned char note_index;
    unsigned char octave;

    if (note < 0 || note >= NOTE_COUNT) {
        return "REST";
    }

    note_index = (unsigned char)note;
    pitch_class_name = pitch_class_names[note_index % NOTES_PER_OCTAVE];
    octave = (unsigned char)(note_index / NOTES_PER_OCTAVE) + NOTE_OCTAVE_MIN;

    note_name[0] = pitch_class_name[0];
    if (pitch_class_name[1] == 0) {
        note_name[1] = (char)('0' + octave);
        note_name[2] = 0;
    } else {
        note_name[1] = pitch_class_name[1];
        note_name[2] = (char)('0' + octave);
        note_name[3] = 0;
    }

    return note_name;
}
