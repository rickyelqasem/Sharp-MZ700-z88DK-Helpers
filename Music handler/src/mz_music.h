#ifndef MZ_MUSIC_H
#define MZ_MUSIC_H

#include "music_engine.h"

#define MZ_MUSIC_MAX_MELODY_NOTES 256u
#define MZ_MUSIC_MAX_NOTES_PER_BAR 16u

#define MZ_MUSIC_REST NOTE_REST
#define MZ_MUSIC_NOTE(octave, pitch) NOTE_MAKE((octave), (pitch))

#define MZ_PITCH_C NOTE_PITCH_C
#define MZ_PITCH_CS NOTE_PITCH_CS
#define MZ_PITCH_D NOTE_PITCH_D
#define MZ_PITCH_DS NOTE_PITCH_DS
#define MZ_PITCH_E NOTE_PITCH_E
#define MZ_PITCH_F NOTE_PITCH_F
#define MZ_PITCH_FS NOTE_PITCH_FS
#define MZ_PITCH_G NOTE_PITCH_G
#define MZ_PITCH_GS NOTE_PITCH_GS
#define MZ_PITCH_A NOTE_PITCH_A
#define MZ_PITCH_AS NOTE_PITCH_AS
#define MZ_PITCH_B NOTE_PITCH_B

#define MZ_MUSIC_PLAY_NOTES(bpm, notes_per_bar, loop, ...) \
    mz_music_play_melody( \
        (bpm), \
        (notes_per_bar), \
        (const note_id_t[]){ __VA_ARGS__ }, \
        (unsigned int)(sizeof((const note_id_t[]){ __VA_ARGS__ }) / sizeof(note_id_t)), \
        (loop))

enum mz_music_error {
    MZ_MUSIC_ERROR_NONE = 0,
    MZ_MUSIC_ERROR_BAD_NOTES_PER_BAR = 1,
    MZ_MUSIC_ERROR_TOO_MANY_NOTES = 2,
    MZ_MUSIC_ERROR_BAD_SONG = 3
};

struct mz_music_song {
    unsigned int bpm;
    const struct melody_step *steps;
    unsigned int step_count;
};

void mz_music_init(void);
void mz_music_shutdown(void);
void mz_music_update(void);
void mz_music_stop(void);
unsigned char mz_music_play_melody(
    unsigned int bpm,
    unsigned char notes_per_bar,
    const note_id_t *notes,
    unsigned int note_count,
    unsigned char loop);
unsigned char mz_music_play_song(const struct mz_music_song *song, unsigned char loop);
void mz_music_preview_note(note_id_t note);
void mz_music_preview_off(void);

unsigned char mz_music_is_playing(void);
unsigned char mz_music_get_mode(void);
note_id_t mz_music_get_current_note(void);
unsigned int mz_music_get_bpm(void);
unsigned int mz_music_get_step_index(void);
unsigned int mz_music_get_step_count(void);
unsigned char mz_music_get_last_notes_per_bar(void);
unsigned char mz_music_get_last_error(void);
const char *mz_music_note_name(note_id_t note);

#endif
