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

#include <gtk/gtk.h>

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
struct KeyboardConfigData
{
    const char *key;
    String      data;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
};

struct KeyboardConfigPage
{
    const char         *label;
    KeyboardConfigData *data;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

// Internal data declaration.
static String __config_command                     = SCIM_PRIME_CONFIG_COMMAND_DEFAULT;
static bool   __config_convert_on_period           = SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT;
static bool   __config_commit_period               = SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT;
static bool   __config_commit_on_upper             = SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT;
static bool   __config_predict_on_preedition       = SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT;
static bool   __config_direct_select_on_prediction = SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION_DEFAULT;
static bool   __config_inline_prediction           = SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT;
static bool   __config_auto_register               = SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT;
static bool   __config_close_cand_win_on_select    = SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT;
static bool   __config_show_annotation             = SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT;
static bool   __config_show_usage                  = SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT;
static bool   __config_show_comment                = SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT;

static bool __have_changed    = true;

static GtkWidget    * __widget_command                     = 0;
static GtkWidget    * __widget_convert_on_period           = 0;
static GtkWidget    * __widget_commit_period               = 0;
static GtkWidget    * __widget_commit_on_upper             = 0;
static GtkWidget    * __widget_predict_on_preedition       = 0;
static GtkWidget    * __widget_direct_select_on_prediction = 0;
static GtkWidget    * __widget_inline_prediction           = 0;
static GtkWidget    * __widget_auto_register               = 0;
static GtkWidget    * __widget_close_cand_win_on_select    = 0;
static GtkWidget    * __widget_show_annotation             = 0;
static GtkWidget    * __widget_show_usage                  = 0;
static GtkWidget    * __widget_show_comment                = 0;
static GtkTooltips  * __widget_tooltips                    = 0;

static KeyboardConfigData __config_keyboards_edit [] =
{
    {
        SCIM_PRIME_CONFIG_SPACE_KEY,
        SCIM_PRIME_CONFIG_SPACE_KEY_DEFAULT,
        N_("Space keys:"),
        N_("Select space keys"),
        N_("The key events to insert space letter on non-preediting state. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY,
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY_DEFAULT,
        N_("Alternative space keys:"),
        N_("Select alternative space keys"),
        N_("The key events to insert alterenative space letter on non-preediting state. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_BACKSPACE_KEY,
        SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace keys:"),
        N_("Select backspace keys"),
        N_("The key events to delete a character before caret. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_DELETE_KEY,
        SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete keys:"),
        N_("Select delete keys"),
        N_("The key events to delete a character after caret. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_convert [] =
{
    {
        SCIM_PRIME_CONFIG_CONVERT_KEY,
        SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert keys:"),
        N_("Select convert keys"),
        N_("The key events to convert the preedit string to kanji. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_CANCEL_KEY,
        SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel keys:"),
        N_("Select cancel keys"),
        N_("The key events to cancel preediting or converting. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_KEY,
        SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT,
        N_("Commit keys:"),
        N_("Select commit keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY,
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY_DEFAULT,
        N_("Commit witout learn keys:"),
        N_("Select commit witout learn keys"),
        N_("The key events to commit the preedit string without learn. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY,
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY_DEFAULT,
        N_("Alternative commit keys:"),
        N_("Select alternative commit keys"),
        N_("The key events to commit the first predicted string. "
           "On inline prediction mode, reading string will be commited by this key events."),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY_DEFAULT,
        N_("Complete keys:"),
        N_("Select complete keys"),
        N_("The key events to complete a word using predictions. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY,
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY_DEFAULT,
        N_("Register a word keys:"),
        N_("Select register a word keys"),
        N_("The key events to invoke registering a word mode. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_mode [] =
{
    {
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT,
        N_("Default mode keys:"),
        N_("Select default mode keys"),
        N_("The key events to set to default mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT,
        N_("Katakana mode keys:"),
        N_("Select katakana mode keys"),
        N_("The key events to set to katakana mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half katakana mode keys:"),
        N_("Select half katakana mode keys"),
        N_("The key events to set to half katakana mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT,
        N_("Wide ascii mode keys:"),
        N_("Select wide ascii mode keys"),
        N_("The key events to set to wide ascii mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT,
        N_("Raw mode keys:"),
        N_("Select raw mode keys"),
        N_("The key events to set to raw mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY,
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY_DEFAULT,
        N_("Toggle language keys:"),
        N_("Select toggle language keys"),
        N_("The key events to toggle language. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_caret [] =
{
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT,
        N_("Move to first keys:"),
        N_("Select move caret to first keys"),
        N_("The key events to move the caret to the first of preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT,
        N_("Move to last keys:"),
        N_("Select move caret to last keys"),
        N_("The key events to move the caret to the last of the preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT,
        N_("Move forward keys:"),
        N_("Select move caret forward keys"),
        N_("The key events to move the caret to forward. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT,
        N_("Move backward keys:"),
        N_("Select move caret backward keys"),
        N_("The key events to move the caret to backward. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_segments [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("First segment keys:"),
        N_("Select first segment keys"),
        N_("The key events to select first segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT,
        N_("Last segment keys:"),
        N_("Select last segment keys"),
        N_("The key events to select last segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT,
        N_("Next segment keys:"),
        N_("Select next segment keys"),
        N_("The key events to select next segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT,
        N_("Previous segment keys:"),
        N_("Select previous segment keys"),
        N_("The key events to select previous segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT,
        N_("Shrink segment keys:"),
        N_("Select shrink segment keys"),
        N_("The key events to shrink the selected segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT,
        N_("Expand segment keys:"),
        N_("Select expand segment keys"),
        N_("The key events to expand the selected segment. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_candidates [] =
{
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT,
        N_("Next keys:"),
        N_("Select next candidate keys"),
        N_("The key events to select next candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT,
        N_("Previous keys:"),
        N_("Select previous candidate keys"),
        N_("The key events to select previous candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY_DEFAULT,
        N_("Page down keys:"),
        N_("Select page down candidates keys"),
        N_("The key events to select page down candidates. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY_DEFAULT,
        N_("Page up keys:"),
        N_("Select page up candidates keys"),
        N_("The key events to select page up candidates. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_direct_select_candidate [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT,
        N_("1st candidate keys:"),
        N_("Select keys to select 1st candidate"),
        N_("The key events to select 1st candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT,
        N_("2nd candidate keys:"),
        N_("Select keys to select 2nd candidate"),
        N_("The key events to select 2nd candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT,
        N_("3rd candidate keys:"),
        N_("Select keys to select 3rd candidate"),
        N_("The key events to select 3rd candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT,
        N_("4th candidate keys:"),
        N_("Select keys to select 4th candidate"),
        N_("The key events to select 4th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT,
        N_("5th candidate keys:"),
        N_("Select keys to select 5th candidate"),
        N_("The key events to select 5th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT,
        N_("6th candidate keys:"),
        N_("Select keys to select 6th candidate"),
        N_("The key events to select 6th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT,
        N_("7th candidate keys:"),
        N_("Select keys to select 7th candidate"),
        N_("The key events to select 7th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT,
        N_("8th candidate keys:"),
        N_("Select keys to select 8th candidate"),
        N_("The key events to select 8th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT,
        N_("9th candidate keys:"),
        N_("Select keys to select 9th candidate"),
        N_("The key events to select 9th candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT,
        N_("10th candidate keys:"),
        N_("Select keys to select 10th candidate"),
        N_("The key events to select 10th candidate. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Edit keys"),       __config_keyboards_edit},
    {N_("Convert keys"),    __config_keyboards_convert},
    {N_("Mode keys"),       __config_keyboards_mode},
    {N_("Caret keys"),      __config_keyboards_caret},
    {N_("Segments keys"),   __config_keyboards_segments},
    {N_("Candidates keys"), __config_keyboards_candidates},
    {N_("Candidates keys (Direct select)"), __config_keyboards_direct_select_candidate},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);


static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
#if 0
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
#endif
static void setup_widget_value ();


#if 0
static GtkWidget *
create_combo_widget (const char *label_text, GtkWidget **widget,
                     gpointer data_p, gpointer candidates_p)
{
    GtkWidget *hbox, *label;

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);

    label = gtk_label_new (label_text);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);

    *widget = gtk_combo_new ();
    gtk_combo_set_value_in_list (GTK_COMBO (*widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (*widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (*widget)->entry), FALSE);
    gtk_widget_show (*widget);
    gtk_box_pack_start (GTK_BOX (hbox), *widget, FALSE, FALSE, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (*widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (*widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      data_p);

    return hbox;
}
#endif

#define APPEND_ENTRY(text, tooltip, widget, i)   \
{ \
    label = gtk_label_new (NULL); \
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), text); \
    gtk_widget_show (label); \
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5); \
    gtk_misc_set_padding (GTK_MISC (label), 4, 0); \
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1, \
                      (GtkAttachOptions) (GTK_FILL), \
                      (GtkAttachOptions) (GTK_FILL), 4, 4); \
    widget = gtk_entry_new (); \
    gtk_widget_show (widget); \
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, i, i+1, \
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), \
                      (GtkAttachOptions) (GTK_FILL), 4, 4); \
    if (tooltip && *tooltip) \
        gtk_tooltips_set_tip (__widget_tooltips, widget, \
                              tooltip, NULL); \
}

static GtkWidget *
create_options_page ()
{
    GtkWidget *vbox, *table, *label;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    APPEND_ENTRY(_("PRIME command:"), _("The PRIME command to use as conversion engine."),
                 __widget_command, 0);

    /* start conversion on inputting comma or period */
    __widget_convert_on_period
        = gtk_check_button_new_with_mnemonic (_("Start conversion on inputting comma or period."));
    gtk_widget_show (__widget_convert_on_period);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_convert_on_period, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_convert_on_period), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_convert_on_period,
                          _("Start conversion on inputting comma or period."), NULL);

    /* commit comma and period */
    __widget_commit_period
        = gtk_check_button_new_with_mnemonic (_("Commit comma and period immediately."));
    gtk_widget_show (__widget_commit_period);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_commit_period, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_commit_period), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_commit_period,
                          _("Commit comma and period immediatly on inputting these characters when no preedition string exists."), NULL);

    /* commit on upper */
    __widget_commit_on_upper
        = gtk_check_button_new_with_mnemonic (_("Commit on inputting upper letter."));
    gtk_widget_show (__widget_commit_on_upper);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_commit_on_upper, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_commit_on_upper), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_commit_on_upper,
                          _("Commit previous preedit string when a upper letter is entered."), NULL);

    // Connect all signals.
    g_signal_connect ((gpointer) __widget_command, "changed",
                      G_CALLBACK (on_default_editable_changed),
                      &__config_command);
    g_signal_connect ((gpointer) __widget_convert_on_period, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_convert_on_period);
    g_signal_connect ((gpointer) __widget_commit_period, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_commit_period);
    g_signal_connect ((gpointer) __widget_commit_on_upper, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_commit_on_upper);

    return vbox;
}

static GtkWidget *
create_prediction_page ()
{
    GtkWidget *vbox, *table;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    /* predict on preedition */
    __widget_predict_on_preedition
        = gtk_check_button_new_with_mnemonic (_("Predict while typing letters."));
    gtk_widget_show (__widget_predict_on_preedition);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_predict_on_preedition, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_predict_on_preedition), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_predict_on_preedition,
                          _("Show candidates window to display predicted candidates while typing letters."), NULL);

    /* use direct select keys on prediction */
    __widget_direct_select_on_prediction
        = gtk_check_button_new_with_mnemonic (_("Use direct select keys also on prediction."));
    gtk_widget_show (__widget_direct_select_on_prediction);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_direct_select_on_prediction, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_direct_select_on_prediction), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_direct_select_on_prediction,
                          _("Use direct select keys not only for conversion state but also for prediction state."), NULL);

    /* use direct select keys on prediction */
    __widget_inline_prediction
        = gtk_check_button_new_with_mnemonic (_("Inline prediction."));
    gtk_widget_show (__widget_inline_prediction);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_inline_prediction, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_inline_prediction), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_inline_prediction,
                          _("Show first candidate of predictions instead of reading on preedit area. Reading is shown by external window."), NULL);

    // Connect all signals.
    g_signal_connect ((gpointer) __widget_predict_on_preedition, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_predict_on_preedition);
    g_signal_connect ((gpointer) __widget_direct_select_on_prediction, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_direct_select_on_prediction);
    g_signal_connect ((gpointer) __widget_inline_prediction, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_inline_prediction);

    return vbox;
}

static GtkWidget *
create_candidates_window_page ()
{
    GtkWidget *vbox, *table;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 4);
    gtk_widget_show (table);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    /* auto register */
    __widget_auto_register
        = gtk_check_button_new_with_mnemonic (_("Change to inline word register mode automatically."));
    gtk_widget_show (__widget_auto_register);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_auto_register, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_auto_register), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_auto_register,
                          _("Invoke the inline word register mode when the cursor in the candidates window is move to out of range."), NULL);

    /* close candidate window on select */
    __widget_close_cand_win_on_select
        = gtk_check_button_new_with_mnemonic (_("Close candidates window when a candidate is selected directly."));
    gtk_widget_show (__widget_close_cand_win_on_select);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_close_cand_win_on_select, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_close_cand_win_on_select), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_close_cand_win_on_select,
                          _("Close candidates window when a candidate is selected directly."), NULL);

    /* show annotation */
    __widget_show_annotation
        = gtk_check_button_new_with_mnemonic (_("Show annotation of the word on the candidates window"));
    gtk_widget_show (__widget_show_annotation);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_annotation, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_annotation), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_annotation,
                          _("Show annotation of the word on the candidates window."), NULL);

    /* show usage */
    __widget_show_usage
        = gtk_check_button_new_with_mnemonic (_("Show usage of the word on the candidates window"));
    gtk_widget_show (__widget_show_usage);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_usage, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_usage), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_usage,
                          _("Show usage of the word on the candidates window."), NULL);

    /* show comment */
    __widget_show_comment
        = gtk_check_button_new_with_mnemonic (_("Show comment of the word on the candidates window"));
    gtk_widget_show (__widget_show_comment);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_comment, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_comment), 4);
    gtk_tooltips_set_tip (__widget_tooltips, __widget_show_comment,
                          _("Show comment of the word on the candidates window."), NULL);

    // Connect all signals.
    g_signal_connect ((gpointer) __widget_auto_register, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_auto_register);
    g_signal_connect ((gpointer) __widget_close_cand_win_on_select, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_close_cand_win_on_select);
    g_signal_connect ((gpointer) __widget_show_annotation, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_annotation);
    g_signal_connect ((gpointer) __widget_show_usage, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_usage);
    g_signal_connect ((gpointer) __widget_show_comment, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_comment);

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
create_keyboard_page (unsigned int page)
{
    GtkWidget *table;
    GtkWidget *label;

    if (page >= __key_conf_pages_num)
        return NULL;

    KeyboardConfigData *data = __key_conf_pages[page].data;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    // Create keyboard setting.
    for (unsigned int i = 0; data[i].key; ++ i) {
        APPEND_ENTRY(_(data[i].label),  _(data[i].tooltip), data[i].entry, i);
        gtk_entry_set_editable (GTK_ENTRY (data[i].entry), FALSE);

        data[i].button = gtk_button_new_with_label ("...");
        gtk_widget_show (data[i].button);
        gtk_table_attach (GTK_TABLE (table), data[i].button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), data[i].button);
    }

    for (unsigned int i = 0; data[i].key; ++ i) {
        g_signal_connect ((gpointer) data[i].button, "clicked",
                          G_CALLBACK (on_default_key_selection_clicked),
                          &(data[i]));
        g_signal_connect ((gpointer) data[i].entry, "changed",
                          G_CALLBACK (on_default_editable_changed),
                          &(data[i].data));
    }

#if 0
    for (unsigned int i = 0; data[i].key; ++ i) {
        gtk_tooltips_set_tip (__widget_tooltips, data[i].entry,
                              _(data[i].tooltip), NULL);
    }
#endif

    return table;
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
        GtkWidget *label = gtk_label_new (_("Options"));
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
        for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
            page = create_keyboard_page (i);
            label = gtk_label_new (_(__key_conf_pages[i].label));
            gtk_widget_show (label);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
        }

        // for preventing enabling left arrow.
	gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        setup_widget_value ();
    }

    return window;
}

#if 0
static void
setup_combo_value (GtkCombo *combo,
                   ComboConfigData *data, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

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
#endif

static void
setup_widget_value ()
{
    if (__widget_command) {
        gtk_entry_set_text (
            GTK_ENTRY (__widget_command),
            __config_command.c_str ());
    }
    if (__widget_convert_on_period) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_convert_on_period),
            __config_convert_on_period);
    }
    if (__widget_commit_period) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_commit_period),
            __config_commit_period);
    }
    if (__widget_commit_on_upper) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_commit_on_upper),
            __config_commit_on_upper);
    }

    if (__widget_predict_on_preedition) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_predict_on_preedition),
            __config_predict_on_preedition);
    }
    if (__widget_direct_select_on_prediction) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_direct_select_on_prediction),
            __config_direct_select_on_prediction);
    }
    if (__widget_inline_prediction) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_inline_prediction),
            __config_inline_prediction);
    }

    if (__widget_auto_register) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_auto_register),
            __config_auto_register);
    }
    if (__widget_close_cand_win_on_select) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_close_cand_win_on_select),
            __config_close_cand_win_on_select);
    }
    if (__widget_show_annotation) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_annotation),
            __config_show_annotation);
    }
    if (__widget_show_usage) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_usage),
            __config_show_usage);
    }
    if (__widget_show_comment) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_comment),
            __config_show_comment);
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].entry) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].entry),
                    __key_conf_pages[j].data[i].data.c_str ());
            }
        }
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    __config_command =
        config->read (String (SCIM_PRIME_CONFIG_COMMAND),
                      __config_command);
    __config_convert_on_period =
        config->read (String (SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD),
                      __config_convert_on_period);
    __config_commit_period =
        config->read (String (SCIM_PRIME_CONFIG_COMMIT_PERIOD),
                      __config_commit_period);
    __config_commit_on_upper =
        config->read (String (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER),
                      __config_commit_on_upper);

    __config_predict_on_preedition=
        config->read (String (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION),
                      __config_predict_on_preedition);
    __config_direct_select_on_prediction =
        config->read (String (SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION),
                      __config_direct_select_on_prediction);
    __config_inline_prediction =
        config->read (String (SCIM_PRIME_CONFIG_INLINE_PREDICTION),
                      __config_inline_prediction);

    __config_auto_register =
        config->read (String (SCIM_PRIME_CONFIG_AUTO_REGISTER),
                      __config_auto_register);
    __config_close_cand_win_on_select =
        config->read (String (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT),
                      __config_close_cand_win_on_select);
    __config_show_annotation =
        config->read (String (SCIM_PRIME_CONFIG_SHOW_ANNOTATION),
                      __config_show_annotation);
    __config_show_usage =
        config->read (String (SCIM_PRIME_CONFIG_SHOW_USAGE),
                      __config_show_usage);
    __config_show_comment =
        config->read (String (SCIM_PRIME_CONFIG_SHOW_COMMENT),
                      __config_show_comment);

    for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            __key_conf_pages[j].data[i].data =
                config->read (String (__key_conf_pages[j].data[i].key),
                              __key_conf_pages[j].data[i].data);
        }
    }

    setup_widget_value ();

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    config->write (String (SCIM_PRIME_CONFIG_COMMAND),
                   __config_command);
    config->write (String (SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD),
                   __config_convert_on_period);
    config->write (String (SCIM_PRIME_CONFIG_COMMIT_PERIOD),
                   __config_commit_period);
    config->write (String (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER),
                   __config_commit_on_upper);

    config->write (String (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION),
                   __config_predict_on_preedition);
    config->write (String (SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION),
                   __config_direct_select_on_prediction);
    config->write (String (SCIM_PRIME_CONFIG_INLINE_PREDICTION),
                   __config_inline_prediction);

    config->write (String (SCIM_PRIME_CONFIG_AUTO_REGISTER),
                   __config_auto_register);
    config->write (String (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT),
                   __config_close_cand_win_on_select);
    config->write (String (SCIM_PRIME_CONFIG_SHOW_ANNOTATION),
                   __config_show_annotation);
    config->write (String (SCIM_PRIME_CONFIG_SHOW_USAGE),
                   __config_show_usage);
    config->write (String (SCIM_PRIME_CONFIG_SHOW_COMMENT),
                   __config_show_comment);

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            config->write (String (__key_conf_pages[j].data[i].key),
                           __key_conf_pages[j].data[i].data);
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
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

#if 0
static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    String *str = static_cast<String *> (user_data);
    ComboConfigData *data
        = static_cast<ComboConfigData *> (g_object_get_data (G_OBJECT (editable),
                                                             DATA_POINTER_KEY));

    if (!str) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            *str = data[i].data;
            __have_changed = true;
            break;
        }
    }
}
#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
