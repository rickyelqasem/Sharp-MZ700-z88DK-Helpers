#include <conio.h>
#include <stdio.h>

#include "generated_song.h"
#include "mz_music.h"

#pragma printf = "%c %s %u"

#define MMIO_8255_PORTA ((volatile unsigned char *)0xE000)
#define MMIO_8255_PORTB ((volatile unsigned char *)0xE001)

#define MZ_KEY_1 0x31
#define MZ_KEY_2 0x32
#define MZ_KEY_3 0x33
#define MZ_KEY_4 0x34
#define MZ_KEY_5 0x35
#define MZ_KEY_6 0x36
#define MZ_KEY_7 0x37
#define MZ_KEY_A 0x01
#define MZ_KEY_B 0x02
#define MZ_KEY_C 0x03
#define MZ_KEY_D 0x04
#define MZ_KEY_E 0x05
#define MZ_KEY_F 0x06
#define MZ_KEY_G 0x07
#define MZ_KEY_M 0x0d
#define MZ_KEY_Q 0x11
#define MZ_KEY_S 0x13

#define MZ_KEY_SHARP_OFFSET 0x80

#define MZ_KEY_STROBE_2 0xf2
#define MZ_KEY_STROBE_3 0xf3
#define MZ_KEY_STROBE_4 0xf4
#define MZ_KEY_STROBE_5 0xf5
#define MZ_KEY_STROBE_SHIFT_BREAK 0xf8

#define KEY2_Q_MASK 0x80
#define KEY2_S_MASK 0x20

#define KEY3_M_MASK 0x08

#define KEY4_A_MASK 0x80
#define KEY4_B_MASK 0x40
#define KEY4_C_MASK 0x20
#define KEY4_D_MASK 0x10
#define KEY4_E_MASK 0x08
#define KEY4_F_MASK 0x04
#define KEY4_G_MASK 0x02

#define KEY5_1_MASK 0x80
#define KEY5_2_MASK 0x40
#define KEY5_3_MASK 0x20
#define KEY5_4_MASK 0x10
#define KEY5_5_MASK 0x08
#define KEY5_6_MASK 0x04
#define KEY5_7_MASK 0x02

#define KEY_SHIFT_MASK 0x01

static unsigned int ui_ticks = 0;
static unsigned char selected_octave = 4;
static unsigned char ui_dirty = 1;
static char status_text[40] = "PRESS M FOR MIDI. 1-7 PICKS OCTAVE.";

static unsigned char mz_scan_row(unsigned char strobe)
{
    *MMIO_8255_PORTA = strobe;
    return *MMIO_8255_PORTB;
}

static unsigned int scan_key_state(void)
{
    unsigned char row2;
    unsigned char row3;
    unsigned char row4;
    unsigned char row5;
    unsigned char row_shift;
    unsigned char shifted;

    row2 = mz_scan_row(MZ_KEY_STROBE_2);
    row3 = mz_scan_row(MZ_KEY_STROBE_3);
    row4 = mz_scan_row(MZ_KEY_STROBE_4);
    row5 = mz_scan_row(MZ_KEY_STROBE_5);
    row_shift = mz_scan_row(MZ_KEY_STROBE_SHIFT_BREAK);

    shifted = (unsigned char)((row_shift & KEY_SHIFT_MASK) == 0);

    if ((row4 & KEY4_C_MASK) == 0) {
        return shifted ? (MZ_KEY_C + MZ_KEY_SHARP_OFFSET) : MZ_KEY_C;
    }
    if ((row4 & KEY4_D_MASK) == 0) {
        return shifted ? (MZ_KEY_D + MZ_KEY_SHARP_OFFSET) : MZ_KEY_D;
    }
    if ((row4 & KEY4_E_MASK) == 0) {
        return shifted ? (MZ_KEY_E + MZ_KEY_SHARP_OFFSET) : MZ_KEY_E;
    }
    if ((row4 & KEY4_F_MASK) == 0) {
        return shifted ? (MZ_KEY_F + MZ_KEY_SHARP_OFFSET) : MZ_KEY_F;
    }
    if ((row4 & KEY4_G_MASK) == 0) {
        return shifted ? (MZ_KEY_G + MZ_KEY_SHARP_OFFSET) : MZ_KEY_G;
    }
    if ((row4 & KEY4_A_MASK) == 0) {
        return shifted ? (MZ_KEY_A + MZ_KEY_SHARP_OFFSET) : MZ_KEY_A;
    }
    if ((row4 & KEY4_B_MASK) == 0) {
        return shifted ? (MZ_KEY_B + MZ_KEY_SHARP_OFFSET) : MZ_KEY_B;
    }

    if ((row5 & KEY5_1_MASK) == 0) {
        return MZ_KEY_1;
    }
    if ((row5 & KEY5_2_MASK) == 0) {
        return MZ_KEY_2;
    }
    if ((row5 & KEY5_3_MASK) == 0) {
        return MZ_KEY_3;
    }
    if ((row5 & KEY5_4_MASK) == 0) {
        return MZ_KEY_4;
    }
    if ((row5 & KEY5_5_MASK) == 0) {
        return MZ_KEY_5;
    }
    if ((row5 & KEY5_6_MASK) == 0) {
        return MZ_KEY_6;
    }
    if ((row5 & KEY5_7_MASK) == 0) {
        return MZ_KEY_7;
    }

    if ((row3 & KEY3_M_MASK) == 0) {
        return MZ_KEY_M;
    }
    if ((row2 & KEY2_S_MASK) == 0) {
        return MZ_KEY_S;
    }
    if ((row2 & KEY2_Q_MASK) == 0) {
        return MZ_KEY_Q;
    }

    return 0;
}

