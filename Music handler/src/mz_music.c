#include "mz_music.h"

static unsigned char tempo_prev = 0;
static unsigned char last_notes_per_bar = 0;
static unsigned char last_error = MZ_MUSIC_ERROR_NONE;
static struct melody_step melody_buffer[MZ_MUSIC_MAX_MELODY_NOTES];

static unsigned char tempo_rising_edge(void)
{
    unsigned char current;
    unsigned char edge;

    current = music_read_tempo_bit();
    edge = (unsigned char)(current != 0 && tempo_prev == 0);
    tempo_prev = current;

    return edge;
}

void mz_music_init(void)
{
    music_init();
    tempo_prev = music_read_tempo_bit();
    last_notes_per_bar = 0;
    last_error = MZ_MUSIC_ERROR_NONE;
}

void mz_music_shutdown(void)
{
    music_shutdown();
}

void mz_music_update(void)
{
    if (tempo_rising_edge()) {
        music_tick();
    }
}

void mz_music_stop(void)
{
    music_stop();
}

unsigned char mz_music_play_melody(
    unsigned int bpm,
    unsigned char notes_per_bar,
    const note_id_t *notes,
    unsigned int note_count,
    unsigned char loop)
{
    unsigned int i;
    unsigned char remainder;
    unsigned char length_16ths;

    if (notes == 0 || note_count == 0) {
        last_error = MZ_MUSIC_ERROR_BAD_SONG;
        return 0;
    }

    if (notes_per_bar == 0 || notes_per_bar > MZ_MUSIC_MAX_NOTES_PER_BAR) {
        last_error = MZ_MUSIC_ERROR_BAD_NOTES_PER_BAR;
        return 0;
    }

    if (note_count > MZ_MUSIC_MAX_MELODY_NOTES) {
        last_error = MZ_MUSIC_ERROR_TOO_MANY_NOTES;
        return 0;
    }

    remainder = 0;
    for (i = 0; i != note_count; ++i) {
        remainder = (unsigned char)(remainder + 16u);
        length_16ths = (unsigned char)(remainder / notes_per_bar);
        remainder = (unsigned char)(remainder % notes_per_bar);

        melody_buffer[i].note = notes[i];
        melody_buffer[i].length_16ths = length_16ths;
    }

    music_set_bpm(bpm);
    music_set_melody(melody_buffer, note_count, loop);
    music_start_melody();

    last_notes_per_bar = notes_per_bar;
    last_error = MZ_MUSIC_ERROR_NONE;

    return 1;
}

unsigned char mz_music_play_song(const struct mz_music_song *song, unsigned char loop)
{
    if (song == 0 || song->steps == 0 || song->step_count == 0) {
        last_error = MZ_MUSIC_ERROR_BAD_SONG;
        return 0;
    }

    music_set_bpm(song->bpm);
    music_set_melody(song->steps, song->step_count, loop);
    music_start_melody();

    last_notes_per_bar = 0;
    last_error = MZ_MUSIC_ERROR_NONE;

    return 1;
}

void mz_music_preview_note(note_id_t note)
{
    music_preview_note(note);
}

void mz_music_preview_off(void)
{
    music_preview_off();
}

unsigned char mz_music_is_playing(void)
{
    return (unsigned char)(music_get_mode() == MUSIC_MODE_MELODY);
}

unsigned char mz_music_get_mode(void)
{
    return music_get_mode();
}

note_id_t mz_music_get_current_note(void)
{
    return music_get_current_note();
}

unsigned int mz_music_get_bpm(void)
{
    return music_get_bpm();
}

unsigned int mz_music_get_step_index(void)
{
    return music_get_step_index();
}

unsigned int mz_music_get_step_count(void)
{
    return music_get_step_count();
}

unsigned char mz_music_get_last_notes_per_bar(void)
{
    return last_notes_per_bar;
}

unsigned char mz_music_get_last_error(void)
{
    return last_error;
}

const char *mz_music_note_name(note_id_t note)
{
    return music_note_name(note);
}
