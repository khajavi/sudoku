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

#include "mainwindow.hpp"
#include "gettext.h"
#include <iostream>
#include <string>
#include <sstream>

#define _(X) gettext(X)

MainWindow::MainWindow(const Glib::RefPtr<SudokuModel> &model) :
    model(model), sudokuView(model) {
    set_title(_("Sudoku"));
    set_default_size(800, 600);
    add(m_box);

    m_refActionGroup = Gtk::ActionGroup::create();

    m_refActionGroup->add(Gtk::Action::create("FileMenu", _("_File")));
    m_refActionGroup->add(Gtk::Action::create("EditMenu", _("_Edit")));

    m_refActionGroup->add(Gtk::Action::create("FileNew", Gtk::Stock::NEW,
            _("_New"), _("Clears the board")), sigc::mem_fun(*this,
            &MainWindow::on_file_new));

    m_refActionGroup->add(Gtk::Action::create("FileCheck", Gtk::Stock::YES,
            _("_Check"), _("Checks the sudoku")), sigc::mem_fun(*this,
            &MainWindow::on_file_check));

    m_refActionGroup->add(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN,
            _("_Open"), _("Opens a sudoku file")), sigc::mem_fun(*this,
            &MainWindow::on_file_open));

    m_refActionGroup->add(Gtk::Action::create("FileExit", Gtk::Stock::QUIT,
            _("E_xit"), _("Exits the application")), sigc::mem_fun(*this,
            &MainWindow::on_file_exit));

    m_refActionGroup->add(Gtk::Action::create("EditCopy", Gtk::Stock::COPY,
            _("_Copy"), _("Copies the grid to the clipboard")),
            sigc::mem_fun(*this, &MainWindow::on_edit_copy));

    m_refActionGroup->add(Gtk::Action::create("EditPaste", Gtk::Stock::PASTE,
            _("_Paste"), _("Pastes a grid from the clipboard")),
            sigc::mem_fun(*this, &MainWindow::on_edit_paste));

    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    add_accel_group(m_refUIManager->get_accel_group());

    try {

        Glib::ustring ui_info = "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='FileMenu'>"
            "      <menuitem action='FileNew' />"
            "      <menuitem action='FileOpen' />"
            "      <menuitem action='FileCheck' />"
            "      <separator />"
            "      <menuitem action='FileExit' />"
            "    </menu>"
            "    <menu action='EditMenu'>"
            "      <menuitem action='EditCopy' />"
            "      <menuitem action='EditPaste' />"
            "    </menu>"
            "  </menubar>"
            "  <toolbar  name='ToolBar'>"
            "    <toolitem action='FileNew'/>"
            "    <toolitem action='FileOpen'/>"
            "    <toolitem action='FileCheck'/>"
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
    if (pMenubar)
        m_box.pack_start(*pMenubar, Gtk::PACK_SHRINK);

    Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar");
    if (pToolbar)
        m_box.pack_start(*pToolbar, Gtk::PACK_SHRINK);

    m_box.pack_start(sudokuView, Gtk::PACK_EXPAND_WIDGET);
    m_box.pack_end(m_statusbar, Gtk::PACK_SHRINK);

    show_all_children();
}

void MainWindow::on_file_new() {
    model->clear();
}

void MainWindow::on_file_exit() {
    hide();
}

void MainWindow::on_file_open() {
    Gtk::FileChooserDialog dialog("Please choose a file",
            Gtk::FILE_CHOOSER_ACTION_OPEN);
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
        } catch (std::exception &ex) {
            // Gtk::Window& parent, const Glib::ustring& message, bool use_markup = false, MessageType type = MESSAGE_INFO, ButtonsType buttons = BUTTONS_OK, bool modal = false
            Gtk::MessageDialog dialog(*this, _("Error opening file"), false,
                    Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
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

void MainWindow::on_edit_copy() {
    std::stringstream out;
    model->print(out);
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->set_text(out.str());
}

void MainWindow::on_edit_paste() {
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->request_text(sigc::mem_fun(*this,
            &MainWindow::on_clipboard_text_received));
}

void MainWindow::on_clipboard_text_received(const Glib::ustring& text) {
    try {
        model->load_from_string(text.c_str());
    } catch (std::exception &ex) {
        Gtk::MessageDialog dialog(*this, _("Invalid Sudoku"), false,
                     Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_secondary_text(_("The clipboad data does not contain a valid sudoku"));
    }
}

void MainWindow::on_file_check() {
    bool result = model->check();
    if (result) {
        Gtk::MessageDialog dialog(*this, _("Sudoku valid"));
        dialog.set_secondary_text(
                _("This Sudoku is valid and has one single solution"));
        dialog.run();
    } else {
        Gtk::MessageDialog dialog(*this, _("Invalid Sudoku"), false,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dialog.set_secondary_text(_("This sudoku is not valid"));
        dialog.run();
    }
}