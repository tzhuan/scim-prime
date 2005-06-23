/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 - 2005 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Based on scim-hangul.
 * Copyright (c) 2004 James Su <suzhe@turbolinux.com.cn>
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_prime_prefs.h"
#include "intl.h"

using namespace scim;

#define scim_module_init prime_imengine_setup_LTX_scim_module_init
#define scim_module_exit prime_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       prime_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    prime_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        prime_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description prime_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     prime_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     prime_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   prime_imengine_setup_LTX_scim_setup_module_query_changed

#define DATA_POINTER_KEY "scim-prime::ConfigPointer"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_PRIME_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("PRIME"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An PRIME IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"


// Internal data structure
struct BoolConfigData
{
    const char *key;
    bool        value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct StringConfigData
{
    const char *key;
    String      value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct ColorConfigData
{
    const char *key;
    String      value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct KeyboardConfigPage
{
    const char       *label;
    StringConfigData *data;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

struct ComboConfigCandidate
{
    const char *label;
    const char *data;
};

enum {
    COLUMN_LABEL = 0,
    COLUMN_VALUE = 1,
    COLUMN_DESC  = 2,
    COLUMN_DATA  = 3,
    N_COLUMNS    = 4,
};

// Internal data declaration.
static bool __have_changed    = true;

static GtkWidget   * __widget_key_categories_menu = NULL;
static GtkWidget   * __widget_key_filter          = NULL;
static GtkWidget   * __widget_key_filter_button   = NULL;
static GtkWidget   * __widget_key_list_view       = NULL;
static GtkTooltips * __widget_tooltips            = NULL;

static BoolConfigData __config_bool_common [] =
{
    {
        SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD,
        SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT,
        N_("_Start conversion on inputting comma or period."),
        NULL,
        N_("Start conversion on inputting comma or period."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_PERIOD,
        SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT,
        N_("Commit comma and period _immediately."),
        NULL,
        N_("Commit comma and period immediatly on inputting these characters "
           "when no preedition string exists."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER,
        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT,
        N_("Commit on inputting _upper letter."),
        NULL,
        N_("Commit previous preedit string when a upper letter is entered."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION,
        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT,
        N_("_Predict while typing letters."),
        NULL,
        N_("Show candidates window to display predicted candidates "
           "while typing letters."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION,
        SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION_DEFAULT,
        N_("Use _direct select keys also on prediction."),
        NULL,
        N_("Use direct select keys not only for conversion state "
           "but also for prediction state."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_INLINE_PREDICTION,
        SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT,
        N_("_Inline prediction."),
        NULL,
        N_("Show first candidate of predictions instead of reading on preedit area. "
           "Reading is shown by external window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_AUTO_REGISTER,
        SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT,
        N_("Change to the inline word _register mode automatically."),
        NULL,
        N_("Invoke the inline word register mode when the cursor in the "
           "candidates window is move to out of range."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT,
        SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT,
        N_("_Close candidates window when a candidate is selected directly."),
        NULL,
        N_("Close candidates window when a candidate is selected directly."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_ANNOTATION,
        SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT,
        N_("Show _annotation of the word on the candidates window"),
        NULL,
        N_("Show annotation of the word on the candidates window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_USAGE,
        SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT,
        N_("Show _usage of the word on the candidates window"),
        NULL,
        N_("Show usage of the word on the candidates window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_COMMENT,
        SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT,
        N_("Show co_mment of the word on the candidates window"),
        NULL,
        N_("Show comment of the word on the candidates window."),
        NULL,
        false,
    },
};
static unsigned int __config_bool_common_num = sizeof (__config_bool_common) / sizeof (BoolConfigData);

static StringConfigData __config_string_common [] =
{
    {
        SCIM_PRIME_CONFIG_COMMAND,
        SCIM_PRIME_CONFIG_COMMAND_DEFAULT,
        N_("PRIME _command:"),
        NULL,
        N_("The PRIME command to use as conversion engine."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_PREDICT_WIN_POS,
        SCIM_PRIME_CONFIG_PREDICT_WIN_POS_DEFAULT,
        N_("Prediction _window position:"),
        NULL,
        N_("The prediction window position to show."),
        NULL,
        false,
    },
};
static unsigned int __config_string_common_num = sizeof (__config_string_common) / sizeof (StringConfigData);

static StringConfigData __config_keyboards_edit [] =
{
    {
        SCIM_PRIME_CONFIG_SPACE_KEY,
        SCIM_PRIME_CONFIG_SPACE_KEY_DEFAULT,
        N_("Insert space"),
        N_("Select space keys"),
        N_("The key events to insert space letter on non-preediting state. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY,
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY_DEFAULT,
        N_("Insert alternative space"),
        N_("Select alternative space keys"),
        N_("The key events to insert alterenative space letter on non-preediting state. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_BACKSPACE_KEY,
        SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace"),
        N_("Select backspace keys"),
        N_("The key events to delete a character before caret. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_DELETE_KEY,
        SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete"),
        N_("Select delete keys"),
        N_("The key events to delete a character after caret. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_convert [] =
{
    {
        SCIM_PRIME_CONFIG_CONVERT_KEY,
        SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert"),
        N_("Select convert keys"),
        N_("The key events to convert the preedit string to kanji. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANCEL_KEY,
        SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel"),
        N_("Select cancel keys"),
        N_("The key events to cancel preediting or converting. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_KEY,
        SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT,
        N_("Commit"),
        N_("Select commit keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY,
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY_DEFAULT,
        N_("Commit witout learn"),
        N_("Select commit witout learn keys"),
        N_("The key events to commit the preedit string without learn. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY,
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY_DEFAULT,
        N_("Alternative commit"),
        N_("Select alternative commit keys"),
        N_("The key events to commit the first predicted string. "
           "On inline prediction mode, reading string will be commited by this key events."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY_DEFAULT,
        N_("Complete"),
        N_("Select complete keys"),
        N_("The key events to complete a word using predictions. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY,
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY_DEFAULT,
        N_("Register a word"),
        N_("Select register a word keys"),
        N_("The key events to invoke registering a word mode. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_mode [] =
{
    {
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT,
        N_("Default mode"),
        N_("Select default mode keys"),
        N_("The key events to set to default mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT,
        N_("Katakana mode"),
        N_("Select katakana mode keys"),
        N_("The key events to set to katakana mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half katakana mode"),
        N_("Select half katakana mode keys"),
        N_("The key events to set to half katakana mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT,
        N_("Wide ascii mode"),
        N_("Select wide ascii mode keys"),
        N_("The key events to set to wide ascii mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT,
        N_("Raw mode"),
        N_("Select raw mode keys"),
        N_("The key events to set to raw mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY,
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY_DEFAULT,
        N_("Toggle language"),
        N_("Select toggle language keys"),
        N_("The key events to toggle language. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_caret [] =
{
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT,
        N_("Move to first"),
        N_("Select move caret to first keys"),
        N_("The key events to move the caret to the first of preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT,
        N_("Move to last"),
        N_("Select move caret to last keys"),
        N_("The key events to move the caret to the last of the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT,
        N_("Move to forward"),
        N_("Select move caret to forward keys"),
        N_("The key events to move the caret to forward. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT,
        N_("Move to backward"),
        N_("Select move caret to backward keys"),
        N_("The key events to move the caret to backward. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_segments [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("First segment"),
        N_("Select first segment keys"),
        N_("The key events to select first segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT,
        N_("Last segment"),
        N_("Select last segment keys"),
        N_("The key events to select last segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT,
        N_("Next segment"),
        N_("Select next segment keys"),
        N_("The key events to select next segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT,
        N_("Previous segment"),
        N_("Select previous segment keys"),
        N_("The key events to select previous segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT,
        N_("Shrink segment"),
        N_("Select shrink segment keys"),
        N_("The key events to shrink the selected segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT,
        N_("Expand segment"),
        N_("Select expand segment keys"),
        N_("The key events to expand the selected segment. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_candidates [] =
{
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT,
        N_("Next candidate"),
        N_("Select next candidate keys"),
        N_("The key events to select next candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT,
        N_("Previous candidate"),
        N_("Select previous candidate keys"),
        N_("The key events to select previous candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY_DEFAULT,
        N_("Page down"),
        N_("Select page down candidates keys"),
        N_("The key events to select page down candidates. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY_DEFAULT,
        N_("Page up"),
        N_("Select page up candidates keys"),
        N_("The key events to select page up candidates. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_direct_select_candidate [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT,
        N_("1st candidate"),
        N_("Select keys to select 1st candidate"),
        N_("The key events to select 1st candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT,
        N_("2nd candidate"),
        N_("Select keys to select 2nd candidate"),
        N_("The key events to select 2nd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT,
        N_("3rd candidate"),
        N_("Select keys to select 3rd candidate"),
        N_("The key events to select 3rd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT,
        N_("4th candidate"),
        N_("Select keys to select 4th candidate"),
        N_("The key events to select 4th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT,
        N_("5th candidate"),
        N_("Select keys to select 5th candidate"),
        N_("The key events to select 5th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT,
        N_("6th candidate"),
        N_("Select keys to select 6th candidate"),
        N_("The key events to select 6th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT,
        N_("7th candidate"),
        N_("Select keys to select 7th candidate"),
        N_("The key events to select 7th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT,
        N_("8th candidate"),
        N_("Select keys to select 8th candidate"),
        N_("The key events to select 8th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT,
        N_("9th candidate"),
        N_("Select keys to select 9th candidate"),
        N_("The key events to select 9th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT,
        N_("10th candidate"),
        N_("Select keys to select 10th candidate"),
        N_("The key events to select 10th candidate. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static ColorConfigData __config_color_common [] =
{
    {
        SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the annotaion text"),
        N_("The color of the annotaion text in the candidate list."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the usage text"),
        N_("The color of the usage text in the candidate list."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the comment text"),
        N_("The color of the comment text in the candidate list."),
        NULL,
        false,
    },
};
static unsigned int __config_color_common_num = sizeof (__config_color_common) / sizeof (ColorConfigData);

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Edit keys"),       __config_keyboards_edit},
    {N_("Convert keys"),    __config_keyboards_convert},
    {N_("Mode keys"),       __config_keyboards_mode},
    {N_("Caret keys"),      __config_keyboards_caret},
    {N_("Segments keys"),   __config_keyboards_segments},
    {N_("Candidates keys"), __config_keyboards_candidates},
    {N_("Direct select keys"),   __config_keyboards_direct_select_candidate},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

const int INDEX_SEARCH_BY_KEY = __key_conf_pages_num;
const int INDEX_ALL           = __key_conf_pages_num + 1;

static ComboConfigCandidate predict_win_pos[] =
{
    {N_("Head of preedition area"), "head"},
    {N_("Tail of preedition area"), "tail"},
    {NULL, NULL},
};


static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
#if 0
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
#endif
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_color_button_set       (GtkColorButton  *colorbutton,
                                               gpointer         user_data);

static void     on_key_filter_selection_clicked   (GtkButton       *button,
                                                   gpointer         user_data);
static void     on_key_category_menu_changed      (GtkOptionMenu   *omenu,
                                                   gpointer         user_data);
static gboolean on_key_list_view_key_press        (GtkWidget       *widget,
                                                   GdkEventKey     *event,
                                                   gpointer         user_data);
static gboolean on_key_list_view_button_press     (GtkWidget       *widget,
                                                   GdkEventButton  *event,
                                                   gpointer         user_data);
static void setup_widget_value ();



static BoolConfigData *
find_bool_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData *entry = &__config_bool_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static StringConfigData *
find_string_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData *entry = &__config_string_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static ColorConfigData *
find_color_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_color_common_num; i++) {
        ColorConfigData *entry = &__config_color_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static GtkWidget *
create_check_button (const char *config_key)
{
    BoolConfigData *entry = find_bool_config_entry (config_key);
    if (!entry)
        return NULL;

    entry->widget = gtk_check_button_new_with_mnemonic (_(entry->label));
    gtk_container_set_border_width (GTK_CONTAINER (entry->widget), 4);
    g_signal_connect (G_OBJECT (entry->widget), "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      entry);
    gtk_widget_show (entry->widget);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
}

static bool
match_key_event (const KeyEventList &list, const KeyEvent &key)
{
    KeyEventList::const_iterator kit;

    for (kit = list.begin (); kit != list.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
             return true;
    }
    return false;
}

static void
create_entry (StringConfigData *data, GtkTable *table, int i)
{
    GtkWidget *label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(data->label));
    gtk_widget_show (label);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 4, 0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    (data)->widget = gtk_entry_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   GTK_WIDGET (data->widget));
    g_signal_connect ((gpointer) (data)->widget, "changed",
                      G_CALLBACK (on_default_editable_changed),
                      data);
    gtk_widget_show (GTK_WIDGET (data->widget));
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (data->widget),
                      1, 2, i, i+1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (data->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET (data->widget),
                              _(data->tooltip), NULL);
}

static GtkWidget *
create_combo (const char *config_key, gpointer candidates_p,
              GtkWidget *table, gint idx)
{
    StringConfigData *entry = find_string_config_entry (config_key);
    if (!entry)
        return NULL;

    GtkWidget *label;

    label = gtk_label_new_with_mnemonic (_(entry->label));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 4, 0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    gtk_widget_show (label);

    entry->widget = gtk_combo_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   GTK_COMBO (entry->widget)->entry);
    gtk_combo_set_value_in_list (GTK_COMBO (entry->widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (entry->widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (entry->widget)->entry), FALSE);
    gtk_widget_show (entry->widget);
    gtk_table_attach (GTK_TABLE (table), entry->widget, 1, 2, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (entry->widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (entry->widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      entry);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
}

static GtkWidget *
create_color_button (const char *config_key)
{
    ColorConfigData *entry = find_color_config_entry (config_key);
    if (!entry)
        return NULL;

    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);

    GtkWidget *label = NULL;
    if (entry->label) {
        label = gtk_label_new_with_mnemonic (_(entry->label));
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
        gtk_widget_show (label);
    }

    entry->widget = gtk_color_button_new ();
    gtk_color_button_set_title (GTK_COLOR_BUTTON (entry->widget), entry->title);
    gtk_container_set_border_width (GTK_CONTAINER (entry->widget), 4);
    g_signal_connect (G_OBJECT (entry->widget), "color-set",
                      G_CALLBACK (on_default_color_button_set),
                      entry);
    gtk_box_pack_start (GTK_BOX (hbox), entry->widget, FALSE, FALSE, 2);
    gtk_widget_show (entry->widget);

    if (label)
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry->widget);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return hbox;
}

static void
append_key_bindings (GtkTreeView *treeview, gint idx, const gchar *filter)
{
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));
    KeyEventList keys1, keys2;
    
    if (filter && *filter)
        scim_string_to_key_list (keys1, filter);

    if (idx < 0 || idx >= (gint) __key_conf_pages_num)
        return;

    for (unsigned int i = 0; __key_conf_pages[idx].data[i].key; i++) {
        if (filter && *filter) {
            scim_string_to_key_list (keys2, __key_conf_pages[idx].data[i].value.c_str());
            KeyEventList::const_iterator kit;
            bool found = true;
            for (kit = keys1.begin (); kit != keys1.end (); ++kit) {
                if (!match_key_event (keys2, *kit)) {
                    found = false;
                    break;
                }
            }
            if (!found)
                continue;
        }

        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_LABEL, _(__key_conf_pages[idx].data[i].label),
                            COLUMN_VALUE, __key_conf_pages[idx].data[i].value.c_str (),
                            COLUMN_DESC,  _(__key_conf_pages[idx].data[i].tooltip),
                            COLUMN_DATA, &__key_conf_pages[idx].data[i],
                            -1);
    }
}

static void
key_list_view_popup_key_selection (GtkTreeView *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreePath *treepath = NULL;
    GtkTreeIter iter;

    gtk_tree_view_get_cursor (treeview, &treepath, NULL);
    if (!treepath) return;
    gtk_tree_model_get_iter (model, &iter, treepath);
    gtk_tree_path_free (treepath);

    StringConfigData *data;
    gtk_tree_model_get (model, &iter,
                        COLUMN_DATA, &data,
                        -1);
    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys
            (SCIM_KEY_SELECTION_DIALOG (dialog),
             data->value.c_str());

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys
                (SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, data->value.c_str())) {
                data->value = keys;
                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                    COLUMN_VALUE, data->value.c_str(),
                                    -1);
                data->changed = true;
                __have_changed = true;
            }
        }

        gtk_widget_destroy (dialog);
    }
}

static GtkWidget *
create_options_page ()
{
    GtkWidget *vbox, *table, *widget;
    StringConfigData *entry;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);

    entry = find_string_config_entry (SCIM_PRIME_CONFIG_COMMAND);
    create_entry (entry, GTK_TABLE (table), 0);

    /* start conversion on inputting comma or period */
    widget = create_check_button (SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* commit comma and period */
    widget = create_check_button (SCIM_PRIME_CONFIG_COMMIT_PERIOD);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* commit on upper */
    widget = create_check_button (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    return vbox;
}

static GtkWidget *
create_prediction_page ()
{
    GtkWidget *vbox, *table, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    /* predict on preedition */
    widget = create_check_button (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* prediction window position */
    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);
    widget = create_combo (SCIM_PRIME_CONFIG_PREDICT_WIN_POS,
                           predict_win_pos, table, 0);

    /* use direct select keys on prediction */
    widget = create_check_button (SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* use direct select keys on prediction */
    widget = create_check_button (SCIM_PRIME_CONFIG_INLINE_PREDICTION);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    return vbox;
}

static GtkWidget *
create_candidates_window_page ()
{
    GtkWidget *vbox, *hbox, *table, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);

    /* auto register */
    widget = create_check_button (SCIM_PRIME_CONFIG_AUTO_REGISTER);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* close candidate window on select */
    widget = create_check_button (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* show annotation */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);
    widget = create_check_button (SCIM_PRIME_CONFIG_SHOW_ANNOTATION);
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);
    /* annotation color */
    widget = create_color_button (SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR);
    gtk_box_pack_end (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

    /* show usage */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);
    widget = create_check_button (SCIM_PRIME_CONFIG_SHOW_USAGE);
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);
    /* usage text color */
    widget = create_color_button (SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR);
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

    /* show comment */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);
    widget = create_check_button (SCIM_PRIME_CONFIG_SHOW_COMMENT);
    gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);
    /* comment text color */
    widget = create_color_button (SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR);
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

    return vbox;
}

#if 0
static GtkWidget *
create_toolbar_page ()
{
    GtkWidget *vbox, *hbox, *label;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    return vbox;
}
#endif

#if 0
static GtkWidget *
create_dict_page (void)
{
    GtkWidget *table;
    GtkWidget *label;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    return table;
}
#endif

static GtkWidget *
create_keyboard_page (void)
{
    GtkWidget *vbox, *hbox;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    // category menu
    GtkWidget *label = gtk_label_new_with_mnemonic (_("_Group:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *omenu = gtk_option_menu_new ();
    __widget_key_categories_menu = omenu;
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), omenu);

    GtkWidget *menu = gtk_menu_new ();

    GtkWidget *menuitem;

    for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
        menuitem = gtk_menu_item_new_with_label (_(__key_conf_pages[i].label));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);
    }

    menuitem = gtk_menu_item_new_with_label (_("Search by key"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    menuitem = gtk_menu_item_new_with_label (_("all"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
    gtk_widget_show (menu);

    GtkWidget *entry = gtk_entry_new ();
    __widget_key_filter = entry;
    gtk_entry_set_editable (GTK_ENTRY (entry), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 2);
    gtk_widget_show(entry);

    GtkWidget *button = gtk_button_new_with_label ("...");
    __widget_key_filter_button = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_key_filter_selection_clicked), entry);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);

    // key bindings view
    GtkWidget *scrwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
                                         GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrwin), 4);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrwin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrwin, TRUE, TRUE, 2);
    gtk_widget_show (scrwin);

    GtkListStore *store = gtk_list_store_new (N_COLUMNS,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_POINTER);
    GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    __widget_key_list_view = treeview;
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Feature"), cell,
                                                       "text", COLUMN_LABEL,
                                                       NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 120);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Key bindings"), cell,
                                                       "text", COLUMN_VALUE,
                                                       NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Description"), cell,
                                                       "text", COLUMN_DESC,
                                                       NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // connect signals
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_key_category_menu_changed), treeview);
    g_signal_connect (G_OBJECT (treeview), "key-press-event",
                      G_CALLBACK (on_key_list_view_key_press), NULL);
    g_signal_connect (G_OBJECT (treeview), "button-press-event",
                      G_CALLBACK (on_key_list_view_button_press), NULL);

    return vbox;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (!window) {
        GtkWidget *notebook = gtk_notebook_new();
        gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook));
        gtk_widget_show (notebook);
        window = notebook;
        gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

        // Create the 1st page.
        GtkWidget *page = create_options_page ();
        GtkWidget *label = gtk_label_new (_("Common"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the 2nd page.
        page = create_prediction_page ();
        label = gtk_label_new (_("Prediction"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the 3rd page.
        page = create_candidates_window_page ();
        label = gtk_label_new (_("Candidates window"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

#if 0
        // Create the 4th page.
        page = create_toolbar_page ();
        label = gtk_label_new (_("Toolbar"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the 5th page.
        page = create_dict_page ();
        label = gtk_label_new (_("Dictionary"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
#endif

        // Create the key bind pages.
        page = create_keyboard_page ();
        label = gtk_label_new (_("Key bindings"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // for preventing enabling left arrow.
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        setup_widget_value ();
    }

    return window;
}

static void
setup_combo_value (GtkCombo *combo, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

    ComboConfigCandidate *data
        = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (GTK_COMBO(combo)->entry),
                            DATA_POINTER_KEY));

    for (unsigned int i = 0; data[i].label; i++) {
        list = g_list_append (list, (gpointer) _(data[i].label));
        if (!strcmp (data[i].data, str.c_str ()))
            defval = _(data[i].label);
    }

    gtk_combo_set_popdown_strings (combo, list);
    g_list_free (list);

    if (defval)
        gtk_entry_set_text (GTK_ENTRY (combo->entry), defval);
}

static void
setup_widget_value ()
{
    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.widget)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry.widget),
                                          entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.widget && GTK_IS_COMBO (entry.widget))
            setup_combo_value (GTK_COMBO (entry.widget), entry.value);
        else if (entry.widget && GTK_IS_ENTRY (entry.widget))
            gtk_entry_set_text (GTK_ENTRY (entry.widget),
                                entry.value.c_str ());
    }

    for (unsigned int i = 0; i < __config_color_common_num; i++) {
        ColorConfigData &entry = __config_color_common[i];
        if (entry.widget) {
            GdkColor color;
            gdk_color_parse (entry.value.c_str (), &color);
            gtk_color_button_set_color (GTK_COLOR_BUTTON (entry.widget),
                                        &color);
        }
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].widget) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].widget),
                    __key_conf_pages[j].data[i].value.c_str ());
            }
        }
    }

    gtk_option_menu_set_history (GTK_OPTION_MENU (__widget_key_categories_menu), 0);
    gtk_widget_set_sensitive (__widget_key_filter, FALSE);
    gtk_widget_set_sensitive (__widget_key_filter_button, FALSE);
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view), 0, NULL);
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int i = 0; i < __config_color_common_num; i++) {
        ColorConfigData &entry = __config_color_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            __key_conf_pages[j].data[i].value =
                config->read (String (__key_conf_pages[j].data[i].key),
                              __key_conf_pages[j].data[i].value);
        }
    }

    setup_widget_value ();

    for (unsigned int i = 0; i < __config_bool_common_num; i++)
        __config_bool_common[i].changed = false;

    for (unsigned int i = 0; i < __config_string_common_num; i++)
        __config_string_common[i].changed = false;

    for (unsigned int i = 0; i < __config_color_common_num; i++)
        __config_color_common[i].changed = false;

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i)
            __key_conf_pages[j].data[i].changed = false;
    }

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int i = 0; i < __config_color_common_num; i++) {
        ColorConfigData &entry = __config_color_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].changed)
                config->write (String (__key_conf_pages[j].data[i].key),
                               __key_conf_pages[j].data[i].value);
            __key_conf_pages[j].data[i].changed = false;
        }
    }

    __have_changed = false;
}

static bool
query_changed ()
{
    return __have_changed;
}


static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    BoolConfigData *entry = static_cast<BoolConfigData*> (user_data);

    if (entry) {
        entry->value = gtk_toggle_button_get_active (togglebutton);
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    StringConfigData *entry = static_cast <StringConfigData*> (user_data);

    if (entry) {
        entry->value = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        entry->changed = true;
        __have_changed = true;
    }
}

#if 0
static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    StringConfigData *data = static_cast <StringConfigData*> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->widget)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->widget))))
                gtk_entry_set_text (GTK_ENTRY (data->widget), keys);
        }

        gtk_widget_destroy (dialog);
    }
}
#endif

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    StringConfigData *entry = static_cast<StringConfigData*> (user_data);
    ComboConfigCandidate *data = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (editable),
                            DATA_POINTER_KEY));

    if (!entry) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            entry->value = data[i].data;
            entry->changed = true;
            __have_changed = true;
            break;
        }
    }
}

static void
on_default_color_button_set (GtkColorButton *colorbutton,
                             gpointer        user_data)
{
    ColorConfigData *entry = static_cast<ColorConfigData*> (user_data);

    if (entry) {
        GdkColor color;
        gchar color_str[8];
        gtk_color_button_get_color (colorbutton, &color);
        g_snprintf (color_str, G_N_ELEMENTS (color_str),
                    "#%02X%02X%02X", 
                    (color.red>>8),
                    (color.green>>8),
                    (color.blue>>8));
        entry->value = String (color_str);
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_key_category_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (user_data);
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));

    gtk_list_store_clear (store);

    gint idx = gtk_option_menu_get_history (omenu);

    bool use_filter = false;

    if (idx >= 0 && idx < (gint) __key_conf_pages_num) {
        append_key_bindings (treeview, idx, NULL);

    } else if (idx == INDEX_SEARCH_BY_KEY) {
        // search by key
        use_filter = true;
        const char *str = gtk_entry_get_text (GTK_ENTRY (__widget_key_filter));
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, str);

    } else if (idx == INDEX_ALL) {
        // all
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, NULL);
    }

    gtk_widget_set_sensitive (__widget_key_filter,        use_filter);
    gtk_widget_set_sensitive (__widget_key_filter_button, use_filter);
}

static void
on_key_filter_selection_clicked (GtkButton *button,
                                 gpointer   user_data)
{
    GtkEntry *entry = static_cast <GtkEntry*> (user_data);

    if (entry) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_("Set key filter"));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (entry));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (entry)))
                gtk_entry_set_text (entry, keys);

            GtkTreeModel *model;
            model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
            gtk_list_store_clear (GTK_LIST_STORE (model));
            for (unsigned int i = 0; i < __key_conf_pages_num; i++)
                append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view),
                                     i, keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static gboolean
on_key_list_view_key_press (GtkWidget *widget, GdkEventKey *event,
                            gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    switch (event->keyval) {
    case GDK_Return:
    case GDK_KP_Enter:
        key_list_view_popup_key_selection (treeview);
        break;
    }

    return FALSE;
}

static gboolean
on_key_list_view_button_press (GtkWidget *widget, GdkEventButton *event,
                               gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    if (event->type == GDK_2BUTTON_PRESS) {
        key_list_view_popup_key_selection (treeview);
        return TRUE;
    }

    return FALSE;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