static unsigned char is_invalid_sharp(unsigned int raw_key)
{
    return (unsigned char)(raw_key == (MZ_KEY_E + MZ_KEY_SHARP_OFFSET) || raw_key == (MZ_KEY_B + MZ_KEY_SHARP_OFFSET));
}

static note_id_t raw_key_to_note(unsigned int raw_key, unsigned char octave)
{
    switch (raw_key) {
    case MZ_KEY_C:
        return music_make_note(octave, NOTE_PITCH_C);
    case MZ_KEY_C + MZ_KEY_SHARP_OFFSET:
        return music_make_note(octave, NOTE_PITCH_CS);
    case MZ_KEY_D:
        return music_make_note(octave, NOTE_PITCH_D);
    case MZ_KEY_D + MZ_KEY_SHARP_OFFSET:
        return music_make_note(octave, NOTE_PITCH_DS);
    case MZ_KEY_E:
        return music_make_note(octave, NOTE_PITCH_E);
    case MZ_KEY_F:
        return music_make_note(octave, NOTE_PITCH_F);
    case MZ_KEY_F + MZ_KEY_SHARP_OFFSET:
        return music_make_note(octave, NOTE_PITCH_FS);
    case MZ_KEY_G:
        return music_make_note(octave, NOTE_PITCH_G);
    case MZ_KEY_G + MZ_KEY_SHARP_OFFSET:
        return music_make_note(octave, NOTE_PITCH_GS);
    case MZ_KEY_A:
        return music_make_note(octave, NOTE_PITCH_A);
    case MZ_KEY_A + MZ_KEY_SHARP_OFFSET:
        return music_make_note(octave, NOTE_PITCH_AS);
    case MZ_KEY_B:
        return music_make_note(octave, NOTE_PITCH_B);
    default:
        return NOTE_REST;
    }
}

static void set_status(const char *text)
{
    unsigned int i;

    for (i = 0; i != 39 && text[i] != 0; ++i) {
        status_text[i] = text[i];
    }
    while (i != 39) {
        status_text[i++] = ' ';
    }
    status_text[39] = 0;
}

static const char *mode_name(unsigned char mode)
{
    switch (mode) {
    case MUSIC_MODE_PREVIEW:
        return "KEY PREVIEW";
    case MUSIC_MODE_MELODY:
        return "MELODY";
    default:
        return "IDLE";
    }
}

static unsigned int displayed_step(unsigned char mode)
{
    if (mode != MUSIC_MODE_MELODY) {
        return 0;
    }

    return (unsigned int)(mz_music_get_step_index() + 1);
}

static void set_octave_status(unsigned char octave)
{
    set_status("OCTAVE 0 SELECTED.                   ");
    status_text[7] = (char)('0' + octave);
}

static void draw_static_ui(void)
{
    clrscr();

    gotoxy(0, 0);
    cprintf("MZ-700 MUSIC + SCREEN UPDATE DEMO");
    gotoxy(0, 2);
    cprintf("C D E F G A B = NATURAL NOTES");
    gotoxy(0, 3);
    cprintf("SHIFT+C/D/F/G/A = SHARPS");
    gotoxy(0, 4);
    cprintf("SHIFT+E AND SHIFT+B HAVE NO BLACK KEY");
    gotoxy(0, 5);
    cprintf("1-7 = SELECT OCTAVE");
    gotoxy(0, 6);
    cprintf("M = PLAY MIDI   S = STOP   Q = QUIT");
}

