/*
 * Copyright (c) 2009, Ralph Juhnke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *    * Neither the name of "Ralph Juhnke" nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !_MSC_VER
#include "../include/config.h"
#endif
#include "../include/gettext.h"
#include "mainwindow.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#define _(X) gettext(X)

MainWindow::MainWindow(const Glib::RefPtr<SudokuModel> &model) :
    model(model), sudokuView(model), statusView(model), hintView(model), printSettings(Gtk::PrintSettings::create()) {
    set_title(_("Sudoku"));
    set_default_size(800, 600);
    add(m_box);

    model->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_model_changed));

    init_actions();

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    add_accel_group(m_refUIManager->get_accel_group());

    try {

        Glib::ustring ui_info = "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='FileMenu'>"
            "      <menuitem action='FileNew' />"
            "      <menuitem action='FileClear' />"
            "      <menuitem action='FileOpen' />"
            "      <menuitem action='FileCheck' />"
            "      <menuitem action='FilePrint' />"
            "      <menuitem action='FilePreview' />"
            "      <separator />"
            "      <menuitem action='FileExit' />"
            "    </menu>"
            "    <menu action='EditMenu'>"
            "      <menuitem action='EditUndo' />"
            "      <menuitem action='EditRedo' />"
            "      <menuitem action='EditCopy' />"
            "      <menuitem action='EditPaste' />"
            "      <menuitem action='EditSolveSingles' />"
            "      <menu action='EditDifficulty'>"
            "         <menuitem action='EditDifficultyEasy' />"
            "         <menuitem action='EditDifficultyMedium' />"
            "         <menuitem action='EditDifficultyHard' />"
            "      </menu>"
            "    </menu>"
            "    <menu action='ViewMenu'>"
            "      <menuitem action='ViewChoices' />"
            "      <menuitem action='ViewSidebar' />"
            "      <menu action='ViewHighlight'>"
            "        <menuitem action='ViewNone' />"
            "        <menuitem action='ViewOne' />"
            "        <menuitem action='ViewTwo' />"
            "        <menuitem action='ViewThree' />"
            "        <menuitem action='ViewFour' />"
            "        <menuitem action='ViewFive' />"
            "        <menuitem action='ViewSix' />"
            "        <menuitem action='ViewSeven' />"
            "        <menuitem action='ViewEight' />"
            "        <menuitem action='ViewNine' />"
            "      </menu>"
            "    </menu>"
            "    <menu action='HelpMenu'>"
            "      <menuitem action='HelpAbout' />"
            "    </menu>"
            "  </menubar>"
            "  <toolbar  name='ToolBar'>"
            "    <toolitem action='FileNew'/>"
            "    <toolitem action='FileOpen'/>"
            "    <toolitem action='FilePrint'/>"
            "    <toolitem action='FileCheck'/>"
            "    <toolitem action='EditUndo'/>"
            "    <toolitem action='EditRedo'/>"
            "    <toolitem action='EditCopy'/>"
            "    <toolitem action='EditPaste'/>"
            "  </toolbar>"
            "</ui>";

        m_refUIManager->add_ui_from_string(ui_info);
    }

    catch (const Glib::Error &ex) {
        std::cerr << "Scheisse: " << ex.what() << std::endl;
    }

    Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
    m_box.pack_start(*pMenubar, Gtk::PACK_SHRINK);

    Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar");
    m_box.pack_start(*pToolbar, Gtk::PACK_SHRINK);

    m_box.pack_start(m_hbox, Gtk::PACK_EXPAND_WIDGET);
    m_hbox.pack_start(sudokuView, Gtk::PACK_EXPAND_WIDGET);
    m_hbox.pack_start(statusView, Gtk::PACK_SHRINK);

    m_box.pack_start(hintView, Gtk::PACK_SHRINK);
    m_box.pack_end(m_statusbar, Gtk::PACK_SHRINK);

    show_all_children();
    statusView.hide();
    hintView.hide();
    context_id_status = m_statusbar.get_context_id("status");
    update_statusbar();
}

void MainWindow::init_actions() {
    m_refActionGroup = Gtk::ActionGroup::create();

    m_refActionGroup->add(Gtk::Action::create("FileMenu", _("_File")));
    m_refActionGroup->add(Gtk::Action::create("EditMenu", _("_Edit")));
    m_refActionGroup->add(Gtk::Action::create("ViewMenu", _("_View")));
    m_refActionGroup->add(Gtk::Action::create("HelpMenu", _("_Help")));

    init_file_actions();
    init_edit_actions();
    init_view_actions();
    init_help_actions();
}

void MainWindow::init_file_actions() {
    m_refActionGroup->add(Gtk::Action::create("FileNew", Gtk::Stock::NEW, _("_New"), _("Generates a new sudoku")), sigc::mem_fun(
            *this, &MainWindow::on_file_new));

    m_refActionGroup->add(Gtk::Action::create("FileClear", Gtk::Stock::NEW, _("_Clear"), _("Clears the board")), Gtk::AccelKey(
            "<control><shift>N"), sigc::mem_fun(*this, &MainWindow::on_file_clear));

    m_refActionGroup->add(Gtk::Action::create("FileCheck", Gtk::Stock::APPLY, _("_Check"), _("Checks the sudoku")), Gtk::AccelKey(
            "<control>T"), sigc::mem_fun(*this, &MainWindow::on_file_check));

    m_refActionGroup->add(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN, _("_Open"), _("Opens a sudoku file")), sigc::mem_fun(
            *this, &MainWindow::on_file_open));

    m_refActionGroup->add(Gtk::Action::create("FilePrint", Gtk::Stock::PRINT, _("_Print"), _("Prints the sudoku")), Gtk::AccelKey(
            "<control>P"), sigc::mem_fun(*this, &MainWindow::on_file_print));

    m_refActionGroup->add(Gtk::Action::create("FilePreview", Gtk::Stock::PRINT_PREVIEW, _("Pre_view"), _("Shows a print preview")),
            Gtk::AccelKey("<control><shift>P"), sigc::mem_fun(*this, &MainWindow::on_file_preview));

    m_refActionGroup->add(Gtk::Action::create("FileExit", Gtk::Stock::QUIT, _("E_xit"), _("Exits the application")), sigc::mem_fun(
            *this, &MainWindow::on_file_exit));
}

void MainWindow::init_edit_actions() {
    Gtk::RadioAction::Group difficulty_group;

    m_actionUndo = Gtk::Action::create("EditUndo", Gtk::Stock::UNDO, _("_Undo"), _("Undoes the last command"));
    m_actionUndo->set_sensitive(false);
    m_refActionGroup->add(m_actionUndo, Gtk::AccelKey("<control>Z"), sigc::mem_fun(*this, &MainWindow::on_edit_undo));

    m_actionRedo = Gtk::Action::create("EditRedo", Gtk::Stock::REDO, _("_Redo"), _("Re executes the last undone command"));
    m_actionRedo->set_sensitive(false);
    m_refActionGroup->add(m_actionRedo, Gtk::AccelKey("<control><shift>Z"), sigc::mem_fun(*this, &MainWindow::on_edit_redo));

    m_refActionGroup->add(Gtk::Action::create("EditCopy", Gtk::Stock::COPY, _("_Copy"), _("Copies the grid to the clipboard")),
            sigc::mem_fun(*this, &MainWindow::on_edit_copy));

    m_refActionGroup->add(Gtk::Action::create("EditPaste", Gtk::Stock::PASTE, _("_Paste"), _("Pastes a grid from the clipboard")),
            sigc::mem_fun(*this, &MainWindow::on_edit_paste));

    m_refActionGroup->add(Gtk::Action::create("EditSolveSingles", _("_Solve Singles"), _("Solves all naked and hidden singles")),
            Gtk::AccelKey("<control><shift>S"), sigc::mem_fun(*this, &MainWindow::on_solve_singles));

    m_refActionGroup->add(Gtk::Action::create("EditDifficulty", _("_Difficulty")));

    m_difficultyEasy = Gtk::RadioAction::create(difficulty_group, "EditDifficultyEasy", _("_Easy"), _("Easy")), Gtk::AccelKey(
            "<control><alt>E");
    m_refActionGroup->add(m_difficultyEasy, sigc::mem_fun(*this, &MainWindow::on_edit_difficulty_easy));
    m_difficultyEasy->set_active();

    m_difficultyMedium = Gtk::RadioAction::create(difficulty_group, "EditDifficultyMedium", _("_Medium"), _("Medium")), Gtk::AccelKey(
            "<control><alt>M"), sigc::mem_fun(*this, &MainWindow::on_edit_difficulty_medium);
    m_refActionGroup->add(m_difficultyMedium, sigc::mem_fun(*this, &MainWindow::on_edit_difficulty_medium));

    m_difficultyHard = Gtk::RadioAction::create(difficulty_group, "EditDifficultyHard", _("_Hard"), _("Hard")), Gtk::AccelKey(
            "<control><alt>H");
    m_refActionGroup->add(m_difficultyHard, sigc::mem_fun(*this, &MainWindow::on_edit_difficulty_hard));
}

void MainWindow::init_view_actions() {
    Glib::RefPtr<Gtk::ToggleAction> toggle_choices = Gtk::ToggleAction::create("ViewChoices", _("_Choices"),
            _("Shows the choices"), true);

    m_refActionGroup->add(toggle_choices, sigc::mem_fun(*this, &MainWindow::on_view_choices));

    Glib::RefPtr<Gtk::ToggleAction> toggle_sidebar = Gtk::ToggleAction::create("ViewSidebar", _("_Sidebar"),
            _("Shows the sidebar"), false);

    m_refActionGroup->add(toggle_sidebar, sigc::mem_fun(*this, &MainWindow::on_view_sidebar));

    init_view_highlight_actions();
}

void MainWindow::init_view_highlight_actions() {
    m_refActionGroup->add(Gtk::Action::create("ViewHighlight", _("_Highlight")));
    Gtk::RadioAction::Group view_choices_group;
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewNone", _("Highlight _nothing"),
            _("Removes all cell highlightings")), Gtk::AccelKey("<control><alt>0"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_nothing));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewOne", _("Highlight _1"),
            _("Shows all cells having a one as candidate")), Gtk::AccelKey("<control><alt>1"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_one));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewTwo", _("Highlight _2"),
            _("Shows all cells having a two as candidate")), Gtk::AccelKey("<control><alt>2"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_two));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewThree", _("Highlight _3"),
            _("Shows all cells having a three as candidate")), Gtk::AccelKey("<control><alt>3"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_three));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewFour", _("Highlight _4"),
            _("Shows all cells having a four as candidate")), Gtk::AccelKey("<control><alt>4"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_four));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewFive", _("Highlight _5"),
            _("Shows all cells having a five as candidate")), Gtk::AccelKey("<control><alt>5"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_five));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewSix", _("Highlight _6"),
            _("Shows all cells having a six as candidate")), Gtk::AccelKey("<control><alt>6"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_six));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewSeven", _("Highlight _7"),
            _("Shows all cells having a seven as candidate")), Gtk::AccelKey("<control><alt>7"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_seven));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewEight", _("Highlight _8"),
            _("Shows all cells having a eight as candidate")), Gtk::AccelKey("<control><alt>8"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_eight));
    m_refActionGroup->add(Gtk::RadioAction::create(view_choices_group, "ViewNine", _("Highlight _9"),
            _("Shows all cells having a nine as candidate")), Gtk::AccelKey("<control><alt>9"), sigc::mem_fun(*this,
            &MainWindow::on_highlight_nine));
}

void MainWindow::init_help_actions() {
    m_refActionGroup->add(Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT, _("_About"), _("Shows information about gsudoku")),
            sigc::mem_fun(*this, &MainWindow::on_help_about));
}

void MainWindow::on_file_new() {
    model->generate();
}

void MainWindow::on_file_clear() {
    model->clear();
}

void MainWindow::on_file_print() {
    print_or_preview(Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

void MainWindow::on_file_preview() {
    print_or_preview(Gtk::PRINT_OPERATION_ACTION_PREVIEW);
}

void MainWindow::print_or_preview(Gtk::PrintOperationAction print_action) {
    Glib::RefPtr<SudokuPrintOperation> print = SudokuPrintOperation::create(model);

    //    print->set_track_print_status();
    // print->set_default_page_setup(pageSetup);
    print->set_print_settings(printSettings);

    print->signal_done().connect(sigc::bind(sigc::mem_fun(*this, &MainWindow::on_printoperation_done), print));

    try {
        print->run(print_action /* print or preview */, *this);
    } catch (const Gtk::PrintError& ex) {
        std::ostringstream os;
        os << _("An error occurred while trying to run a print operation:") << ex.what() << std::endl;
        Gtk::MessageDialog err_dialog(*this, os.str(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        err_dialog.run();
    }
}

void MainWindow::on_printoperation_done(Gtk::PrintOperationResult result, const Glib::RefPtr<SudokuPrintOperation>& operation) {
    if (result == Gtk::PRINT_OPERATION_RESULT_ERROR) {
        Gtk::MessageDialog err_dialog(*this, "Error printing form", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        err_dialog.run();
    } else if (result == Gtk::PRINT_OPERATION_RESULT_APPLY) {
        printSettings = operation->get_print_settings();
    }
}

void MainWindow::on_file_exit() {
    hide();
}

void MainWindow::on_file_open() {
    Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:

    Gtk::FileFilter filter_text;
    filter_text.set_name(_("Text files"));
    filter_text.add_mime_type("text/plain");
    dialog.add_filter(filter_text);

    Gtk::FileFilter filter_cpp;
    filter_cpp.set_name(_("C/C++ files"));
    filter_cpp.add_mime_type("text/x-c");
    filter_cpp.add_mime_type("text/x-c++");
    filter_cpp.add_mime_type("text/x-c-header");
    dialog.add_filter(filter_cpp);

    Gtk::FileFilter filter_any;
    filter_any.set_name(_("Any files"));
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);

    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch (result) {
    case (Gtk::RESPONSE_OK): {
        std::string filename = dialog.get_filename();
        try {
            model->load(filename);
        } catch (std::exception & /*ex*/) {
            // Gtk::Window& parent, const Glib::ustring& message, bool use_markup = false, MessageType type = MESSAGE_INFO, ButtonsType buttons = BUTTONS_OK, bool modal = false
            Gtk::MessageDialog dialog(*this, _("Error opening file"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            dialog.set_secondary_text(_("Cannot open file: ") + filename);
            dialog.run();

        }
        break;
    }
    case (Gtk::RESPONSE_CANCEL):
        break;
    default:
        break;
    }
}

void MainWindow::on_edit_undo() {
    model->undo();
}

void MainWindow::on_edit_redo() {
    model->redo();
}

void MainWindow::on_solve_singles() {
    model->solve_singles();
}

void MainWindow::update_statusbar() {
    m_statusbar.pop(context_id_status);
    int todo = model->get_todo();
    std::ostringstream os;
    os << _("Todo: ") << todo << _(" Done: ") << 81 - todo << _(" Choices: ") << model->get_num_choices();
    m_statusbar.pop(context_id_status);
    m_statusbar.push(os.str(), context_id_status);
}

void MainWindow::on_model_changed() {
    m_actionUndo->set_sensitive(model->can_undo());
    m_actionRedo->set_sensitive(model->can_redo());
    update_statusbar();
}

void MainWindow::on_edit_copy() {
    std::stringstream out;
    model->print(out);
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->set_text(out.str());
}

void MainWindow::on_edit_paste() {
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->request_text(sigc::mem_fun(*this, &MainWindow::on_clipboard_text_received));
}

void MainWindow::on_edit_difficulty_easy() {
    model->set_difficulty_level(SudokuModel::EASY);
}

void MainWindow::on_edit_difficulty_medium() {
    model->set_difficulty_level(SudokuModel::MEDIUM);
}

void MainWindow::on_edit_difficulty_hard() {
    model->set_difficulty_level(SudokuModel::HARD);
}

void MainWindow::on_clipboard_text_received(const Glib::ustring& text) {
    try {
        model->load_from_string(text.c_str());
    } catch (std::exception & /*ex*/) {
        Gtk::MessageDialog dialog(*this, _("Invalid Sudoku"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_secondary_text(_("The clipboad data does not contain a valid sudoku"));
    }
}

void MainWindow::on_file_check() {
    bool result = model->check();
    if (result) {
        Gtk::MessageDialog dialog(*this, _("Sudoku valid"));
        dialog.set_secondary_text(_("This Sudoku is valid and has one single solution"));
        dialog.run();
    } else {
        Gtk::MessageDialog dialog(*this, _("Invalid Sudoku"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_secondary_text(_("This sudoku is not valid"));
        dialog.run();
    }
}

void MainWindow::on_view_choices() {
    model->set_show_choices(!model->get_show_choices());
}

void MainWindow::on_view_sidebar() {
    if (statusView.is_visible()) {
        statusView.hide();
        hintView.hide();
    } else {
        statusView.show();
        hintView.show();
    }
}

void MainWindow::on_highlight_nothing() {
    model->set_highlighted_choice(0);
}

void MainWindow::on_highlight_one() {
    model->set_highlighted_choice(1);
}

void MainWindow::on_highlight_two() {
    model->set_highlighted_choice(2);
}

void MainWindow::on_highlight_three() {
    model->set_highlighted_choice(3);
}

void MainWindow::on_highlight_four() {
    model->set_highlighted_choice(4);
}

void MainWindow::on_highlight_five() {
    model->set_highlighted_choice(5);
}

void MainWindow::on_highlight_six() {
    model->set_highlighted_choice(6);
}

void MainWindow::on_highlight_seven() {
    model->set_highlighted_choice(7);
}

void MainWindow::on_highlight_eight() {
    model->set_highlighted_choice(8);
}

void MainWindow::on_highlight_nine() {
    model->set_highlighted_choice(9);
}

void MainWindow::on_help_about() {
    Gtk::AboutDialog dlg;
    std::vector<Glib::ustring> a;
    a.push_back("Ralph Juhnke");
    Glib::StringArrayHandle authors(a);
    dlg.set_name("gsudoku");
    dlg.set_version("0.1");
    dlg.set_copyright("Ralph Juhnke");
    dlg.set_license("Copyright (c) 2009, Ralph Juhnke\n"
        "All rights reserved.\n"
        "\n"
        "Redistribution and use in source and binary forms, with or\n"
        "without modification, are permitted provided that the following conditions\n"
        "are met:\n\n"
        "  * Redistributions of source code must retain the above copyright notice,\n"
        "    this list of conditions and the following disclaimer.\n"
        "\n"
        "  * Redistributions in binary form must reproduce the above copyright\n"
        "    notice, this list of conditions and the following disclaimer in the\n"
        "    documentation and/or other materials provided with the distribution.\n"
        "\n"
        "  * Neither the name of \"Ralph Juhnke\" nor the names of its\n"
        "    contributors may be used to endorse or promote products derived from\n"
        "    this software without specific prior written permission.\n"
        "\n"
        "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
        "AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,\n"
        "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n"
        "PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR\n"
        "CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
        "EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
        "PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;\n"
        "OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"
        "WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR\n"
        "OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF\n"
        "ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n");

    dlg.set_authors(authors);
    dlg.set_transient_for(*this);
    dlg.run();
}
