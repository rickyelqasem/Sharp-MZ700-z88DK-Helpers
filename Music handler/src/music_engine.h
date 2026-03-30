#ifndef MUSIC_ENGINE_H
#define MUSIC_ENGINE_H

typedef signed char note_id_t;

#define NOTES_PER_OCTAVE 12
#define NOTE_OCTAVE_MIN 1
#define NOTE_OCTAVE_MAX 7
#define NOTE_COUNT ((NOTE_OCTAVE_MAX - NOTE_OCTAVE_MIN + 1) * NOTES_PER_OCTAVE)
#define NOTE_REST ((note_id_t)-1)

enum note_pitch_class {
    NOTE_PITCH_C = 0,
    NOTE_PITCH_CS = 1,
    NOTE_PITCH_D = 2,
    NOTE_PITCH_DS = 3,
    NOTE_PITCH_E = 4,
    NOTE_PITCH_F = 5,
    NOTE_PITCH_FS = 6,
    NOTE_PITCH_G = 7,
    NOTE_PITCH_GS = 8,
    NOTE_PITCH_A = 9,
    NOTE_PITCH_AS = 10,
    NOTE_PITCH_B = 11
};

#define NOTE_MAKE(octave, pitch) \
    ((note_id_t)((((octave) - NOTE_OCTAVE_MIN) * NOTES_PER_OCTAVE) + (pitch)))

#define NOTE_C4 NOTE_MAKE(4, NOTE_PITCH_C)
#define NOTE_CS4 NOTE_MAKE(4, NOTE_PITCH_CS)
#define NOTE_D4 NOTE_MAKE(4, NOTE_PITCH_D)
#define NOTE_DS4 NOTE_MAKE(4, NOTE_PITCH_DS)
#define NOTE_E4 NOTE_MAKE(4, NOTE_PITCH_E)
#define NOTE_F4 NOTE_MAKE(4, NOTE_PITCH_F)
#define NOTE_FS4 NOTE_MAKE(4, NOTE_PITCH_FS)
#define NOTE_G4 NOTE_MAKE(4, NOTE_PITCH_G)
#define NOTE_GS4 NOTE_MAKE(4, NOTE_PITCH_GS)
#define NOTE_A4 NOTE_MAKE(4, NOTE_PITCH_A)
#define NOTE_AS4 NOTE_MAKE(4, NOTE_PITCH_AS)
#define NOTE_B4 NOTE_MAKE(4, NOTE_PITCH_B)

enum music_mode {
    MUSIC_MODE_IDLE = 0,
    MUSIC_MODE_PREVIEW = 1,
    MUSIC_MODE_MELODY = 2
};

struct melody_step {
    note_id_t note;
    unsigned char length_16ths;
};

void music_init(void);
void music_shutdown(void);
void music_set_bpm(unsigned int bpm);
void music_set_melody(const struct melody_step *steps, unsigned int count, unsigned char loop);
void music_start_melody(void);
void music_stop(void);
void music_preview_note(note_id_t note);
void music_preview_off(void);
void music_tick(void);
unsigned char music_read_tempo_bit(void);
note_id_t music_make_note(unsigned char octave, unsigned char pitch_class);

unsigned char music_get_mode(void);
note_id_t music_get_current_note(void);
unsigned int music_get_bpm(void);
unsigned int music_get_step_index(void);
unsigned int music_get_step_count(void);
const char *music_note_name(note_id_t note);

#endif