static void draw_dynamic_ui(unsigned int raw_key)
{
    static const char spinner[] = "|/-\\";
    note_id_t note;
    unsigned char mode;

    ++ui_ticks;

    mode = mz_music_get_mode();
    note = mz_music_get_current_note();

    gotoxy(0, 8);
    cprintf("MODE: %s              ", mode_name(mode));
    gotoxy(0, 9);
    cprintf("NOTE: %s              ", mz_music_note_name(note));
    gotoxy(0, 10);
    cprintf("OCT : %u              ", (unsigned int)selected_octave);
    gotoxy(0, 11);
    cprintf("BPM : %u              ", mz_music_get_bpm());
    gotoxy(0, 12);
    cprintf("STEP: %u / %u         ", displayed_step(mode), (unsigned int)mz_music_get_step_count());
    gotoxy(0, 13);
    cprintf("RAW : %u              ", raw_key);
    gotoxy(0, 14);
    cprintf("DRAW: %u   %c         ", ui_ticks, spinner[ui_ticks & 3]);
    gotoxy(0, 16);
    cprintf("%s", status_text);
}

static void handle_command_key(unsigned int raw_key, unsigned int previous_raw, unsigned char *running)
{
    if (raw_key == previous_raw) {
        return;
    }

    if (raw_key == MZ_KEY_M || raw_key == (MZ_KEY_M + MZ_KEY_SHARP_OFFSET)) {
        mz_music_play_song(&generated_song, 0);
        set_status("MIDI TUNE STARTED.                   ");
        return;
    }

    if (raw_key == MZ_KEY_S || raw_key == (MZ_KEY_S + MZ_KEY_SHARP_OFFSET)) {
        mz_music_stop();
        set_status("PLAYBACK STOPPED.                    ");
        return;
    }

    if (raw_key == MZ_KEY_Q || raw_key == (MZ_KEY_Q + MZ_KEY_SHARP_OFFSET)) {
        *running = 0;
        set_status("EXITING.                             ");
    }
}

static unsigned char handle_octave_key(unsigned int raw_key)
{
    if (raw_key < MZ_KEY_1 || raw_key > MZ_KEY_7) {
        return 0;
    }

    selected_octave = (unsigned char)(raw_key - MZ_KEY_1 + 1);
    set_octave_status(selected_octave);
    return 1;
}

static void handle_note_key(unsigned int raw_key, note_id_t note)
{
    if (note == NOTE_REST) {
        if (is_invalid_sharp(raw_key)) {
            mz_music_stop();
            set_status("NO BLACK KEY BETWEEN E/F OR B/C.     ");
        }
        return;
    }

    mz_music_preview_note(note);
    set_status("KEY PREVIEW ACTIVE.                  ");
}

void main(void)
{
    unsigned char handled_octave;
    unsigned char running;
    unsigned int raw_key;
    unsigned int previous_raw;
    unsigned int previous_step;
    unsigned int current_step;
    note_id_t note;

    running = 1;
    raw_key = 0;
    previous_raw = 0;
    previous_step = 0xffffu;

    mz_music_init();

    draw_static_ui();
    draw_dynamic_ui(0);
    ui_dirty = 0;

    while (running) {
        mz_music_update();
        current_step = mz_music_get_step_index();
        if (mz_music_is_playing()) {
            if (current_step != previous_step && (current_step & 7u) == 0) {
                ui_dirty = 1;
            }
            previous_step = current_step;
        } else {
            previous_step = 0xffffu;
        }

        handled_octave = 0;
        raw_key = scan_key_state();
        note = raw_key_to_note(raw_key, selected_octave);

        if (note != NOTE_REST) {
            if (raw_key != previous_raw) {
                handle_note_key(raw_key, note);
                ui_dirty = 1;
            }
        } else if (raw_key == 0) {
            if (mz_music_get_mode() == MUSIC_MODE_PREVIEW) {
                mz_music_preview_off();
                set_status("KEY RELEASED.                        ");
                ui_dirty = 1;
            }
        } else {
            if (mz_music_get_mode() == MUSIC_MODE_PREVIEW) {
                mz_music_preview_off();
                set_status("KEY PREVIEW STOPPED.                 ");
                ui_dirty = 1;
            }

            if (raw_key != previous_raw) {
                handled_octave = handle_octave_key(raw_key);
                if (handled_octave) {
                    ui_dirty = 1;
                } else {
                    handle_command_key(raw_key, previous_raw, &running);
                }

                if (note == NOTE_REST && !is_invalid_sharp(raw_key) && !handled_octave) {
                    ui_dirty = 1;
                }
                if (is_invalid_sharp(raw_key)) {
                    handle_note_key(raw_key, note);
                    ui_dirty = 1;
                }
            }
        }

        previous_raw = raw_key;

        if (ui_dirty) {
            draw_dynamic_ui(raw_key);
            ui_dirty = 0;
        }
    }

    mz_music_shutdown();
    gotoxy(0, 18);
    cprintf("PROGRAM STOPPED.                     ");
}
